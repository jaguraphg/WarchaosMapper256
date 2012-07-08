//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "RegExpr.hpp"
#include "WarchaosMapperDecl.h"
#include "TAdderThread.h"
#include <winsock2.h>
#pragma package(smart_init)
extern TGlobalOptions *GlobalOptions;
//---------------------------------------------------------------------------

__fastcall TAdderThread::TAdderThread(bool CreateSuspended) : TThread(CreateSuspended)
{
      adderPause = imstopped = false;
      appendpages = SocketsUsed = 0;
      last_x = last_y = 0;
      mpa = mobjects = NULL;
      ChCounter = 0;
}
//---------------------------------------------------------------------------
void __fastcall TAdderThread::Execute()
{
      double TimeWhoLive;
      TSnifSocket *tmpsocket;
      Sockets = new TList;
      HTML = new TStringList;
      r = new TRegExpr;
      r->ModifierI = true;

      LARGE_INTEGER tmp;
      QueryPerformanceFrequency(&tmp);
      ClockRate = tmp.QuadPart;

      try
      {
         while(!Terminated)
         {
            if (adderPause && imstopped) continue;
            if (adderPause)
            {
               imstopped = true;
               continue;
            }
            for (int i=0; i<Sockets->Count; i++)                                // Поиск и обработка принятых до конца потоков
            {
               tmpsocket = (TSnifSocket *)Sockets->Items[i];
               if (tmpsocket->Ready)
               {
                  GetStringsFromStream(HTML, tmpsocket->Data);
                  tmpsocket->Data->Clear();
                  tmpsocket->Port = 0;
                  tmpsocket->Ready = false;
                  if (GlobalOptions->DebugHTML) HTML->SaveToFile("debug\\HTMLtext"+IntToStr(htmls++)+".debug");
                  if (AppendHTMLCode(HTML)) appendpages++;
                  break;
               }
            }
            QueryPerformanceCounter(&tmp);
            for (int i=0; i<Sockets->Count; i++)                                // Поиск и очистка сокетов с просроченным TTL
            {
               tmpsocket = (TSnifSocket *)Sockets->Items[i];
               if (!tmpsocket->Ready && tmpsocket->Port!=0)
               {
                  TimeWhoLive = (tmp.QuadPart-tmpsocket->TTLWhenCreate)/ClockRate;
                  if (!tmpsocket->Ready && tmpsocket->Port!=0 && TimeWhoLive > GlobalOptions->SocketsTTL)
                  {
                     tmpsocket->Data->Clear();
                     tmpsocket->Port = 0;
                  }
               }
            }

            // Delete many empty sockets

         }
      }
      __finally
      {
         delete r;
         delete HTML;
         delete Sockets;
      }

}
//---------------------------------------------------------------------------
// Аналог TStringList:LoadFromStream тока понимает разбитие строк на 0D, 0A, 0D0A
void __fastcall TAdderThread::GetStringsFromStream(TStringList *Strings, TMemoryStream *Stream)
{
     char *curpos = (char *)Stream->Memory;
     char *endpos = curpos + Stream->Size;
     char *findpos;
     int strlen;
     Strings->Clear();
     while (curpos < endpos)
     {
        findpos = (char *)memchr(curpos, 0x0A, endpos-curpos);
        if (findpos)
        {
          strlen = findpos - curpos;
          if (strlen>0 && findpos[-1]==0x0D) strlen--;
          Strings->Add(AnsiString(curpos,strlen));
          curpos = findpos + 1;
        }
        else
        {
          strlen = endpos - curpos;
          Strings->Add(AnsiString(curpos,strlen));
          curpos = endpos;
        }
     }
}
//---------------------------------------------------------------------------
// Получает строки начиная с HTTP заголовка до конца HTML страницы
// Выдирает JavaScript, перегруппировывает все в строки и вызывает процу AppendVBCode
bool __fastcall TAdderThread::AppendHTMLCode(TStringList *Strings)
{
      AnsiString str, content;
      bool TextHtml = false, result = false;
      int tmp, HtmlLineFrom;
      for (int i=0; i<10; i++)
      {
         if (i >= Strings->Count) break;
         str = Strings->Strings[i];
         if (str.SubString(1,13) == "Content-Type:")
         {
            tmp = AnsiStrPosCS(str, "text/html");
            if (tmp != 0) { TextHtml=true; HtmlLineFrom=i+1; break; }
         }
      }
      if (!TextHtml) return false;

      for (int i=HtmlLineFrom; i<Strings->Count; i++)
         content += Strings->Strings[i] + " ";
      TStringList *VB = new TStringList;
      try
      {
         r->Expression = "< *script *language *= *javascript *> *(.+) *< */ *script *>";
         if (r->Exec(content))
         {
            content = r->Match[1];
            r->Expression = " *; *";
            r->Split(content, VB);
            if (GlobalOptions->DebugVB1) VB->SaveToFile("debug\\VBdebug1.debug");
            result = AppendVBCode(VB);
         }
      }
      __finally
      {
         delete VB;
      }
      return result;
}
//---------------------------------------------------------------------------
// Получает строки VB программы, формирует из них все необходимые массивы для составления кадра карты
// При успешном получении массивов вызывает функцию обработки этих массивов
bool __fastcall TAdderThread::AppendVBCode(TStringList *Strings)
{
      bool m1, result = false;
      mobjects = new TStringList;
      mpa = new TStringList;
      players = new TStringList;
      try
      {
         m1 = GetArrayParameter(Strings, "mobjects", mobjects);
         if (m1)
         {
            m1 &= GetArrayParameter(Strings, "mpa", mpa);
            m1 &= GetArrayParameter(Strings, "players", players);
            m1 &= GetStringParameter(Strings, "land", land);
            m1 &= GetStringParameter(Strings, "obja", s_obja);
            m1 &= GetVBMap();
         }
         result = m1;
      }
      __finally
      {
         if (players) { delete players; players = NULL; }
         if (mpa) { delete mpa; mpa = NULL; }
         if (mobjects) { delete mobjects; mobjects = NULL; }
      }
      return result;
}
//---------------------------------------------------------------------------
// В строках Strings ищет параметр с именем param и в случае нахождения закидывает его в массив строк dest
bool __fastcall TAdderThread::GetArrayParameter(TStringList *Strings, char *param, TStringList *dest)
{
      bool result = false;
      AnsiString str;
      r->Expression = "[. ]" + AnsiString(param) + " *=";
      for (int i=0; i<Strings->Count; i++)
      {
         if (r->Exec(Strings->Strings[i]))
         {
            r->Expression = "=.*?[(\\[](.+)[)\\]]";
            if (r->Exec())
            {
               str = r->Match[1];
               r->Expression = " *, *";
               r->Split(str, dest);
               for (int s=0; s<dest->Count; s++)
               {
                  while (GetSubStringsCount(dest->Strings[s],"\"") == 1)
                     { dest->Strings[s] = dest->Strings[s]+','+dest->Strings[s+1]; dest->Delete(s+1); }
                  r->Expression = "^\" *(.*) *\"$";
                  if (r->Exec(dest->Strings[s]))
                  {
                     dest->Strings[s] = r->Match[1];
                  }
               }
               if (GlobalOptions->DebugVB2) dest->SaveToFile("debug\\VBdebug2_"+IntToStr(strs++)+".debug");
               result = true;
            }
            break;
         }
      }
      return result;
}
//---------------------------------------------------------------------------
// В строках Strings ищет параметр с именем param и в случае нахождения закидывает его в строку dest
bool __fastcall TAdderThread::GetStringParameter(TStringList *Strings, char *param, AnsiString &dest)
{
      bool result = false;
      r->Expression = "[. ]" + AnsiString(param) + " *=";
      for (int i=0; i<Strings->Count; i++)
      {
         if (r->Exec(Strings->Strings[i]))
         {
            r->Expression = "= *\"?([^\"]+)";
            if (r->Exec())
            {
               dest = r->Match[1];
               result = true;
               break;
            }
         }
      }
      return result;
}
//---------------------------------------------------------------------------
// Собственно составляет строчки всей видимой карты и помещает в БД
bool __fastcall TAdderThread::GetVBMap(void)
{
      bool result = false;
      int radius, x, y, x1, y1, x2, y2, obja, pl, mapwidth, cp, tmp, c, mapstrnum, objtype, i;
      char *ld[] = { "", "", "Вода", "Равнина", "Деревья", "Горы", "Дорога", "Болото" };
      AnsiString landir, tip, cs, mapstr, tmpstr, tmptip, src, obj;
      TColor color;

      if (mobjects->Strings[0].ToInt() != 1) return false;
      obja = s_obja.ToInt();
      x = mobjects->Strings[obja].ToInt();
      y = mobjects->Strings[obja+1].ToInt();
      tmp = mpa->Strings[3].ToInt();
      radius = tmp!=0 ? tmp : 7;
      x1 = x - radius; x2 = x + radius;
      y1 = y - radius; y2 = y + radius;
      mapwidth = radius + radius + 1;
      int *mapobjects = new int[mapwidth*mapwidth];
      ZeroMemory(mapobjects, sizeof(int)*mapwidth*mapwidth);

      try
      {
         for (i=1; i<mobjects->Count; i+=9)
         {
            int cx = mobjects->Strings[i].ToInt();
            int cy = mobjects->Strings[i+1].ToInt();
            if ((cx>=x1)&&(cx<=x2)&&(cy>=y1)&&(cy<=y2))
            {
               cp = mapwidth*(cy-y1)+cx-x1;
               if (!mapobjects[cp]) mapobjects[cp] = i;
            }
         }
         landir="land0/";
         cp = 0;
         for(int cy=y1; cy<=y2; cy++)
         {
            for(int cx=x1; cx<=x2; cx++, cp++)
            {
               if (cx<1 || cx>256 || cy<1 || cy>256) continue;
               obj = tmptip = "";
               objtype = 0;
               c = land[cp+1];
               if (c==114||c==115||c==116||c==117||c==118) c = 112;             // Очистка болота
               tip = (c!=113) ? ld[c>>4] : "Темнота";
               cs=landir + IntToStr(c);
               tip += " x:"+IntToStr(cx)+" y:"+IntToStr(cy);
               i = mapobjects[cp];
               if (!i)
               {
                  src = cs + ".gif";
               }
               else
               {
                  src = cs + ".gif";
                  objtype = mobjects->Strings[i+2].ToInt();
                  if (objtype == 342) objtype = 9342;                           // Подмена дозорной башни
                  if (objtype==3||objtype==13||objtype==23|objtype==33||objtype==43||objtype==53||objtype==9342)
                  {
                     pl = mobjects->Strings[i+3].ToInt();
                     obj = IntToStr(objtype) + ".gif";\
                     tmpstr = mobjects->Strings[i+6];
                     if (!tmpstr.IsEmpty())
                        tmptip = tmpstr + '$';
                     if(pl >= 0)
                     {
                        tmptip += players->Strings[pl+1];
                        tmpstr = players->Strings[pl+2];
                        if(tmpstr != "0") tmptip += " ["+tmpstr+"]";
                        tmptip += "$";
                     }
                     tip = tmptip + tip;
                  }
               }


               mapstr = "<TD><src=\"" + src + "\"";
               if (!obj.IsEmpty()) mapstr += " obj=\"" + obj + "\"";
               mapstr += " tip=\"" + tip + "\"></TD>";

               mapstrnum = RowIndexes[cy-1] + cx-1;
               MapStrings->Strings[mapstrnum] = mapstr;

               if (objtype==3||objtype==13||objtype==23||objtype==33||objtype==43||objtype==53||objtype==9342)
               {
                  color = clRed;
                  for (int i=0; i<GlobalOptions->Buddies->Count; i++)
                  {
                     if (tip.Pos(GlobalOptions->Buddies->Strings[i]) == 0) continue;
                     color = clYellow;
                     break;
                  }
               }
               else
               {
                  color = (TColor)LandColors[c-32];
               }
               TMiniMapCh *ChRel = &MiniMapCh[ChCounter&(MiniMapChBufferLength-1)];
               ChRel->x = cx-1;
               ChRel->y = cy-1;
               ChRel->color = color;
               ChCounter++;
            }
         }
         last_x = x; last_y = y;
         result = true;
      }
      __finally
      {
         delete[] mapobjects;
      }
      return result;
}
//---------------------------------------------------------------------------
// Создает (или занимает один из свободных) сокет под выбранный порт
void __fastcall TAdderThread::CreateSocket(WORD Port, int BaseSequency)
{
      LARGE_INTEGER tmp;
      TSnifSocket *tmpsocket, *Socket = NULL;
      for (int i=0; i<Sockets->Count; i++)
      {
         tmpsocket = (TSnifSocket *)Sockets->Items[i];
         if (tmpsocket->Port == 0)
         {
            Socket = tmpsocket;
            break;
         }
      }
      if (Socket == NULL)
      {
         Socket = new TSnifSocket;
         Sockets->Add(Socket);
         SocketsUsed = Sockets->Count;
      }
      QueryPerformanceCounter(&tmp);
      Socket->TTLWhenCreate = tmp.QuadPart;

      Socket->Port = Port;
      Socket->BaseSequency = htonl(BaseSequency) + 1;
      Socket->SumLength = Socket->TotalLength = 0;
}
//---------------------------------------------------------------------------
// Записывает данные в сокет с портом Port по адресу SequenceNumber
// Флаг Fin указывает на то что это последний пакет в последовательности
bool __fastcall TAdderThread::AddPacketsToSocket(WORD Port, ULONG SequenceNumber, char *Buffer, int Length, bool Fin)
{
      TSnifSocket *tmpsocket, *Socket = NULL;
      for (int i=0; i<Sockets->Count; i++)
      {
         tmpsocket = (TSnifSocket *)Sockets->Items[i];
         if (!tmpsocket->Ready && tmpsocket->Port == Port)
         {
            Socket = tmpsocket;
            break;
         }
      }
      if (Socket)
      {
         Integer seq = htonl(SequenceNumber)-Socket->BaseSequency;
         if (Length != 0)
         {
            if (seq < 1000000) {
                Socket->Data->Seek(seq, soFromBeginning);
                Socket->Data->Write(Buffer, Length);
                Socket->SumLength += Length;
            }
         }
         Socket->Data->SaveToFile("debug\\block-"+IntToStr(Port)+AnsiString("(")+IntToStr(seq)+")"+(Fin?"FIN":"")+".block");
         if (Fin) {
            Socket->TotalLength = Socket->Data->Position;
         }
         if (Socket->TotalLength!=0 && Socket->SumLength >= Socket->TotalLength)
         {
            Socket->Ready = true;
         }
      }
      return Socket;
}
//---------------------------------------------------------------------------
int __fastcall TAdderThread::GetSubStringsCount(AnsiString str, AnsiString sam)
{
      int count = 0;
      while (int p=str.Pos(sam))
      {
         str.Delete(1,p);
         count++;
      }
      return count;
}
//---------------------------------------------------------------------------

