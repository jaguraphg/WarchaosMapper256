//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <inifiles.hpp>               
#include "WarchaosMapperDecl.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Ф У Н К Ц И И
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//Принимает указатель на строку "...=.......param??=??value" и возвращает value
int GetHTMValueToInt(char *ptstr, char *param)
{
      int value = -1;
      char *lastr = AnsiStrPos(ptstr, ">");
      while (true)
      {
         ptstr = AnsiStrPos(ptstr, param) + 1;
         if (ptstr >= lastr) break;
         ptstr += StrLen(param)-1;
         if (!(*ptstr == ' ' || *ptstr == '=')) continue;
         while (*ptstr == ' ') ptstr++;
         if (*ptstr != '=') continue;
         ptstr++;
         while (*ptstr == ' ') ptstr++;
         if (*ptstr < '0' || *ptstr > '9') continue;
         char *tut = ptstr;
         while (*ptstr >= '0' && *ptstr <= '9') ptstr++;
         if (!(*ptstr == ' ' || *ptstr == '>')) continue;
         AnsiString param = AnsiString(tut,ptstr-tut);
         value = param.ToInt();
      }

      return value;
}
//---------------------------------------------------------------------------
//Производит поиск обратного тега Tag с места ptsrt (не учитывая открываемые/закрываемые теги)
char* FindBackTag(char *ptstr, char *Tag)
{
      AnsiString bTag = AnsiString("/")+Tag;

      return ptstr;
}
//---------------------------------------------------------------------------
//Принимает указатель на строку "<...>?<...>text<" и возвращает "text"
AnsiString GetHTMPrintable(char *ptstr)
{
      while (*ptstr == '<')
         ptstr = AnsiStrPos(ptstr, ">") + 1;
      AnsiString hstr = AnsiString(ptstr, AnsiStrPos(ptstr, "<")-ptstr);
      return hstr;
}
//---------------------------------------------------------------------------
// Конвертор кодировки UTF8 в Ansi
AnsiString __fastcall UTF8ToAnsi(AnsiString &string)
{
      wchar_t *buf1 = NULL;
      char *buf2 = NULL;
      int res_len, err=0;
      AnsiString ansi;
	res_len = MultiByteToWideChar(
		CP_UTF8,			// Code page
		0,	  			// No flags
		string.c_str(),		// Multibyte characters string
		-1,		  		// The string is NULL terminated
		NULL,				// No buffer yet, allocate it later
		0		  		// No buffer
		);
      if (res_len !=0 && ((buf1 = (wchar_t*)calloc(sizeof(wchar_t), res_len))) !=0 )
      {
         err = MultiByteToWideChar(
		CP_UTF8,			// Code page
		0,		  		// No flags
		string.c_str(),		// Multibyte characters string
		-1,		  		// The string is NULL terminated
		buf1,				// Output buffer
		res_len			// buffer size
		);
      }
      if (err != 0)
      {
         res_len = WideCharToMultiByte(
		1251,				// Code page
		0,				// Default replacement of illegal chars
		buf1,       		// Multibyte characters string
		-1,				// Number of unicode chars is not known
		NULL,				// No buffer yet, allocate it later
		0,				// No buffer
		NULL,				// Use system default
		NULL				// We are not interested whether the default char was used
		);
         if (res_len !=0 && (buf2 = (char*)calloc(sizeof(wchar_t), res_len)) != 0)
         {
      	err = WideCharToMultiByte(
      	   1251,			// Code page
	         0,				// Default replacement of illegal chars
		   buf1,          	// Multibyte characters string
               -1,			// Number of unicode chars is not known
               buf2,			// Output buffer
		   res_len,			// buffer size
		   NULL,			// Use system default
		   NULL			// We are not interested whether the default char was used
		   );
         }
         if (err != 0)
         {
            ansi = AnsiString(buf2);
         }

      }
      if (buf2) free(buf2);
      if (buf1) free(buf1);
      return ansi;
}
//---------------------------------------------------------------------------
// Поиск подстроки SubStr в строке string с/без учета регистра
// Возвращает указатель на первый найденный фрагмени или NULL если не найдено.
char* __fastcall AnsiStrPosCS(char *string, char *SubStr, bool CaseSensivity)
{
      bool find;
      char *FindBuf = NULL;
      int PatLen, strLen;
      AnsiString Pat1 = AnsiUpperCase(AnsiString(SubStr));
      AnsiString Pat2 = AnsiLowerCase(Pat1);
      char *sPat1 = Pat1.c_str();
      char *sPat2 = Pat2.c_str();
      PatLen = Pat1.Length();
      strLen = StrLen(string);
      while (strLen >= PatLen)
      {
         if (*string == *sPat1 || *string == *sPat2)
         {
            find = true;
            for (int i=1; i<PatLen; i++)
            {
               if (!(string[i] == sPat1[i] || string[i] == sPat2[i]))
                  { find = false; break; }
            }
            if (find)
               { FindBuf = string; break; }
         }
         string++; strLen--;
      }
      return FindBuf;
}
//---------------------------------------------------------------------------
// Поиск подстроки SubStr в строке string с/без учета регистра
// Возвращает порядковый номер символа, с которого начинается совпадение или 0 если не найдено.
int __fastcall AnsiStrPosCS(AnsiString string, char *SubStr, bool CaseSensivity)
{
      bool find;
      int FindIndex = 0;
      char *sstr = string.c_str();
      char *fstr = AnsiStrPosCS(sstr, SubStr, CaseSensivity);
      if (fstr) FindIndex = fstr - sstr + 1;
      return FindIndex;
}
//---------------------------------------------------------------------------
// Поиск подстроки SubStr в памяти начиная с адреса buffer длиной buflen с/без учета регистра
// Возвращает указатель на первый найденный фрагмени или NULL если не найдено.
char* __fastcall FindSubStringCS(char *buffer, char *SubStr, int buflen, bool CaseSensivity)
{
      bool find;
      char *FindBuf = NULL;
      int PatLen;
      AnsiString Pat1 = AnsiUpperCase(AnsiString(SubStr));
      AnsiString Pat2 = AnsiLowerCase(Pat1);
      char *sPat1 = Pat1.c_str();
      char *sPat2 = Pat2.c_str();
      PatLen = Pat1.Length();
      while (buflen >= PatLen)
      {
         if (*buffer == *sPat1 || *buffer == *sPat2)
         {
            find = true;
            for (int i=1; i<PatLen; i++)
            {
               if (!(buffer[i] == sPat1[i] || buffer[i] == sPat2[i]))
                  { find = false; break; }
            }
            if (find)
               { FindBuf = buffer; break; }
         }
         buffer++; buflen--;
      }
      return FindBuf;
}
//---------------------------------------------------------------------------
// Переводит закодированную строку (Byzak) в нормальный AnsiString и возвращает его
AnsiString _fastcall ByzakToStr(AnsiString Byzak)
{
      AnsiString Str;
      Byte ch;
      try
      {
         for (int i = Byzak.Length()-2; i>=0; i-=2)
         {
            ch = StrToInt("0x" + Byzak.SubString(i+1,2));
            ch ^= 0xBB;
            Str += (char)ch;
         }
      }
      catch (...) { Str = ""; }
      return Str;
}
//---------------------------------------------------------------------------
// Переводит закодированную строку (Byzak) в нормальный AnsiString и возвращает его
AnsiString _fastcall StrToByzak(AnsiString Str)
{
      AnsiString Byzak;
      Byte ch;
      for (int i = Str.Length(); i>0; i--)
      {
         ch = Str[i]^0xBB;
         Byzak += IntToHex(ch, 2);
      }

      return Byzak;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// К Л А С С Ы
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
                         // *** TSnifSocket *** //
TSnifSocket::TSnifSocket()
{
      Port = 0;
      Ready = false;
      Data = new TMemoryStream;
}
//---------------------------------------------------------------------------
TSnifSocket::~TSnifSocket()
{
      delete Data;
}
//---------------------------------------------------------------------------
void TSnifSocket::AddPacket(void)
{
//      StartTime = tmp.QuadPart;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
                         // *** TSysTimer *** //
TSysTimer::TSysTimer()
{
      LARGE_INTEGER tmp;
      QueryPerformanceFrequency(&tmp);
      ClockRate = StartTime = tmp.QuadPart;
}
//---------------------------------------------------------------------------
TSysTimer::~TSysTimer()
{
}
//---------------------------------------------------------------------------
void TSysTimer::StartTimer(void)
{
      LARGE_INTEGER tmp;
      QueryPerformanceCounter(&tmp);
      StartTime = tmp.QuadPart;
}
//---------------------------------------------------------------------------
Double TSysTimer::ReadTimer(void)
{
      Double Result;
      LARGE_INTEGER tmp;
      QueryPerformanceCounter(&tmp);
      Result = (tmp.QuadPart - StartTime)/ClockRate;
      return Result;
}
//---------------------------------------------------------------------------
void TSysTimer::Pause(Double time)
{
      LARGE_INTEGER tmp, currtmp;
      QueryPerformanceCounter(&currtmp);
      do
      {
         QueryPerformanceCounter(&tmp);
         Application->ProcessMessages();
      }
      while ((tmp.QuadPart - currtmp.QuadPart)/ClockRate < time);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
                         // *** TGlobalOptions *** //
TGlobalOptions::TGlobalOptions()
{
      ReadOptions();
      Buddies = new TStringList;
      Buddies->LoadFromFile(ExtractFilePath(Application->ExeName) + BuddyFileName);
}
//---------------------------------------------------------------------------
TGlobalOptions::~TGlobalOptions()
{
      delete Buddies;
      WriteOptions();
}
//---------------------------------------------------------------------------
void TGlobalOptions::ReadOptions(void)
{
      AnsiString Section;
      TIniFile *INI = new TIniFile(GetINIFileName());

// ReadSomeParams
      Section = "SOME";
      DefaultNetworkInterface = INI->ReadInteger(Section, "DefaultNetworkInterface", 0);
      SocketsTTL = INI->ReadInteger(Section, "SocketsTTL", 8);
      FilterByIP = INI->ReadBool(Section, "FilterByIP", true);
      FilterIP = INI->ReadString(Section, "FilterIP", "89.111.184.43");

      Section = "FILES";
      BuddyFileName = INI->ReadString(Section, "BuddyFileName", "buddy.txt");
      ImagesPath = INI->ReadString(Section, "ImagesPath", "buddy.txt");
      DebugHTML = INI->ReadBool(Section, "DebugHTML", false);
      DebugVB1 = INI->ReadBool(Section, "DebugVB1", false);
      DebugVB2 = INI->ReadBool(Section, "DebugVB2", false);

      delete INI;
}
//---------------------------------------------------------------------------
void TGlobalOptions::WriteOptions(void)
{
      AnsiString Section;
      TIniFile *INI = new TIniFile(GetINIFileName());

// WriteSomeParams
      Section = "SOME";
      INI->WriteInteger(Section, "DefaultNetworkInterface", DefaultNetworkInterface);

      delete INI;
}
//---------------------------------------------------------------------------

AnsiString __fastcall TGlobalOptions::GetINIFileName(void)
{
      AnsiString IniName = ExtractFilePath(Application->ExeName) + "WCmapConfig.ini";
      return IniName;
}
//---------------------------------------------------------------------------

