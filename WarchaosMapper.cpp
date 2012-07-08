//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
USERES("WarchaosMapper.res");
USEFORM("MainForm.cpp", FMain);
USEUNIT("WarchaosMapperDecl.cpp");
USELIB("..\..\Lib\wininet.lib");
USEUNIT("..\..\..\regexpr\Source\RegExpr.pas");
USEUNIT("THTTPSniffer.cpp");
USEUNIT("TAdderThread.cpp");
USELIB("..\..\Lib\PSDK\iphlpapi.lib");
USELIB("..\..\..\WpdPack\Lib\wpcap.lib");
USELIB("..\..\..\WpdPack\Lib\Packet.lib");
//#pragma comment (lib, "wpcap.lib")
//#pragma comment (lib, "Packet.lib")
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
      try
      {
             Application->Initialize();
             Application->CreateForm(__classid(TFMain), &FMain);
             Application->Run();
      }
      catch (Exception &exception)
      {
             Application->ShowException(&exception);
      }
      return 0;
}
//---------------------------------------------------------------------------
