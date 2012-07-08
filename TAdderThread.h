//---------------------------------------------------------------------------

#ifndef TAdderThreadH
#define TAdderThreadH
//---------------------------------------------------------------------------
#include "RegExpr.hpp"
#include <Classes.hpp>
#include "WarchaosMapperDecl.h"
//---------------------------------------------------------------------------
class TAdderThread : public TThread
{
private:
      int htmls,strs;
      TStringList *HTML;
      TRegExpr *r;
      TStringList *mobjects, *mpa, *players;
      Double ClockRate;
      AnsiString land, s_obja;
      void __fastcall GetStringsFromStream(TStringList *Strings, TMemoryStream *Stream);
      bool __fastcall AppendHTMLCode(TStringList *Strings);
      bool __fastcall AppendVBCode(TStringList *Strings);
      bool __fastcall GetArrayParameter(TStringList *Strings, char *param, TStringList *dest);
      bool __fastcall GetStringParameter(TStringList *Strings, char *param, AnsiString &dest);
      bool __fastcall GetVBMap(void);
      int __fastcall GetSubStringsCount(AnsiString str, AnsiString sam);
      TList *Sockets;
protected:
      void __fastcall Execute();
public:
      __fastcall TAdderThread(bool CreateSuspended);
      void __fastcall CreateSocket(WORD Port, int BaseSequency);
      bool __fastcall AddPacketsToSocket(WORD Port, ULONG SequenceNumber, char *Buffer, int Length, bool Fin);
      int appendpages, SocketsUsed;
      TStringList *MapStrings;
      int *RowIndexes;
      int last_x, last_y;
      bool adderPause, imstopped;
      TMiniMapCh *MiniMapCh;
      unsigned int ChCounter;
};
//---------------------------------------------------------------------------
#endif
