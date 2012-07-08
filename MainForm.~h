//---------------------------------------------------------------------------

#ifndef MainFormH
#define MainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Mask.hpp>
#include <Graphics.hpp>
#include <Grids.hpp>
#include "PicClip.hpp"
#include "RxGIF.hpp"
#include <Dialogs.hpp>
#include <Filectrl.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
class TFMain : public TForm
{
__published:	// IDE-managed Components
      TDrawGrid *BigMap;
      TPanel *Panel3;
      TLabel *LTCPPackets;
      TLabel *Label2;
      TLabel *Label1;
      TLabel *LAppendPages;
      TLabel *Label3;
      TButton *Button1;
      TButton *SkormitButton;
      TPanel *Panel1;
      TImage *MiniMap;
      TBevel *MapBevel;
      TButton *BtnStart;
      TMemo *Memo1;
      TButton *BtnStop;
      TComboBox *CBNetworkInterfaces;
      TPanel *Panel2;
      TLabel *LName;
      TLabel *LPlayer;
      TLabel *LLand;
      TButton *Button2;
      TButton *MergeButton;
      TOpenDialog *OpenDialogHTML;
      TOpenDialog *OpenDialogMERGE;
      TLabel *Label4;
      TLabel *LSocketsUsed;
      TTimer *Timer;
      TButton *Button3;
      TProgressBar *BmpProgress;
    TButton *Button4;
      void __fastcall FormCreate(TObject *Sender);
      void __fastcall FormDestroy(TObject *Sender);
      void __fastcall Button1Click(TObject *Sender);
      void __fastcall BigMapDrawCell(TObject *Sender, int ACol,
          int ARow, TRect &Rect, TGridDrawState State);
      void __fastcall SkormitButtonClick(TObject *Sender);
      void __fastcall MiniMapClick(TObject *Sender);
      void __fastcall BigMapTopLeftChanged(TObject *Sender);
      void __fastcall BigMapMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
      void __fastcall BtnStartClick(TObject *Sender);
      void __fastcall TimerTimer(TObject *Sender);
      void __fastcall BtnStopClick(TObject *Sender);
      void __fastcall CBNetworkInterfacesChange(TObject *Sender);
      void __fastcall BigMapSelectCell(TObject *Sender, int ACol, int ARow,
          bool &CanSelect);
      void __fastcall BigMapKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
      void __fastcall Button2Click(TObject *Sender);
//      void __fastcall zShapeBtn1Click(TObject *Sender);
      void __fastcall MiniMapMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
      void __fastcall MergeButtonClick(TObject *Sender);
      void __fastcall Button3Click(TObject *Sender);
    void __fastcall Button4Click(TObject *Sender);
private:	// User declarations
      int OldRedrawRow, OldRedrawCol;
      int pause;
      int AppendPages, TCPPackets, SocketsUsed;
      TStringList *MapStrings;
      int *RowIndexes;
      TGIFImage *GifImage;
      TRegExpr *r;
      UINT GridHintTimerHandle;
      THTTPSniffer* SnifThread;
      TAdderThread *AdderThread;
      unsigned int ChPtr;
      void __fastcall EmulateCopyToClipBoard(void);
      AnsiString __fastcall GetCellStringFromMap(int Col, int Row);
      void __fastcall RefreshRowIndexes(int *Indexes, TStringList *Strings);
      void __fastcall MarkAllPoints(TPoint* CenterPoints);
      void __fastcall AssignMapFile(AnsiString MapFileName);
      void __fastcall AssignOneMapCell(char *tstr, AnsiString& BASE);
      AnsiString __fastcall ExtractStringParameter(AnsiString& str, AnsiString param);
      void __fastcall ReconstructMiniMap(void);
      void __fastcall DisplayGridHint(int Pause);
      static void CALLBACK GridHintFunc(HWND Wnd, UINT Msg, UINT TimerID, DWORD Time);
      void __fastcall GridHintExpired(void);
      void __fastcall StopGridHintTimer(void);
      void __fastcall GetNetworkAdapters(void);
      void __fastcall DeleteDebugFiles(void);
public:		// User declarations
      __fastcall TFMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFMain *FMain;
//---------------------------------------------------------------------------
#endif