/*
   radius=mpa[3]?mpa[3]:7;
   x1=x-radius;  x2=x+radius;
   y1=y-radius;  y2=y+radius;
   var mapwidth=radius+radius+1;
   var mapobjects=new Array(mapwidth*mapwidth);
   for(i=1; i<mobjects.length; i+=9)
   {
      cx=mobjects[i];      cy=mobjects[i+1];
      if(((cx>=x1)&&(cx<=x2))&&((cy>=y1)&&(cy<=y2)))
      {
         cp=mapwidth*(cy-y1)+cx-x1;
         if(i==objnav && mapobjects[cp]) navpt=cp;
         if(!mapobjects[cp]) mapobjects[cp]=i;
      }
   }
   if(mpa[0])
   {
      if(mpa[0]==2) targetText="$<font color=blue>"+mpa[1]+"</font>";
      else targetText='';
   }
   else targetText='';

   if(!mpa[3])
   {
      ht+="<table border=0 cellspacing=0 cellpadding=0><tr><td height=12 colspan=3 bgcolor=#202020>";
      if(mpa[5]==1) ht+="<span class=nav1 style='margin-left: "+mpa[6]+"px'>"+mpa[7]+"</span>";
      ht+="</td></tr><tr><td width=12 bgcolor=#202020 valign=top>";
      if(mpa[5]==2){ ht+="<div class=nav2 style='margin-top: "+mpa[6]+"px'>";
      if( mpa[7]>=100 ){ ht+=parseInt(mpa[7]/100)+"<br>"; mpa[7]%=100;}
      if( mpa[7]>=10 ){ ht+=parseInt(mpa[7]/10)+"<br>"; mpa[7]%=10;}
      ht+=mpa[7];
      ht+="</div>";}
      ht+="</td><td>";
   }
   ht+="<table border=0 cellspacing=0 cellpadding=0>";
   var ld = new Array("","","Вода","Равнина","Деревья","Горы","Дорога","Болото");
   var landir="land"+mpa[4]+"/";
   for(cy=y1, cp=0; cy<=y2; cy++)
   {
      ht+="<tr>";
      ptry = (cy<(y-mpa[2]))||(cy>(y+mpa[2]));

      for(cx=x1; cx<=x2; cx++, cp++)
      {
         c=land.charCodeAt(cp);
         tip = (c!=113) ? ld[c>>4] : "Темнота";
         cs=landir+c;
         tip+=" x:"+cx+" y:"+cy+"";
         if(!mapobjects[cp])
         {
            ptrx = ptry || cx<(x-mpa[2]) || cx>(x+mpa[2]) || c==113;
            ht+="<td valign=middle align=center width=42 height=42><img src="+cs+".gif width=42 height=42 tooltip=\"";
            ht+=tip;
            if(!ptrx)
            {
               ht+=targetText;
               ht+="\" class=trg3 onClick=\"cmIComm(5,"+cx+","+cy+",0);";
            }
            ht+="\"></td>";
         }
         else
         {
            ht+="<td background="+cs+".gif valign=middle align=center width=42 height=42>";
            i=mapobjects[cp];
            targ=0;
            objtype=mobjects[i+2]%10;
            ht+="<img src=";
            if(objtype==4)ht+="it/";
            ht+=mobjects[i+2]+".gif width=40 height=";
            ht+=(mobjects[i+4]&8)?30:40;
            pl=mobjects[i+3];
            if(cp==navpt) ht+=" class=dot";

            if(objtype==9)
            {
               ht+=" style=\"cursor: pointer; cursor:hand\" onClick=\"cmI6('p"+mobjects[i+5]+"');\"";
            }
            else if(mobjects[i+4]&2)
            {
               if(mpa[0])
               {
                  if(mpa[0]==2) ht+=" class=trg3";
                  else if(mobjects[i+4]&128) ht+=" class=trg2"; else ht+=" class=trg1";
               }
               ht+=" onClick=\"cmIComm(3,"+mobjects[i]+","+mobjects[i+1]+","+mobjects[i+5]+");\"";
               targ=1;
            }
            else if(mobjects[i+4]&1)
            {
               ht+=" style=\"cursor: pointer; cursor:hand;\" onClick=\"cmIA("+mobjects[i+5]+",'',arguments[0]);\"";
            }
            else
               ht+=" onclick=\"hlr(arguments[0],1,"+mobjects[obja+5]+","+mobjects[i+5]+");\"";
            ht+=" tooltip=\"";
            if(mobjects[i+6])
            {
               if(players[pl+3] && pl>=0)ht+="<img src=http://warhaos.ru/avatars/"+players[pl+3]+(players[pl+3]&1?".jpg":".gif")+" align=left>";
               ht+=mobjects[i+6];
               ht+="$";
            }
            if(pl>=0)
            {
               ht+="<font color="+gcol[players[pl]]+"><b>"+players[pl+1]+"</b>";
               if(players[pl+2]) ht+=" ["+players[pl+2]+"]";
               ht+="</font>$";
            }
            ht+=tip;
            if(targ) ht+=targetText;
            ht+="\">";

            if((pl>=0)&&(cp!=navpt))
            {
               ht+="<br><img src=ind.gif ";
               wdth=7;
               if(mobjects[i+7]>0) wdth=7+5*(mobjects[i+7]/10+1);
               else wdth=7-10*mobjects[i+7];
               if(wdth>30) wdth=30;
               ht+=" width="+wdth;
               ht+=" class=ind"+players[pl]+">";
            }
            ht+="</td>";
         }
      }
      ht+="</tr>";
   }
   ht+="</table>";
   if(!mpa[3])
   {
      ht+="</td><td width=12 bgcolor=#202020 valign=top>";
      if(mpa[5]==3){ ht+="<div class=nav2 style='margin-top: "+mpa[6]+"px'>";
      if( mpa[7]>=100 ){ ht+=parseInt(mpa[7]/100)+"<br>"; mpa[7]%=100;}
      if( mpa[7]>=10 ){ ht+=parseInt(mpa[7]/10)+"<br>"; mpa[7]%=10;}
      ht+=mpa[7];
      ht+="</div>";}
      ht+="</td></tr><tr><td height=12 colspan=3 bgcolor=#202020>";
      if(mpa[5]==4) ht+="<span class=nav1 style='margin-left: "+mpa[6]+"px'>"+mpa[7]+"</span>";
      ht+="</td></tr></table>";
   }
*/
