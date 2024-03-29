//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include <winsock.h>
//#include "pcap.h"
#include "RegExpr.hpp"
#include "WarchaosMapperDecl.h"
#include "TAdderThread.h"
#include "THTTPSniffer.h"
#include "MainForm.h"
#include <iphlpapi.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "RxGIF"
#pragma resource "*.dfm"
TFMain *FMain;
TGlobalOptions *GlobalOptions;
//---------------------------------------------------------------------------
__fastcall TFMain::TFMain(TComponent* Owner) : TForm(Owner)
{
      ChPtr = 0;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormCreate(TObject *Sender)
{
      bool tbool;
      char buf[100];
      DeleteDebugFiles();
      GlobalOptions = new TGlobalOptions();
      MapStrings = new TStringList;
      MapStrings->LoadFromFile(ExtractFilePath(Application->ExeName) + "map.txt");
      RowIndexes = new int[256];
      RefreshRowIndexes(RowIndexes, MapStrings);
      GifImage = new TGIFImage;
      r = new TRegExpr;
      r->ModifierI = true;
      ReconstructMiniMap();
      SnifThread = new THTTPSniffer(true);
      SnifThread->Priority = tpLower;
      AdderThread = new TAdderThread(true);
      AdderThread->Priority = tpLower;
      SnifThread->AdderThread = AdderThread;
      AdderThread->MiniMapCh = new TMiniMapCh[MiniMapChBufferLength];
      AdderThread->MapStrings = MapStrings;
      AdderThread->RowIndexes = RowIndexes;
      GetNetworkAdapters();
      BigMapSelectCell(NULL, 0, 0, tbool);                                      // �������� ������� OnSelectCell
      if (!DirectoryExists(GlobalOptions->ImagesPath))
      {
         MessageDlg("��������!\n���������� ������� � ����������� �� ���� ��������� ������� � ����� warrchaos (����� ���������).", mtError, TMsgDlgButtons() << mbOK, 0);
         Application->Terminate();
      }
      Timer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::FormDestroy(TObject *Sender)
{
      if (!AdderThread->Suspended)
      {
         AdderThread->Terminate();
         AdderThread->WaitFor();
      }
      delete[] AdderThread->MiniMapCh;
      delete AdderThread;
      if (!SnifThread->Suspended)
      {
         SnifThread->Terminate();
         SnifThread->WaitFor();
      }
      delete SnifThread;
      delete r;
      delete GifImage;
      delete[] RowIndexes;
      delete GlobalOptions;
      MapStrings->SaveToFile(ExtractFilePath(Application->ExeName) + "map.txt");
      delete MapStrings;
}
//---------------------------------------------------------------------------

// �� ���� ����������� �� ������� ������ �� ������ ��������
void __fastcall TFMain::MarkAllPoints(TPoint* CenterPoints)
{
      TPoint pt;
      TCanvas* DeskTop = new TCanvas;
      DeskTop->Handle = GetDC(HWND_DESKTOP);
      DeskTop->Brush->Color = clRed;
      for (int i = 0; i < 100; i++)
      {
         pt = CenterPoints[i];
         if (pt.x == 0 && pt.y == 0) break;
         DeskTop->Ellipse(pt.x-2, pt.y-2, pt.x+2, pt.y+2);
      }
      ReleaseDC(0, DeskTop->Handle);
      delete DeskTop;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Button1Click(TObject *Sender)
{
      TStringList* hi = new TStringList;
      AnsiString tmpstr;
      for (int y=1; y<=256; y++)
      {
         tmpstr.sprintf("<TR> <!- ROW � %d ->", y);
         hi->Add(tmpstr);
         for (int x=1; x<=256; x++)
         {
            tmpstr.sprintf("<TD><src=\"land0/113.gif\" tip=\"�� ��������� x:%d y:%d\"></TD>", x, y);
            hi->Add(tmpstr);
         }
         hi->Add("</TR>");
      }
      hi->SaveToFile(ExtractFilePath(Application->ExeName) + "txt.txt");
      delete hi;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BigMapDrawCell(TObject *Sender, int ACol,
      int ARow, TRect &Rect, TGridDrawState State)
{
      int tmp, x, y;
      AnsiString str, src, obj;
      str = GetCellStringFromMap(ACol+1, ARow+1);
      src = str.SubString(str.Pos("src=\"")+5,128);
      src.SetLength(src.Pos("\"")-1);
      if ((tmp=str.Pos("obj=\""))!=0)
      {
         obj = str.SubString(tmp+5,128);
         obj.SetLength(obj.Pos("\"")-1);
      }
      GifImage->LoadFromFile(GlobalOptions->ImagesPath + src);
      x = (ACol-BigMap->LeftCol)*BigMap->DefaultColWidth; y = (ARow-BigMap->TopRow)*BigMap->DefaultRowHeight;
      BigMap->Canvas->Draw(x, y, GifImage);
      if (!obj.IsEmpty())
      {
         GifImage->LoadFromFile(GlobalOptions->ImagesPath + obj);
         BigMap->Canvas->Draw(x+(BigMap->DefaultColWidth-GifImage->Width)/2, y+(BigMap->DefaultRowHeight-GifImage->Height)/2, GifImage);
      }
}
//---------------------------------------------------------------------------
// ����� � MapStrings ������, ���������� ������ � ������ Col Row. ������� ������ �� ��� ������.
AnsiString __fastcall TFMain::GetCellStringFromMap(int Col, int Row)
{
      int num = RowIndexes[Row-1];
      return MapStrings->Strings[num+Col-1];
}
//---------------------------------------------------------------------------
// ��������� � ������� RowIndexes �������� ������� �����, ��������������� ������ �������� ����� � ��������
void __fastcall TFMain::RefreshRowIndexes(int *Indexes, TStringList *Strings)
{
      int x = 0;
      for (int i=0; i<Strings->Count; i++)
      {
         AnsiString str = Strings->Strings[i];
         if (str.SubString(1,8) == "<TR> <!-")
            Indexes[x++] = i+1;
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::SkormitButtonClick(TObject *Sender)
{
      if (OpenDialogHTML->Execute())
      {
         for (int i=0; i<OpenDialogHTML->Files->Count; i++)
            AssignMapFile(OpenDialogHTML->Files->Strings[i]);
         ReconstructMiniMap();
      }
}
//---------------------------------------------------------------------------
// ��������� ����� MapFileName - ������� ����������� ����� 15�15 � ������
void __fastcall TFMain::AssignMapFile(AnsiString MapFileName)
{
      char *strFORM = "<FORM";
      char *strTABLE = "<TABLE";
      char *strTD = "<TD";
      char *strBTD = "</TD";
      char *strTR = "<TR";
      char *strBTR = "</TR";
      AnsiString tstr, BASE;
      TStringList *htm = new TStringList;
      htm->LoadFromFile(MapFileName);
      for (int i=0; i<htm->Count; i++)
         tstr += Trim(htm->Strings[i])+' ';
      delete htm;
      AnsiString basestr = tstr.SubString(1, AnsiStrPosCS(tstr, strFORM));
      BASE = ExtractStringParameter(basestr, "BASE href");
      if (BASE.SubString(1,4) == "file") BASE.Insert("/",6);
      char *ptstr = tstr.c_str();
      ptstr = AnsiStrPosCS(ptstr, strFORM);
      ptstr = AnsiStrPosCS(ptstr, strTABLE)+1;
      ptstr = AnsiStrPosCS(ptstr, strTABLE)+1;
      ptstr = AnsiStrPosCS(ptstr, strTABLE);
      for (int y=0; y<15; y++)
      {
         ptstr = AnsiStrPosCS(ptstr, strTR);
         for (int x=0; x<15; x++)
         {
            ptstr = AnsiStrPosCS(ptstr, strTD);
            AssignOneMapCell(ptstr, BASE);
            ptstr = AnsiStrPosCS(ptstr, strBTD);
         }
         ptstr = AnsiStrPosCS(ptstr, strBTR);
      }
}
//---------------------------------------------------------------------------
// ��������� tstr ��������� �� ������ �������� ������.
// ������������ �� ���������� � ��������� ��������� � �������� �����
void __fastcall TFMain::AssignOneMapCell(char *tstr, AnsiString& BASE)
{
      int x, y, srcnum, objnum;
      char *strBTD = "</TD";
      char *tts = AnsiStrPosCS(tstr, strBTD);
      AnsiString background, src, obj, tip;
      AnsiString onestr = AnsiString(tstr+4, tts-tstr-1);
      tip = ExtractStringParameter(onestr, "tooltip");
      r->InputString = tip;                                        
      r->Expression = "x:(-?\\d+).+y:(-?\\d+)";
      r->Exec();
      x = r->Match[1].ToInt();
      y = r->Match[2].ToInt();
      r->Expression = "<.+?>";
      tip = r->Replace(tip, "", false);
      if (x>=1 && x<=256 && y>=1 && y<=256)
      {
         background = ExtractStringParameter(onestr, "background");
         src = ExtractStringParameter(onestr, "src");
         r->InputString = background;
         r->Expression = BASE + "(.*)";
         if (r->Exec())
            background = r->Match[1];
         r->InputString = src;
         if (r->Exec())
            src = r->Match[1];
         r->Expression = "land1";
         background = r->Replace(background, "land0", false);                          // ������ �� land0
         src = r->Replace(src, "land0", false);                                        // ������ �� land0
         if (!background.IsEmpty())
         {
            obj = src;
            src = background;
            r->InputString = obj;
            r->Expression = "(.*?)(\\d+)(\\.gif)";
            r->Exec();
            objnum = r->Match[2].ToInt();
            if (objnum==342)
               obj = r->Match[1]+IntToStr(9342)+r->Match[3];                           // ������� �������� �����
            if (!(objnum==3||objnum==13||objnum==23|objnum==33||objnum==43||objnum==53||objnum==342))
            {                                                                          // ���� ��� �� ���������/�������� �����, �� �� �����
               obj = "";
               r->InputString = tip;
               r->Expression = ".+\\$(.+)";
               r->Exec();
               tip = r->Match[1];
            }
         }
         r->InputString = src;
         r->Expression = "(.*?)(\\d+)(\\.gif)";
         r->Exec();
         srcnum = r->Match[2].ToInt();
         if (srcnum==114||srcnum==115||srcnum==116||srcnum==117||srcnum==118)
            src = r->Match[1]+IntToStr(112)+r->Match[3];                               // ������� ������

         AnsiString mapstr = "<TD><src=\"" + src + "\"";
         if (!obj.IsEmpty()) mapstr += " obj=\"" + obj + "\"";
         mapstr += " tip=\"" + tip + "\"></TD>";

         int mapstrnum = RowIndexes[y-1] + x-1;
         MapStrings->Strings[mapstrnum] = mapstr;
      }
}
//---------------------------------------------------------------------------
// ������� �� ������ str ���������, ���������� ��� ����� (param *= *"?pval"?)
AnsiString __fastcall TFMain::ExtractStringParameter(AnsiString& str, AnsiString param)
{
      AnsiString pval;
      r->Expression = param + " *= *\"(.+?)\"";
      if (r->Exec(str)) pval = r->Match[1];
      else
      {
         r->Expression = param + " *= *(.+?)[ >]";
         if (r->Exec()) pval = r->Match[1];
      }

      return pval;
}
//---------------------------------------------------------------------------
// ��������������� ����� ��������� �� ������ ����������� �������� ������ �����
void __fastcall TFMain::ReconstructMiniMap(void)
{
      int srcnum, objnum, mapstrnum, tmp;
      AnsiString str, src, obj, tip;
      TColor color;
      for (int y=0; y<256; y++)
      {
         for (int x=0; x<256; x++)
         {
            objnum = 0;
            mapstrnum = RowIndexes[y] + x;
            str = MapStrings->Strings[mapstrnum];
            if ((tmp=str.Pos("src="))!=0)
            {
               src = str.SubString(tmp,128);
               src.SetLength(AnsiStrPosCS(src,".gif")-1);
               tmp = src.Length()-1;
               while (src[tmp]>='0' && src[tmp]<='9') tmp--;
               src.Delete(1,tmp);
               srcnum = src.ToInt();
            }
            if ((tmp=str.Pos("obj="))!=0)
            {
               obj = str.SubString(tmp,128);
               obj.SetLength(AnsiStrPosCS(obj,".gif")-1);
               tmp = obj.Length()-1;
               while (obj[tmp]>='0' && obj[tmp]<='9') tmp--;
               obj.Delete(1,tmp);
               objnum = obj.ToInt();
            }
            if (objnum==3||objnum==13||objnum==23||objnum==33||objnum==43||objnum==53||objnum==9342)
            {
               color = clRed;
               tip = ExtractStringParameter(str, "tip");
               for (int i=0; i<GlobalOptions->Buddies->Count; i++)
               {
                  if (tip.Pos(GlobalOptions->Buddies->Strings[i]) == 0) continue;
                  color = clYellow;
                  break;
               }
            }
            else
            {
               color = (TColor)LandColors[srcnum-32];
            }
            MiniMap->Canvas->Pixels[x][y] = color;
         }
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::MiniMapClick(TObject *Sender)
{
      bool tbool;
      int xx,yy;
      TGridRect sel;
      TPoint pt = MiniMap->ScreenToClient(Mouse->CursorPos);
      xx = pt.x - BigMap->Width/BigMap->DefaultColWidth/2;
      if (xx < 0) xx = 0;
      yy = pt.y - BigMap->Height/BigMap->DefaultRowHeight/2;
      if (yy < 0) yy = 0;
      BigMap->Visible = false;
      BigMap->LeftCol = xx;
      BigMap->TopRow  = yy;
      BigMap->DoubleBuffered = true;
      BigMap->Visible = true;
      BigMap->Repaint();
      BigMap->DoubleBuffered = false;
      sel.Left = sel.Right = pt.x;
      sel.Top = sel.Bottom = pt.y;
      BigMap->Selection = sel;
      BigMapSelectCell(NULL, pt.x, pt.y, tbool);                                // �������� ������� OnSelectCell
      BigMap->SetFocus();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BigMapTopLeftChanged(TObject *Sender)
{
      MapBevel->Left = BigMap->LeftCol+1;
      MapBevel->Top  = BigMap->TopRow+1;
      MapBevel->Width  = BigMap->Width / BigMap->DefaultColWidth;
      MapBevel->Height = BigMap->Height / BigMap->DefaultRowHeight;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BigMapMouseMove(TObject *Sender, TShiftState Shift,
      int X, int Y)
{
      int NewACol, NewARow;
      AnsiString onestr, tooltip;
      BigMap->MouseToCell(X, Y, NewACol, NewARow);
      if (NewARow != OldRedrawRow || NewACol != OldRedrawCol)
      {
         int mapstrnum = RowIndexes[NewARow] + NewACol;
         onestr = MapStrings->Strings[mapstrnum];
         tooltip = ExtractStringParameter(onestr, "tip");
         BigMap->Hint = tooltip;
         DisplayGridHint(Application->HintPause);
      Application->ActivateHint(Mouse->CursorPos);
         OldRedrawRow = NewARow; OldRedrawCol = NewACol;
      }
}
//---------------------------------------------------------------------------
// ������� ������ � ����������� ��� BigMap
void __fastcall TFMain::DisplayGridHint(int Pause)
{
      StopGridHintTimer();
      GridHintTimerHandle = SetTimer(this, 0, Pause, reinterpret_cast<TIMERPROC>(GridHintFunc));
      if (!GridHintTimerHandle) Application->CancelHint();
}
// ������� �������
void CALLBACK TFMain::GridHintFunc(HWND Wnd, UINT Msg, UINT TimerID, DWORD Time)
{
//      TObject* Object = reinterpret_cast<TObject*>(Wnd);
//      TFMain* pGrid = dynamic_cast<TFMain*>(Object);
//      if (pGrid) pGrid->GridHintExpired();
//      FMain->GridHintExpired();
}
void __fastcall TFMain::GridHintExpired(void)
{
      StopGridHintTimer();
      Application->ActivateHint(Mouse->CursorPos);
}
void __fastcall TFMain::StopGridHintTimer(void)
{
      if (GridHintTimerHandle)
      {
         KillTimer(this, GridHintTimerHandle);
         GridHintTimerHandle = 0;
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BtnStartClick(TObject *Sender)
{
      SnifThread->FilterByIP = GlobalOptions->FilterByIP;
      SnifThread->FilterIPAddress = inet_addr(GlobalOptions->FilterIP.c_str());
      SnifThread->snifPause = false;
      SnifThread->imstopped = false;
      AdderThread->adderPause = false;
      AdderThread->imstopped = false;
      if (SnifThread->Suspended)
         SnifThread->Resume();
      if (AdderThread->Suspended)
         AdderThread->Resume();
      BtnStop->Enabled = true;
      BtnStart->Enabled = false;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BtnStopClick(TObject *Sender)
{
      SnifThread->snifPause = true;
      AdderThread->adderPause = true;
      while (!SnifThread->imstopped && !AdderThread->imstopped) Application->ProcessMessages();
      SnifThread->Suspend();
      AdderThread->Suspend();
      BtnStop->Enabled = false;
      BtnStart->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::TimerTimer(TObject *Sender)
{
      if (AppendPages != AdderThread->appendpages)
      {
         AppendPages = AdderThread->appendpages;
         LAppendPages->Caption = IntToStr(AppendPages);
      }
      if (TCPPackets != SnifThread->packets)
      {
         TCPPackets = SnifThread->packets;
         LTCPPackets->Caption = IntToStr(TCPPackets);
      }
      if (SocketsUsed != AdderThread->SocketsUsed)
      {
         SocketsUsed = AdderThread->SocketsUsed;
         LSocketsUsed->Caption = IntToStr(SocketsUsed);
      }
      while (ChPtr != AdderThread->ChCounter)
      {
         TMiniMapCh *ChRel = &(AdderThread->MiniMapCh[ChPtr&(MiniMapChBufferLength-1)]);
         MiniMap->Canvas->Pixels[ChRel->x][ChRel->y] = ChRel->color;
         ChPtr++;
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::GetNetworkAdapters(void)
{
      ULONG Len;
      PIP_ADAPTER_INFO InterfaceInfo;
      PIP_ADAPTER_INFO TmpPointer;
      PIP_ADDR_STRING IP;

      if (GetAdaptersInfo(NULL, &Len) == ERROR_BUFFER_OVERFLOW)                 // ������� ������� ������ ��� ���������?
      {
         InterfaceInfo = (PIP_ADAPTER_INFO) new Byte[Len];                      // ����� ������ ���-��
         try
         {
            if (GetAdaptersInfo(InterfaceInfo, &Len) == ERROR_SUCCESS)          // ���������� �������
            {
               CBNetworkInterfaces->Items->Clear();
               TmpPointer = InterfaceInfo;                                      // ����������� ��� ������� ����������
               do
               {
                  IP = &TmpPointer->IpAddressList;                              // ����������� ��� IP ������ ������� ����������
                  do
                  {
                     CBNetworkInterfaces->Items->Add(Format("%s - [%s]", ARRAYOFCONST((IP->IpAddress.String,Trim((AnsiString)TmpPointer->Description)))));
                     IP = IP->Next;
                  }
                  while (IP != NULL);
                  TmpPointer = TmpPointer->Next;
               }
               while (TmpPointer != NULL);
            }
         }
         __finally
         {
            delete[] InterfaceInfo;                                             // ����������� ������� ������
         }
      }
      if (CBNetworkInterfaces->Items->Count == 0)                               // ������� - ����� �� �� ���������� ������ ���������?
      {
         CBNetworkInterfaces->Text = "������� ���������� �� ����������.";
         BtnStart->Enabled = false;
      }
      else
      {
         if (GlobalOptions->DefaultNetworkInterface < CBNetworkInterfaces->Items->Count)
            CBNetworkInterfaces->ItemIndex = GlobalOptions->DefaultNetworkInterface;
         else
            CBNetworkInterfaces->ItemIndex = 0;
         CBNetworkInterfacesChange(NULL);
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::CBNetworkInterfacesChange(TObject *Sender)
{
      SnifThread->Host = CBNetworkInterfaces->Text.SubString(1, CBNetworkInterfaces->Text.Pos(" ")-1);
      GlobalOptions->DefaultNetworkInterface = CBNetworkInterfaces->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BigMapSelectCell(TObject *Sender, int ACol,
      int ARow, bool &CanSelect)
{
      TColor Color;
      AnsiString str, tooltip, Name, Player, Land;
      int mapstrnum = RowIndexes[ARow] + ACol;
      str = MapStrings->Strings[mapstrnum];
      tooltip = ExtractStringParameter(str, "tip");
      TStringList *tips = new TStringList;
      r->Expression = "\\$";
      r->Split(tooltip, tips);

      Land = tips->Strings[tips->Count - 1];
      if (tips->Count > 1)
      {
         Name = tips->Strings[0];
         Player = tips->Strings[1];
         Color = clRed;
         for (int i=0; i<GlobalOptions->Buddies->Count; i++)
         {
            if (tooltip.Pos(GlobalOptions->Buddies->Strings[i]) == 0) continue;
            Color = clYellow;
            break;
         }
      }
      else
      {
         Name = Player = "";
      }
      LName->Caption = Name;
      LPlayer->Caption = Player;
      LLand->Caption = Land;
      LPlayer->Font->Color = Color;

      delete tips;
}
//---------------------------------------------------------------------------

void __fastcall TFMain::BigMapKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
      bool tbool;
      int xes, yes, lc, tr, tmp;
      TGridRect sel;
      xes = BigMap->Width/BigMap->DefaultColWidth;
      yes = BigMap->Height/BigMap->DefaultRowHeight;
      if ((Key == VK_UP || Key == VK_DOWN) && Shift == TShiftState() << ssCtrl)
      {
         tr = BigMap->TopRow;
         tmp = (Key==VK_UP) ? -1 : 1;
         tr += tmp;
         if (tr>=0 && tr<=256-yes)
         {
            BigMap->TopRow = tr;
            sel = BigMap->Selection;
            sel.Top += tmp;
            sel.Bottom += tmp;
            BigMap->Selection = sel;
            BigMapSelectCell(NULL, sel.Left, sel.Top, tbool);                   // �������� ������� OnSelectCell
         }
         Key = NULL;
      }
 else if ((Key == VK_RIGHT || Key == VK_LEFT) && Shift == TShiftState() << ssCtrl)
      {
         lc = BigMap->LeftCol;
         tmp = (Key==VK_LEFT) ? -1 : 1;
         lc += tmp;
         if (lc>=0 && lc<=256-xes)
         {
            BigMap->LeftCol = lc;
            sel = BigMap->Selection;
            sel.Left += tmp;
            sel.Right += tmp;
            BigMap->Selection = sel;
            BigMapSelectCell(NULL, sel.Left, sel.Top, tbool);                   // �������� ������� OnSelectCell
         }
         Key = NULL;
      }
 else if (Key == VK_RETURN && Shift == TShiftState())
      {
         lc = BigMap->Selection.Left - xes/2;
         tr = BigMap->Selection.Top - yes/2;
         if (lc<0) lc = 0;
         if (tr<0) tr = 0;
         if (lc>256-xes) lc = 256-xes;
         if (tr>256-yes) tr = 256-yes;
         BigMap->LeftCol = lc;
         BigMap->TopRow = tr;
         Key = NULL;
      }
 else if (Key == VK_RETURN && Shift == TShiftState() << ssCtrl && AdderThread->last_x!=0 && AdderThread->last_y!=0)
      {
         lc = AdderThread->last_x-1 - xes/2;
         tr = AdderThread->last_y-1 - yes/2;
         if (lc<0) lc = 0;
         if (tr<0) tr = 0;
         if (lc>256-xes) lc = 256-xes;
         if (tr>256-yes) tr = 256-yes;
         BigMap->Visible = false;
         BigMap->LeftCol = lc;
         BigMap->TopRow = tr;
         BigMap->Visible = true;
         sel.Left = sel.Right = AdderThread->last_x-1;
         sel.Top = sel.Bottom = AdderThread->last_y-1;
         BigMap->Selection = sel;
         BigMapSelectCell(NULL, sel.Left, sel.Top, tbool);                      // �������� ������� OnSelectCell
         BigMap->SetFocus();
         Key = NULL;
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Button2Click(TObject *Sender)
{
        Application->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::DeleteDebugFiles(void)
{
        TSearchRec SearchRec;
        AnsiString DirPath = ExtractFilePath(Application->ExeName) + "debug\\";

        if (DirectoryExists(DirPath))
        {                                                                       // �������� debug ���������� �� ���� ������
           if (FindFirst(DirPath+"*.debug",  faAnyFile,  SearchRec) == 0)
           {
              do
              {
                 DeleteFile(DirPath+SearchRec.Name);
              }
              while (FindNext(SearchRec) == 0);
              FindClose(SearchRec);
           }
        }
        else
        {
           CreateDir(DirPath);                                                  // ������� debug ����������
        }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Button4Click(TObject *Sender)
{
      GetNetworkAdapters();
}
//---------------------------------------------------------------------------

void __fastcall TFMain::MiniMapMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
      int ptx, pty;
      TPoint ptc, pt;
      if (X <= MiniMap->Width/2) pt.x = MiniMap->Width/3 + X;
      else pt.x = X - MiniMap->Width/3;
      if (Y <= MiniMap->Height/2) pt.y = MiniMap->Height/3 + Y;
      else pt.y = Y - MiniMap->Height/3;
      AnsiString tooltip;
      MiniMap->Hint = tooltip.sprintf("x:%d, y:%d", X+1, Y+1);
      ptc = MiniMap->ClientToScreen(pt);
      Application->ActivateHint(ptc);
}
//---------------------------------------------------------------------------

void __fastcall TFMain::MergeButtonClick(TObject *Sender)
{
      int mapstrnum1, mapstrnum2, tmp;
      int MergedIndexes[256];
      AnsiString str1, str2;
      TStringList *MergedStrings;
      if (OpenDialogMERGE->Execute())
      {
         MergedStrings = new TStringList;
         MergedStrings->LoadFromFile(OpenDialogMERGE->FileName);
         RefreshRowIndexes(MergedIndexes, MergedStrings);
         for (int y=0; y<256; y++)
         {
            for (int x=0; x<256; x++)
            {
               mapstrnum1 = RowIndexes[y] + x;
               mapstrnum2 = MergedIndexes[y] + x;
               str1 = MapStrings->Strings[mapstrnum1];
               str2 = MergedStrings->Strings[mapstrnum2];
               if (str1.Pos("�� ���������") != 0 && str1 != str2)
                  MapStrings->Strings[mapstrnum1] = str2;
            }
         }
         delete MergedStrings;
         ReconstructMiniMap();
      }
}
//---------------------------------------------------------------------------

void __fastcall TFMain::Button3Click(TObject *Sender)
{
      AnsiString str,src,obj;
      int xc, yc, tmp;
      BmpProgress->Position = 0;
      BmpProgress->Visible = true;
      TPicture *MapPic;
      TCanvas* bCanvas;

      MapPic = new(TPicture);
      MapPic->Bitmap->Width = 42 * 128;
      MapPic->Bitmap->Height = 42 * 128;
      bCanvas = MapPic->Bitmap->Canvas;
         for (int y=0; y<128; y++)
         {
            for (int x=0; x<128; x++)
            {
               obj = "";
               str = GetCellStringFromMap(x+1, y+1);
               src = str.SubString(str.Pos("src=\"")+5,128);
               src.SetLength(src.Pos("\"")-1);
               if ((tmp=str.Pos("obj=\""))!=0)
               {
                  obj = str.SubString(tmp+5,128);
                  obj.SetLength(obj.Pos("\"")-1);
               }
               tmp = src.Pos("land0"); if (tmp) src[tmp+4] = '1';
               GifImage->LoadFromFile(GlobalOptions->ImagesPath + src);
               xc = (x-0)*42; yc = (y-0)*42;
               bCanvas->Draw(xc, yc, GifImage);
               if (!obj.IsEmpty())
               {
                  GifImage->LoadFromFile(GlobalOptions->ImagesPath + obj);
                  bCanvas->Draw(xc+(BigMap->DefaultColWidth-GifImage->Width)/2, yc+(BigMap->DefaultRowHeight-GifImage->Height)/2, GifImage);
               }
            }
            BmpProgress->StepIt();
            Application->ProcessMessages();
         }
      MapPic->Bitmap->SaveToFile("mapa1.bmp");
      delete MapPic;

      MapPic = new(TPicture);
      MapPic->Bitmap->Width = 42 * 128;
      MapPic->Bitmap->Height = 42 * 128;
      bCanvas = MapPic->Bitmap->Canvas;
         for (int y=0; y<128; y++)
         {
            for (int x=128; x<256; x++)
            {
               obj = "";
               str = GetCellStringFromMap(x+1, y+1);
               src = str.SubString(str.Pos("src=\"")+5,128);
               src.SetLength(src.Pos("\"")-1);
               if ((tmp=str.Pos("obj=\""))!=0)
               {
                  obj = str.SubString(tmp+5,128);
                  obj.SetLength(obj.Pos("\"")-1);
               }
               tmp = src.Pos("land0"); if (tmp) src[tmp+4] = '1';
               GifImage->LoadFromFile(GlobalOptions->ImagesPath + src);
               xc = (x-128)*42; yc = (y-0)*42;
               bCanvas->Draw(xc, yc, GifImage);
               if (!obj.IsEmpty())
               {
                  GifImage->LoadFromFile(GlobalOptions->ImagesPath + obj);
                  bCanvas->Draw(xc+(BigMap->DefaultColWidth-GifImage->Width)/2, yc+(BigMap->DefaultRowHeight-GifImage->Height)/2, GifImage);
               }
            }
            BmpProgress->StepIt();
            Application->ProcessMessages();
         }
      MapPic->Bitmap->SaveToFile("mapa2.bmp");
      delete MapPic;

      MapPic = new(TPicture);
      MapPic->Bitmap->Width = 42 * 128;
      MapPic->Bitmap->Height = 42 * 128;
      bCanvas = MapPic->Bitmap->Canvas;
         for (int y=128; y<256; y++)
         {
            for (int x=0; x<128; x++)
            {
               obj = "";
               str = GetCellStringFromMap(x+1, y+1);
               src = str.SubString(str.Pos("src=\"")+5,128);
               src.SetLength(src.Pos("\"")-1);
               if ((tmp=str.Pos("obj=\""))!=0)
               {
                  obj = str.SubString(tmp+5,128);
                  obj.SetLength(obj.Pos("\"")-1);
               }
               tmp = src.Pos("land0"); if (tmp) src[tmp+4] = '1';
               GifImage->LoadFromFile(GlobalOptions->ImagesPath + src);
               xc = (x-0)*42; yc = (y-128)*42;
               bCanvas->Draw(xc, yc, GifImage);
               if (!obj.IsEmpty())
               {
                  GifImage->LoadFromFile(GlobalOptions->ImagesPath + obj);
                  bCanvas->Draw(xc+(BigMap->DefaultColWidth-GifImage->Width)/2, yc+(BigMap->DefaultRowHeight-GifImage->Height)/2, GifImage);
               }
            }
            BmpProgress->StepIt();
            Application->ProcessMessages();
         }
      MapPic->Bitmap->SaveToFile("mapa3.bmp");
      delete MapPic;

      MapPic = new(TPicture);
      MapPic->Bitmap->Width = 42 * 128;
      MapPic->Bitmap->Height = 42 * 128;
      bCanvas = MapPic->Bitmap->Canvas;
         for (int y=128; y<256; y++)
         {
            for (int x=128; x<256; x++)
            {
               obj = "";
               str = GetCellStringFromMap(x+1, y+1);
               src = str.SubString(str.Pos("src=\"")+5,128);
               src.SetLength(src.Pos("\"")-1);
               if ((tmp=str.Pos("obj=\""))!=0)
               {
                  obj = str.SubString(tmp+5,128);
                  obj.SetLength(obj.Pos("\"")-1);
               }
               tmp = src.Pos("land0"); if (tmp) src[tmp+4] = '1';
               GifImage->LoadFromFile(GlobalOptions->ImagesPath + src);
               xc = (x-128)*42; yc = (y-128)*42;
               bCanvas->Draw(xc, yc, GifImage);
               if (!obj.IsEmpty())
               {
                  GifImage->LoadFromFile(GlobalOptions->ImagesPath + obj);
                  bCanvas->Draw(xc+(BigMap->DefaultColWidth-GifImage->Width)/2, yc+(BigMap->DefaultRowHeight-GifImage->Height)/2, GifImage);
               }
            }
            BmpProgress->StepIt();
            Application->ProcessMessages();
         }
      MapPic->Bitmap->SaveToFile("mapa4.bmp");
      delete MapPic;

      BmpProgress->Visible = false;

}
//---------------------------------------------------------------------------

