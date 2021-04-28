/***** File: unicode.hpp *****/


#ifndef _RAR_UNICODE_
#define _RAR_UNICODE_

#define MBFUNCTIONS

#if defined(MBFUNCTIONS )
#define UNICODE_SUPPORTED
#endif

void WideToChar(const wchar *Src, char *Dest, int DestSize = 0x10000000);
void CharToWide(const char *Src, wchar *Dest, int DestSize = 0x10000000);
byte *WideToRaw(const wchar *Src, byte *Dest, int DestSize = 0x10000000);
wchar *RawToWide(const byte *Src, wchar *Dest, int DestSize = 0x10000000);
bool LowAscii(const wchar *Str);
int strlenw(const wchar *str);
wchar *strcpyw(wchar *dest, const wchar *src);
wchar *strncpyw(wchar *dest, const wchar *src, int n);
wchar *strcatw(wchar *dest, const wchar *src);
wchar *strncatw(wchar *dest, const wchar *src, int n);
int strcmpw(const wchar *s1, const wchar *s2);
int strncmpw(const wchar *s1, const wchar *s2, int n);
int stricmpw(const wchar *s1, const wchar *s2);
int strnicmpw(const wchar *s1, const wchar *s2, int n);
wchar *strchrw(const wchar *s, int c);
wchar *strrchrw(const wchar *s, int c);
wchar *strpbrkw(const wchar *s1, const wchar *s2);
wchar *strlowerw(wchar *Str);
wchar *strupperw(wchar *Str);
int toupperw(int ch);
int atoiw(const wchar *s);

#ifdef DBCS_SUPPORTED
class SupportDBCS {
public:
	SupportDBCS();

	char *charnext(const char *s);
	char *strchrd(const char *s, int c);
	char *strrchrd(const char *s, int c);

	bool IsLeadByte[256];
	bool DBCSMode;
};

extern SupportDBCS gdbcs;

inline char *charnext(const char *s) {
	return (char *)(gdbcs.DBCSMode ? gdbcs.charnext(s) : s + 1);
}
inline char *strchrd(const char *s, int c) {
	return (char *)(gdbcs.DBCSMode ? gdbcs.strchrd(s, c) : strchr(s, c));
}
inline char *strrchrd(const char *s, int c) {
	return (char *)(gdbcs.DBCSMode ? gdbcs.strrchrd(s, c) : strrchr(s, c));
}

#else
#define charnext(s) ((s)+1)
#define strchrd strchr
#define strrchrd strrchr
#endif

#endif
