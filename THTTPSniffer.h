//---------------------------------------------------------------------------

#ifndef THTTPSnifferH
#define THTTPSnifferH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
class THTTPSniffer : public TThread
{
private:
      char *Buffer;
protected:
      void __fastcall Execute();
public:
      __fastcall THTTPSniffer(bool CreateSuspended);
      AnsiString Host;
      int packets;
      TAdderThread *AdderThread;
      bool snifPause, imstopped;
      bool FilterByIP;
      ULONG FilterIPAddress;
};
//---------------------------------------------------------------------------
#endif
