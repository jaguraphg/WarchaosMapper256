#ifndef ThermoServer_DeclH
#define ThermoServer_DeclH
//---------------------------------------------------------------------------

// ���������
typedef enum { tc_SelectNextCompany, tc_SelectNextCompanyPage, tc_ProcessCurrentCompany,
               tc_SelectUsersPage, tc_ProcessCurrentUser, tc_SelectNextUser,
               tc_SelectNextUserPage, tc_RegionDoneSuccess} TCStep;
typedef enum { sd_resDer, sd_resMet, sd_resKam, sd_resZol,
               sd_resSunduk, sd_btnOk, sd_btnGet, sd_InvSn, sd_InvNn, sd_PutSn, sd_PutNn,
               sd_GetSn, sd_GetNn, sd_SelAll, sd_nav0, sd_nav1, sd_nav2, sd_nav3,
               sd_nav4, sd_nav5, sd_nav6, sd_nav7, sd_nav8, sd_bmTop, sd_bmMid, sd_bmBottom } WDsprdef;

const int LandColors[] = { 0x9C5108, 0xB56D29, 0xB56D29, 0xB56D29, 0xB56D29, 0xB56D29, 0x9C7539, 0x9C7539,
                           0x9C7539, 0x9C7539, 0xA55508, 0xA55508, 0xA55508, 0xA55508, 0xA03000, 0xFFFFFF,
                           0x085910, 0x085910, 0x085910, 0x085910, 0x185D31, 0x185D31, 0x185D31, 0x5A655A,
                           0x1875AD, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
                           0x215929, 0x215929, 0x215929, 0x215929, 0x215929, 0x215929, 0x215929, 0x215929,
                           0x215929, 0x215929, 0x215929, 0x215929, 0x215929, 0x215929, 0x1A441A, 0xFFFFFF,
                           0x9C9EA5, 0x637563, 0x637563, 0x637563, 0x637563, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
                           0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF,
                           0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C,
                           0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C, 0x18698C, 0xFFFFFF,
                           0x524D08, 0x000000, 0x524D08, 0x524D08, 0x524D08, 0x524D08, 0x524D08, 0xFFFFFF };


const char *const ProgramVersionNumber = "1.18";

#define MiniMapChBufferLength       4096

#define WORKINGREGIONFROM   0
#define WORKINGREGIONTO     1
#define NAVIGATEREGIONFROM  2
#define NAVIGATEREGIONTO    3

#define RAND(min,max) rand()%(max-min)+min


// ��������� �������
int GetHTMValueToInt(char *ptstr, char *param);
char* FindBackTag(char *ptstr, char *Tag);
AnsiString GetHTMPrintable(char *ptstr);
AnsiString __fastcall UTF8ToAnsi(AnsiString &string);
char* __fastcall AnsiStrPosCS(char *string, char *SubStr, bool CaseSensivity=false);
int __fastcall AnsiStrPosCS(AnsiString string, char *SubStr, bool CaseSensivity=false);
char* __fastcall FindSubStringCS(char *buffer, char *SubStr, int buflen, bool CaseSensivity=false);
AnsiString _fastcall ByzakToStr(AnsiString Byzak);
AnsiString _fastcall StrToByzak(AnsiString Str);


// ���������

struct TMiniMapCh
{
      USHORT   x;
      USHORT   y;
      TColor  color;
};

// ������
class TSnifSocket
{
private:
      double ClockRate;
public:
      TSnifSocket();
      ~TSnifSocket();
      void AddPacket(void);

      bool Ready;
      ULONG BaseSequency;
      int SumLength;
      int TotalLength;
      TMemoryStream *Data;
      double TTLWhenCreate;
      WORD Port;                                                                // ����� ����� (����=0 �� ����� ���������)

};

class TSysTimer
{
private:
      Double ClockRate;
      Double StartTime;
public:
      TSysTimer();
      ~TSysTimer();
      void StartTimer(void);
      Double ReadTimer(void);
      void Pause(Double time);
};

class TGlobalOptions
{
private:
      void ReadOptions(void);
      AnsiString __fastcall GetINIFileName(void);
public:
      TGlobalOptions();
      ~TGlobalOptions();
      void WriteOptions(void);

      int DefaultNetworkInterface;
      int SocketsTTL;
      AnsiString BuddyFileName;
      AnsiString ImagesPath;
      TStringList *Buddies;
      bool DebugHTML, DebugVB1, DebugVB2;
      bool FilterByIP;
      AnsiString FilterIP;
};


//---------------------------------------------------------------------------
#endif
