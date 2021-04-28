/***** File: rar.cpp *****/

#include "rar.hpp"

ErrorHandler ErrHandler;


int ToPercent(Int64 N1, Int64 N2) {
	if (N2 == 0)
		return (0);
	if (N2 < N1)
		return (100);
	return (int64to32(N1 * 100 / N2));
}

int main(int argc, char *argv[]) {
#ifndef SFX_MODULE
	setbuf(stdout, NULL);

#endif

	ErrHandler.SetSignalHandlers(true);

	RARInitData();

	bool ShutdownOnClose;

	{

		CommandData Cmd;
#ifdef SFX_MODULE
		strcpy(Cmd.Command, "X");
		char *Switch = NULL;
		Switch = argc > 1 ? argv[1] : NULL;
		if (Switch != NULL && Cmd.IsSwitch(Switch[0])) {
			int UpperCmd = toupper(Switch[1]);
			switch (UpperCmd) {
			case 'T':
			case 'V':
				Cmd.Command[0] = UpperCmd;
				break;
			case '?':
				Cmd.OutHelp();
				break;
			}
		}
		Cmd.AddArcName(ModuleName, NULL);
#else
		if (Cmd.IsConfigEnabled(argc, argv)) {
			Cmd.ReadConfig(argc, argv);
			Cmd.ParseEnvVar();
		}
		for (int I = 1; I < argc; I++)
			Cmd.ParseArg(argv[I]);
#endif
		Cmd.ParseDone();


		InitConsoleOptions(Cmd.MsgStream, Cmd.Sound);
		InitSystemOptions(Cmd.SleepTime);
		InitLogOptions(Cmd.LogName);
		ErrHandler.SetSilent(Cmd.AllYes || Cmd.MsgStream == MSG_NULL);
		ErrHandler.SetShutdown(Cmd.Shutdown);

		Cmd.OutTitle();
		Cmd.ProcessCommand();
	}
	File::RemoveCreated();
	return (ErrHandler.GetErrorCode());
}


void RARInitData() {
	InitCRC();
#ifndef SFX_MODULE
	InitTime();
#endif
	ErrHandler.Clean();
}

const char *St(MSGID StringId) {
	return (StringId);
}


/***** File: strlist.cpp *****/

StringList::StringList() {
	Reset();
}


StringList::~StringList() {
}


void StringList::Reset() {
	Rewind();
	StringData.Reset();
	StringDataW.Reset();
	PosDataW.Reset();
	StringsCount = 0;
	SavePosNumber = 0;
}


unsigned int StringList::AddString(const char *Str) {
	return (AddString(Str, NULL));
}


unsigned int StringList::AddString(const char *Str, const wchar *StrW) {
	int PrevSize = StringData.Size();
	StringData.Add(strlen(Str) + 1);
	strcpy(&StringData[PrevSize], Str);
	if (StrW != NULL && *StrW != 0) {
		int PrevPos = PosDataW.Size();
		PosDataW.Add(1);
		PosDataW[PrevPos] = PrevSize;

		int PrevSizeW = StringDataW.Size();
		StringDataW.Add(strlenw(StrW) + 1);
		strcpyw(&StringDataW[PrevSizeW], StrW);
	}
	StringsCount++;
	return (PrevSize);
}


bool StringList::GetString(char *Str, int MaxLength) {
	return (GetString(Str, NULL, MaxLength));
}


bool StringList::GetString(char *Str, wchar *StrW, int MaxLength) {
	char *StrPtr;
	wchar *StrPtrW;
	if (Str == NULL || !GetString(&StrPtr, &StrPtrW))
		return (false);
	strncpy(Str, StrPtr, MaxLength);
	if (StrW != NULL)
		strncpyw(StrW, NullToEmpty(StrPtrW), MaxLength);
	return (true);
}


#ifndef SFX_MODULE
bool StringList::GetString(char *Str, wchar *StrW, int MaxLength, int StringNum) {
	SavePosition();
	Rewind();
	bool RetCode = true;
	while (StringNum-- >= 0)
		if (!GetString(Str, StrW, MaxLength)) {
			RetCode = false;
			break;
		}
	RestorePosition();
	return (RetCode);
}
#endif


char *StringList::GetString() {
	char *Str;
	GetString(&Str, NULL);
	return (Str);
}



bool StringList::GetString(char **Str, wchar **StrW) {
	if (CurPos >= StringData.Size()) {
		*Str = NULL;
		return (false);
	}
	*Str = &StringData[CurPos];
	if (PosDataItem < PosDataW.Size() && PosDataW[PosDataItem] == CurPos) {
		PosDataItem++;
		if (StrW != NULL)
			*StrW = &StringDataW[CurPosW];
		CurPosW += strlenw(&StringDataW[CurPosW]) + 1;
	} else if (StrW != NULL)
		*StrW = NULL;
	CurPos += strlen(*Str) + 1;
	return (true);
}


char *StringList::GetString(unsigned int StringPos) {
	if (StringPos >= StringData.Size())
		return (NULL);
	return (&StringData[StringPos]);
}


void StringList::Rewind() {
	CurPos = 0;
	CurPosW = 0;
	PosDataItem = 0;
}


int StringList::GetBufferSize() {
	return (StringData.Size() + StringDataW.Size());
}


#ifndef SFX_MODULE
bool StringList::Search(char *Str, wchar *StrW, bool CaseSensitive) {
	SavePosition();
	Rewind();
	bool Found = false;
	char *CurStr;
	wchar *CurStrW;
	while (GetString(&CurStr, &CurStrW)) {
		if ((CaseSensitive ? strcmp(Str, CurStr) : stricomp(Str, CurStr)) != 0)
			continue;
		if (StrW != NULL && CurStrW != NULL)
			if ((CaseSensitive ? strcmpw(StrW, CurStrW) : stricmpw(StrW, CurStrW)) != 0)
				continue;
		Found = true;
		break;
	}
	RestorePosition();
	return (Found);
}
#endif


#ifndef SFX_MODULE
void StringList::SavePosition() {
	if (SavePosNumber < sizeof(SaveCurPos) / sizeof(SaveCurPos[0])) {
		SaveCurPos[SavePosNumber] = CurPos;
		SaveCurPosW[SavePosNumber] = CurPosW;
		SavePosDataItem[SavePosNumber] = PosDataItem;
		SavePosNumber++;
	}
}
#endif


#ifndef SFX_MODULE
void StringList::RestorePosition() {
	if (SavePosNumber > 0) {
		SavePosNumber--;
		CurPos = SaveCurPos[SavePosNumber];
		CurPosW = SaveCurPosW[SavePosNumber];
		PosDataItem = SavePosDataItem[SavePosNumber];
	}
}
#endif


/***** File: strfn.cpp *****/

const char *NullToEmpty(const char *Str) {
	return (Str == NULL ? "" : Str);
}


const wchar *NullToEmpty(const wchar *Str) {
	return (Str == NULL ? L"" : Str);
}


char *IntNameToExt(const char *Name) {
	static char OutName[NM];
	IntToExt(Name, OutName);
	return (OutName);
}


void ExtToInt(const char *Src, char *Dest) {
	if (Dest != Src)
		strcpy(Dest, Src);
}


void IntToExt(const char *Src, char *Dest) {
	if (Dest != Src)
		strcpy(Dest, Src);
}


char *strlower(char *Str) {
	for (char *ChPtr = Str; *ChPtr; ChPtr++)
		*ChPtr = (char)loctolower(*ChPtr);
	return (Str);
}


char *strupper(char *Str) {
	for (char *ChPtr = Str; *ChPtr; ChPtr++)
		*ChPtr = (char)loctoupper(*ChPtr);
	return (Str);
}


int stricomp(const char *Str1, const char *Str2) {
	char S1[NM * 2], S2[NM * 2];
	strncpy(S1, Str1, sizeof(S1));
	strncpy(S2, Str2, sizeof(S2));
	return (strcmp(strupper(S1), strupper(S2)));
}


int strnicomp(const char *Str1, const char *Str2, int N) {
	char S1[512], S2[512];
	strncpy(S1, Str1, sizeof(S1));
	strncpy(S2, Str2, sizeof(S2));
	return (strncmp(strupper(S1), strupper(S2), N));
}


char *RemoveEOL(char *Str) {
	for (int I = strlen(Str) - 1; I >= 0 && (Str[I] == '\r' || Str[I] == '\n' || Str[I] == ' ' || Str[I] == '\t'); I--)
		Str[I] = 0;
	return (Str);
}


char *RemoveLF(char *Str) {
	for (int I = strlen(Str) - 1; I >= 0 && (Str[I] == '\r' || Str[I] == '\n'); I--)
		Str[I] = 0;
	return (Str);
}


unsigned int loctolower(unsigned int ch) {
	return (tolower(ch));
}


unsigned int loctoupper(unsigned int ch) {
	return (toupper(ch));
}


/***** File: pathfn.cpp *****/

char *PointToName(const char *Path) {
	const char *Found = NULL;
	for (const char *s = Path; *s != 0; s = charnext(s))
		if (IsPathDiv(*s))
			Found = (char *)(s + 1);
	if (Found != NULL)
		return ((char *)Found);
	return (char *)((*Path && IsDriveDiv(Path[1]) && charnext(Path) == Path + 1) ? Path + 2 : Path);
}


wchar *PointToName(const wchar *Path) {
	for (int I = strlenw(Path) - 1; I >= 0; I--)
		if (IsPathDiv(Path[I]))
			return (wchar *)&Path[I + 1];
	return (wchar *)((*Path && IsDriveDiv(Path[1])) ? Path + 2 : Path);
}


char *PointToLastChar(const char *Path) {
	for (const char *s = Path, *p = Path;; p = s, s = charnext(s))
		if (*s == 0)
			return ((char *)p);
}


char *ConvertPath(const char *SrcPath, char *DestPath) {
	const char *DestPtr = SrcPath;
	for (const char *s = DestPtr; *s != 0; s++)
		if (IsPathDiv(s[0]) && s[1] == '.' && s[2] == '.' && IsPathDiv(s[3]))
			DestPtr = s + 4;
	while (*DestPtr) {
		const char *s = DestPtr;
		if (s[0] && IsDriveDiv(s[1]))
			s += 2;
		if (s[0] == '\\' && s[1] == '\\') {
			const char *Slash = strchr(s + 2, '\\');
			if (Slash != NULL && (Slash = strchr(Slash + 1, '\\')) != NULL)
				s = Slash + 1;
		}
		for (const char *t = s; *t != 0; t++)
			if (IsPathDiv(*t))
				s = t + 1;
			else if (*t != '.')
				break;
		if (s == DestPtr)
			break;
		DestPtr = s;
	}
	if (DestPath != NULL) {
		char TmpStr[NM];
		strncpy(TmpStr, DestPtr, sizeof(TmpStr) - 1);
		strcpy(DestPath, TmpStr);
	}
	return ((char *)DestPtr);
}


wchar *ConvertPath(const wchar *SrcPath, wchar *DestPath) {
	const wchar *DestPtr = *SrcPath && IsDriveDiv(SrcPath[1]) ? SrcPath + 2 : SrcPath;
	if (SrcPath[0] == '\\' && SrcPath[1] == '\\') {
		const wchar *ChPtr = strchrw(SrcPath + 2, '\\');
		if (ChPtr != NULL && (ChPtr = strchrw(ChPtr + 1, '\\')) != NULL)
			DestPtr = ChPtr + 1;
	}
	for (const wchar *ChPtr = DestPtr; *ChPtr != 0; ChPtr++)
		if (IsPathDiv(ChPtr[0]) && ChPtr[1] == '.' && ChPtr[2] == '.' && IsPathDiv(ChPtr[3]))
			DestPtr = ChPtr + 4;
	for (const wchar *ChPtr = DestPtr; *ChPtr != 0; ChPtr++)
		if (*ChPtr == CPATHDIVIDER)
			DestPtr = ChPtr + 1;
		else if (*ChPtr != '.')
			break;
	if (DestPath != NULL) {
		wchar TmpStr[NM];
		strncpyw(TmpStr, DestPtr, sizeof(TmpStr) - 1);
		strcpyw(DestPath, TmpStr);
	}
	return ((wchar *)DestPtr);
}


void SetExt(char *Name, const char *NewExt) {
	char *Dot = GetExt(Name);
	if (NewExt == NULL) {
		if (Dot != NULL)
			*Dot = 0;
	} else if (Dot == NULL) {
		strcat(Name, ".");
		strcat(Name, NewExt);
	} else
		strcpy(Dot + 1, NewExt);
}


#ifndef SFX_MODULE
void SetExt(wchar *Name, const wchar *NewExt) {
	if (Name == NULL || *Name == 0)
		return;
	wchar *Dot = GetExt(Name);
	if (NewExt == NULL) {
		if (Dot != NULL)
			*Dot = 0;
	} else if (Dot == NULL) {
		strcatw(Name, L".");
		strcatw(Name, NewExt);
	} else
		strcpyw(Dot + 1, NewExt);
}
#endif


#ifndef SFX_MODULE
void SetSFXExt(char *SFXName) {
	SetExt(SFXName, "sfx");
}
#endif


char *GetExt(const char *Name) {
	return (strrchrd(PointToName(Name), '.'));
}


wchar *GetExt(const wchar *Name) {
	return (Name == NULL ? (wchar *)L"" : strrchrw(PointToName(Name), '.'));
}


bool CmpExt(const char *Name, const char *Ext) {
	char *NameExt = GetExt(Name);
	return (NameExt != NULL && stricomp(NameExt + 1, Ext) == 0);
}


bool IsWildcard(const char *Str, const wchar *StrW) {
	if (StrW != NULL && *StrW != 0)
		return (strpbrkw(StrW, L"*?") != NULL);
	return (Str == NULL ? false : strpbrk(Str, "*?") != NULL);
}


bool IsPathDiv(int Ch) {
	return (Ch == CPATHDIVIDER);
}


bool IsDriveDiv(int Ch) {
	return (false);
}


int GetPathDisk(const char *Path) {
	if (IsDiskLetter(Path))
		return (toupper(*Path) - 'A');
	else
		return (-1);
}


void AddEndSlash(char *Path) {
	char *LastChar = PointToLastChar(Path);
	if (*LastChar != 0 && *LastChar != CPATHDIVIDER)
		strcat(LastChar, PATHDIVIDER);
}


void AddEndSlash(wchar *Path) {
	int Length = strlenw(Path);
	if (Length > 0 && Path[Length - 1] != CPATHDIVIDER)
		strcatw(Path, PATHDIVIDERW);
}


void GetFilePath(const char *FullName, char *Path) {
	int PathLength = PointToName(FullName) - FullName;
	strncpy(Path, FullName, PathLength);
	Path[PathLength] = 0;
}


void GetFilePath(const wchar *FullName, wchar *Path) {
	const wchar *PathPtr =/*(*FullName && IsDriveDiv(FullName[1])) ? FullName+2:*/FullName;
	int PathLength = PointToName(FullName) - FullName;
	strncpyw(Path, PathPtr, PathLength);
	Path[PathLength] = 0;
}


void RemoveNameFromPath(char *Path) {
	char *Name = PointToName(Path);
	if (Name >= Path + 2 && (!IsDriveDiv(Path[1]) || Name >= Path + 4))
		Name--;
	*Name = 0;
}


#ifndef SFX_MODULE
void RemoveNameFromPath(wchar *Path) {
	wchar *Name = PointToName(Path);
	if (Name >= Path + 2 && (!IsDriveDiv(Path[1]) || Name >= Path + 4))
		Name--;
	*Name = 0;
}
#endif


#ifndef SFX_MODULE
void GetConfigName(const char *Name, char *FullName) {
	char *EnvStr = getenv("HOME");
	if (EnvStr != NULL) {
		strcpy(FullName, EnvStr);
		AddEndSlash(FullName);
	}
	strcat(FullName, Name);
	if (!WildFileExist(FullName)) {
		char HomeName[NM];
		strcpy(HomeName, FullName);
		static char *AltPath[] = {
			"/etc/", "/usr/lib/", "/usr/local/lib/", "/usr/local/etc/"
		};
		for (int I = 0; I < sizeof(AltPath) / sizeof(AltPath[0]); I++) {
			strcpy(FullName, AltPath[I]);
			strcat(FullName, Name);
			if (WildFileExist(FullName))
				break;
			strcpy(FullName, HomeName);
		}
	}
}
#endif


void NextVolumeName(char *ArcName, bool OldNumbering) {
	char *ChPtr;
	if ((ChPtr = GetExt(ArcName)) == NULL) {
		strcat(ArcName, ".rar");
		ChPtr = GetExt(ArcName);
	} else if (ChPtr[1] == 0 || stricomp(ChPtr + 1, "exe") == 0 || stricomp(ChPtr + 1, "sfx") == 0)
		strcpy(ChPtr + 1, "rar");
	if (!OldNumbering) {
		while (!isdigit(*ChPtr) && ChPtr > ArcName)
			ChPtr--;
		while ((++(*ChPtr)) == '9' + 1) {
			*ChPtr = '0';
			ChPtr--;
			if (ChPtr < ArcName || !isdigit(*ChPtr)) {
				for (char *EndPtr = ArcName + strlen(ArcName); EndPtr != ChPtr; EndPtr--)
					*(EndPtr + 1) = *EndPtr;
				*(ChPtr + 1) = '1';
				break;
			}
		}
	} else if (!isdigit(*(ChPtr + 2)) || !isdigit(*(ChPtr + 3)))
		strcpy(ChPtr + 2, "00");
	else {
		ChPtr += 3;
		while ((++(*ChPtr)) == '9' + 1)
			if (*(ChPtr - 1) == '.') {
				*ChPtr = 'A';
				break;
			} else {
				*ChPtr = '0';
				ChPtr--;
			}
	}
}

/*
bool PrevVolumeName(char *ArcName,bool OldNumbering)
{
  char *ChPtr;
  if ((ChPtr=GetExt(ArcName))==NULL)
    return(false);
  if (OldNumbering && (stricomp(ChPtr,".rar")==0 || stricomp(ChPtr,".exe")==0))
    return(false);
  if (!OldNumbering)
  {
    while (!isdigit(*ChPtr) && ChPtr>ArcName)
      ChPtr--;
    while ((--(*ChPtr))=='0'-1)
    {
      *ChPtr='9';
      ChPtr--;
    }
  }
  else
    if (strcmp(ChPtr+2,"00")==0)
      strcpy(ChPtr+2,"rar");
    else
    {
      ChPtr+=3;
      while ((--(*ChPtr))=='0'-1)
      {
        *ChPtr='9';
        ChPtr--;
      }
    }
}
*/

bool IsNameUsable(const char *Name) {
	if (Name[0] && Name[1] && strchr(Name + 2, ':') != NULL)
		return (false);
	return (*Name != 0 && strpbrk(Name, "?*<>|") == NULL);
}


void MakeNameUsable(char *Name, bool Extended) {
	for (int I = 0; Name[I] != 0; I++) {
		if (strchr(Extended ? "?*<>|\"" : "?*", Name[I]) != NULL ||
		        Extended && Name[I] < 32)
			Name[I] = '_';
	}
}


char *DosSlashToUnix(char *SrcName, char *DestName) {
	if (DestName != NULL && DestName != SrcName)
		strcpy(DestName, SrcName);
	for (char *s = SrcName; *s != 0; s = charnext(s)) {
		if (*s == '\\')
			if (DestName == NULL)
				*s = '/';
			else
				DestName[s - SrcName] = '/';
	}
	return (DestName == NULL ? SrcName : DestName);
}


bool IsFullPath(const char *Path) {
	char PathOnly[NM];
	GetFilePath(Path, PathOnly);
	if (IsWildcard(PathOnly))
		return (true);
	return (IsPathDiv(Path[0]));
}


bool IsDiskLetter(const char *Path) {
	char Letter = toupper(Path[0]);
	return (Letter >= 'A' && Letter <= 'Z' && IsDriveDiv(Path[1]));
}


void GetPathRoot(const char *Path, char *Root) {
	*Root = 0;
	if (IsDiskLetter(Path))
		sprintf(Root, "%c:\\", *Path);
	else if (Path[0] == '\\' && Path[1] == '\\') {
		const char *Slash = strchr(Path + 2, '\\');
		if (Slash != NULL) {
			int Length;
			if ((Slash = strchr(Slash + 1, '\\')) != NULL)
				Length = Slash - Path + 1;
			else
				Length = strlen(Path);
			strncpy(Root, Path, Length);
			Root[Length] = 0;
		}
	}
}


int ParseVersionFileName(char *Name, wchar *NameW, bool Truncate) {
	int Version = 0;
	char *VerText = strrchrd(Name, ';');
	if (VerText != NULL) {
		Version = atoi(VerText + 1);
		if (Truncate)
			*VerText = 0;
	}
	if (NameW != NULL) {
		wchar *VerTextW = strrchrw(NameW, ';');
		if (VerTextW != NULL) {
			if (Version == 0)
				Version = atoiw(VerTextW + 1);
			if (Truncate)
				*VerTextW = 0;
		}
	}
	return (Version);
}


char *VolNameToFirstName(const char *VolName, char *FirstName, bool NewNumbering) {
	if (FirstName != VolName)
		strcpy(FirstName, VolName);
	char *VolNumStart = FirstName;
	if (NewNumbering) {
		int N = '1';
		for (char *ChPtr = FirstName + strlen(FirstName) - 1; ChPtr > FirstName; ChPtr--)
			if (isdigit(*ChPtr)) {
				*ChPtr = N;
				N = '0';
			} else if (N == '0') {
				VolNumStart = ChPtr + 1;
				break;
			}
	} else {
		SetExt(FirstName, "rar");
		VolNumStart = GetExt(FirstName);
	}
	if (!FileExist(FirstName)) {
		char Mask[NM];
		strcpy(Mask, FirstName);
		SetExt(Mask, "*");
		FindFile Find;
		Find.SetMask(Mask);
		struct FindData FD;
		while (Find.Next(&FD)) {
			Archive Arc;
			if (Arc.Open(FD.Name, FD.NameW) && Arc.IsArchive(true) && !Arc.NotFirstVolume) {
				strcpy(FirstName, FD.Name);
				break;
			}
		}
	}
	return (VolNumStart);
}


/***** File: int64.cpp *****/

void itoa(Int64 n, char *Str) {
	if (n <= 0xffffffff) {
		sprintf(Str, "%u", int64to32(n));
		return;
	}

	char NumStr[50];
	int Pos = 0;

	do {
		NumStr[Pos++] = int64to32(n % 10) + '0';
		n = n / 10;
	} while (n != 0);

	for (int I = 0; I < Pos; I++)
		Str[I] = NumStr[Pos - I - 1];
	Str[Pos] = 0;
}


Int64 atoil(char *Str) {
	Int64 n = 0;
	while (*Str >= '0' && *Str <= '9') {
		n = n * 10 + *Str - '0';
		Str++;
	}
	return (n);
}


/***** File: savepos.cpp *****/

SaveFilePos::SaveFilePos(File &SaveFile) {
	SaveFilePos::SaveFile = &SaveFile;
	SavePos = SaveFile.Tell();
}


SaveFilePos::~SaveFilePos() {
	SaveFile->Seek(SavePos, SEEK_SET);
}


/***** File: file.cpp *****/

static File *CreatedFiles[16];

File::File() {
	hFile = BAD_HANDLE;
	*FileName = 0;
	*FileNameW = 0;
	NewFile = false;
	LastWrite = false;
	HandleType = FILE_HANDLENORMAL;
	SkipClose = false;
	IgnoreReadErrors = false;
	ErrorType = FILE_SUCCESS;
	OpenShared = false;
	AllowDelete = true;
}


File::~File() {
	if (hFile != BAD_HANDLE && !SkipClose)
		if (NewFile)
			Delete();
		else
			Close();
}


void File::operator = (File &SrcFile) {
	hFile = SrcFile.hFile;
	strcpy(FileName, SrcFile.FileName);
	NewFile = SrcFile.NewFile;
	LastWrite = SrcFile.LastWrite;
	HandleType = SrcFile.HandleType;
	SrcFile.SkipClose = true;
}


bool File::Open(const char *Name, const wchar *NameW, bool OpenShared, bool Update) {
	ErrorType = FILE_SUCCESS;
	FileHandle hNewFile;
	int flags = Update ? O_RDWR : O_RDONLY;
#ifdef O_BINARY
	flags |= O_BINARY;
#endif
	int handle = open(Name, flags);
	hNewFile = handle == -1 ? BAD_HANDLE : fdopen(handle, Update ? UPDATEBINARY : READBINARY);
	if (hNewFile == BAD_HANDLE && errno == ENOENT)
		ErrorType = FILE_NOTFOUND;

	NewFile = false;
	HandleType = FILE_HANDLENORMAL;
	SkipClose = false;
	bool Success = hNewFile != BAD_HANDLE;
	if (Success) {
		hFile = hNewFile;
		if (NameW != NULL)
			strcpyw(FileNameW, NameW);
		strcpy(FileName, Name);
	}
	return (Success);
}


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void File::TOpen(const char *Name, const wchar *NameW) {
	if (!WOpen(Name, NameW))
		ErrHandler.Exit(OPEN_ERROR);
}
#endif


bool File::WOpen(const char *Name, const wchar *NameW) {
	if (Open(Name, NameW))
		return (true);
	ErrHandler.OpenErrorMsg(Name);
	return (false);
}


bool File::Create(const char *Name, const wchar *NameW) {
	hFile = fopen(Name, CREATEBINARY);

	NewFile = true;
	HandleType = FILE_HANDLENORMAL;
	SkipClose = false;
	if (NameW != NULL)
		strcpyw(FileNameW, NameW);
	strcpy(FileName, Name);
	if (hFile != BAD_HANDLE)
		for (int I = 0; I < sizeof(CreatedFiles) / sizeof(CreatedFiles[0]); I++)
			if (CreatedFiles[I] == NULL) {
				CreatedFiles[I] = this;
				break;
			}
	return (hFile != BAD_HANDLE);
}


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void File::TCreate(const char *Name, const wchar *NameW) {
	if (!WCreate(Name, NameW))
		ErrHandler.Exit(FATAL_ERROR);
}
#endif


bool File::WCreate(const char *Name, const wchar *NameW) {
	if (Create(Name, NameW))
		return (true);
	ErrHandler.CreateErrorMsg(Name);
	return (false);
}


bool File::Close() {
	bool Success = true;
	if (HandleType != FILE_HANDLENORMAL)
		HandleType = FILE_HANDLENORMAL;
	else if (hFile != BAD_HANDLE) {
		if (!SkipClose) {
			Success = fclose(hFile) != EOF;
			for (int I = 0; I < sizeof(CreatedFiles) / sizeof(CreatedFiles[0]); I++)
				if (CreatedFiles[I] == this) {
					CreatedFiles[I] = NULL;
					break;
				}
		}
		hFile = BAD_HANDLE;
		if (!Success)
			ErrHandler.CloseError(FileName);
	}
	return (Success);
}


void File::Flush() {
	fflush(hFile);
}


bool File::Delete() {
	if (HandleType != FILE_HANDLENORMAL || !AllowDelete)
		return (false);
	if (hFile != BAD_HANDLE)
		Close();
	bool RetCode;
	RetCode = (remove(FileName) == 0);
	return (RetCode);
}


bool File::Rename(const char *NewName) {
	bool Success = strcmp(FileName, NewName) == 0 || rename(FileName, NewName) == 0;
	if (Success) {
		strcpy(FileName, NewName);
		*FileNameW = 0;
	}
	return (Success);
}


void File::Write(const void *Data, int Size) {
	if (Size == 0)
		return;
	if (HandleType != FILE_HANDLENORMAL)
		switch (HandleType) {
		case FILE_HANDLESTD:
			hFile = stdout;
			break;
		case FILE_HANDLEERR:
			hFile = stderr;
			break;
		}
	while (1) {
		bool Success;
		Success = (fwrite(Data, 1, Size, hFile) == Size && !ferror(hFile));
		if (!Success && HandleType == FILE_HANDLENORMAL) {
			if (ErrHandler.AskRepeatWrite(FileName))
				continue;
			ErrHandler.WriteError(FileName);
		}
		break;
	}
	LastWrite = true;
}


int File::Read(void *Data, int Size) {
	Int64 FilePos;
	if (IgnoreReadErrors)
		FilePos = Tell();
	int ReadSize;
	while (true) {
		ReadSize = DirectRead(Data, Size);
		if (ReadSize == -1)
			if (IgnoreReadErrors) {
				ReadSize = 0;
				for (int I = 0; I < Size; I += 512) {
					Seek(FilePos + I, SEEK_SET);
					int SizeToRead = Min(Size - I, 512);
					int ReadCode = DirectRead(Data, SizeToRead);
					ReadSize += (ReadCode == -1) ? 512 : ReadCode;
				}
			} else {
				if (HandleType == FILE_HANDLENORMAL && ErrHandler.AskRepeatRead(FileName))
					continue;
				ErrHandler.ReadError(FileName);
			}
		break;
	}
	return (ReadSize);
}


int File::DirectRead(void *Data, int Size) {
	if (HandleType == FILE_HANDLESTD) {
		hFile = stdin;
	}
	if (LastWrite) {
		fflush(hFile);
		LastWrite = false;
	}
	clearerr(hFile);
	int ReadSize = fread(Data, 1, Size, hFile);
	if (ferror(hFile))
		return (-1);
	return (ReadSize);
}


void File::Seek(Int64 Offset, int Method) {
	if (!RawSeek(Offset, Method))
		ErrHandler.SeekError(FileName);
}


bool File::RawSeek(Int64 Offset, int Method) {
	if (hFile == BAD_HANDLE)
		return (true);
	LastWrite = false;
	if (fseek(hFile, int64to32(Offset), Method) != 0)
		return (false);
	return (true);
}


Int64 File::Tell() {
	return (ftell(hFile));
}


void File::Prealloc(Int64 Size) {
}


byte File::GetByte() {
	byte Byte = 0;
	Read(&Byte, 1);
	return (Byte);
}


void File::PutByte(byte Byte) {
	Write(&Byte, 1);
}


bool File::Truncate() {
	return (false);
}


void File::SetOpenFileTime(uint ft) {
}


void File::SetCloseFileTime(uint ft) {
	struct utimbuf ut;
	ut.actime = ut.modtime = DosTimeToUnix(ft);
	utime(FileName, &ut);
}


uint File::GetOpenFileTime() {
	struct stat st;
	fstat(fileno(hFile), &st);
	return (UnixTimeToDos(st.st_mtime));
}


void File::SetOpenFileStat(uint FileTime) {
}


void File::SetCloseFileStat(uint FileTime, uint FileAttr) {
	SetCloseFileTime(FileTime);
	chmod(FileName, (mode_t)FileAttr);
}


Int64 File::FileLength() {
	SaveFilePos SavePos(*this);
	Seek(0, SEEK_END);
	return (Tell());
}


void File::SetHandleType(FILE_HANDLETYPE Type) {
	HandleType = Type;
}


bool File::IsDevice() {
	if (hFile == BAD_HANDLE)
		return (false);
	return (isatty(fileno(hFile)));
}


#ifndef SFX_MODULE
void File::fprintf(const char *fmt, ...) {
	va_list argptr;
	va_start(argptr, fmt);
	char Msg[8192], OutMsg[8192];
	vsprintf(Msg, fmt, argptr);
	strcpy(OutMsg, Msg);
	Write(OutMsg, strlen(OutMsg));
	va_end(argptr);
}
#endif


void File::RemoveCreated() {
	for (int I = 0; I < sizeof(CreatedFiles) / sizeof(CreatedFiles[0]); I++)
		if (CreatedFiles[I] != NULL)
			CreatedFiles[I]->Delete();
}


#ifndef SFX_MODULE
long File::Copy(File &Dest, Int64 Length) {
	Array<char> Buffer(0x10000);
	long CopySize = 0;
	bool CopyAll = (Length == INT64ERR);

	while (CopyAll || Length > 0) {
		Wait();
		int SizeToRead = (!CopyAll && Length < Buffer.Size()) ? int64to32(Length) : Buffer.Size();
		int ReadSize = Read(&Buffer[0], SizeToRead);
		if (ReadSize == 0)
			break;
		Dest.Write(&Buffer[0], ReadSize);
		CopySize += ReadSize;
		if (!CopyAll)
			Length -= ReadSize;
	}
	return (CopySize);
}
#endif


/***** File: filefn.cpp *****/

MKDIR_CODE MakeDir(const char *Name, const wchar *NameW, uint Attr) {
	int prevmask = umask(0);
	int ErrCode = mkdir(Name, (mode_t)Attr);
	umask(prevmask);
	if (ErrCode == -1)
		return (errno == ENOENT ? MKDIR_BADPATH : MKDIR_ERROR);
	return (MKDIR_SUCCESS);
}


void CreatePath(const char *Path, const wchar *PathW, bool SkipLastName) {
	uint DirAttr = 0777;
	int PosW = 0;
	for (const char *s = Path; *s != 0 && PosW < NM; s = charnext(s), PosW++) {
		bool Wide = PathW != NULL && *PathW != 0;
		if (Wide && PathW[PosW] == CPATHDIVIDER || !Wide && *s == CPATHDIVIDER) {
			wchar *DirPtrW = NULL;
			if (Wide) {
				wchar DirNameW[NM];
				strncpyw(DirNameW, PathW, PosW);
				DirNameW[PosW] = 0;
				DirPtrW = DirNameW;
			}
			char DirName[NM];
			strncpy(DirName, Path, s - Path);
			DirName[s - Path] = 0;
			if (MakeDir(DirName, DirPtrW, DirAttr) == MKDIR_SUCCESS) {
#ifndef GUI
				mprintf(St(MCreatDir), DirName);
				mprintf(" %s", St(MOk));
#endif
			}
		}
	}
	if (!SkipLastName)
		MakeDir(Path, PathW, DirAttr);
}


void SetDirTime(const char *Name, uint ft) {
	struct utimbuf ut;
	ut.actime = ut.modtime = DosTimeToUnix(ft);
	utime(Name, &ut);
}


bool IsRemovable(const char *FileName) {
	return (false);
}


#ifndef SFX_MODULE
Int64 GetFreeDisk(const char *FileName) {
	return (1457664);
}
#endif


bool FileExist(const char *FileName, const wchar *FileNameW) {
	struct FindData FD;
	return (FindFile::FastFind(FileName, FileNameW, &FD));
}


bool WildFileExist(const char *FileName, const wchar *FileNameW) {
	if (IsWildcard(FileName, FileNameW)) {
		FindFile Find;
		Find.SetMask(FileName);
		Find.SetMaskW(FileNameW);
		struct FindData fd;
		return (Find.Next(&fd));
	}
	return (FileExist(FileName, FileNameW));
}


bool IsDir(uint Attr) {
	return ((Attr & 0xF000) == 0x4000);
}


bool IsUnreadable(uint Attr) {
	return (S_ISFIFO(Attr) || S_ISSOCK(Attr) || S_ISCHR(Attr));
}


bool IsLabel(uint Attr) {
	return (false);
}


bool IsLink(uint Attr) {
	return ((Attr & 0xF000) == 0xA000);
}






bool IsDeleteAllowed(uint FileAttr) {
	return (false);
}


void PrepareToDelete(const char *Name, const wchar *NameW) {
	chmod(Name, S_IRUSR | S_IWUSR);
}


uint GetFileAttr(const char *Name, const wchar *NameW) {
	struct stat st;
	if (stat(Name, &st) != 0)
		return (0);
	return (st.st_mode);
}


bool SetFileAttr(const char *Name, const wchar *NameW, uint Attr) {
	bool Success;
	Success = chmod(Name, (mode_t)Attr) == 0;
	return (Success);
}


void ConvertNameToFull(const char *Src, char *Dest) {
	char FullName[NM];
	if (IsPathDiv(*Src) || *Src != 0 && IsDriveDiv(Src[1]))
		strcpy(FullName, Src);
	else {
		getcwd(FullName, sizeof(FullName));
		AddEndSlash(FullName);
		strcat(FullName, Src);
	}
	strcpy(Dest, FullName);
}


#ifndef SFX_MODULE
void ConvertNameToFull(const wchar *Src, wchar *Dest) {
	if (Src == NULL || *Src == 0) {
		*Dest = 0;
		return;
	}
	char AnsiName[NM];
	WideToChar(Src, AnsiName);
	ConvertNameToFull(AnsiName, AnsiName);
	CharToWide(AnsiName, Dest);
}
#endif


#ifndef SFX_MODULE
char *MkTemp(char *Name) {
	int Length = strlen(Name);
	if (Length <= 6)
		return (NULL);
	for (int Random = clock(), Attempt = 0;; Attempt++) {
		sprintf(Name + Length - 6, "%06u", Random + Attempt);
		Name[Length - 4] = '.';
		if (!FileExist(Name))
			break;
		if (Attempt == 1000)
			return (NULL);
	}
	return (Name);
}
#endif


#ifndef SFX_MODULE
uint CalcFileCRC(File *SrcFile, Int64 Size) {
	SaveFilePos SavePos(*SrcFile);
	const int BufSize = 0x10000;
	Array<byte> Data(BufSize);
	int ReadSize, BlockCount = 0;
	uint DataCRC = 0xffffffff;
	SrcFile->Seek(0, SEEK_SET);
	while ((ReadSize = SrcFile->Read(&Data[0], Size == INT64ERR ? BufSize : Min(BufSize, Size))) != 0) {
		if ((++BlockCount & 15) == 0)
			Wait();
		DataCRC = CRC(DataCRC, &Data[0], ReadSize);
		if (Size != INT64ERR)
			Size -= ReadSize;
	}
	return (DataCRC ^ 0xffffffff);
}
#endif


/***** File: filcreat.cpp *****/

bool FileCreate(RAROptions *Cmd, File *NewFile, char *Name, wchar *NameW,
                OVERWRITE_MODE Mode, bool *UserReject, Int64 FileSize,
                uint FileTime) {
	if (UserReject != NULL)
		*UserReject = false;
	while (FileExist(Name, NameW)) {
		if (Mode == OVERWRITE_NONE) {
			if (UserReject != NULL)
				*UserReject = true;
			return (false);
		}
#ifdef SILENT
		Mode = OVERWRITE_ALL;
#endif
		if (Cmd->AllYes || Mode == OVERWRITE_ALL)
			break;
		if (Mode == OVERWRITE_ASK) {
			eprintf(St(MFileExists), Name);
			int Choice = Ask(St(MYesNoAllRenQ));
			if (Choice == 1)
				break;
			if (Choice == 2) {
				if (UserReject != NULL)
					*UserReject = true;
				return (false);
			}
			if (Choice == 3) {
				Cmd->Overwrite = OVERWRITE_ALL;
				break;
			}
			if (Choice == 4) {
				if (UserReject != NULL)
					*UserReject = true;
				Cmd->Overwrite = OVERWRITE_NONE;
				return (false);
			}
			if (Choice == 5) {
				mprintf(St(MAskNewName));

				char NewName[NM];
				fgets(NewName, sizeof(NewName), stdin);
				RemoveLF(NewName);
				if (PointToName(NewName) == NewName)
					strcpy(PointToName(Name), NewName);
				else
					strcpy(Name, NewName);
				if (NameW != NULL)
					*NameW = 0;
				continue;
			}
			if (Choice == 6)
				ErrHandler.Exit(USER_BREAK);
		}
	}
	if (NewFile->Create(Name, NameW))
		return (true);
	PrepareToDelete(Name, NameW);
	CreatePath(Name, NameW, true);
	return (NewFile->Create(Name, NameW));
}


/***** File: archive.cpp *****/

Archive::Archive(RAROptions *InitCmd)
#ifndef SHELL_EXT
	: SubDataIO(NULL)
#endif
{
	Cmd = InitCmd == NULL ? &DummyCmd : InitCmd;
	SetOpenShared(Cmd->OpenShared);
	OldFormat = false;
	Solid = false;
	Volume = false;
	MainComment = false;
	Locked = false;
	Signed = false;
	NotFirstVolume = false;
	SFXSize = 0;
	LatestTime = 0;
	Protected = false;
	Encrypted = false;
	BrokenFileHeader = false;
	LastReadBlock = 0;

	CurBlockPos = 0;
	NextBlockPos = 0;

	RecoveryPos = SIZEOF_MARKHEAD;
	RecoverySectors = -1;

	memset(&NewMhd, 0, sizeof(NewMhd));
	NewMhd.HeadType = MAIN_HEAD;
	NewMhd.HeadSize = SIZEOF_NEWMHD;
	HeaderCRC = 0;
	VolWrite = 0;
	AddingFilesSize = 0;
	AddingHeadersSize = 0;
	*FirstVolumeName = 0;
	*FirstVolumeNameW = 0;

	Splitting = false;
	NewArchive = false;

	SilentOpen = false;
}


#ifndef SHELL_EXT
void Archive::CheckArc(bool EnableBroken) {
	if (!IsArchive(EnableBroken)) {
		Log(FileName, St(MBadArc), FileName);
		ErrHandler.Exit(FATAL_ERROR);
	}
}
#endif


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void Archive::CheckOpen(char *Name, wchar *NameW) {
	TOpen(Name, NameW);
	CheckArc(false);
}
#endif


bool Archive::WCheckOpen(char *Name, wchar *NameW) {
	if (!WOpen(Name, NameW))
		return (false);
	if (!IsArchive(false)) {
#ifndef SHELL_EXT
		Log(FileName, St(MNotRAR), FileName);
#endif
		Close();
		return (false);
	}
	return (true);
}


bool Archive::IsSignature(byte *D) {
	bool Valid = false;
	if (D[0] == 0x52)
#ifndef SFX_MODULE
		if (D[1] == 0x45 && D[2] == 0x7e && D[3] == 0x5e) {
			OldFormat = true;
			Valid = true;
		} else
#endif
			if (D[1] == 0x61 && D[2] == 0x72 && D[3] == 0x21 && D[4] == 0x1a && D[5] == 0x07 && D[6] == 0x00) {
				OldFormat = false;
				Valid = true;
			}
	return (Valid);
}


bool Archive::IsArchive(bool EnableBroken) {
	Encrypted = false;
#ifndef SFX_MODULE
	if (IsDevice()) {
#ifndef SHELL_EXT
		Log(FileName, St(MInvalidName), FileName);
#endif
		return (false);
	}
#endif
	if (Read(MarkHead.Mark, SIZEOF_MARKHEAD) != SIZEOF_MARKHEAD)
		return (false);
	SFXSize = 0;
	if (IsSignature(MarkHead.Mark)) {
		if (OldFormat)
			Seek(0, SEEK_SET);
	} else {
		Array<char> Buffer(0x40000);
		long CurPos = int64to32(Tell());
		int ReadSize = Read(&Buffer[0], Buffer.Size() - 16);
		for (int I = 0; I < ReadSize; I++)
			if (Buffer[I] == 0x52 && IsSignature((byte *)&Buffer[I])) {
				SFXSize = CurPos + I;
				Seek(SFXSize, SEEK_SET);
				if (!OldFormat)
					Read(MarkHead.Mark, SIZEOF_MARKHEAD);
				break;
			}
		if (SFXSize == 0)
			return (false);
	}
	ReadHeader();
	SeekToNext();
#ifndef SFX_MODULE
	if (OldFormat) {
		NewMhd.Flags = OldMhd.Flags & 0x3f;
		NewMhd.HeadSize = OldMhd.HeadSize;
	} else
#endif
	{
		if (HeaderCRC != NewMhd.HeadCRC) {
#ifndef SHELL_EXT
			Log(FileName, St(MLogMainHead));
#endif
			Alarm();
			if (!EnableBroken)
				return (false);
		}
	}
	Volume = (NewMhd.Flags & MHD_VOLUME);
	Solid = (NewMhd.Flags & MHD_SOLID) != 0;
	MainComment = (NewMhd.Flags & MHD_COMMENT) != 0;
	Locked = (NewMhd.Flags & MHD_LOCK) != 0;
	Signed = (NewMhd.PosAV != 0);
	Protected = (NewMhd.Flags & MHD_PROTECT) != 0;
	Encrypted = (NewMhd.Flags & MHD_PASSWORD) != 0;

	if (!SilentOpen || !Encrypted) {
		SaveFilePos SavePos(*this);
		Int64 SaveCurBlockPos = CurBlockPos, SaveNextBlockPos = NextBlockPos;

		NotFirstVolume = false;
		while (ReadHeader()) {
			int HeaderType = GetHeaderType();
			if (HeaderType == NEWSUB_HEAD) {
				if (SubHead.CmpName(SUBHEAD_TYPE_CMT))
					MainComment = true;
				if ((SubHead.Flags & LHD_SPLIT_BEFORE) ||
				        Volume && (NewMhd.Flags & MHD_FIRSTVOLUME) == 0)
					NotFirstVolume = true;
			} else {
				if (HeaderType == FILE_HEAD && ((NewLhd.Flags & LHD_SPLIT_BEFORE) != 0 ||
				                                Volume && NewLhd.UnpVer >= 29 && (NewMhd.Flags & MHD_FIRSTVOLUME) == 0))
					NotFirstVolume = true;
				break;
			}
			SeekToNext();
		}
		CurBlockPos = SaveCurBlockPos;
		NextBlockPos = SaveNextBlockPos;
	}
	return (true);
}




void Archive::SeekToNext() {
	Seek(NextBlockPos, SEEK_SET);
}


#ifndef SFX_MODULE
int Archive::GetRecoverySize(bool Required) {
	if (!Protected)
		return (0);
	if (RecoverySectors != -1 || !Required)
		return (RecoverySectors);
	SaveFilePos SavePos(*this);
	Seek(SFXSize, SEEK_SET);
	SearchSubBlock(SUBHEAD_TYPE_RR);
	return (RecoverySectors);
}
#endif


/***** File: arcread.cpp *****/

int Archive::SearchBlock(int BlockType) {
	int Size, Count = 0;
	while ((Size = ReadHeader()) != 0 &&
	        (BlockType == ENDARC_HEAD || GetHeaderType() != ENDARC_HEAD)) {
		if ((++Count & 127) == 0)
			Wait();
		if (GetHeaderType() == BlockType)
			return (Size);
		SeekToNext();
	}
	return (0);
}


int Archive::SearchSubBlock(const char *Type) {
	int Size;
	while ((Size = ReadHeader()) != 0) {
		if (GetHeaderType() == NEWSUB_HEAD && SubHead.CmpName(Type))
			return (Size);
		SeekToNext();
	}
	return (0);
}


int Archive::ReadHeader() {
	CurBlockPos = Tell();

#ifndef SFX_MODULE
	if (OldFormat)
		return (ReadOldHeader());
#endif

	RawRead Raw(this);

	bool Decrypt = Encrypted && CurBlockPos >= SFXSize + SIZEOF_MARKHEAD + SIZEOF_NEWMHD;

	if (Decrypt) {
		return (0);
	}

	Raw.Read(SIZEOF_SHORTBLOCKHEAD);
	if (Raw.Size() == 0) {
		Int64 ArcSize = FileLength();
		if (CurBlockPos > ArcSize || NextBlockPos > ArcSize) {
#ifndef SHELL_EXT
			Log(FileName, St(MLogUnexpEOF));
#endif
			ErrHandler.SetErrorCode(WARNING);
		}
		return (0);
	}

	Raw.Get(ShortBlock.HeadCRC);
	byte HeadType;
	Raw.Get(HeadType);
	ShortBlock.HeadType = (HEADER_TYPE)HeadType;
	Raw.Get(ShortBlock.Flags);
	Raw.Get(ShortBlock.HeadSize);
	if (ShortBlock.HeadSize < SIZEOF_SHORTBLOCKHEAD) {
#ifndef SHELL_EXT
		Log(FileName, St(MLogFileHead), "???");
#endif
		BrokenFileHeader = true;
		ErrHandler.SetErrorCode(CRC_ERROR);
		return (0);
	}

	if (ShortBlock.HeadType == COMM_HEAD)
		Raw.Read(SIZEOF_COMMHEAD - SIZEOF_SHORTBLOCKHEAD);
	else if (ShortBlock.HeadType == MAIN_HEAD && (ShortBlock.Flags & MHD_COMMENT) != 0)
		Raw.Read(SIZEOF_NEWMHD - SIZEOF_SHORTBLOCKHEAD);
	else
		Raw.Read(ShortBlock.HeadSize - SIZEOF_SHORTBLOCKHEAD);

	NextBlockPos = CurBlockPos + ShortBlock.HeadSize;

	switch (ShortBlock.HeadType) {
	case MAIN_HEAD:
		*(BaseBlock *)&NewMhd = ShortBlock;
		Raw.Get(NewMhd.HighPosAV);
		Raw.Get(NewMhd.PosAV);
		break;
	case ENDARC_HEAD:
		*(BaseBlock *)&EndArcHead = ShortBlock;
		if (EndArcHead.Flags & EARC_DATACRC)
			Raw.Get(EndArcHead.ArcDataCRC);
		break;
	case FILE_HEAD:
	case NEWSUB_HEAD: {
		FileHeader *hd = ShortBlock.HeadType == FILE_HEAD ? &NewLhd : &SubHead;
		*(BaseBlock *)hd = ShortBlock;
		Raw.Get(hd->PackSize);
		Raw.Get(hd->UnpSize);
		Raw.Get(hd->HostOS);
		Raw.Get(hd->FileCRC);
		Raw.Get(hd->FileTime);
		Raw.Get(hd->UnpVer);
		Raw.Get(hd->Method);
		Raw.Get(hd->NameSize);
		Raw.Get(hd->FileAttr);
		if (hd->Flags & LHD_LARGE) {
			Raw.Get(hd->HighPackSize);
			Raw.Get(hd->HighUnpSize);
		} else
			hd->HighPackSize = hd->HighUnpSize = 0;
		hd->FullPackSize = int32to64(hd->HighPackSize, hd->PackSize);
		hd->FullUnpSize = int32to64(hd->HighUnpSize, hd->UnpSize);

		char FileName[NM * 4];
		int NameSize = Min(hd->NameSize, sizeof(FileName) - 1);
		Raw.Get((byte *)FileName, NameSize);
		FileName[NameSize] = 0;

		strncpy(hd->FileName, FileName, sizeof(hd->FileName));
		hd->FileName[sizeof(hd->FileName) - 1] = 0;

		if (hd->HeadType == NEWSUB_HEAD) {
			int DataSize = hd->HeadSize - hd->NameSize - SIZEOF_NEWLHD;
			if (hd->Flags & LHD_SALT)
				DataSize -= SALT_SIZE;
			if (DataSize > 0) {
				hd->SubData.Alloc(DataSize);
				Raw.Get(&hd->SubData[0], DataSize);
				if (hd->CmpName(SUBHEAD_TYPE_RR)) {
					byte *D = &hd->SubData[8];
					RecoverySectors = D[0] + ((uint)D[1] << 8) + ((uint)D[2] << 16) + ((uint)D[3] << 24);
				}
			}
		} else if (hd->HeadType == FILE_HEAD) {
			if (hd->Flags & LHD_UNICODE) {
				EncodeFileName NameCoder;
				int Length = strlen(FileName) + 1;
				NameCoder.Decode(FileName, (byte *)FileName + Length,
				                 hd->NameSize - Length, hd->FileNameW,
				                 sizeof(hd->FileNameW) / sizeof(hd->FileNameW[0]));
				if (*hd->FileNameW == 0)
					hd->Flags &= ~LHD_UNICODE;
			} else
				*hd->FileNameW = 0;
#ifndef SFX_MODULE
			ConvertNameCase(hd->FileName);
			ConvertNameCase(hd->FileNameW);
#endif

			ConvertUnknownHeader();
		}
		if (hd->Flags & LHD_SALT)
			Raw.Get(hd->Salt, SALT_SIZE);
		NextBlockPos += hd->FullPackSize;
		bool CRCProcessedOnly = (hd->Flags & LHD_COMMENT) != 0;
		HeaderCRC = ~Raw.GetCRC(CRCProcessedOnly) & 0xffff;
		if (hd->HeadCRC != HeaderCRC) {
			if (hd->HeadType == NEWSUB_HEAD)
				strcat(hd->FileName, "- ???");
			BrokenFileHeader = true;
			ErrHandler.SetErrorCode(WARNING);
#ifndef SHELL_EXT
			Log(FileName, St(MLogFileHead), IntNameToExt(hd->FileName));
			Alarm();
#endif
		}
	}
	break;
#ifndef SFX_MODULE
	case COMM_HEAD:
		*(BaseBlock *)&CommHead = ShortBlock;
		Raw.Get(CommHead.UnpSize);
		Raw.Get(CommHead.UnpVer);
		Raw.Get(CommHead.Method);
		Raw.Get(CommHead.CommCRC);
		break;
	case SIGN_HEAD:
		*(BaseBlock *)&SignHead = ShortBlock;
		Raw.Get(SignHead.CreationTime);
		Raw.Get(SignHead.ArcNameSize);
		Raw.Get(SignHead.UserNameSize);
		break;
	case AV_HEAD:
		*(BaseBlock *)&AVHead = ShortBlock;
		Raw.Get(AVHead.UnpVer);
		Raw.Get(AVHead.Method);
		Raw.Get(AVHead.AVVer);
		Raw.Get(AVHead.AVInfoCRC);
		break;
	case PROTECT_HEAD:
		*(BaseBlock *)&ProtectHead = ShortBlock;
		Raw.Get(ProtectHead.DataSize);
		Raw.Get(ProtectHead.Version);
		Raw.Get(ProtectHead.RecSectors);
		Raw.Get(ProtectHead.TotalBlocks);
		Raw.Get(ProtectHead.Mark, 8);
		NextBlockPos += ProtectHead.DataSize;
		RecoverySectors = ProtectHead.RecSectors;
		break;
	case SUB_HEAD:
		*(BaseBlock *)&SubBlockHead = ShortBlock;
		Raw.Get(SubBlockHead.DataSize);
		NextBlockPos += SubBlockHead.DataSize;
		Raw.Get(SubBlockHead.SubType);
		Raw.Get(SubBlockHead.Level);
		switch (SubBlockHead.SubType) {
		case UO_HEAD:
			*(SubBlockHeader *)&UOHead = SubBlockHead;
			Raw.Get(UOHead.OwnerNameSize);
			Raw.Get(UOHead.GroupNameSize);
			Raw.Get((byte *)UOHead.OwnerName, UOHead.OwnerNameSize);
			Raw.Get((byte *)UOHead.GroupName, UOHead.GroupNameSize);
			UOHead.OwnerName[UOHead.OwnerNameSize] = 0;
			UOHead.GroupName[UOHead.GroupNameSize] = 0;
			break;
		case MAC_HEAD:
			*(SubBlockHeader *)&MACHead = SubBlockHead;
			Raw.Get(MACHead.fileType);
			Raw.Get(MACHead.fileCreator);
			break;
		case EA_HEAD:
		case BEEA_HEAD:
		case NTACL_HEAD:
			*(SubBlockHeader *)&EAHead = SubBlockHead;
			Raw.Get(EAHead.UnpSize);
			Raw.Get(EAHead.UnpVer);
			Raw.Get(EAHead.Method);
			Raw.Get(EAHead.EACRC);
			break;
		case STREAM_HEAD:
			*(SubBlockHeader *)&StreamHead = SubBlockHead;
			Raw.Get(StreamHead.UnpSize);
			Raw.Get(StreamHead.UnpVer);
			Raw.Get(StreamHead.Method);
			Raw.Get(StreamHead.StreamCRC);
			Raw.Get(StreamHead.StreamNameSize);
			Raw.Get((byte *)StreamHead.StreamName, StreamHead.StreamNameSize);
			StreamHead.StreamName[StreamHead.StreamNameSize] = 0;
			break;
		}
		break;
#endif
	default:
		if (ShortBlock.Flags & LONG_BLOCK) {
			uint DataSize;
			Raw.Get(DataSize);
			NextBlockPos += DataSize;
		}
		break;
	}
	HeaderCRC = ~Raw.GetCRC(false) & 0xffff;
	CurHeaderType = ShortBlock.HeadType;
	if (Decrypt) {
		NextBlockPos += Raw.PaddedSize() + SALT_SIZE;
		if (ShortBlock.HeadCRC != HeaderCRC) {
#ifndef SILENT
			Log(FileName, St(MEncrBadCRC), FileName);
#endif
			Close();
			ErrHandler.Exit(CRC_ERROR);
		}
	}

	if (NextBlockPos <= CurBlockPos) {
#ifndef SHELL_EXT
		Log(FileName, St(MLogFileHead), "???");
#endif
		BrokenFileHeader = true;
		ErrHandler.SetErrorCode(CRC_ERROR);
		return (0);
	}
	return (Raw.Size());
}


#ifndef SFX_MODULE
int Archive::ReadOldHeader() {
	RawRead Raw(this);
	if (CurBlockPos <= SFXSize) {
		Raw.Read(SIZEOF_OLDMHD);
		Raw.Get(OldMhd.Mark, 4);
		Raw.Get(OldMhd.HeadSize);
		Raw.Get(OldMhd.Flags);
		NextBlockPos = CurBlockPos + OldMhd.HeadSize;
		CurHeaderType = MAIN_HEAD;
	} else {
		OldFileHeader OldLhd;
		Raw.Read(SIZEOF_OLDLHD);
		NewLhd.HeadType = FILE_HEAD;
		Raw.Get(NewLhd.PackSize);
		Raw.Get(NewLhd.UnpSize);
		Raw.Get(OldLhd.FileCRC);
		Raw.Get(NewLhd.HeadSize);
		Raw.Get(NewLhd.FileTime);
		Raw.Get(OldLhd.FileAttr);
		Raw.Get(OldLhd.Flags);
		Raw.Get(OldLhd.UnpVer);
		Raw.Get(OldLhd.NameSize);
		Raw.Get(OldLhd.Method);

		NewLhd.Flags = OldLhd.Flags | LONG_BLOCK;
		NewLhd.UnpVer = (OldLhd.UnpVer == 2) ? 13 : 10;
		NewLhd.Method = OldLhd.Method + 0x30;
		NewLhd.NameSize = OldLhd.NameSize;
		NewLhd.FileAttr = OldLhd.FileAttr;
		NewLhd.FileCRC = OldLhd.FileCRC;
		NewLhd.FullPackSize = NewLhd.PackSize;
		NewLhd.FullUnpSize = NewLhd.UnpSize;

		Raw.Read(OldLhd.NameSize);
		Raw.Get((byte *)NewLhd.FileName, OldLhd.NameSize);
		NewLhd.FileName[OldLhd.NameSize] = 0;
		ConvertNameCase(NewLhd.FileName);
		*NewLhd.FileNameW = 0;

		if (Raw.Size() != 0)
			NextBlockPos = CurBlockPos + NewLhd.HeadSize + NewLhd.PackSize;
		CurHeaderType = FILE_HEAD;
	}
	return (Raw.Size());
}
#endif


void Archive::ConvertNameCase(char *Name) {
	if (Cmd->ConvertNames == NAMES_UPPERCASE) {
		IntToExt(Name, Name);
		strupper(Name);
		ExtToInt(Name, Name);
	}
	if (Cmd->ConvertNames == NAMES_LOWERCASE) {
		IntToExt(Name, Name);
		strlower(Name);
		ExtToInt(Name, Name);
	}
}


#ifndef SFX_MODULE
void Archive::ConvertNameCase(wchar *Name) {
	if (Cmd->ConvertNames == NAMES_UPPERCASE)
		strupperw(Name);
	if (Cmd->ConvertNames == NAMES_LOWERCASE)
		strlowerw(Name);
}
#endif


bool Archive::IsArcDir() {
	return ((NewLhd.Flags & LHD_WINDOWMASK) == LHD_DIRECTORY);
}


bool Archive::IsArcLabel() {
	return (NewLhd.HostOS <= WIN_32 && (NewLhd.FileAttr & 8));
}


void Archive::ConvertAttributes() {
	static mode_t mask = (mode_t) -1;

	if (mask == (mode_t) -1) {
		mask = umask(022);
		umask(mask);
	}
	switch (NewLhd.HostOS) {
	case MS_DOS:
	case OS2:
	case WIN_32:
		if (NewLhd.FileAttr & 0x10)
			NewLhd.FileAttr = 0x41ff & ~mask;
		else if (NewLhd.FileAttr & 1)
			NewLhd.FileAttr = 0x8124 & ~mask;
		else
			NewLhd.FileAttr = 0x81b6 & ~mask;
		break;
	case UNIX:
	case BEOS:
		break;
	default:
		if ((NewLhd.Flags & LHD_WINDOWMASK) == LHD_DIRECTORY)
			NewLhd.FileAttr = 0x41ff & ~mask;
		else
			NewLhd.FileAttr = 0x81b6 & ~mask;
		break;
	}
}


void Archive::ConvertUnknownHeader() {
	if (NewLhd.UnpVer < 20 && (NewLhd.FileAttr & 0x10))
		NewLhd.Flags |= LHD_DIRECTORY;
	if (NewLhd.HostOS > BEOS) {
		if ((NewLhd.Flags & LHD_WINDOWMASK) == LHD_DIRECTORY)
			NewLhd.FileAttr = 0x10;
		else
			NewLhd.FileAttr = 0x20;
	}
	for (char *s = NewLhd.FileName; *s != 0; s = charnext(s)) {
		if (*s == '/' || *s == '\\')
			*s = CPATHDIVIDER;
	}
}


int Archive::LhdSize() {
	return ((NewLhd.Flags & LHD_LARGE) ? SIZEOF_NEWLHD + 8 : SIZEOF_NEWLHD);
}


int Archive::LhdExtraSize() {
	return (NewLhd.HeadSize - NewLhd.NameSize - LhdSize());
}


#ifndef SHELL_EXT
bool Archive::ReadSubData(Array<byte> *UnpData, File *DestFile) {
	if (HeaderCRC != SubHead.HeadCRC) {
#ifndef SHELL_EXT
		Log(FileName, St(MSubHeadCorrupt));
#endif
		ErrHandler.SetErrorCode(CRC_ERROR);
		return (false);
	}
	if (SubHead.Method < 0x30 || SubHead.Method > 0x35 || SubHead.UnpVer > PACK_VER) {
#ifndef SHELL_EXT
		Log(FileName, St(MSubHeadUnknown));
#endif
		return (false);
	}

	if (SubHead.PackSize == 0 && (SubHead.Flags & LHD_SPLIT_AFTER) == 0)
		return (true);

//  ComprDataIO DataIO(NULL);
	SubDataIO.Init();
	Unpack Unpack(&SubDataIO);
	Unpack.Init();

	if (DestFile == NULL) {
		UnpData->Alloc(SubHead.UnpSize);
		SubDataIO.SetUnpackToMemory(&(*UnpData)[0], SubHead.UnpSize);
	}
	if (SubHead.Flags & LHD_PASSWORD)
		if (*Cmd->Password)
			SubDataIO.SetEncryption(SubHead.UnpVer, Cmd->Password,
			                        (SubHead.Flags & LHD_SALT) ? SubHead.Salt : NULL, false);
		else
			return (false);
	SubDataIO.SetPackedSizeToRead(SubHead.PackSize);
	SubDataIO.EnableShowProgress(false);
	SubDataIO.SetFiles(this, DestFile);
	SubDataIO.UnpVolume = (SubHead.Flags & LHD_SPLIT_AFTER);
	SubDataIO.SetSubHeader(&SubHead, NULL);
	Unpack.SetDestSize(SubHead.UnpSize);
	if (SubHead.Method == 0x30)
		CmdExtract::UnstoreFile(SubDataIO, SubHead.UnpSize);
	else
		Unpack.DoUnpack(SubHead.UnpVer, false);

	if (SubHead.FileCRC != ~SubDataIO.UnpFileCRC) {
#ifndef SHELL_EXT
		Log(FileName, St(MSubHeadDataCRC), SubHead.FileName);
#endif
		ErrHandler.SetErrorCode(CRC_ERROR);
		if (UnpData != NULL)
			UnpData->Reset();
		return (false);
	}
	return (true);
}
#endif


/***** File: unicode.cpp *****/

void WideToChar(const wchar_t *Src, char *Dest, int DestSize) {
#ifdef MBFUNCTIONS
	if (wcstombs(Dest, Src, DestSize) == -1)
		*Dest = 0;
#else
	for (int I = 0; I < DestSize; I++) {
		Dest[I] = (char)Src[I];
		if (Src[I] == 0)
			break;
	}
#endif
}


void CharToWide(const char *Src, wchar_t *Dest, int DestSize) {
#ifdef MBFUNCTIONS
	mbstowcs(Dest, Src, DestSize);
#else
	for (int I = 0; I < DestSize; I++) {
		Dest[I] = (wchar_t)Src[I];
		if (Src[I] == 0)
			break;
	}
#endif
}


wchar *RawToWide(const byte *Src, wchar *Dest, int DestSize) {
	for (int I = 0; I < DestSize; I++)
		if ((Dest[I] = Src[I * 2] + (Src[I * 2 + 1] << 8)) == 0)
			break;
	return (Dest);
}


bool LowAscii(const wchar *Str) {
	for (int I = 0; Str[I] != 0; I++)
		if (Str[I] < 32 || Str[I] > 127)
			return (false);
	return (true);
}


int strlenw(const wchar *str) {
	int length = 0;
	while (*(str++) != 0)
		length++;
	return (length);
}


wchar *strcpyw(wchar *dest, const wchar *src) {
	do {
		*(dest++) = *src;
	} while (*(src++) != 0);
	return (dest);
}


wchar *strncpyw(wchar *dest, const wchar *src, int n) {
	do {
		*(dest++) = *src;
	} while (*(src++) != 0 && --n > 0);
	return (dest);
}


wchar *strcatw(wchar *dest, const wchar *src) {
	return (strcpyw(dest + strlenw(dest), src));
}


#ifndef SFX_MODULE
wchar *strncatw(wchar *dest, const wchar *src, int n) {
	dest += strlenw(dest);
	while (true)
		if (--n < 0) {
			*dest = 0;
			break;
		} else if ((*(dest++) = *(src++)) == 0)
			break;
	return (dest);
}
#endif


int strcmpw(const wchar *s1, const wchar *s2) {
	while (*s1 == *s2) {
		if (*s1 == 0)
			return (0);
		s1++;
		s2++;
	}
	return (*s1 < *s2 ? -1 : 1);
}


int strncmpw(const wchar *s1, const wchar *s2, int n) {
	while (n-- > 0) {
		if (*s1 < *s2)
			return (-1);
		if (*s1 > *s2)
			return (-1);
		if (*s1 == 0)
			break;
		s1++;
		s2++;
	}
	return (0);
}


#ifndef SFX_MODULE
int stricmpw(const wchar *s1, const wchar *s2) {
	char Ansi1[NM * sizeof(wchar)], Ansi2[NM * sizeof(wchar)];
	WideToChar(s1, Ansi1, sizeof(Ansi1));
	WideToChar(s2, Ansi2, sizeof(Ansi2));
	return (stricomp(Ansi1, Ansi2));
}
#endif


wchar *strchrw(const wchar *s, int c) {
	while (*s) {
		if (*s == c)
			return ((wchar *)s);
		s++;
	}
	return (NULL);
}


wchar *strrchrw(const wchar *s, int c) {
	for (int I = strlenw(s) - 1; I >= 0; I--)
		if (s[I] == c)
			return ((wchar *)(s + I));
	return (NULL);
}


wchar *strpbrkw(const wchar *s1, const wchar *s2) {
	while (*s1) {
		if (strchrw(s2, *s1) != NULL)
			return ((wchar *)s1);
		s1++;
	}
	return (NULL);
}


#ifndef SFX_MODULE
wchar *strlowerw(wchar *Str) {
	for (wchar *ChPtr = Str; *ChPtr; ChPtr++)
		if (*ChPtr < 128)
			*ChPtr = loctolower(*ChPtr);
	return (Str);
}
#endif


#ifndef SFX_MODULE
wchar *strupperw(wchar *Str) {
	for (wchar *ChPtr = Str; *ChPtr; ChPtr++)
		if (*ChPtr < 128)
			*ChPtr = loctoupper(*ChPtr);
	return (Str);
}
#endif


int atoiw(const wchar *s) {
	int n = 0;
	while (*s >= '0' && *s <= '9') {
		n = n * 10 + (*s - '0');
		s++;
	}
	return (n);
}


/***** File: system.cpp *****/

static int SleepTime = 0;

void InitSystemOptions(int SleepTime) {
	::SleepTime = SleepTime;
}


#ifndef SFX_MODULE
void SetPriority(int Priority) {
}
#endif


void Wait() {
}


/***** File: crc.cpp *****/

uint CRCTab[256];

void InitCRC() {
	for (int I = 0; I < 256; I++) {
		uint C = I;
		for (int J = 0; J < 8; J++)
			C = (C & 1) ? (C >> 1) ^ 0xEDB88320L : (C >> 1);
		CRCTab[I] = C;
	}
}


uint CRC(uint StartCRC, void *Addr, uint Size) {
	if (CRCTab[1] == 0)
		InitCRC();
	byte *Data = (byte *)Addr;
#if defined(LITTLE_ENDIAN) && defined(PRESENT_INT32)
	while (Size >= 8) {
		StartCRC ^= *(uint32 *)Data;
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC ^= *(uint32 *)(Data + 4);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		StartCRC = CRCTab[(byte)StartCRC] ^ (StartCRC >> 8);
		Data += 8;
		Size -= 8;
	}
#endif
	for (int I = 0; I < Size; I++)
		StartCRC = CRCTab[(byte)(StartCRC ^ Data[I])] ^ (StartCRC >> 8);
	return (StartCRC);
}

#ifndef SFX_MODULE
ushort OldCRC(ushort StartCRC, void *Addr, uint Size) {
	byte *Data = (byte *)Addr;
	for (int I = 0; I < Size; I++) {
		StartCRC = (StartCRC + Data[I]) & 0xffff;
		StartCRC = ((StartCRC << 1) | (StartCRC >> 15)) & 0xffff;
	}
	return (StartCRC);
}
#endif


/***** File: rawread.cpp *****/

RawRead::RawRead(File *SrcFile) {
	RawRead::SrcFile = SrcFile;
	ReadPos = 0;
	DataSize = 0;
}


void RawRead::Read(int Size) {
	if (Size != 0) {
		Data.Add(Size);
		DataSize += SrcFile->Read(&Data[DataSize], Size);
	}
}


void RawRead::Read(byte *SrcData, int Size) {
	if (Size != 0) {
		Data.Add(Size);
		memcpy(&Data[DataSize], SrcData, Size);
		DataSize += Size;
	}
}


void RawRead::Get(byte &Field) {
	Field = Data[ReadPos];
	ReadPos++;
}


void RawRead::Get(ushort &Field) {
	Field = Data[ReadPos] + (Data[ReadPos + 1] << 8);
	ReadPos += 2;
}


void RawRead::Get(uint &Field) {
	Field = Data[ReadPos] + (Data[ReadPos + 1] << 8) + (Data[ReadPos + 2] << 16) +
	        (Data[ReadPos + 3] << 24);
	ReadPos += 4;
}


void RawRead::Get8(Int64 &Field) {
	uint Low, High;
	Get(Low);
	Get(High);
	Field = int32to64(High, Low);
}


void RawRead::Get(byte *Field, int Size) {
	memcpy(Field, &Data[ReadPos], Size);
	ReadPos += Size;
}


void RawRead::Get(wchar *Field, int Size) {
	RawToWide(&Data[ReadPos], Field, Size);
	ReadPos += 2 * Size;
}


uint RawRead::GetCRC(bool ProcessedOnly) {
	return (DataSize > 2 ? CRC(0xffffffff, &Data[2], (ProcessedOnly ? ReadPos : DataSize) - 2) : 0xffffffff);
}


/***** File: encname.cpp *****/

EncodeFileName::EncodeFileName() {
	Flags = 0;
	FlagBits = 0;
	FlagsPos = 0;
	DestSize = 0;
}




void EncodeFileName::Decode(char *Name, byte *EncName, int EncSize, wchar *NameW,
                            int MaxDecSize) {
	int EncPos = 0, DecPos = 0;
	byte HighByte = EncName[EncPos++];
	while (EncPos < EncSize && DecPos < MaxDecSize) {
		if (FlagBits == 0) {
			Flags = EncName[EncPos++];
			FlagBits = 8;
		}
		switch (Flags >> 6) {
		case 0:
			NameW[DecPos++] = EncName[EncPos++];
			break;
		case 1:
			NameW[DecPos++] = EncName[EncPos++] + (HighByte << 8);
			break;
		case 2:
			NameW[DecPos++] = EncName[EncPos] + (EncName[EncPos + 1] << 8);
			EncPos += 2;
			break;
		case 3: {
			int Length = EncName[EncPos++];
			if (Length & 0x80) {
				byte Correction = EncName[EncPos++];
				for (Length = (Length & 0x7f) + 2; Length > 0 && DecPos < MaxDecSize; Length--, DecPos++)
					NameW[DecPos] = ((Name[DecPos] + Correction) & 0xff) + (HighByte << 8);
			} else
				for (Length += 2; Length > 0 && DecPos < MaxDecSize; Length--, DecPos++)
					NameW[DecPos] = Name[DecPos];
		}
		break;
		}
		Flags <<= 2;
		FlagBits -= 2;
	}
	NameW[DecPos < MaxDecSize ? DecPos : MaxDecSize - 1] = 0;
}


/***** File: match.cpp *****/

static bool match(char *pattern, char *string);
static bool match(wchar *pattern, wchar *string);

inline uint toupperc(uint ch) {
	/*
	*/
	return (ch);
}


inline uint touppercw(uint ch) {
	/*
	*/
	return (ch);
}


bool CmpName(char *Wildcard, char *Name, int CmpPath) {
	if (CmpPath != MATCH_NAMES) {
		int WildLength = strlen(Wildcard);
		if (strnicompc(Wildcard, Name, WildLength) == 0) {
			char NextCh = Name[WildLength];
			if (NextCh == '\\' || NextCh == '/' || NextCh == 0)
				return (true);
		}
		char Path1[NM], Path2[NM];
		GetFilePath(Wildcard, Path1);
		GetFilePath(Name, Path2);
		if (stricompc(Wildcard, Path2) == 0)
			return (true);
		if (CmpPath == MATCH_PATH && stricompc(Path1, Path2) != 0)
			return (false);
		if (CmpPath == MATCH_SUBPATH || CmpPath == MATCH_WILDSUBPATH)
			if (IsWildcard(Path1))
				return (match(Wildcard, Name));
			else if (CmpPath == MATCH_SUBPATH || IsWildcard(Wildcard)) {
				if (strnicompc(Path1, Path2, strlen(Path1)) != 0)
					return (false);
			} else if (stricompc(Path1, Path2) != 0)
				return (false);
	}
	char *Name1 = PointToName(Wildcard);
	char *Name2 = PointToName(Name);
	if (strnicompc("__rar_", Name2, 6) == 0)
		return (false);
	return (match(Name1, Name2));
}


#ifndef SFX_MODULE
bool CmpName(wchar *Wildcard, wchar *Name, int CmpPath) {
	if (CmpPath != MATCH_NAMES) {
		int WildLength = strlenw(Wildcard);
		if (strnicompcw(Wildcard, Name, WildLength) == 0) {
			wchar NextCh = Name[WildLength];
			if (NextCh == L'\\' || NextCh == L'/' || NextCh == 0)
				return (true);
		}
		wchar Path1[NM], Path2[NM];
		GetFilePath(Wildcard, Path1);
		GetFilePath(Name, Path2);
		if (CmpPath == MATCH_PATH && stricompcw(Path1, Path2) != 0)
			return (false);
		if (CmpPath == MATCH_SUBPATH || CmpPath == MATCH_WILDSUBPATH)
			if (IsWildcard(NULL, Path1))
				return (match(Wildcard, Name));
			else if (CmpPath == MATCH_SUBPATH || IsWildcard(NULL, Wildcard)) {
				if (strnicompcw(Path1, Path2, strlenw(Path1)) != 0)
					return (false);
			} else if (stricompcw(Path1, Path2) != 0)
				return (false);
	}
	wchar *Name1 = PointToName(Wildcard);
	wchar *Name2 = PointToName(Name);
	if (strnicompcw(L"__rar_", Name2, 6) == 0)
		return (false);
	return (match(Name1, Name2));
}
#endif


bool match(char *pattern, char *string) {
	for (;; ++string) {
		char stringc = toupperc(*string);
		char patternc = toupperc(*pattern++);
		switch (patternc) {
		case 0:
			return (stringc == 0);
		case '?':
			if (stringc == 0)
				return (false);
			break;
		case '*':
			if (*pattern == 0)
				return (true);
			if (*pattern == '.') {
				if (pattern[1] == '*' && pattern[2] == 0)
					return (true);
				char *dot = strchr(string, '.');
				if (pattern[1] == 0)
					return (dot == NULL || dot[1] == 0);
				if (dot != NULL) {
					string = dot;
					if (strpbrk(pattern, "*?") == NULL && strchr(string + 1, '.') == NULL)
						return (stricompc(pattern + 1, string + 1) == 0);
				}
			}

			while (*string)
				if (match(pattern, string++))
					return (true);
			return (false);
		default:
			if (patternc != stringc)
				if (patternc == '.' && stringc == 0)
					return (match(pattern, string));
				else
					return (false);
			break;
		}
	}
}


#ifndef SFX_MODULE
bool match(wchar *pattern, wchar *string) {
	for (;; ++string) {
		wchar stringc = touppercw(*string);
		wchar patternc = touppercw(*pattern++);
		switch (patternc) {
		case 0:
			return (stringc == 0);
		case '?':
			if (stringc == 0)
				return (false);
			break;
		case '*':
			if (*pattern == 0)
				return (true);
			if (*pattern == '.') {
				if (pattern[1] == '*' && pattern[2] == 0)
					return (true);
				wchar *dot = strchrw(string, '.');
				if (pattern[1] == 0)
					return (dot == NULL || dot[1] == 0);
				if (dot != NULL) {
					string = dot;
					if (strpbrkw(pattern, L"*?") == NULL && strchrw(string + 1, '.') == NULL)
						return (stricompcw(pattern + 1, string + 1) == 0);
				}
			}

			while (*string)
				if (match(pattern, string++))
					return (true);
			return (false);
		default:
			if (patternc != stringc)
				if (patternc == '.' && stringc == 0)
					return (match(pattern, string));
				else
					return (false);
			break;
		}
	}
}
#endif


int stricompc(const char *Str1, const char *Str2) {
	/*
	*/
	return (strcmp(Str1, Str2));
}


#ifndef SFX_MODULE
int stricompcw(const wchar *Str1, const wchar *Str2) {
	/*
	*/
	return (strcmpw(Str1, Str2));
}
#endif


int strnicompc(const char *Str1, const char *Str2, int N) {
	/*
	*/
	return (strncmp(Str1, Str2, N));
}


#ifndef SFX_MODULE
int strnicompcw(const wchar *Str1, const wchar *Str2, int N) {
	/*
	*/
	return (strncmpw(Str1, Str2, N));
}
#endif


/***** File: timefn.cpp *****/

static time_t SystemTime;


void InitTime() {
	time(&SystemTime);
}


#ifndef SFX_MODULE
uint SecondsToDosTime(uint Seconds) {
	return (UnixTimeToDos(SystemTime - Seconds));
}
#endif


void ConvertDate(uint ft, char *DateStr, bool FullYear) {
	int Day = (ft >> 16) & 0x1f;
	int Month = (ft >> 21) & 0xf;
	int Year = (ft >> 25) + 1980;
	int Hour = (ft >> 11) & 0x1f;
	int Minute = (ft >> 5) & 0x3f;
	if (FullYear)
		sprintf(DateStr, "%02u-%02u-%u %02u:%02u", Day, Month, Year, Hour, Minute);
	else
		sprintf(DateStr, "%02u-%02u-%02u %02u:%02u", Day, Month, Year % 100, Hour, Minute);
}


#ifndef SFX_MODULE
const char *GetMonthName(int Month) {
#ifdef SILENT
	return ("");
#else
	static MSGID MonthID[] = {
		MMonthJan, MMonthFeb, MMonthMar, MMonthApr, MMonthMay, MMonthJun,
		MMonthJul, MMonthAug, MMonthSep, MMonthOct, MMonthNov, MMonthDec
	};
	return (St(MonthID[Month]));
#endif
}
#endif


#ifndef SFX_MODULE
uint TextAgeToSeconds(char *TimeText) {
	uint Seconds = 0, Value = 0;
	for (int I = 0; TimeText[I] != 0; I++) {
		int Ch = TimeText[I];
		if (isdigit(Ch))
			Value = Value * 10 + Ch - '0';
		else {
			switch (toupper(Ch)) {
			case 'D':
				Seconds += Value * 24 * 3600;
				break;
			case 'H':
				Seconds += Value * 3600;
				break;
			case 'M':
				Seconds += Value * 60;
				break;
			case 'S':
				Seconds += Value;
				break;
			}
			Value = 0;
		}
	}
	return (Seconds);
}
#endif


#ifndef SFX_MODULE
uint IsoTextToDosTime(char *TimeText) {
	int Field[6];
	memset(Field, 0, sizeof(Field));
	for (int DigitCount = 0; *TimeText != 0; TimeText++)
		if (isdigit(*TimeText)) {
			int FieldPos = DigitCount < 4 ? 0 : (DigitCount - 4) / 2 + 1;
			if (FieldPos < sizeof(Field) / sizeof(Field[0]))
				Field[FieldPos] = Field[FieldPos] * 10 + *TimeText - '0';
			DigitCount++;
		}
	if (Field[1] == 0)
		Field[1] = 1;
	if (Field[2] == 0)
		Field[2] = 1;
	uint DosTime = ((Field[0] - 1980) << 25) | (Field[1] << 21) | (Field[2] << 16) |
	               (Field[3] << 11) | (Field[4] << 5) | (Field[5] / 2);
	return (DosTime);
}
#endif


uint UnixTimeToDos(time_t UnixTime) {
	struct tm *t;
	uint DosTime;
	t = localtime(&UnixTime);
	DosTime = (t->tm_sec / 2) | (t->tm_min << 5) | (t->tm_hour << 11) |
	          (t->tm_mday << 16) | ((t->tm_mon + 1) << 21) | ((t->tm_year - 80) << 25);
	return (DosTime);
}


time_t DosTimeToUnix(uint DosTime) {
	struct tm t;

	t.tm_sec = (DosTime & 0x1f) * 2;
	t.tm_min = (DosTime >> 5) & 0x3f;
	t.tm_hour = (DosTime >> 11) & 0x1f;
	t.tm_mday = (DosTime >> 16) & 0x1f;
	t.tm_mon = ((DosTime >> 21) - 1) & 0x0f;
	t.tm_year = (DosTime >> 25) + 80;
	t.tm_isdst = -1;
	return (mktime(&t));
}


/***** File: rdwrfn.cpp *****/

ComprDataIO::ComprDataIO(CmdAdd *Command) {
	ComprDataIO::Command = Command;
	Init();
}


void ComprDataIO::Init() {
	SrcUnpack = NULL;
	PackFromMemory = false;
	UnpackFromMemory = false;
	UnpackToMemory = false;
	UnpPackedSize = 0;
	ShowProgress = true;
	TestMode = false;
	SkipUnpCRC = false;
	PackVolume = false;
	UnpVolume = false;
	SrcFile = NULL;
	DestFile = NULL;
	UnpWrSize = 0;
	ComprDataIO::Command = Command;
	Encryption = 0;
	Decryption = 0;
	TotalPackRead = 0;
	CurPackRead = CurPackWrite = CurUnpRead = CurUnpWrite = 0;
	PackFileCRC = UnpFileCRC = PackedCRC = 0xffffffff;
	LastPercent = -1;
	SubHead = NULL;
	SubHeadPos = NULL;
	CurrentCommand = 0;
	ProcessedArcSize = TotalArcSize = 0;
}




int ComprDataIO::UnpRead(byte *Addr, uint Count) {
	int RetCode = 0, TotalRead = 0;
	byte *ReadAddr;
	ReadAddr = Addr;
	while (Count > 0) {
		Archive *SrcArc = (Archive *)SrcFile;

		uint ReadSize = (Count > UnpPackedSize) ? int64to32(UnpPackedSize) : Count;
		if (UnpackFromMemory) {
			memcpy(Addr, UnpackFromMemoryAddr, UnpackFromMemorySize);
			RetCode = UnpackFromMemorySize;
			UnpackFromMemorySize = 0;
		} else {
			if (!SrcFile->IsOpened())
				return (-1);
			RetCode = SrcFile->Read(ReadAddr, ReadSize);
			FileHeader *hd = SubHead != NULL ? SubHead : &SrcArc->NewLhd;
			if (hd->Flags & LHD_SPLIT_AFTER)
				PackedCRC = CRC(PackedCRC, ReadAddr, ReadSize);
		}
		CurUnpRead += RetCode;
		ReadAddr += RetCode;
		TotalRead += RetCode;
		Count -= RetCode;
		UnpPackedSize -= RetCode;
		if (UnpPackedSize == 0 && UnpVolume) {
#ifndef NOVOLUME
			if (!MergeArchive(*SrcArc, this, true, CurrentCommand))
#endif
				return (-1);
		} else
			break;
	}
	Archive *SrcArc = (Archive *)SrcFile;
	if (SrcArc != NULL)
		ShowUnpRead(SrcArc->CurBlockPos + CurUnpRead, UnpArcSize);
	if (RetCode != -1) {
		RetCode = TotalRead;
	}
	Wait();
	return (RetCode);
}


void ComprDataIO::UnpWrite(byte *Addr, uint Count) {
	UnpWrAddr = Addr;
	UnpWrSize = Count;
	if (SrcUnpack != NULL) {
		int NewSize = RepackUnpDataEnd + Count;
		RepackUnpData.Alloc(NewSize);
		memcpy(&RepackUnpData[RepackUnpDataEnd], Addr, Count);
		RepackUnpDataEnd = NewSize;

		SrcUnpack->SetSuspended(true);
	}
	if (UnpackToMemory) {
		if (Count <= UnpackToMemorySize) {
			memcpy(UnpackToMemoryAddr, Addr, Count);
			UnpackToMemoryAddr += Count;
			UnpackToMemorySize -= Count;
		}
	} else if (!TestMode)
		DestFile->Write(Addr, Count);
	CurUnpWrite += Count;
	if (!SkipUnpCRC)
#ifndef SFX_MODULE
		if (((Archive *)SrcFile)->OldFormat)
			UnpFileCRC = OldCRC((ushort)UnpFileCRC, Addr, Count);
		else
#endif
			UnpFileCRC = CRC(UnpFileCRC, Addr, Count);
	ShowUnpWrite();
	Wait();
}

void ComprDataIO::ShowUnpRead(Int64 ArcPos, Int64 ArcSize) {
	if (ShowProgress && SrcUnpack == NULL && SrcFile != NULL) {
		Archive *SrcArc = (Archive *)SrcFile;
		RAROptions *Cmd = SrcArc->GetRAROptions();
		if (TotalArcSize != 0)
			ArcSize = TotalArcSize;
		ArcPos += ProcessedArcSize;
		if (!SrcArc->Volume) {
			int CurPercent = ToPercent(ArcPos, ArcSize);
			if (!Cmd->DisablePercentage && CurPercent != LastPercent) {
				mprintf("\b\b\b\b%3d%%", CurPercent);
				LastPercent = CurPercent;
			}
		}
	}
}


void ComprDataIO::ShowUnpWrite() {
}








void ComprDataIO::SetFiles(File *SrcFile, File *DestFile) {
	if (SrcFile != NULL)
		ComprDataIO::SrcFile = SrcFile;
	if (DestFile != NULL)
		ComprDataIO::DestFile = DestFile;
	LastPercent = -1;
}


void ComprDataIO::GetUnpackedData(byte **Data, uint *Size) {
	*Data = UnpWrAddr;
	*Size = UnpWrSize;
}


void ComprDataIO::SetEncryption(int Method, char *Password, byte *Salt, bool Encrypt) {
	if (Encrypt) {
		Encryption = *Password ? Method : 0;
	} else {
		Decryption = *Password ? Method : 0;
	}
}



void ComprDataIO::SetUnpackToMemory(byte *Addr, uint Size) {
	UnpackToMemory = true;
	UnpackToMemoryAddr = Addr;
	UnpackToMemorySize = Size;
}


#ifndef GUI
/***** File: log.cpp *****/

static void WriteToLog(const char *ArcName, const char *Message);

static char LogName[NM];

void InitLogOptions(char *LogName) {
	strcpy(::LogName, LogName);
}


void Log(const char *ArcName, const char *Format, ...) {
	char Msg[4096];
	va_list ArgPtr;
	va_start(ArgPtr, Format);
	vsprintf(Msg, Format, ArgPtr);
	va_end(ArgPtr);
	eprintf("%s", Msg);
}

#endif


/***** File: consio.cpp *****/

static void RawPrint(char *Msg, MESSAGE_TYPE MessageType);

static MESSAGE_TYPE MsgStream = MSG_STDOUT;
static bool Sound = false;
const int MaxMsgSize = 4096;

void InitConsoleOptions(MESSAGE_TYPE MsgStream, bool Sound) {
	::MsgStream = MsgStream;
	::Sound = Sound;
}

#if !defined(GUI) && !defined(SILENT)
void mprintf(const char *fmt, ...) {
	if (MsgStream == MSG_NULL)
		return;
	char Msg[MaxMsgSize];
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(Msg, fmt, argptr);
	RawPrint(Msg, MsgStream);
	va_end(argptr);
}
#endif


#if !defined(GUI) && !defined(SILENT)
void eprintf(const char *fmt, ...) {
	if (MsgStream == MSG_NULL)
		return;
	char Msg[MaxMsgSize];
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(Msg, fmt, argptr);
	RawPrint(Msg, MSG_STDERR);
	va_end(argptr);
}
#endif


#if !defined(GUI) && !defined(SILENT)
void RawPrint(char *Msg, MESSAGE_TYPE MessageType) {
	File OutFile;
	switch (MessageType) {
	case MSG_STDOUT:
		OutFile.SetHandleType(FILE_HANDLESTD);
		break;
	case MSG_STDERR:
		OutFile.SetHandleType(FILE_HANDLEERR);
		break;
	default:
		return;
	}

	char OutMsg[MaxMsgSize], *OutPos = OutMsg;
	for (int I = 0; Msg[I] != 0; I++)
		if (Msg[I] != '\r')
			*(OutPos++) = Msg[I];
	*OutPos = 0;
	strcpy(Msg, OutMsg);

	OutFile.Write(Msg, strlen(Msg));
//  OutFile.Flush();
}
#endif


#ifndef SILENT
void Alarm() {
#ifndef SFX_MODULE
	if (Sound)
		putchar('\007');
#endif
}
#endif


#ifndef SILENT
#ifndef GUI
void GetPasswordText(char *Str, int MaxLength) {
	strncpy(Str, getpass(""), MaxLength - 1);
	RemoveLF(Str);
}
#endif
#endif


#if !defined(GUI) && !defined(SILENT)
unsigned int GetKey() {
#ifdef SILENT
	return (0);
#else
	char Str[80];
	fgets(Str, sizeof(Str), stdin);
	return (Str[0]);
#endif
}
#endif


#ifndef SILENT
bool GetPassword(PASSWORD_TYPE Type, const char *FileName, char *Password, int MaxLength) {
	Alarm();
	while (true) {
		char PromptStr[256];
		strcpy(PromptStr, St(MAskPsw));
		if (Type != PASSWORD_GLOBAL) {
			strcat(PromptStr, St(MFor));
			strcat(PromptStr, PointToName(FileName));
		}
		eprintf("\n%s: ", PromptStr);
		GetPasswordText(Password, MaxLength);
		if (*Password == 0 && Type == PASSWORD_GLOBAL)
			return (false);
		if (Type == PASSWORD_GLOBAL) {
			strcpy(PromptStr, St(MReAskPsw));
			eprintf(PromptStr);
			char CmpStr[256];
			GetPasswordText(CmpStr, sizeof(CmpStr));
			if (*CmpStr == 0 || strcmp(Password, CmpStr) != 0) {
				strcpy(PromptStr, St(MNotMatchPsw));
				eprintf(PromptStr);
				memset(Password, 0, MaxLength);
				memset(CmpStr, 0, sizeof(CmpStr));
				continue;
			}
			memset(CmpStr, 0, sizeof(CmpStr));
		}
		break;
	}
	return (true);
}
#endif


#if !defined(GUI) && !defined(SILENT)
int Ask(const char *AskStr) {
	const int MaxItems = 10;
	char Item[MaxItems][40];
	int ItemKeyPos[MaxItems], NumItems = 0;

	for (const char *NextItem = AskStr; NextItem != NULL; NextItem = strchr(NextItem + 1, '_')) {
		char *CurItem = Item[NumItems];
		strncpy(CurItem, NextItem + 1, sizeof(Item[0]));
		char *EndItem = strchr(CurItem, '_');
		if (EndItem != NULL)
			*EndItem = 0;
		int KeyPos = 0, CurKey;
		while ((CurKey = CurItem[KeyPos]) != 0) {
			bool Found = false;
			for (int I = 0; I < NumItems && !Found; I++)
				if (loctoupper(Item[I][ItemKeyPos[I]]) == loctoupper(CurKey))
					Found = true;
			if (!Found && CurKey != ' ')
				break;
			KeyPos++;
		}
		ItemKeyPos[NumItems] = KeyPos;
		NumItems++;
	}

	for (int I = 0; I < NumItems; I++) {
		eprintf(I == 0 ? (NumItems > 4 ? "\n" : " ") : ", ");
		int KeyPos = ItemKeyPos[I];
		for (int J = 0; J < KeyPos; J++)
			eprintf("%c", Item[I][J]);
		eprintf("[%c]%s", Item[I][KeyPos], &Item[I][KeyPos + 1]);
	}
	eprintf(" ");
	int Ch = GetKey();
	Ch = loctoupper(Ch);
	for (int I = 0; I < NumItems; I++)
		if (Ch == Item[I][ItemKeyPos[I]])
			return (I + 1);
	return (0);
}
#endif


int KbdAnsi(char *Addr, int Size) {
	int RetCode = 0;
#ifndef GUI
	for (int I = 0; I < Size; I++)
		if (Addr[I] == 27 && Addr[I + 1] == '[') {
			for (int J = I + 2; J < Size; J++) {
				if (Addr[J] == '\"')
					return (2);
				if (!isdigit(Addr[J]) && Addr[J] != ';')
					break;
			}
			RetCode = 1;
		}
#endif
	return (RetCode);
}


void OutComment(char *Comment, int Size) {
#ifndef GUI
	if (KbdAnsi(Comment, Size) == 2)
		return;
	const int MaxOutSize = 0x400;
	for (int I = 0; I < Size; I += MaxOutSize) {
		char Msg[MaxOutSize + 1];
		strncpy(Msg, Comment + I, MaxOutSize);
		Msg[Min(MaxOutSize, Size - I)] = 0;
		mprintf("%s", Msg);
	}
	mprintf("\n");
#endif
}


/***** File: options.cpp *****/

RAROptions::RAROptions() {
	Init();
}


RAROptions::~RAROptions() {
	memset(this, 0, sizeof(RAROptions));
}


void RAROptions::Init() {
	memset(this, 0, sizeof(RAROptions));
	WinSize = 0x400000;
	Overwrite = OVERWRITE_ASK;
	Method = 3;
	MsgStream = MSG_STDOUT;
	ConvertNames = NAMES_ORIGINALCASE;
	ProcessEA = true;
}


/***** File: ulinks.cpp *****/


int ExtractLink(ComprDataIO &DataIO, Archive &Arc, char *DestName, uint &LinkCRC, bool Create) {
	return (0);
}


/***** File: errhnd.cpp *****/

static bool UserBreak;

ErrorHandler::ErrorHandler() {
	Clean();
}


void ErrorHandler::Clean() {
	ExitCode = SUCCESS;
	ErrCount = 0;
	EnableBreak = true;
	Silent = false;
	DoShutdown = false;
}


void ErrorHandler::MemoryError() {
	MemoryErrorMsg();
	Throw(MEMORY_ERROR);
}


void ErrorHandler::OpenError(const char *FileName) {
#ifndef SILENT
	OpenErrorMsg(FileName);
	Throw(OPEN_ERROR);
#endif
}


void ErrorHandler::CloseError(const char *FileName) {
#ifndef SILENT
	ErrMsg(NULL, St(MErrFClose), FileName);
	Throw(FATAL_ERROR);
#endif
}


void ErrorHandler::ReadError(const char *FileName) {
#ifndef SILENT
	ReadErrorMsg(FileName);
	Throw(FATAL_ERROR);
#endif
}


bool ErrorHandler::AskRepeatRead(const char *FileName) {
#if !defined(SILENT) && !defined(SFX_MODULE)
	if (!Silent) {
		mprintf("\n");
		Log(NULL, St(MErrRead), FileName);
		return (Ask(St(MRetryAbort)) == 1);
	}
#endif
	return (false);
}


void ErrorHandler::WriteError(const char *FileName) {
#ifndef SILENT
	ErrMsg(NULL, St(MErrWrite), FileName);
	Throw(WRITE_ERROR);
#endif
}


bool ErrorHandler::AskRepeatWrite(const char *FileName) {
#ifndef SILENT
	if (!Silent) {
		mprintf("\n");
		Log(NULL, St(MErrWrite), FileName);
		return (Ask(St(MRetryAbort)) == 1);
	}
#endif
	return (false);
}


void ErrorHandler::SeekError(const char *FileName) {
#ifndef SILENT
	ErrMsg(NULL, St(MErrSeek), FileName);
	Throw(FATAL_ERROR);
#endif
}


void ErrorHandler::MemoryErrorMsg() {
#ifndef SILENT
	ErrMsg(NULL, St(MErrOutMem));
#endif
}


void ErrorHandler::OpenErrorMsg(const char *FileName) {
#ifndef SILENT
	Log(NULL, St(MCannotOpen), FileName);
	Alarm();
#endif
}


void ErrorHandler::CreateErrorMsg(const char *FileName) {
#ifndef SILENT
	Log(NULL, St(MCannotCreate), FileName);
	Alarm();
#endif
}


void ErrorHandler::ReadErrorMsg(const char *FileName) {
#ifndef SILENT
	ErrMsg(NULL, St(MErrRead), FileName);
#endif
}


void ErrorHandler::Exit(int ExitCode) {
#ifndef SFX_MODULE
	Alarm();
#endif
	Throw(ExitCode);
}


#ifndef GUI
void ErrorHandler::ErrMsg(char *ArcName, const char *fmt, ...) {
	char Msg[NM + 256];
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(Msg, fmt, argptr);
	va_end(argptr);
	Alarm();
	if (*Msg) {
		Log(ArcName, "\n%s", Msg);
		mprintf("\n%s\n", St(MProgAborted));
	}
}
#endif


void ErrorHandler::SetErrorCode(int Code) {
	switch (Code) {
	case WARNING:
	case USER_BREAK:
		if (ExitCode == SUCCESS)
			ExitCode = Code;
		break;
	case FATAL_ERROR:
		if (ExitCode == SUCCESS || ExitCode == WARNING)
			ExitCode = FATAL_ERROR;
		break;
	default:
		ExitCode = Code;
		break;
	}
	ErrCount++;
}


#if !defined(GUI)
void _stdfunction ProcessSignal(int SigType) {
	UserBreak = true;
	mprintf(St(MBreak));
	File::RemoveCreated();
	exit(USER_BREAK);
}
#endif


void ErrorHandler::SetSignalHandlers(bool Enable) {
	EnableBreak = Enable;
#if !defined(GUI)
	signal(SIGINT, Enable ? ProcessSignal : SIG_IGN);
	signal(SIGTERM, Enable ? ProcessSignal : SIG_IGN);
#endif
}


void ErrorHandler::Throw(int Code) {
	if (Code == USER_BREAK && !EnableBreak)
		return;
	ErrHandler.SetErrorCode(Code);

	File::RemoveCreated();
	exit(Code);
}


/***** File: rarvm.cpp *****/

#define VMCF_OP0             0
#define VMCF_OP1             1
#define VMCF_OP2             2
#define VMCF_OPMASK          3
#define VMCF_BYTEMODE        4
#define VMCF_JUMP            8
#define VMCF_PROC           16
#define VMCF_USEFLAGS       32
#define VMCF_CHFLAGS        64

static byte VM_CmdFlags[] = {
	/* VM_MOV   */ VMCF_OP2 | VMCF_BYTEMODE,
	/* VM_CMP   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_ADD   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_SUB   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_JZ    */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_JNZ   */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_INC   */ VMCF_OP1 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_DEC   */ VMCF_OP1 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_JMP   */ VMCF_OP1 | VMCF_JUMP,
	/* VM_XOR   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_AND   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_OR    */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_TEST  */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_JS    */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_JNS   */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_JB    */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_JBE   */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_JA    */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_JAE   */ VMCF_OP1 | VMCF_JUMP | VMCF_USEFLAGS,
	/* VM_PUSH  */ VMCF_OP1,
	/* VM_POP   */ VMCF_OP1,
	/* VM_CALL  */ VMCF_OP1 | VMCF_PROC,
	/* VM_RET   */ VMCF_OP0 | VMCF_PROC,
	/* VM_NOT   */ VMCF_OP1 | VMCF_BYTEMODE,
	/* VM_SHL   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_SHR   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_SAR   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_NEG   */ VMCF_OP1 | VMCF_BYTEMODE | VMCF_CHFLAGS,
	/* VM_PUSHA */ VMCF_OP0,
	/* VM_POPA  */ VMCF_OP0,
	/* VM_PUSHF */ VMCF_OP0 | VMCF_USEFLAGS,
	/* VM_POPF  */ VMCF_OP0 | VMCF_CHFLAGS,
	/* VM_MOVZX */ VMCF_OP2,
	/* VM_MOVSX */ VMCF_OP2,
	/* VM_XCHG  */ VMCF_OP2 | VMCF_BYTEMODE,
	/* VM_MUL   */ VMCF_OP2 | VMCF_BYTEMODE,
	/* VM_DIV   */ VMCF_OP2 | VMCF_BYTEMODE,
	/* VM_ADC   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_USEFLAGS | VMCF_CHFLAGS,
	/* VM_SBB   */ VMCF_OP2 | VMCF_BYTEMODE | VMCF_USEFLAGS | VMCF_CHFLAGS,
	/* VM_PRINT */ VMCF_OP0
};

RarVM::RarVM() {
	Mem = NULL;
}


RarVM::~RarVM() {
	delete Mem;
}


void RarVM::Init() {
	if (Mem == NULL)
		Mem = new byte[VM_MEMSIZE + 4];
}


inline uint RarVM::GetValue(bool ByteMode, uint *Addr) {
	if (ByteMode)
		return (*(byte *)Addr);
	else {
#ifdef BIG_ENDIAN
		byte *B = (byte *)Addr;
		return ((uint)B[0] | ((uint)B[1] << 8) | ((uint)B[2] << 16) | ((uint)B[3] << 24));
#else
		return (*Addr);
#endif
	}
}

#ifdef BIG_ENDIAN
#define GET_VALUE(ByteMode,Addr) GetValue(ByteMode,(uint *)Addr)
#else
#define GET_VALUE(ByteMode,Addr) ((ByteMode) ? (*(byte *)(Addr)):(*(uint *)(Addr)))
#endif


inline void RarVM::SetValue(bool ByteMode, uint *Addr, uint Value) {
	if (ByteMode)
		*(byte *)Addr = Value;
	else {
#ifdef BIG_ENDIAN
		((byte *)Addr)[0] = (byte)Value;
		((byte *)Addr)[1] = (byte)(Value >> 8);
		((byte *)Addr)[2] = (byte)(Value >> 16);
		((byte *)Addr)[3] = (byte)(Value >> 24);
#else
		*Addr = Value;
#endif
	}
}

#ifdef BIG_ENDIAN
#define SET_VALUE(ByteMode,Addr,Value) SetValue(ByteMode,(uint *)Addr,Value)
#else
#define SET_VALUE(ByteMode,Addr,Value) ((ByteMode) ? (*(byte *)(Addr)=(Value)):(*(uint *)(Addr)=(Value)))
#endif


void RarVM::SetValue(uint *Addr, uint Value) {
	SetValue(false, Addr, Value);
}


inline uint *RarVM::GetOperand(VM_PreparedOperand *CmdOp) {
	if (CmdOp->Type == VM_OPREGMEM)
		return ((uint *)&Mem[(*CmdOp->Addr + CmdOp->Base)&VM_MEMMASK]);
	else
		return (CmdOp->Addr);
}


void RarVM::Execute(VM_PreparedProgram *Prg) {
	memcpy(R, Prg->InitR, sizeof(Prg->InitR));
	unsigned int GlobalSize = Min(Prg->GlobalData.Size(), VM_GLOBALMEMSIZE);
	if (GlobalSize)
		memcpy(Mem + VM_GLOBALMEMADDR, &Prg->GlobalData[0], GlobalSize);
	unsigned int StaticSize = Min(Prg->StaticData.Size(), VM_GLOBALMEMSIZE - GlobalSize);
	if (StaticSize)
		memcpy(Mem + VM_GLOBALMEMADDR + GlobalSize, &Prg->StaticData[0], StaticSize);

	R[7] = VM_MEMSIZE;
	Flags = 0;

	VM_PreparedCommand *PreparedCode = Prg->AltCmd ? Prg->AltCmd : &Prg->Cmd[0];
	if (!ExecuteCode(PreparedCode, Prg->CmdCount))
		PreparedCode[0].OpCode = VM_RET;
	uint NewBlockPos = GET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x20])&VM_MEMMASK;
	uint NewBlockSize = GET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x1c])&VM_MEMMASK;
	if (NewBlockPos + NewBlockSize >= VM_MEMSIZE)
		NewBlockPos = NewBlockSize = 0;
	Prg->FilteredData = Mem + NewBlockPos;
	Prg->FilteredDataSize = NewBlockSize;

	Prg->GlobalData.Reset();
	uint DataSize = Min(GET_VALUE(false, (uint *)&Mem[VM_GLOBALMEMADDR + 0x30]), VM_GLOBALMEMSIZE);
	if (DataSize != 0) {
		Prg->GlobalData.Add(DataSize + VM_FIXEDGLOBALSIZE);
		memcpy(&Prg->GlobalData[0], &Mem[VM_GLOBALMEMADDR], DataSize + VM_FIXEDGLOBALSIZE);
	}
}


#define SET_IP(IP)                      \
	if ((IP)>=CodeSize)                   \
		return(true);                       \
	if (--MaxOpCount<=0)                  \
		return(false);                      \
	Cmd=PreparedCode+(IP);

bool RarVM::ExecuteCode(VM_PreparedCommand *PreparedCode, int CodeSize) {
	int MaxOpCount = 25000000;
	VM_PreparedCommand *Cmd = PreparedCode;
	while (1) {
		uint *Op1 = GetOperand(&Cmd->Op1);
		uint *Op2 = GetOperand(&Cmd->Op2);
		switch (Cmd->OpCode) {
#ifndef NORARVM
		case VM_MOV:
			SET_VALUE(Cmd->ByteMode, Op1, GET_VALUE(Cmd->ByteMode, Op2));
			break;
#ifdef VM_OPTIMIZE
		case VM_MOVB:
			SET_VALUE(true, Op1, GET_VALUE(true, Op2));
			break;
		case VM_MOVD:
			SET_VALUE(false, Op1, GET_VALUE(false, Op2));
			break;
#endif
		case VM_CMP: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint Result = Value1 - GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : (Result > Value1) | (Result & VM_FS);
		}
		break;
#ifdef VM_OPTIMIZE
		case VM_CMPB: {
			uint Value1 = GET_VALUE(true, Op1);
			uint Result = Value1 - GET_VALUE(true, Op2);
			Flags = Result == 0 ? VM_FZ : (Result > Value1) | (Result & VM_FS);
		}
		break;
		case VM_CMPD: {
			uint Value1 = GET_VALUE(false, Op1);
			uint Result = Value1 - GET_VALUE(false, Op2);
			Flags = Result == 0 ? VM_FZ : (Result > Value1) | (Result & VM_FS);
		}
		break;
#endif
		case VM_ADD: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint Result = Value1 + GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : (Result < Value1) | (Result & VM_FS);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
#ifdef VM_OPTIMIZE
		case VM_ADDB:
			SET_VALUE(true, Op1, GET_VALUE(true, Op1) + GET_VALUE(true, Op2));
			break;
		case VM_ADDD:
			SET_VALUE(false, Op1, GET_VALUE(false, Op1) + GET_VALUE(false, Op2));
			break;
#endif
		case VM_SUB: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint Result = Value1 - GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : (Result > Value1) | (Result & VM_FS);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
#ifdef VM_OPTIMIZE
		case VM_SUBB:
			SET_VALUE(true, Op1, GET_VALUE(true, Op1) - GET_VALUE(true, Op2));
			break;
		case VM_SUBD:
			SET_VALUE(false, Op1, GET_VALUE(false, Op1) - GET_VALUE(false, Op2));
			break;
#endif
		case VM_JZ:
			if ((Flags & VM_FZ) != 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_JNZ:
			if ((Flags & VM_FZ) == 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_INC: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1) + 1;
			SET_VALUE(Cmd->ByteMode, Op1, Result);
			Flags = Result == 0 ? VM_FZ : Result & VM_FS;
		}
		break;
#ifdef VM_OPTIMIZE
		case VM_INCB:
			SET_VALUE(true, Op1, GET_VALUE(true, Op1) + 1);
			break;
		case VM_INCD:
			SET_VALUE(false, Op1, GET_VALUE(false, Op1) + 1);
			break;
#endif
		case VM_DEC: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1) - 1;
			SET_VALUE(Cmd->ByteMode, Op1, Result);
			Flags = Result == 0 ? VM_FZ : Result & VM_FS;
		}
		break;
#ifdef VM_OPTIMIZE
		case VM_DECB:
			SET_VALUE(true, Op1, GET_VALUE(true, Op1) - 1);
			break;
		case VM_DECD:
			SET_VALUE(false, Op1, GET_VALUE(false, Op1) - 1);
			break;
#endif
		case VM_JMP:
			SET_IP(GET_VALUE(false, Op1));
			continue;
		case VM_XOR: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1)^GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : Result & VM_FS;
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_AND: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1)&GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : Result & VM_FS;
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_OR: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1) | GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : Result & VM_FS;
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_TEST: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1)&GET_VALUE(Cmd->ByteMode, Op2);
			Flags = Result == 0 ? VM_FZ : Result & VM_FS;
		}
		break;
		case VM_JS:
			if ((Flags & VM_FS) != 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_JNS:
			if ((Flags & VM_FS) == 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_JB:
			if ((Flags & VM_FC) != 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_JBE:
			if ((Flags & (VM_FC | VM_FZ)) != 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_JA:
			if ((Flags & (VM_FC | VM_FZ)) == 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_JAE:
			if ((Flags & VM_FC) == 0) {
				SET_IP(GET_VALUE(false, Op1));
				continue;
			}
			break;
		case VM_PUSH:
			R[7] -= 4;
			SET_VALUE(false, (uint *)&Mem[R[7]&VM_MEMMASK], GET_VALUE(false, Op1));
			break;
		case VM_POP:
			SET_VALUE(false, Op1, GET_VALUE(false, (uint *)&Mem[R[7] & VM_MEMMASK]));
			R[7] += 4;
			break;
		case VM_CALL:
			R[7] -= 4;
			SET_VALUE(false, (uint *)&Mem[R[7]&VM_MEMMASK], Cmd - PreparedCode + 1);
			SET_IP(GET_VALUE(false, Op1));
			continue;
		case VM_NOT:
			SET_VALUE(Cmd->ByteMode, Op1, ~GET_VALUE(Cmd->ByteMode, Op1));
			break;
		case VM_SHL: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint Value2 = GET_VALUE(Cmd->ByteMode, Op2);
			uint Result = Value1 << Value2;
			Flags = (Result == 0 ? VM_FZ : (Result & VM_FS)) | ((Value1 << (Value2 - 1)) & 0x80000000 ? VM_FC : 0);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_SHR: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint Value2 = GET_VALUE(Cmd->ByteMode, Op2);
			uint Result = Value1 >> Value2;
			Flags = (Result == 0 ? VM_FZ : (Result & VM_FS)) | ((Value1 >> (Value2 - 1))&VM_FC);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_SAR: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint Value2 = GET_VALUE(Cmd->ByteMode, Op2);
			uint Result = ((int)Value1) >> Value2;
			Flags = (Result == 0 ? VM_FZ : (Result & VM_FS)) | ((Value1 >> (Value2 - 1))&VM_FC);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_NEG: {
			uint Result = -GET_VALUE(Cmd->ByteMode, Op1);
			Flags = Result == 0 ? VM_FZ : VM_FC | (Result & VM_FS);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
#ifdef VM_OPTIMIZE
		case VM_NEGB:
			SET_VALUE(true, Op1, -GET_VALUE(true, Op1));
			break;
		case VM_NEGD:
			SET_VALUE(false, Op1, -GET_VALUE(false, Op1));
			break;
#endif
		case VM_PUSHA: {
			const int RegCount = sizeof(R) / sizeof(R[0]);
			for (int I = 0, SP = R[7] - 4; I < RegCount; I++, SP -= 4)
				SET_VALUE(false, (uint *)&Mem[SP & VM_MEMMASK], R[I]);
			R[7] -= RegCount * 4;
		}
		break;
		case VM_POPA: {
			const int RegCount = sizeof(R) / sizeof(R[0]);
			for (uint I = 0, SP = R[7]; I < RegCount; I++, SP += 4)
				R[7 - I] = GET_VALUE(false, (uint *)&Mem[SP & VM_MEMMASK]);
		}
		break;
		case VM_PUSHF:
			R[7] -= 4;
			SET_VALUE(false, (uint *)&Mem[R[7]&VM_MEMMASK], Flags);
			break;
		case VM_POPF:
			Flags = GET_VALUE(false, (uint *)&Mem[R[7] & VM_MEMMASK]);
			R[7] += 4;
			break;
		case VM_MOVZX:
			SET_VALUE(false, Op1, GET_VALUE(true, Op2));
			break;
		case VM_MOVSX:
			SET_VALUE(false, Op1, (signed char)GET_VALUE(true, Op2));
			break;
		case VM_XCHG: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			SET_VALUE(Cmd->ByteMode, Op1, GET_VALUE(Cmd->ByteMode, Op2));
			SET_VALUE(Cmd->ByteMode, Op2, Value1);
		}
		break;
		case VM_MUL: {
			uint Result = GET_VALUE(Cmd->ByteMode, Op1) * GET_VALUE(Cmd->ByteMode, Op2);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_DIV: {
			uint Divider = GET_VALUE(Cmd->ByteMode, Op2);
			if (Divider != 0) {
				uint Result = GET_VALUE(Cmd->ByteMode, Op1) / Divider;
				SET_VALUE(Cmd->ByteMode, Op1, Result);
			}
		}
		break;
		case VM_ADC: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint FC = (Flags & VM_FC);
			uint Result = Value1 + GET_VALUE(Cmd->ByteMode, Op2) + FC;
			Flags = Result == 0 ? VM_FZ : (Result < Value1 || Result == Value1 && FC) | (Result & VM_FS);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
		case VM_SBB: {
			uint Value1 = GET_VALUE(Cmd->ByteMode, Op1);
			uint FC = (Flags & VM_FC);
			uint Result = Value1 - GET_VALUE(Cmd->ByteMode, Op2) - FC;
			Flags = Result == 0 ? VM_FZ : (Result > Value1 || Result == Value1 && FC) | (Result & VM_FS);
			SET_VALUE(Cmd->ByteMode, Op1, Result);
		}
		break;
#endif
		case VM_RET:
			if (R[7] >= VM_MEMSIZE)
				return (true);
			SET_IP(GET_VALUE(false, (uint *)&Mem[R[7] & VM_MEMMASK]));
			R[7] += 4;
			continue;
#ifdef VM_STANDARDFILTERS
		case VM_STANDARD:
			ExecuteStandardFilter((VM_StandardFilters)Cmd->Op1.Data);
			break;
#endif
		case VM_PRINT:
			break;
		}
		Cmd++;
		--MaxOpCount;
	}
}


void RarVM::PrintState(uint IP) {
}


void RarVM::Prepare(byte *Code, int CodeSize, VM_PreparedProgram *Prg) {
	InitBitInput();
	memcpy(InBuf, Code, Min(CodeSize, BitInput::MAX_SIZE));

	byte XorSum = 0;
	for (int I = 1; I < CodeSize; I++)
		XorSum ^= Code[I];

	faddbits(8);

	Prg->CmdCount = 0;
	if (XorSum == Code[0]) {
#ifdef VM_STANDARDFILTERS
		VM_StandardFilters FilterType = IsStandardFilter(Code, CodeSize);
		if (FilterType != VMSF_NONE) {
			Prg->Cmd.Add(1);
			VM_PreparedCommand *CurCmd = &Prg->Cmd[Prg->CmdCount++];
			CurCmd->OpCode = VM_STANDARD;
			CurCmd->Op1.Data = FilterType;
			CurCmd->Op1.Addr = &CurCmd->Op1.Data;
			CurCmd->Op2.Addr = &CurCmd->Op2.Data;
			CodeSize = 0;
		}
#endif
		uint DataFlag = fgetbits();
		faddbits(1);
		if (DataFlag & 0x8000) {
			int DataSize = ReadData(*this) + 1;
			for (int I = 0; InAddr < CodeSize && I < DataSize; I++) {
				Prg->StaticData.Add(1);
				Prg->StaticData[I] = fgetbits() >> 8;
				faddbits(8);
			}
		}
		while (InAddr < CodeSize) {
			Prg->Cmd.Add(1);
			VM_PreparedCommand *CurCmd = &Prg->Cmd[Prg->CmdCount];
			uint Data = fgetbits();
			if ((Data & 0x8000) == 0) {
				CurCmd->OpCode = (VM_Commands)(Data >> 12);
				faddbits(4);
			} else {
				CurCmd->OpCode = (VM_Commands)((Data >> 10) - 24);
				faddbits(6);
			}
			if (VM_CmdFlags[CurCmd->OpCode] & VMCF_BYTEMODE) {
				CurCmd->ByteMode = fgetbits() >> 15;
				faddbits(1);
			} else
				CurCmd->ByteMode = 0;
			CurCmd->Op1.Type = CurCmd->Op2.Type = VM_OPNONE;
			int OpNum = (VM_CmdFlags[CurCmd->OpCode] & VMCF_OPMASK);
			CurCmd->Op1.Addr = &CurCmd->Op1.Data;
			CurCmd->Op2.Addr = &CurCmd->Op2.Data;
			if (OpNum > 0) {
				DecodeArg(CurCmd->Op1, CurCmd->ByteMode);
				if (OpNum == 2)
					DecodeArg(CurCmd->Op2, CurCmd->ByteMode);
				else {
					if (CurCmd->Op1.Type == VM_OPINT && (VM_CmdFlags[CurCmd->OpCode] & (VMCF_JUMP | VMCF_PROC))) {
						int Distance = CurCmd->Op1.Data;
						if (Distance >= 256)
							Distance -= 256;
						else {
							if (Distance >= 136)
								Distance -= 264;
							else if (Distance >= 16)
								Distance -= 8;
							else if (Distance >= 8)
								Distance -= 16;
							Distance += Prg->CmdCount;
						}
						CurCmd->Op1.Data = Distance;
					}
				}
			}
			Prg->CmdCount++;
		}
	}
	Prg->Cmd.Add(1);
	VM_PreparedCommand *CurCmd = &Prg->Cmd[Prg->CmdCount++];
	CurCmd->OpCode = VM_RET;
	CurCmd->Op1.Addr = &CurCmd->Op1.Data;
	CurCmd->Op2.Addr = &CurCmd->Op2.Data;

#ifdef VM_OPTIMIZE
	if (CodeSize != 0)
		Optimize(Prg);
#endif
}


void RarVM::DecodeArg(VM_PreparedOperand &Op, bool ByteMode) {
	uint Data = fgetbits();
	if (Data & 0x8000) {
		Op.Type = VM_OPREG;
		Op.Data = (Data >> 12) & 7;
		Op.Addr = &R[Op.Data];
		faddbits(4);
	} else if ((Data & 0xc000) == 0) {
		Op.Type = VM_OPINT;
		if (ByteMode) {
			Op.Data = (Data >> 6) & 0xff;
			faddbits(10);
		} else {
			faddbits(2);
			Op.Data = ReadData(*this);
		}
		Op.Addr = &Op.Data;
	} else {
		Op.Type = VM_OPREGMEM;
		if ((Data & 0x2000) == 0) {
			Op.Data = (Data >> 10) & 7;
			Op.Addr = &R[Op.Data];
			Op.Base = 0;
			faddbits(6);
		} else {
			if ((Data & 0x1000) == 0) {
				Op.Data = (Data >> 9) & 7;
				Op.Addr = &R[Op.Data];
				faddbits(7);
			} else {
				Op.Data = 0;
				Op.Addr = &Op.Data;
				faddbits(4);
			}
			Op.Base = ReadData(*this);
		}
	}
}


uint RarVM::ReadData(BitInput &Inp) {
	uint Data = Inp.fgetbits();
	switch (Data & 0xc000) {
	case 0:
		Inp.faddbits(6);
		return ((Data >> 10) & 0xf);
	case 0x4000:
		if ((Data & 0x3c00) == 0) {
			Data = 0xffffff00 | ((Data >> 2) & 0xff);
			Inp.faddbits(14);
		} else {
			Data = (Data >> 6) & 0xff;
			Inp.faddbits(10);
		}
		return (Data);
	case 0x8000:
		Inp.faddbits(2);
		Data = Inp.fgetbits();
		Inp.faddbits(16);
		return (Data);
	default:
		Inp.faddbits(2);
		Data = (Inp.fgetbits() << 16);
		Inp.faddbits(16);
		Data |= Inp.fgetbits();
		Inp.faddbits(16);
		return (Data);
	}
}


void RarVM::SetMemory(unsigned int Pos, byte *Data, unsigned int DataSize) {
	if (Pos < VM_MEMSIZE && Data != Mem + Pos)
		memmove(Mem + Pos, Data, Min(DataSize, VM_MEMSIZE - Pos));
}


#ifdef VM_OPTIMIZE
void RarVM::Optimize(VM_PreparedProgram *Prg) {
	VM_PreparedCommand *Code = &Prg->Cmd[0];
	int CodeSize = Prg->CmdCount;

	for (int I = 0; I < CodeSize; I++) {
		VM_PreparedCommand *Cmd = Code + I;
		switch (Cmd->OpCode) {
		case VM_MOV:
			Cmd->OpCode = Cmd->ByteMode ? VM_MOVB : VM_MOVD;
			continue;
		case VM_CMP:
			Cmd->OpCode = Cmd->ByteMode ? VM_CMPB : VM_CMPD;
			continue;
		}
		if ((VM_CmdFlags[Cmd->OpCode] & VMCF_CHFLAGS) == 0)
			continue;
		bool FlagsRequired = false;
		for (int J = I + 1; J < CodeSize; J++) {
			int Flags = VM_CmdFlags[Code[J].OpCode];
			if (Flags & (VMCF_JUMP | VMCF_PROC | VMCF_USEFLAGS)) {
				FlagsRequired = true;
				break;
			}
			if (Flags & VMCF_CHFLAGS)
				break;
		}
		if (FlagsRequired)
			continue;
		switch (Cmd->OpCode) {
		case VM_ADD:
			Cmd->OpCode = Cmd->ByteMode ? VM_ADDB : VM_ADDD;
			continue;
		case VM_SUB:
			Cmd->OpCode = Cmd->ByteMode ? VM_SUBB : VM_SUBD;
			continue;
		case VM_INC:
			Cmd->OpCode = Cmd->ByteMode ? VM_INCB : VM_INCD;
			continue;
		case VM_DEC:
			Cmd->OpCode = Cmd->ByteMode ? VM_DECB : VM_DECD;
			continue;
		case VM_NEG:
			Cmd->OpCode = Cmd->ByteMode ? VM_NEGB : VM_NEGD;
			continue;
		}
	}
}
#endif


#ifdef VM_STANDARDFILTERS
VM_StandardFilters RarVM::IsStandardFilter(byte *Code, int CodeSize) {
	struct StandardFilterSignature {
		int Length;
		uint CRC;
		VM_StandardFilters Type;
	} StdList[] = {
		53, 0xad576887, VMSF_E8,
		57, 0x3cd7e57e, VMSF_E8E9,
		120, 0x3769893f, VMSF_ITANIUM,
		29, 0x0e06077d, VMSF_DELTA,
		149, 0x1c2c5dc8, VMSF_RGB,
		216, 0xbc85e701, VMSF_AUDIO,
		40, 0x46b9c560, VMSF_UPCASE
	};
	uint CodeCRC = CRC(0xffffffff, Code, CodeSize) ^ 0xffffffff;
	for (int I = 0; I < sizeof(StdList) / sizeof(StdList[0]); I++)
		if (StdList[I].CRC == CodeCRC && StdList[I].Length == CodeSize)
			return (StdList[I].Type);
	return (VMSF_NONE);
}


void RarVM::ExecuteStandardFilter(VM_StandardFilters FilterType) {
	switch (FilterType) {
	case VMSF_E8:
	case VMSF_E8E9: {
		byte *Data = Mem;
		int DataSize = R[4];
		uint FileOffset = R[6];

		if (DataSize >= VM_GLOBALMEMADDR)
			break;

		const int FileSize = 0x1000000;
		byte CmpByte2 = FilterType == VMSF_E8E9 ? 0xe9 : 0xe8;
		for (uint CurPos = 0; CurPos < DataSize - 4;) {
			byte CurByte = *(Data++);
			CurPos++;
			if (CurByte == 0xe8 || CurByte == CmpByte2) {
				long Offset = CurPos + FileOffset;
				long Addr = GET_VALUE(false, Data);
				if (Addr < 0) {
					if (Addr + Offset >= 0)
						SET_VALUE(false, Data, Addr + FileSize);
				} else if (Addr < FileSize)
					SET_VALUE(false, Data, Addr - Offset);
				Data += 4;
				CurPos += 4;
			}
		}
	}
	break;
	case VMSF_ITANIUM: {
		byte *Data = Mem;
		int DataSize = R[4];
		uint FileOffset = R[6];

		if (DataSize >= VM_GLOBALMEMADDR)
			break;

		uint CurPos = 0;

		FileOffset >>= 4;

		while (CurPos < DataSize - 21) {
			int Byte = (Data[0] & 0x1f) - 0x10;
			if (Byte >= 0) {
				static byte Masks[16] = {4, 4, 6, 6, 0, 0, 7, 7, 4, 4, 0, 0, 4, 4, 0, 0};
				byte CmdMask = Masks[Byte];
				if (CmdMask != 0)
					for (int I = 0; I <= 2; I++)
						if (CmdMask & (1 << I)) {
							int StartPos = I * 41 + 5;
							int OpType = FilterItanium_GetBits(Data, StartPos + 37, 4);
							if (OpType == 5) {
								int Offset = FilterItanium_GetBits(Data, StartPos + 13, 20);
								FilterItanium_SetBits(Data, (Offset - FileOffset) & 0xfffff, StartPos + 13, 20);
							}
						}
			}
			Data += 16;
			CurPos += 16;
			FileOffset++;
		}
	}
	break;
	case VMSF_DELTA: {
		int DataSize = R[4], Channels = R[0], SrcPos = 0, Border = DataSize * 2;
		SET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x20], DataSize);
		if (DataSize >= VM_GLOBALMEMADDR / 2)
			break;
		for (int CurChannel = 0; CurChannel < Channels; CurChannel++) {
			byte PrevByte = 0;
			for (int DestPos = DataSize + CurChannel; DestPos < Border; DestPos += Channels)
				Mem[DestPos] = (PrevByte -= Mem[SrcPos++]);
		}
	}
	break;
	case VMSF_RGB: {
		int DataSize = R[4], Width = R[0] - 3, PosR = R[1];
		byte *SrcData = Mem, *DestData = SrcData + DataSize;
		const int Channels = 3;
		SET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x20], DataSize);
		if (DataSize >= VM_GLOBALMEMADDR / 2)
			break;
		for (int CurChannel = 0; CurChannel < Channels; CurChannel++) {
			unsigned int PrevByte = 0;

			for (int I = CurChannel; I < DataSize; I += Channels) {
				unsigned int Predicted;
				int UpperPos = I - Width;
				if (UpperPos >= 3) {
					byte *UpperData = DestData + UpperPos;
					unsigned int UpperByte = *UpperData;
					unsigned int UpperLeftByte = *(UpperData - 3);
					Predicted = PrevByte + UpperByte - UpperLeftByte;
					int pa = abs((int)(Predicted - PrevByte));
					int pb = abs((int)(Predicted - UpperByte));
					int pc = abs((int)(Predicted - UpperLeftByte));
					if (pa <= pb && pa <= pc)
						Predicted = PrevByte;
					else if (pb <= pc)
						Predicted = UpperByte;
					else
						Predicted = UpperLeftByte;
				} else
					Predicted = PrevByte;
				DestData[I] = PrevByte = (byte)(Predicted - * (SrcData++));
			}
		}
		for (int I = PosR, Border = DataSize - 2; I < Border; I += 3) {
			byte G = DestData[I + 1];
			DestData[I] += G;
			DestData[I + 2] += G;
		}
	}
	break;
	case VMSF_AUDIO: {
		int DataSize = R[4], Channels = R[0];
		byte *SrcData = Mem, *DestData = SrcData + DataSize;
		SET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x20], DataSize);
		if (DataSize >= VM_GLOBALMEMADDR / 2)
			break;
		for (int CurChannel = 0; CurChannel < Channels; CurChannel++) {
			unsigned int PrevByte = 0, PrevDelta = 0, Dif[7];
			int D1 = 0, D2 = 0, D3;
			int K1 = 0, K2 = 0, K3 = 0;
			memset(Dif, 0, sizeof(Dif));

			for (int I = CurChannel, ByteCount = 0; I < DataSize; I += Channels, ByteCount++) {
				D3 = D2;
				D2 = PrevDelta - D1;
				D1 = PrevDelta;

				unsigned int Predicted = 8 * PrevByte + K1 * D1 + K2 * D2 + K3 * D3;
				Predicted = (Predicted >> 3) & 0xff;

				unsigned int CurByte = *(SrcData++);

				Predicted -= CurByte;
				DestData[I] = Predicted;
				PrevDelta = (signed char)(Predicted - PrevByte);
				PrevByte = Predicted;

				int D = ((signed char)CurByte) << 3;

				Dif[0] += abs(D);
				Dif[1] += abs(D - D1);
				Dif[2] += abs(D + D1);
				Dif[3] += abs(D - D2);
				Dif[4] += abs(D + D2);
				Dif[5] += abs(D - D3);
				Dif[6] += abs(D + D3);

				if ((ByteCount & 0x1f) == 0) {
					unsigned int MinDif = Dif[0], NumMinDif = 0;
					Dif[0] = 0;
					for (int J = 1; J < sizeof(Dif) / sizeof(Dif[0]); J++) {
						if (Dif[J] < MinDif) {
							MinDif = Dif[J];
							NumMinDif = J;
						}
						Dif[J] = 0;
					}
					switch (NumMinDif) {
					case 1:
						if (K1 >= -16) K1--;
						break;
					case 2:
						if (K1 < 16) K1++;
						break;
					case 3:
						if (K2 >= -16) K2--;
						break;
					case 4:
						if (K2 < 16) K2++;
						break;
					case 5:
						if (K3 >= -16) K3--;
						break;
					case 6:
						if (K3 < 16) K3++;
						break;
					}
				}
			}
		}
	}
	break;
	case VMSF_UPCASE: {
		int DataSize = R[4], SrcPos = 0, DestPos = DataSize;
		if (DataSize >= VM_GLOBALMEMADDR / 2)
			break;
		while (SrcPos < DataSize) {
			byte CurByte = Mem[SrcPos++];
			if (CurByte == 2 && (CurByte = Mem[SrcPos++]) != 2)
				CurByte -= 32;
			Mem[DestPos++] = CurByte;
		}
		SET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x1c], DestPos - DataSize);
		SET_VALUE(false, &Mem[VM_GLOBALMEMADDR + 0x20], DataSize);
	}
	break;
	}
}


unsigned int RarVM::FilterItanium_GetBits(byte *Data, int BitPos, int BitCount) {
	int InAddr = BitPos / 8;
	int InBit = BitPos & 7;
	unsigned int BitField = (uint)Data[InAddr++];
	BitField |= (uint)Data[InAddr++] << 8;
	BitField |= (uint)Data[InAddr++] << 16;
	BitField |= (uint)Data[InAddr] << 24;
	BitField >>= InBit;
	return (BitField & (0xffffffff >> (32 - BitCount)));
}


void RarVM::FilterItanium_SetBits(byte *Data, unsigned int BitField, int BitPos,
                                  int BitCount) {
	int InAddr = BitPos / 8;
	int InBit = BitPos & 7;
	unsigned int AndMask = 0xffffffff >> (32 - BitCount);
	AndMask = ~(AndMask << InBit);

	BitField <<= InBit;

	for (int I = 0; I < 4; I++) {
		Data[InAddr + I] &= AndMask;
		Data[InAddr + I] |= BitField;
		AndMask = (AndMask >> 8) | 0xff000000;
		BitField >>= 8;
	}
}
#endif


/***** File: getbits.cpp *****/

BitInput::BitInput() {
	InBuf = new byte[MAX_SIZE];
}


BitInput::~BitInput() {
	delete[] InBuf;
}


void BitInput::faddbits(int Bits) {
	addbits(Bits);
}


unsigned int BitInput::fgetbits() {
	return (getbits());
}


/***** File: extinfo.cpp *****/

void ExtractUnixOwner(Archive &Arc, char *FileName) {
	if (Arc.HeaderCRC != Arc.UOHead.HeadCRC) {
		Log(Arc.FileName, St(MOwnersBroken), FileName);
		ErrHandler.SetErrorCode(CRC_ERROR);
		return;
	}

	struct passwd *pw;
	if ((pw = getpwnam(Arc.UOHead.OwnerName)) == NULL) {
		Log(Arc.FileName, St(MErrGetOwnerID), Arc.UOHead.OwnerName);
		ErrHandler.SetErrorCode(WARNING);
		return;
	}
	uid_t OwnerID = pw->pw_uid;

	struct group *gr;
	if ((gr = getgrnam(Arc.UOHead.GroupName)) == NULL) {
		Log(Arc.FileName, St(MErrGetGroupID), Arc.UOHead.GroupName);
		ErrHandler.SetErrorCode(CRC_ERROR);
		return;
	}
	gid_t GroupID = gr->gr_gid;
	if (chown(FileName, OwnerID, GroupID) != 0) {
		Log(Arc.FileName, St(MSetOwnersError), FileName);
		ErrHandler.SetErrorCode(CRC_ERROR);
	}
}


void ExtractUnixOwnerNew(Archive &Arc, char *FileName) {
	char *OwnerName = (char *)&Arc.SubHead.SubData[0];
	int OwnerSize = strlen(OwnerName) + 1;
	int GroupSize = Arc.SubHead.SubData.Size() - OwnerSize;
	char GroupName[NM];
	strncpy(GroupName, (char *)&Arc.SubHead.SubData[OwnerSize], GroupSize);
	GroupName[GroupSize] = 0;

	struct passwd *pw;
	if ((pw = getpwnam(OwnerName)) == NULL) {
		Log(Arc.FileName, St(MErrGetOwnerID), OwnerName);
		ErrHandler.SetErrorCode(WARNING);
		return;
	}
	uid_t OwnerID = pw->pw_uid;

	struct group *gr;
	if ((gr = getgrnam(GroupName)) == NULL) {
		Log(Arc.FileName, St(MErrGetGroupID), GroupName);
		ErrHandler.SetErrorCode(CRC_ERROR);
		return;
	}
	gid_t GroupID = gr->gr_gid;
	if (chown(FileName, OwnerID, GroupID) != 0) {
		Log(Arc.FileName, St(MSetOwnersError), FileName);
		ErrHandler.SetErrorCode(CRC_ERROR);
	}
}


#ifndef SFX_MODULE
void SetExtraInfo(CommandData *Cmd, Archive &Arc, char *Name, wchar *NameW) {
	switch (Arc.SubBlockHead.SubType) {
	case UO_HEAD:
		if (Cmd->ProcessOwners)
			ExtractUnixOwner(Arc, Name);
		break;
	}
}
#endif


void SetExtraInfoNew(CommandData *Cmd, Archive &Arc, char *Name, wchar *NameW) {
	if (Cmd->ProcessOwners && Arc.SubHead.CmpName(SUBHEAD_TYPE_UOWNER))
		ExtractUnixOwnerNew(Arc, Name);
}


/***** File: extract.cpp *****/

CmdExtract::CmdExtract(): DataIO(NULL) {
	TotalFileCount = 0;
	Unp = new Unpack(&DataIO);
	Unp->Init(NULL);
}


CmdExtract::~CmdExtract() {
	delete Unp;
}


void CmdExtract::DoExtract(CommandData *Cmd) {
	DataIO.SetCurrentCommand(*Cmd->Command);

	struct FindData FD;
	while (Cmd->GetArcName(ArcName, ArcNameW, sizeof(ArcName)))
		if (FindFile::FastFind(ArcName, ArcNameW, &FD))
			DataIO.TotalArcSize += FD.Size;
	Cmd->ArcNames->Rewind();
	while (Cmd->GetArcName(ArcName, ArcNameW, sizeof(ArcName))) {
		while (ExtractArchive(Cmd) == EXTRACT_ARC_REPEAT)
			;
		if (FindFile::FastFind(ArcName, ArcNameW, &FD))
			DataIO.ProcessedArcSize += FD.Size;
	}

	if (TotalFileCount == 0) {
		mprintf(St(MExtrNoFiles));
		ErrHandler.SetErrorCode(WARNING);
	}
#ifndef GUI
	else if (*Cmd->Command == 'I')
		mprintf(St(MDone));
	else if (ErrHandler.GetErrorCount() == 0)
		mprintf(St(MExtrAllOk));
	else
		mprintf(St(MExtrTotalErr), ErrHandler.GetErrorCount());
#endif
}


void CmdExtract::ExtractArchiveInit(CommandData *Cmd, Archive &Arc) {
	DataIO.UnpArcSize = Arc.FileLength();

	FileCount = 0;
	MatchedArgs = 0;
#ifndef SFX_MODULE
	FirstFile = true;
#endif

	strcpy(Password, Cmd->Password);
	PasswordAll = (*Password != 0);

	DataIO.UnpVolume = false;

	PrevExtracted = false;
	SignatureFound = false;
	AllMatchesExact = true;
	ReconstructDone = false;
}


EXTRACT_ARC_CODE CmdExtract::ExtractArchive(CommandData *Cmd) {
	Archive Arc(Cmd);
	if (!Arc.WOpen(ArcName, ArcNameW))
		return (EXTRACT_ARC_NEXT);

	if (!Arc.IsArchive(true)) {
#ifndef GUI
		mprintf(St(MNotRAR), ArcName);
#endif
		if (CmpExt(ArcName, "rar"))
			ErrHandler.SetErrorCode(WARNING);
		return (EXTRACT_ARC_NEXT);
	}

#ifndef SFX_MODULE
	if (Arc.Volume && Arc.NotFirstVolume) {
		char FirstVolName[NM];

		VolNameToFirstName(ArcName, FirstVolName, (Arc.NewMhd.Flags & MHD_NEWNUMBERING));
		if (stricomp(ArcName, FirstVolName) != 0 && FileExist(FirstVolName) &&
		        Cmd->ArcNames->Search(FirstVolName, NULL, false))
			return (EXTRACT_ARC_NEXT);
	}
#endif
	ExtractArchiveInit(Cmd, Arc);

	if (*Cmd->Command == 'T' || *Cmd->Command == 'I')
		Cmd->Test = true;

#ifndef GUI
	if (*Cmd->Command == 'I')
		Cmd->DisablePercentage = true;
	else if (Cmd->Test)
		mprintf(St(MExtrTest), ArcName);
	else
		mprintf(St(MExtracting), ArcName);
#endif

	while (1) {
		int Size = Arc.ReadHeader();
		bool Repeat = false;
		if (!ExtractCurrentFile(Cmd, Arc, Size, Repeat))
			if (Repeat) {
				return (EXTRACT_ARC_REPEAT);
			} else
				break;
	}
	memset(Password, 0, sizeof(Password));
	return (EXTRACT_ARC_NEXT);
}


bool CmdExtract::ExtractCurrentFile(CommandData *Cmd, Archive &Arc, int HeaderSize, bool &Repeat) {
	char Command = *Cmd->Command;
	if (HeaderSize <= 0)
		if (DataIO.UnpVolume) {
#ifdef NOVOLUME
			return (false);
#else
			if (!MergeArchive(Arc, NULL, false, Command)) {
				ErrHandler.SetErrorCode(WARNING);
				return (false);
			}
			SignatureFound = false;
#endif
		} else
			return (false);
	int HeadType = Arc.GetHeaderType();
	if (HeadType != FILE_HEAD) {
		if (HeadType == AV_HEAD || HeadType == SIGN_HEAD)
			SignatureFound = true;
#ifndef SFX_MODULE
		if (HeadType == SUB_HEAD && PrevExtracted)
			SetExtraInfo(Cmd, Arc, DestFileName, *DestFileNameW ? DestFileNameW : NULL);
#endif
		if (HeadType == NEWSUB_HEAD) {
			if (Arc.SubHead.CmpName(SUBHEAD_TYPE_AV))
				SignatureFound = true;
#ifndef NOSUBBLOCKS
			if (PrevExtracted)
				SetExtraInfoNew(Cmd, Arc, DestFileName, *DestFileNameW ? DestFileNameW : NULL);
#endif
		}
		if (HeadType == ENDARC_HEAD)
			if (Arc.EndArcHead.Flags & EARC_NEXT_VOLUME) {
#ifndef NOVOLUME
				if (!MergeArchive(Arc, NULL, false, Command)) {
					ErrHandler.SetErrorCode(WARNING);
					return (false);
				}
				SignatureFound = false;
#endif
				Arc.Seek(Arc.CurBlockPos, SEEK_SET);
				return (true);
			} else
				return (false);
		Arc.SeekToNext();
		return (true);
	}
	PrevExtracted = false;

	if (SignatureFound ||
	        !Cmd->Recurse && MatchedArgs >= Cmd->FileArgs->ItemsCount() &&
	        AllMatchesExact)
		return (false);

	char ArcFileName[NM];
	IntToExt(Arc.NewLhd.FileName, Arc.NewLhd.FileName);
	strcpy(ArcFileName, Arc.NewLhd.FileName);

	wchar ArcFileNameW[NM];
	*ArcFileNameW = 0;

	bool EqualNames = false;
	bool ExactMatch = Cmd->IsProcessFile(Arc.NewLhd, &EqualNames);
	if (ExactMatch && !EqualNames)
		AllMatchesExact = false;

	bool WideName = (Arc.NewLhd.Flags & LHD_UNICODE);
	wchar *DestNameW = WideName ? DestFileNameW : NULL;

#ifdef UNICODE_SUPPORTED
	if (WideName) {
		ConvertPath(Arc.NewLhd.FileNameW, ArcFileNameW);
		char Name[NM];
		WideToChar(ArcFileNameW, Name);
		if (IsNameUsable(Name))
			strcpy(ArcFileName, Name);
	}
#endif

	ConvertPath(ArcFileName, ArcFileName);

	if (Arc.IsArcLabel())
		return (true);

	if (Arc.NewLhd.Flags & LHD_VERSION) {
		if (Cmd->VersionControl != 1 && !EqualNames) {
			if (Cmd->VersionControl == 0)
				ExactMatch = false;
			int Version = ParseVersionFileName(ArcFileName, ArcFileNameW, false);
			if (Cmd->VersionControl - 1 == Version)
				ParseVersionFileName(ArcFileName, ArcFileNameW, true);
			else
				ExactMatch = false;
		}
	} else if (!Arc.IsArcDir() && Cmd->VersionControl > 1)
		ExactMatch = false;

	Arc.ConvertAttributes();

#ifndef SFX_MODULE
	if ((Arc.NewLhd.Flags & (LHD_SPLIT_BEFORE/*|LHD_SOLID*/)) && FirstFile) {
		char CurVolName[NM];
		strcpy(CurVolName, ArcName);

		VolNameToFirstName(ArcName, ArcName, (Arc.NewMhd.Flags & MHD_NEWNUMBERING));
		if (stricomp(ArcName, CurVolName) != 0 && FileExist(ArcName)) {
			Repeat = true;
			return (false);
		}
		if (!ReconstructDone) {
			ReconstructDone = true;

			RecVolumes RecVol;
			if (RecVol.Restore(Cmd, Arc.FileName, Arc.FileNameW, true)) {
				Repeat = true;
				return (false);
			}
		}
		strcpy(ArcName, CurVolName);
	}
#endif
	DataIO.UnpVolume = (Arc.NewLhd.Flags & LHD_SPLIT_AFTER);

	Arc.Seek(Arc.NextBlockPos - Arc.NewLhd.FullPackSize, SEEK_SET);

	bool TestMode = false;
	bool ExtrFile = false;
	bool SkipSolid = false;

#ifndef SFX_MODULE
	if (FirstFile && (ExactMatch || Arc.Solid) && (Arc.NewLhd.Flags & (LHD_SPLIT_BEFORE/*|LHD_SOLID*/)) != 0) {
		if (ExactMatch) {
			Log(Arc.FileName, St(MUnpCannotMerge), ArcFileName);
		}
		ExactMatch = false;
	}

	FirstFile = false;
#endif

	if (ExactMatch || (SkipSolid = Arc.Solid) != 0) {
		if (Arc.NewLhd.Flags & LHD_PASSWORD)
			if (*Password == 0) {
				if (!GetPassword(PASSWORD_FILE, ArcFileName, Password, sizeof(Password))) {
					return (false);
				}
			}
#if !defined(GUI) && !defined(SILENT)
			else if (!PasswordAll && !Arc.Solid) {
				eprintf(St(MUseCurPsw), ArcFileName);
				switch (Cmd->AllYes ? 1 : Ask(St(MYesNoAll))) {
				case -1:
					ErrHandler.Exit(USER_BREAK);
				case 2:
					if (!GetPassword(PASSWORD_FILE, ArcFileName, Password, sizeof(Password))) {
						return (false);
					}
					break;
				case 3:
					PasswordAll = true;
					break;
				}
			}
#endif

		strcpy(DestFileName, Cmd->ExtrPath);

#ifndef SFX_MODULE
		if (Cmd->AppendArcNameToPath) {
			strcat(DestFileName, PointToName(Arc.FileName));
			SetExt(DestFileName, NULL);
			AddEndSlash(DestFileName);
		}
#endif

		char *ExtrName = ArcFileName;

#ifndef SFX_MODULE
		int Length = strlen(Cmd->ArcPath);
		if (Length > 0 && strnicomp(Cmd->ArcPath, ArcFileName, Length) == 0) {
			ExtrName += Length;
			while (*ExtrName == CPATHDIVIDER)
				ExtrName++;
		}
#endif

		if (Command == 'E' || Cmd->ExclPath == EXCL_SKIPWHOLEPATH)
			strcat(DestFileName, PointToName(ExtrName));
		else
			strcat(DestFileName, ExtrName);

		if (WideName) {
			CharToWide(Cmd->ExtrPath, DestFileNameW);

#ifndef SFX_MODULE
			if (Cmd->AppendArcNameToPath) {
				strcatw(DestFileNameW, PointToName(Arc.FileNameW));
				SetExt(DestFileNameW, NULL);
				AddEndSlash(DestFileNameW);
			}
#endif
			wchar *ExtrNameW = ArcFileNameW;
#ifndef SFX_MODULE
			if (Length > 0) {
				wchar ArcPathW[NM];
				CharToWide(Cmd->ArcPath, ArcPathW);
				Length = strlenw(ArcPathW);
			}
			ExtrNameW += Length;
			while (*ExtrNameW == CPATHDIVIDER)
				ExtrNameW++;
#endif

			if (Command == 'E' || Cmd->ExclPath == EXCL_SKIPWHOLEPATH)
				strcatw(DestFileNameW, PointToName(ExtrNameW));
			else
				strcatw(DestFileNameW, ExtrNameW);
		} else
			*DestFileNameW = 0;

		ExtrFile = !SkipSolid/* && *ExtrName*/;
		if ((Cmd->FreshFiles || Cmd->UpdateFiles) && (Command == 'E' || Command == 'X')) {
			struct FindData FD;
			if (FindFile::FastFind(DestFileName, DestNameW, &FD)) {
				if (FD.FileTime >= Arc.NewLhd.FileTime)
					ExtrFile = false;
			} else if (Cmd->FreshFiles)
				ExtrFile = false;
		}

#ifdef SFX_MODULE
		if (Arc.NewLhd.UnpVer != UNP_VER && Arc.NewLhd.Method != 0x30)
#else
		if (Arc.NewLhd.UnpVer < 13 || Arc.NewLhd.UnpVer > UNP_VER)
#endif
		{
#ifndef SILENT
			Log(Arc.FileName, St(MUnknownMeth), ArcFileName);
#ifndef SFX_MODULE
			Log(Arc.FileName, St(MVerRequired), Arc.NewLhd.UnpVer / 10, Arc.NewLhd.UnpVer % 10);
#endif
#endif
			ExtrFile = false;
			ErrHandler.SetErrorCode(WARNING);
		}

		File CurFile;

		if (!IsLink(Arc.NewLhd.FileAttr))
			if (Arc.IsArcDir()) {
				if (!ExtrFile || Command == 'P' || Command == 'E' || Cmd->ExclPath == EXCL_SKIPWHOLEPATH)
					return (true);
				if (SkipSolid) {
#ifndef GUI
					mprintf(St(MExtrSkipFile), ArcFileName);
#endif
					return (true);
				}
				TotalFileCount++;
				if (Cmd->Test) {
#ifndef GUI
					mprintf(St(MExtrTestFile), ArcFileName);
					mprintf(" %s", St(MOk));
#endif
					return (true);
				}
				MKDIR_CODE MDCode = MakeDir(DestFileName, DestNameW, Arc.NewLhd.FileAttr);
				if (MDCode != MKDIR_SUCCESS && !FileExist(DestFileName, DestNameW)) {
					CreatePath(DestFileName, DestNameW, true);
					MDCode = MakeDir(DestFileName, DestNameW, Arc.NewLhd.FileAttr);
				}
				if (MDCode == MKDIR_SUCCESS) {
#ifndef GUI
					mprintf(St(MCreatDir), DestFileName);
					mprintf(" %s", St(MOk));
#endif
					PrevExtracted = true;
				} else if (FileExist(DestFileName, DestNameW)) {
					SetFileAttr(DestFileName, DestNameW, Arc.NewLhd.FileAttr);
					PrevExtracted = true;
				} else {
					Log(Arc.FileName, St(MExtrErrMkDir), DestFileName);
					ErrHandler.SetErrorCode(WARNING);
				}
				if (PrevExtracted)
					SetDirTime(DestFileName, Arc.NewLhd.FileTime);
				return (true);
			} else {
				if (Cmd->Test && ExtrFile)
					TestMode = true;
#if !defined(GUI) && !defined(SFX_MODULE)
				if (Command == 'P' && ExtrFile)
					CurFile.SetHandleType(FILE_HANDLESTD);
#endif
				if ((Command == 'E' || Command == 'X') && ExtrFile && !Cmd->Test) {
					bool UserReject;
					if (!FileCreate(Cmd, &CurFile, DestFileName, DestNameW, Cmd->Overwrite, &UserReject, Arc.NewLhd.UnpSize, Arc.NewLhd.FileTime)) {
						ExtrFile = false;
						if (!UserReject) {
							ErrHandler.CreateErrorMsg(DestFileName);
							ErrHandler.SetErrorCode(WARNING);
							if (!IsNameUsable(DestFileName)) {
								Log(Arc.FileName, St(MCorrectingName));
								MakeNameUsable(DestFileName, true);
								CreatePath(DestFileName, NULL, true);
								if (FileCreate(Cmd, &CurFile, DestFileName, NULL, Cmd->Overwrite, &UserReject, Arc.NewLhd.FullUnpSize, Arc.NewLhd.FileTime))
									ExtrFile = true;
								else {
									ErrHandler.CreateErrorMsg(DestFileName);
								}
							}
						}
					}
				}
			}

		if (!ExtrFile && Arc.Solid) {
			SkipSolid = true;
			TestMode = true;
			ExtrFile = true;
		}
		if (ExtrFile) {
			if (!SkipSolid) {
				if (!TestMode && Command != 'P' && CurFile.IsDevice()) {
					Log(Arc.FileName, St(MInvalidName), DestFileName);
					ErrHandler.WriteError(DestFileName);
				}
				TotalFileCount++;
			}
			FileCount++;
#ifndef GUI
			if (Command != 'I')
				if (SkipSolid)
					mprintf(St(MExtrSkipFile), ArcFileName);
				else
					switch (Cmd->Test ? 'T' : Command) {
					case 'T':
						mprintf(St(MExtrTestFile), ArcFileName);
						break;
#ifndef SFX_MODULE
					case 'P':
						mprintf(St(MExtrPrinting), ArcFileName);
						break;
#endif
					case 'X':
					case 'E':
						mprintf(St(MExtrFile), DestFileName);
						break;
					}
			if (!Cmd->DisablePercentage)
				mprintf("     ");
#endif
			DataIO.CurUnpRead = 0;
			DataIO.CurUnpWrite = 0;
			DataIO.UnpFileCRC = Arc.OldFormat ? 0 : 0xffffffff;
			DataIO.PackedCRC = 0xffffffff;
			DataIO.SetEncryption(
			    (Arc.NewLhd.Flags & LHD_PASSWORD) ? Arc.NewLhd.UnpVer : 0, Password,
			    (Arc.NewLhd.Flags & LHD_SALT) ? Arc.NewLhd.Salt : NULL, false);
			DataIO.SetPackedSizeToRead(Arc.NewLhd.FullPackSize);
			DataIO.SetFiles(&Arc, &CurFile);
			DataIO.SetTestMode(TestMode);
			DataIO.SetSkipUnpCRC(SkipSolid);
			if (!TestMode && Arc.NewLhd.FullUnpSize > 0x1000)
				CurFile.Prealloc(Arc.NewLhd.FullUnpSize);

			CurFile.SetAllowDelete(!Cmd->KeepBroken);

			if (!ExtractLink(DataIO, Arc, DestFileName, DataIO.UnpFileCRC, Command == 'X' || Command == 'E') &&
			        (Arc.NewLhd.Flags & LHD_SPLIT_BEFORE) == 0)
				if (Arc.NewLhd.Method == 0x30)
					UnstoreFile(DataIO, Arc.NewLhd.FullUnpSize);
				else {
					Unp->SetDestSize(Arc.NewLhd.FullUnpSize);
#ifndef SFX_MODULE
					if (Arc.NewLhd.UnpVer <= 15)
						Unp->DoUnpack(15, FileCount > 1 && Arc.Solid);
					else
#endif
						Unp->DoUnpack(Arc.NewLhd.UnpVer, Arc.NewLhd.Flags & LHD_SOLID);
				}

			if (Arc.IsOpened())
				Arc.SeekToNext();

			bool BrokenFile = false;
			if (!SkipSolid) {
				if (Arc.OldFormat && DataIO.UnpFileCRC == Arc.NewLhd.FileCRC ||
				        !Arc.OldFormat && DataIO.UnpFileCRC == (Arc.NewLhd.FileCRC ^ 0xffffffff)) {
#ifndef GUI
					if (Command != 'P' && Command != 'I')
						mprintf("%s%s ", Cmd->DisablePercentage ? " " : "\b\b\b\b\b ", St(MOk));
#endif
				} else {
					char *BadArcName = (Arc.NewLhd.Flags & LHD_SPLIT_BEFORE) ? NULL : Arc.FileName;
					if (Arc.NewLhd.Flags & LHD_PASSWORD) {
						Log(BadArcName, St(MEncrBadCRC), ArcFileName);
					} else {
						Log(BadArcName, St(MCRCFailed), ArcFileName);
					}
					BrokenFile = true;
					ErrHandler.SetErrorCode(CRC_ERROR);
					Alarm();
				}
			}
#ifndef GUI
			else
				mprintf("\b\b\b\b\b     ");
#endif

			if (!TestMode && (Command == 'X' || Command == 'E') &&
			        !IsLink(Arc.NewLhd.FileAttr)) {
				if (!BrokenFile || Cmd->KeepBroken) {
					if (BrokenFile)
						CurFile.Truncate();
					CurFile.SetOpenFileStat(Arc.NewLhd.FileTime);
					CurFile.Close();
					CurFile.SetCloseFileStat(Arc.NewLhd.FileTime, Arc.NewLhd.FileAttr);
					PrevExtracted = true;
				}
			}
		}
	}
	if (ExactMatch)
		MatchedArgs++;
	if (!Arc.IsOpened())
		return (false);
	if (!ExtrFile)
		if (!Arc.Solid)
			Arc.SeekToNext();
		else if (!SkipSolid)
			return (false);
	return (true);
}


void CmdExtract::UnstoreFile(ComprDataIO &DataIO, Int64 DestUnpSize) {
	Array<byte> Buffer(0x10000);
	while (1) {
		unsigned int Code = DataIO.UnpRead(&Buffer[0], Buffer.Size());
		if (Code == 0 || (int)Code == -1)
			break;
		Code = Code < DestUnpSize ? Code : int64to32(DestUnpSize);
		DataIO.UnpWrite(&Buffer[0], Code);
		if (DestUnpSize >= 0)
			DestUnpSize -= Code;
	}
}


/***** File: volume.cpp *****/

static void GetFirstNewVolName(const char *ArcName, char *VolName,
                               Int64 VolSize, Int64 TotalSize);



bool MergeArchive(Archive &Arc, ComprDataIO *DataIO, bool ShowFileName, char Command) {
	RAROptions *Cmd = Arc.GetRAROptions();

	int HeaderType = Arc.GetHeaderType();
	FileHeader *hd = HeaderType == NEWSUB_HEAD ? &Arc.SubHead : &Arc.NewLhd;
	bool SplitHeader = (HeaderType == FILE_HEAD || HeaderType == NEWSUB_HEAD) &&
	                   (hd->Flags & LHD_SPLIT_AFTER) != 0;

	if (DataIO != NULL && SplitHeader && hd->UnpVer >= 20 &&
	        hd->FileCRC != 0xffffffff && DataIO->PackedCRC != ~hd->FileCRC) {
		Log(Arc.FileName, St(MDataBadCRC), hd->FileName, Arc.FileName);
	}

	Arc.Close();

	char NextName[NM];
	strcpy(NextName, Arc.FileName);
	NextVolumeName(NextName, (Arc.NewMhd.Flags & MHD_NEWNUMBERING) == 0 || Arc.OldFormat);

#if !defined(SFX_MODULE)
	bool RecoveryDone = false;
#endif

	while (!Arc.Open(NextName)) {
#ifndef SFX_MODULE
		if (!RecoveryDone) {
			RecVolumes RecVol;
			RecVol.Restore(Cmd, Arc.FileName, Arc.FileNameW, true);
			RecoveryDone = true;
			continue;
		}
#endif

#ifndef GUI
		if (!Cmd->VolumePause && !IsRemovable(NextName)) {
			Log(Arc.FileName, St(MAbsNextVol), NextName);
			return (false);
		}
#endif
#ifndef SILENT
		if (Cmd->AllYes || !AskNextVol(NextName))
#endif
			return (false);
	}
	Arc.CheckArc(true);

	if (Command == 'T' || Command == 'X' || Command == 'E')
		mprintf(St(Command == 'T' ? MTestVol : MExtrVol), Arc.FileName);
	if (SplitHeader)
		Arc.SearchBlock(HeaderType);
	else
		Arc.ReadHeader();
	if (Arc.GetHeaderType() == FILE_HEAD) {
		Arc.ConvertAttributes();
		Arc.Seek(Arc.NextBlockPos - Arc.NewLhd.FullPackSize, SEEK_SET);
	}
#ifndef GUI
	if (ShowFileName) {
		mprintf(St(MExtrPoints), IntNameToExt(Arc.NewLhd.FileName));
		if (!Cmd->DisablePercentage)
			mprintf("     ");
	}
#endif
	if (DataIO != NULL) {
		if (HeaderType == ENDARC_HEAD)
			DataIO->UnpVolume = false;
		else {
			DataIO->UnpVolume = (hd->Flags & LHD_SPLIT_AFTER);
			DataIO->SetPackedSizeToRead(hd->FullPackSize);
		}
		DataIO->PackedCRC = 0xffffffff;
//    DataIO->SetFiles(&Arc,NULL);
	}
	return (true);
}






#ifndef SILENT
bool AskNextVol(char *ArcName) {
	eprintf(St(MAskNextVol), ArcName);
	if (Ask(St(MContinueQuit)) == 2)
		return (false);
	return (true);
}
#endif


/***** File: list.cpp *****/

static void ListFileHeader(FileHeader &hd, bool Verbose, bool Technical, bool &TitleShown);
static void ListFileAttr(uint A, int HostOS);
static void ListOldSubHeader(Archive &Arc);
static void ListNewSubHeader(CommandData *Cmd, Archive &Arc, bool Technical);

void ListArchive(CommandData *Cmd) {
	Int64 SumPackSize = 0, SumUnpSize = 0;
	uint ArcCount = 0, SumFileCount = 0;
	bool Technical = (Cmd->Command[1] == 'T');
	bool Verbose = (*Cmd->Command == 'V');

	char ArcName[NM];
	wchar ArcNameW[NM];

	while (Cmd->GetArcName(ArcName, ArcNameW, sizeof(ArcName))) {
		Archive Arc(Cmd);
		if (!Arc.WOpen(ArcName, ArcNameW))
			continue;
		bool FileMatched = true;
		while (1) {
			Int64 TotalPackSize = 0, TotalUnpSize = 0;
			uint FileCount = 0;
			if (Arc.IsArchive(true)) {
				bool TitleShown = false;
//        Arc.SkipMhdExtra();
				mprintf("\n");
				if (Arc.Solid)
					mprintf(St(MListSolid));
				if (Arc.SFXSize > 0)
					mprintf(St(MListSFX));
				if (Arc.Volume)
					if (Arc.Solid)
						mprintf(St(MListVol1));
					else
						mprintf(St(MListVol2));
				else if (Arc.Solid)
					mprintf(St(MListArc1));
				else
					mprintf(St(MListArc2));
				mprintf(" %s\n", Arc.FileName);
				if (Technical) {
					if (Arc.Protected)
						mprintf(St(MListRecRec));
					if (Arc.Locked)
						mprintf(St(MListLock));
				}
				while (Arc.ReadHeader() > 0) {
					switch (Arc.GetHeaderType()) {
					case FILE_HEAD:
						IntToExt(Arc.NewLhd.FileName, Arc.NewLhd.FileName);
						if ((FileMatched = Cmd->IsProcessFile(Arc.NewLhd)) == true) {
							ListFileHeader(Arc.NewLhd, Verbose, Technical, TitleShown);
							if (!(Arc.NewLhd.Flags & LHD_SPLIT_BEFORE)) {
								TotalUnpSize += Arc.NewLhd.FullUnpSize;
								FileCount++;
							}
							TotalPackSize += Arc.NewLhd.FullPackSize;
						}
						break;
#ifndef SFX_MODULE
					case SUB_HEAD:
						if (Technical && FileMatched)
							ListOldSubHeader(Arc);
						break;
#endif
					case NEWSUB_HEAD:
						if (FileMatched) {
							if (Technical)
								ListFileHeader(Arc.SubHead, Verbose, true, TitleShown);
							ListNewSubHeader(Cmd, Arc, Technical);
						}
						break;
					}
					Arc.SeekToNext();
				}
				if (TitleShown) {
					mprintf("\n");
					for (int I = 0; I < 79; I++)
						mprintf("-");
					char UnpSizeText[20];
					itoa(TotalUnpSize, UnpSizeText);

					char PackSizeText[20];
					itoa(TotalPackSize, PackSizeText);

					mprintf("\n%5lu %16s %8s %3d%%\n", FileCount, UnpSizeText,
					        PackSizeText, ToPercent(TotalPackSize, TotalUnpSize));
					SumFileCount += FileCount;
					SumUnpSize += TotalUnpSize;
					SumPackSize += TotalPackSize;
				} else
					mprintf(St(MListNoFiles));

				ArcCount++;

#ifndef NOVOLUME
				if (Cmd->VolSize != 0 && ((Arc.NewLhd.Flags & LHD_SPLIT_AFTER) ||
				                          Arc.GetHeaderType() == ENDARC_HEAD &&
				                          (Arc.EndArcHead.Flags & EARC_NEXT_VOLUME) != 0) &&
				        MergeArchive(Arc, NULL, false, *Cmd->Command)) {
					Arc.Seek(0, SEEK_SET);
				} else
#endif
					break;
			} else {
				if (Cmd->ArcNames->ItemsCount() < 2)
					mprintf(St(MNotRAR), Arc.FileName);
				break;
			}
		}
	}
	if (ArcCount > 1) {
		char UnpSizeText[20], PackSizeText[20];
		itoa(SumUnpSize, UnpSizeText);
		itoa(SumPackSize, PackSizeText);
		mprintf("\n%5lu %16s %8s %3d%%\n", SumFileCount, UnpSizeText,
		        PackSizeText, ToPercent(SumPackSize, SumUnpSize));
	}
}


void ListFileHeader(FileHeader &hd, bool Verbose, bool Technical, bool &TitleShown) {
	if (!TitleShown) {
		if (Verbose)
			mprintf(St(MListPathComm));
		else
			mprintf(St(MListName));
		mprintf(St(MListTitle));
		if (Technical)
			mprintf(St(MListTechTitle));
		for (int I = 0; I < 79; I++)
			mprintf("-");
		TitleShown = true;
	}

	if (hd.HeadType == NEWSUB_HEAD)
		mprintf(St(MSubHeadType), hd.FileName);

	mprintf("\n%c", (hd.Flags & LHD_PASSWORD) ? '*' : ' ');

	if (Verbose)
		mprintf("%s\n%12s ", hd.FileName, "");
	else
		mprintf("%-12s", PointToName(hd.FileName));

	char UnpSizeText[20], PackSizeText[20];
	itoa(hd.FullUnpSize, UnpSizeText);
	itoa(hd.FullPackSize, PackSizeText);

	mprintf(" %8s %8s ", UnpSizeText, PackSizeText);

	if ((hd.Flags & LHD_SPLIT_BEFORE) && (hd.Flags & LHD_SPLIT_AFTER))
		mprintf(" <->");
	else if (hd.Flags & LHD_SPLIT_BEFORE)
		mprintf(" <--");
	else if (hd.Flags & LHD_SPLIT_AFTER)
		mprintf(" -->");
	else
		mprintf("%3d%%", ToPercent(hd.FullPackSize, hd.FullUnpSize));

	char DateStr[50];
	ConvertDate(hd.FileTime, DateStr, false);
	mprintf(" %s ", DateStr);

	if (hd.HeadType == NEWSUB_HEAD)
		mprintf("  %c....B  ", (hd.SubFlags & SUBHEAD_FLAGS_INHERITED) ? 'I' : '.');
	else
		ListFileAttr(hd.FileAttr, hd.HostOS);

	mprintf(" %8.8lX", hd.FileCRC);
	mprintf(" m%d", hd.Method - 0x30);
	if ((hd.Flags & LHD_WINDOWMASK) <= 6 * 32)
		mprintf("%c", ((hd.Flags & LHD_WINDOWMASK) >> 5) + 'a');
	else
		mprintf(" ");
	mprintf(" %d.%d", hd.UnpVer / 10, hd.UnpVer % 10);

	static char *RarOS[] = {
		"DOS", "OS/2", "Win95/NT", "Unix", "MacOS", "BeOS", "", "", "", ""
	};

	if (Technical)
		mprintf("\n%22s %8s %4s", RarOS[hd.HostOS],
		        (hd.Flags & LHD_SOLID) ? St(MYes) : St(MNo),
		        (hd.Flags & LHD_VERSION) ? St(MYes) : St(MNo));
}


void ListFileAttr(uint A, int HostOS) {
	switch (HostOS) {
	case MS_DOS:
	case OS2:
	case WIN_32:
	case MAC_OS:
		mprintf("  %c%c%c%c%c%c  ",
		        (A & 0x08) ? 'V' : '.',
		        (A & 0x10) ? 'D' : '.',
		        (A & 0x01) ? 'R' : '.',
		        (A & 0x02) ? 'H' : '.',
		        (A & 0x04) ? 'S' : '.',
		        (A & 0x20) ? 'A' : '.');
		break;
	case UNIX:
	case BEOS:
		switch (A & 0xF000) {
		case 0x4000:
			mprintf("d");
			break;
		case 0xA000:
			mprintf("l");
			break;
		default:
			mprintf("-");
			break;
		}
		mprintf("%c%c%c%c%c%c%c%c%c",
		        (A & 0x0100) ? 'r' : '-',
		        (A & 0x0080) ? 'w' : '-',
		        (A & 0x0040) ? ((A & 0x0800) ? 's' : 'x') : ((A & 0x0800) ? 'S' : '-'),
		        (A & 0x0020) ? 'r' : '-',
		        (A & 0x0010) ? 'w' : '-',
		        (A & 0x0008) ? ((A & 0x0400) ? 's' : 'x') : ((A & 0x0400) ? 'S' : '-'),
		        (A & 0x0004) ? 'r' : '-',
		        (A & 0x0002) ? 'w' : '-',
		        (A & 0x0001) ? 'x' : '-');
		break;
	}
}


#ifndef SFX_MODULE
void ListOldSubHeader(Archive &Arc) {
	switch (Arc.SubBlockHead.SubType) {
	case EA_HEAD:
		mprintf(St(MListEAHead));
		break;
	case UO_HEAD:
		mprintf(St(MListUOHead), Arc.UOHead.OwnerName, Arc.UOHead.GroupName);
		break;
	case MAC_HEAD:
		mprintf(St(MListMACHead1), Arc.MACHead.fileType >> 24, Arc.MACHead.fileType >> 16, Arc.MACHead.fileType >> 8, Arc.MACHead.fileType);
		mprintf(St(MListMACHead2), Arc.MACHead.fileCreator >> 24, Arc.MACHead.fileCreator >> 16, Arc.MACHead.fileCreator >> 8, Arc.MACHead.fileCreator);
		break;
	case BEEA_HEAD:
		mprintf(St(MListBeEAHead));
		break;
	case NTACL_HEAD:
		mprintf(St(MListNTACLHead));
		break;
	case STREAM_HEAD:
		mprintf(St(MListStrmHead), Arc.StreamHead.StreamName);
		break;
	default:
		mprintf(St(MListUnkHead), Arc.SubBlockHead.SubType);
		break;
	}
}
#endif


void ListNewSubHeader(CommandData *Cmd, Archive &Arc, bool Technical) {
}


/***** File: find.cpp *****/

FindFile::FindFile() {
	*FindMask = 0;
	*FindMaskW = 0;
	FirstCall = TRUE;
	dirp = NULL;
}


FindFile::~FindFile() {
	if (dirp != NULL)
		closedir(dirp);
}


void FindFile::SetMask(const char *FindMask) {
	strcpy(FindFile::FindMask, FindMask);
	if (*FindMaskW == 0)
		CharToWide(FindMask, FindMaskW);
	FirstCall = TRUE;
}


void FindFile::SetMaskW(const wchar *FindMaskW) {
	if (FindMaskW == NULL)
		return;
	strcpyw(FindFile::FindMaskW, FindMaskW);
	if (*FindMask == 0)
		WideToChar(FindMaskW, FindMask);
	FirstCall = TRUE;
}


bool FindFile::Next(struct FindData *fd, bool GetSymLink) {
	fd->Error = false;
	if (*FindMask == 0)
		return (false);
	if (FirstCall) {
		char DirName[NM];
		strcpy(DirName, FindMask);
		RemoveNameFromPath(DirName);
		if (*DirName == 0)
			strcpy(DirName, ".");
		/*
		    else
		    {
		      int Length=strlen(DirName);
		      if (Length>1 && DirName[Length-1]==CPATHDIVIDER && (Length!=3 || !IsDriveDiv(DirName[1])))
		        DirName[Length-1]=0;
		    }
		*/
		if ((dirp = opendir(DirName)) == NULL) {
			fd->Error = (errno != ENOENT);
			return (false);
		}
	}
	while (1) {
		struct dirent *ent = readdir(dirp);
		if (ent == NULL)
			return (false);
		if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
			continue;
		if (CmpName(FindMask, ent->d_name, MATCH_NAMES)) {
			char FullName[NM];
			strcpy(FullName, FindMask);
			strcpy(PointToName(FullName), ent->d_name);
			if (!FastFind(FullName, NULL, fd, GetSymLink)) {
				ErrHandler.OpenErrorMsg(FullName);
				continue;
			}
			strcpy(fd->Name, FullName);
			break;
		}
	}
	*fd->NameW = 0;

	fd->IsDir = IsDir(fd->FileAttr);
	FirstCall = FALSE;
	char *Name = PointToName(fd->Name);
	if (strcmp(Name, ".") == 0 || strcmp(Name, "..") == 0)
		return (Next(fd));
	return (true);
}


bool FindFile::FastFind(const char *FindMask, const wchar *FindMaskW, struct FindData *fd, bool GetSymLink) {
	fd->Error = false;
	if (IsWildcard(FindMask, FindMaskW))
		return (false);

	struct stat st;
	if (GetSymLink) {
		if (stat(FindMask, &st) != 0) {
			fd->Error = (errno != ENOENT);
			return (false);
		}
	} else if (stat(FindMask, &st) != 0) {
		fd->Error = (errno != ENOENT);
		return (false);
	}
	fd->FileAttr = st.st_mode;
	fd->IsDir = IsDir(st.st_mode);
	fd->Size = st.st_size;
	fd->FileTime = UnixTimeToDos(st.st_mtime);
	strcpy(fd->Name, FindMask);
	*fd->NameW = 0;

	fd->IsDir = IsDir(fd->FileAttr);
	return (true);
}


/***** File: coder.cpp *****/

inline unsigned int RangeCoder::GetChar() {
	return (UnpackRead->GetChar());
}


void RangeCoder::InitDecoder(Unpack *UnpackRead) {
	RangeCoder::UnpackRead = UnpackRead;

	low = code = 0;
	range = uint(-1);
	for (int i = 0; i < 4; i++)
		code = (code << 8) | GetChar();
}


#define ARI_DEC_NORMALIZE(code,low,range,read)                           \
	{                                                                        \
		while ((low^(low+range))<TOP || range<BOT && ((range=-low&(BOT-1)),1)) \
		{                                                                      \
			code=(code << 8) | read->GetChar();                                  \
			range <<= 8;                                                         \
			low <<= 8;                                                           \
		}                                                                      \
	}


inline int RangeCoder::GetCurrentCount() {
	return (code - low) / (range /= SubRange.scale);
}


inline uint RangeCoder::GetCurrentShiftCount(uint SHIFT) {
	return (code - low) / (range >>= SHIFT);
}


inline void RangeCoder::Decode() {
	low += range * SubRange.LowCount;
	range *= SubRange.HighCount - SubRange.LowCount;
}


/***** File: suballoc.cpp *****/


/****************************************************************************
 *  This file is part of PPMd project                                       *
 *  Written and distributed to public domain by Dmitry Shkarin 1997,        *
 *  1999-2000                                                               *
 *  Contents: memory allocation routines                                    *
 ****************************************************************************/

SubAllocator::SubAllocator() {
	Clean();
}


void SubAllocator::Clean() {
	SubAllocatorSize = 0;
}


inline void SubAllocator::InsertNode(void *p, int indx) {
	((NODE *) p)->next = FreeList[indx].next;
	FreeList[indx].next = (NODE *) p;
}


inline void *SubAllocator::RemoveNode(int indx) {
	NODE *RetVal = FreeList[indx].next;
	FreeList[indx].next = RetVal->next;
	return RetVal;
}


inline uint SubAllocator::U2B(int NU) {
	return /*8*NU+4*NU*/UNIT_SIZE * NU;
}


inline void SubAllocator::SplitBlock(void *pv, int OldIndx, int NewIndx) {
	int i, UDiff = Indx2Units[OldIndx] - Indx2Units[NewIndx];
	byte *p = ((byte *) pv) + U2B(Indx2Units[NewIndx]);
	if (Indx2Units[i = Units2Indx[UDiff - 1]] != UDiff) {
		InsertNode(p, --i);
		p += U2B(i = Indx2Units[i]);
		UDiff -= i;
	}
	InsertNode(p, Units2Indx[UDiff - 1]);
}




void SubAllocator::StopSubAllocator() {
	if (SubAllocatorSize) {
		SubAllocatorSize = 0;
		free(HeapStart);
	}
}


bool SubAllocator::StartSubAllocator(int SASize) {
	uint t = SASize << 20;
	if (SubAllocatorSize == t)
		return TRUE;
	StopSubAllocator();
	uint AllocSize = t / FIXED_UNIT_SIZE * UNIT_SIZE + UNIT_SIZE;
	if ((HeapStart = (byte *)malloc(AllocSize)) == NULL) {
		ErrHandler.MemoryError();
		return FALSE;
	}
	HeapEnd = HeapStart + AllocSize - UNIT_SIZE;
	SubAllocatorSize = t;
	return TRUE;
}


void SubAllocator::InitSubAllocator() {
	int i, k;
	memset(FreeList, 0, sizeof(FreeList));
	pText = HeapStart;
	uint Size2 = FIXED_UNIT_SIZE * (SubAllocatorSize / 8 / FIXED_UNIT_SIZE * 7);
	uint RealSize2 = Size2 / FIXED_UNIT_SIZE * UNIT_SIZE;
	uint Size1 = SubAllocatorSize - Size2;
	uint RealSize1 = Size1 / FIXED_UNIT_SIZE * UNIT_SIZE + Size1 % FIXED_UNIT_SIZE;
	HiUnit = HeapStart + SubAllocatorSize;
	LoUnit = UnitsStart = HeapStart + RealSize1;
	FakeUnitsStart = HeapStart + Size1;
	HiUnit = LoUnit + RealSize2;
	for (i = 0, k = 1; i < N1     ; i++, k += 1)
		Indx2Units[i] = k;
	for (k++; i < N1 + N2      ; i++, k += 2)
		Indx2Units[i] = k;
	for (k++; i < N1 + N2 + N3   ; i++, k += 3)
		Indx2Units[i] = k;
	for (k++; i < N1 + N2 + N3 + N4; i++, k += 4)
		Indx2Units[i] = k;
	for (GlueCount = k = i = 0; k < 128; k++) {
		i += (Indx2Units[i] < k + 1);
		Units2Indx[k] = i;
	}
}


inline void SubAllocator::GlueFreeBlocks() {
	MEM_BLK s0, * p, * p1;
	int i, k, sz;
	if (LoUnit != HiUnit)
		*LoUnit = 0;
	for (i = 0, s0.next = s0.prev = &s0; i < N_INDEXES; i++)
		while (FreeList[i].next) {
			p = (MEM_BLK *)RemoveNode(i);
			p->insertAt(&s0);
			p->Stamp = 0xFFFF;
			p->NU = Indx2Units[i];
		}
	for (p = s0.next; p != &s0; p = p->next)
		while ((p1 = p + p->NU)->Stamp == 0xFFFF && int(p->NU) + p1->NU < 0x10000) {
			p1->remove();
			p->NU += p1->NU;
		}
	while ((p = s0.next) != &s0) {
		for (p->remove(), sz = p->NU; sz > 128; sz -= 128, p += 128)
			InsertNode(p, N_INDEXES - 1);
		if (Indx2Units[i = Units2Indx[sz - 1]] != sz) {
			k = sz - Indx2Units[--i];
			InsertNode(p + (sz - k), k - 1);
		}
		InsertNode(p, i);
	}
}

void *SubAllocator::AllocUnitsRare(int indx) {
	if (!GlueCount) {
		GlueCount = 255;
		GlueFreeBlocks();
		if (FreeList[indx].next)
			return RemoveNode(indx);
	}
	int i = indx;
	do {
		if (++i == N_INDEXES) {
			GlueCount--;
			i = U2B(Indx2Units[indx]);
			int j = 12 * Indx2Units[indx];
			if (FakeUnitsStart - pText > j) {
				FakeUnitsStart -= j;
				UnitsStart -= i;
				return (UnitsStart);
			}
			return (NULL);
		}
	} while (!FreeList[i].next);
	void *RetVal = RemoveNode(i);
	SplitBlock(RetVal, i, indx);
	return RetVal;
}


inline void *SubAllocator::AllocUnits(int NU) {
	int indx = Units2Indx[NU - 1];
	if (FreeList[indx].next)
		return RemoveNode(indx);
	void *RetVal = LoUnit;
	LoUnit += U2B(Indx2Units[indx]);
	if (LoUnit <= HiUnit)
		return RetVal;
	LoUnit -= U2B(Indx2Units[indx]);
	return AllocUnitsRare(indx);
}


void *SubAllocator::AllocContext() {
	if (HiUnit != LoUnit)
		return (HiUnit -= UNIT_SIZE);
	if (FreeList->next)
		return RemoveNode(0);
	return AllocUnitsRare(0);
}


void *SubAllocator::ExpandUnits(void *OldPtr, int OldNU) {
	int i0 = Units2Indx[OldNU - 1], i1 = Units2Indx[OldNU - 1 + 1];
	if (i0 == i1)
		return OldPtr;
	void *ptr = AllocUnits(OldNU + 1);
	if (ptr) {
		memcpy(ptr, OldPtr, U2B(OldNU));
		InsertNode(OldPtr, i0);
	}
	return ptr;
}


void *SubAllocator::ShrinkUnits(void *OldPtr, int OldNU, int NewNU) {
	int i0 = Units2Indx[OldNU - 1], i1 = Units2Indx[NewNU - 1];
	if (i0 == i1)
		return OldPtr;
	if (FreeList[i1].next) {
		void *ptr = RemoveNode(i1);
		memcpy(ptr, OldPtr, U2B(NewNU));
		InsertNode(OldPtr, i0);
		return ptr;
	} else {
		SplitBlock(OldPtr, i0, i1);
		return OldPtr;
	}
}


void SubAllocator::FreeUnits(void *ptr, int OldNU) {
	InsertNode(ptr, Units2Indx[OldNU - 1]);
}


/***** File: model.cpp *****/


/****************************************************************************
 *  This file is part of PPMd project                                       *
 *  Written and distributed to public domain by Dmitry Shkarin 1997,        *
 *  1999-2000                                                               *
 *  Contents: model description and encoding/decoding routines              *
 ****************************************************************************/

inline PPM_CONTEXT *PPM_CONTEXT::createChild(ModelPPM *Model, STATE *pStats,
        STATE &FirstState) {
	PPM_CONTEXT *pc = (PPM_CONTEXT *) Model->SubAlloc.AllocContext();
	if (pc) {
		pc->NumStats = 1;
		pc->OneState = FirstState;
		pc->Suffix = this;
		pStats->Successor = pc;
	}
	return pc;
}


ModelPPM::ModelPPM() {
	MinContext = NULL;
	MaxContext = NULL;
	MedContext = NULL;
}


void ModelPPM::RestartModelRare() {
	int i, k, m;
	memset(CharMask, 0, sizeof(CharMask));
	SubAlloc.InitSubAllocator();
	InitRL = -(MaxOrder < 12 ? MaxOrder : 12) - 1;
	MinContext = MaxContext = (PPM_CONTEXT *) SubAlloc.AllocContext();
	MinContext->Suffix = NULL;
	OrderFall = MaxOrder;
	MinContext->U.SummFreq = (MinContext->NumStats = 256) + 1;
	FoundState = MinContext->U.Stats = (STATE *)SubAlloc.AllocUnits(256 / 2);
	for (RunLength = InitRL, PrevSuccess = i = 0; i < 256; i++) {
		MinContext->U.Stats[i].Symbol = i;
		MinContext->U.Stats[i].Freq = 1;
		MinContext->U.Stats[i].Successor = NULL;
	}

	static const ushort InitBinEsc[] = {
		0x3CDD, 0x1F3F, 0x59BF, 0x48F3, 0x64A1, 0x5ABC, 0x6632, 0x6051
	};

	for (i = 0; i < 128; i++)
		for (k = 0; k < 8; k++)
			for (m = 0; m < 64; m += 8)
				BinSumm[i][k + m] = BIN_SCALE - InitBinEsc[k] / (i + 2);
	for (i = 0; i < 25; i++)
		for (k = 0; k < 16; k++)
			SEE2Cont[i][k].init(5 * i + 10);
}


void ModelPPM::StartModelRare(int MaxOrder) {
	int i, k, m, Step;
	EscCount = 1;
	/*
	  if (MaxOrder < 2)
	  {
	    memset(CharMask,0,sizeof(CharMask));
	    OrderFall=ModelPPM::MaxOrder;
	    MinContext=MaxContext;
	    while (MinContext->Suffix != NULL)
	    {
	      MinContext=MinContext->Suffix;
	      OrderFall--;
	    }
	    FoundState=MinContext->U.Stats;
	    MinContext=MaxContext;
	  }
	  else
	*/
	{
		ModelPPM::MaxOrder = MaxOrder;
		RestartModelRare();
		NS2BSIndx[0] = 2 * 0;
		NS2BSIndx[1] = 2 * 1;
		memset(NS2BSIndx + 2, 2 * 2, 9);
		memset(NS2BSIndx + 11, 2 * 3, 256 - 11);
		for (i = 0; i < 3; i++)
			NS2Indx[i] = i;
		for (m = i, k = Step = 1; i < 256; i++) {
			NS2Indx[i] = m;
			if (!--k) {
				k = ++Step;
				m++;
			}
		}
		memset(HB2Flag, 0, 0x40);
		memset(HB2Flag + 0x40, 0x08, 0x100 - 0x40);
		DummySEE2Cont.Shift = PERIOD_BITS;
	}
}


void PPM_CONTEXT::rescale(ModelPPM *Model) {
	int OldNS = NumStats, i = NumStats - 1, Adder, EscFreq;
	STATE *p1, * p;
	for (p = Model->FoundState; p != U.Stats; p--)
		_PPMD_SWAP(p[0], p[-1]);
	U.Stats->Freq += 4;
	U.SummFreq += 4;
	EscFreq = U.SummFreq - p->Freq;
	Adder = (Model->OrderFall != 0);
	U.SummFreq = (p->Freq = (p->Freq + Adder) >> 1);
	do {
		EscFreq -= (++p)->Freq;
		U.SummFreq += (p->Freq = (p->Freq + Adder) >> 1);
		if (p[0].Freq > p[-1].Freq) {
			STATE tmp = *(p1 = p);
			do {
				p1[0] = p1[-1];
			} while (--p1 != U.Stats && tmp.Freq > p1[-1].Freq);
			*p1 = tmp;
		}
	} while (--i);
	if (p->Freq == 0) {
		do {
			i++;
		} while ((--p)->Freq == 0);
		EscFreq += i;
		if ((NumStats -= i) == 1) {
			STATE tmp = *U.Stats;
			do {
				tmp.Freq -= (tmp.Freq >> 1);
				EscFreq >>= 1;
			} while (EscFreq > 1);
			Model->SubAlloc.FreeUnits(U.Stats, (OldNS + 1) >> 1);
			*(Model->FoundState = &OneState) = tmp;
			return;
		}
	}
	U.SummFreq += (EscFreq -= (EscFreq >> 1));
	int n0 = (OldNS + 1) >> 1, n1 = (NumStats + 1) >> 1;
	if (n0 != n1)
		U.Stats = (STATE *) Model->SubAlloc.ShrinkUnits(U.Stats, n0, n1);
	Model->FoundState = U.Stats;
}


inline PPM_CONTEXT *ModelPPM::CreateSuccessors(bool Skip, STATE *p1) {
#ifdef __ICL
	static
#endif
	STATE UpState;
	PPM_CONTEXT *pc = MinContext, * UpBranch = FoundState->Successor;
	STATE *p, * ps[MAX_O], ** pps = ps;
	if (!Skip) {
		*pps++ = FoundState;
		if (!pc->Suffix)
			goto NO_LOOP;
	}
	if (p1) {
		p = p1;
		pc = pc->Suffix;
		goto LOOP_ENTRY;
	}
	do {
		pc = pc->Suffix;
		if (pc->NumStats != 1) {
			if ((p = pc->U.Stats)->Symbol != FoundState->Symbol)
				do {
					p++;
				} while (p->Symbol != FoundState->Symbol);
		} else
			p = &(pc->OneState);
LOOP_ENTRY:
		if (p->Successor != UpBranch) {
			pc = p->Successor;
			break;
		}
		*pps++ = p;
	} while (pc->Suffix);
NO_LOOP:
	if (pps == ps)
		return pc;
	UpState.Symbol = *(byte *) UpBranch;
	UpState.Successor = (PPM_CONTEXT *)(((byte *) UpBranch) + 1);
	if (pc->NumStats != 1) {
		if ((byte *) pc <= SubAlloc.pText)
			return (NULL);
		if ((p = pc->U.Stats)->Symbol != UpState.Symbol)
			do {
				p++;
			} while (p->Symbol != UpState.Symbol);
		uint cf = p->Freq - 1;
		uint s0 = pc->U.SummFreq - pc->NumStats - cf;
		UpState.Freq = 1 + ((2 * cf <= s0) ? (5 * cf > s0) : ((2 * cf + 3 * s0 - 1) / (2 * s0)));
	} else
		UpState.Freq = pc->OneState.Freq;
	do {
		pc = pc->createChild(this, *--pps, UpState);
		if (!pc)
			return NULL;
	} while (pps != ps);
	return pc;
}


inline void ModelPPM::UpdateModel() {
	STATE fs = *FoundState, *p = NULL;
	PPM_CONTEXT *pc, *Successor;
	uint ns1, ns, cf, sf, s0;
	if (fs.Freq < MAX_FREQ / 4 && (pc = MinContext->Suffix) != NULL) {
		if (pc->NumStats != 1) {
			if ((p = pc->U.Stats)->Symbol != fs.Symbol) {
				do {
					p++;
				} while (p->Symbol != fs.Symbol);
				if (p[0].Freq >= p[-1].Freq) {
					_PPMD_SWAP(p[0], p[-1]);
					p--;
				}
			}
			if (p->Freq < MAX_FREQ - 9) {
				p->Freq += 2;
				pc->U.SummFreq += 2;
			}
		} else {
			p = &(pc->OneState);
			p->Freq += (p->Freq < 32);
		}
	}
	if (!OrderFall) {
		MinContext = MaxContext = FoundState->Successor = CreateSuccessors(TRUE, p);
		if (!MinContext)
			goto RESTART_MODEL;
		return;
	}
	*SubAlloc.pText++ = fs.Symbol;
	Successor = (PPM_CONTEXT *) SubAlloc.pText;
	if (SubAlloc.pText >= SubAlloc.FakeUnitsStart)
		goto RESTART_MODEL;
	if (fs.Successor) {
		if ((byte *) fs.Successor <= SubAlloc.pText &&
		        (fs.Successor = CreateSuccessors(FALSE, p)) == NULL)
			goto RESTART_MODEL;
		if (!--OrderFall) {
			Successor = fs.Successor;
			SubAlloc.pText -= (MaxContext != MinContext);
		}
	} else {
		FoundState->Successor = Successor;
		fs.Successor = MinContext;
	}
	s0 = MinContext->U.SummFreq - (ns = MinContext->NumStats) - (fs.Freq - 1);
	for (pc = MaxContext; pc != MinContext; pc = pc->Suffix) {
		if ((ns1 = pc->NumStats) != 1) {
			if ((ns1 & 1) == 0) {
				pc->U.Stats = (STATE *) SubAlloc.ExpandUnits(pc->U.Stats, ns1 >> 1);
				if (!pc->U.Stats)
					goto RESTART_MODEL;
			}
			pc->U.SummFreq += (2 * ns1 < ns) + 2 * ((4 * ns1 <= ns) & (pc->U.SummFreq <= 8 * ns1));
		} else {
			p = (STATE *) SubAlloc.AllocUnits(1);
			if (!p)
				goto RESTART_MODEL;
			*p = pc->OneState;
			pc->U.Stats = p;
			if (p->Freq < MAX_FREQ / 4 - 1)
				p->Freq += p->Freq;
			else
				p->Freq  = MAX_FREQ - 4;
			pc->U.SummFreq = p->Freq + InitEsc + (ns > 3);
		}
		cf = 2 * fs.Freq * (pc->U.SummFreq + 6);
		sf = s0 + pc->U.SummFreq;
		if (cf < 6 * sf) {
			cf = 1 + (cf > sf) + (cf >= 4 * sf);
			pc->U.SummFreq += 3;
		} else {
			cf = 4 + (cf >= 9 * sf) + (cf >= 12 * sf) + (cf >= 15 * sf);
			pc->U.SummFreq += cf;
		}
		p = pc->U.Stats + ns1;
		p->Successor = Successor;
		p->Symbol = fs.Symbol;
		p->Freq = cf;
		pc->NumStats = ++ns1;
	}
	MaxContext = MinContext = fs.Successor;
	return;
RESTART_MODEL:
	RestartModelRare();
	EscCount = 0;
}


// Tabulated escapes for exponential symbol distribution
static const byte ExpEscape[16] = { 25, 14, 9, 7, 5, 5, 4, 4, 4, 3, 3, 3, 2, 2, 2, 2 };
#define GET_MEAN(SUMM,SHIFT,ROUND) ((SUMM+(1 << (SHIFT-ROUND))) >> (SHIFT))



inline void PPM_CONTEXT::decodeBinSymbol(ModelPPM *Model) {
	STATE &rs = OneState;
	Model->HiBitsFlag = Model->HB2Flag[Model->FoundState->Symbol];
	ushort &bs = Model->BinSumm[rs.Freq - 1][Model->PrevSuccess +
	             Model->NS2BSIndx[Suffix->NumStats - 1] +
	             Model->HiBitsFlag + 2 * Model->HB2Flag[rs.Symbol] +
	             ((Model->RunLength >> 26) & 0x20)];
	if (Model->Coder.GetCurrentShiftCount(TOT_BITS) < bs) {
		Model->FoundState = &rs;
		rs.Freq += (rs.Freq < 128);
		Model->Coder.SubRange.LowCount = 0;
		Model->Coder.SubRange.HighCount = bs;
		bs = SHORT16(bs + INTERVAL - GET_MEAN(bs, PERIOD_BITS, 2));
		Model->PrevSuccess = 1;
		Model->RunLength++;
	} else {
		Model->Coder.SubRange.LowCount = bs;
		bs = SHORT16(bs - GET_MEAN(bs, PERIOD_BITS, 2));
		Model->Coder.SubRange.HighCount = BIN_SCALE;
		Model->InitEsc = ExpEscape[bs >> 10];
		Model->NumMasked = 1;
		Model->CharMask[rs.Symbol] = Model->EscCount;
		Model->PrevSuccess = 0;
		Model->FoundState = NULL;
	}
}


inline void PPM_CONTEXT::update1(ModelPPM *Model, STATE *p) {
	(Model->FoundState = p)->Freq += 4;
	U.SummFreq += 4;
	if (p[0].Freq > p[-1].Freq) {
		_PPMD_SWAP(p[0], p[-1]);
		Model->FoundState = --p;
		if (p->Freq > MAX_FREQ)
			rescale(Model);
	}
}




inline bool PPM_CONTEXT::decodeSymbol1(ModelPPM *Model) {
	Model->Coder.SubRange.scale = U.SummFreq;
	STATE *p = U.Stats;
	int i, HiCnt;
	int count = Model->Coder.GetCurrentCount();
	if (count >= Model->Coder.SubRange.scale)
		return (false);
	if (count < (HiCnt = p->Freq)) {
		Model->PrevSuccess = (2 * (Model->Coder.SubRange.HighCount = HiCnt) > Model->Coder.SubRange.scale);
		Model->RunLength += Model->PrevSuccess;
		(Model->FoundState = p)->Freq = (HiCnt += 4);
		U.SummFreq += 4;
		if (HiCnt > MAX_FREQ)
			rescale(Model);
		Model->Coder.SubRange.LowCount = 0;
		return (true);
	} else if (Model->FoundState == NULL)
		return (false);
	Model->PrevSuccess = 0;
	i = NumStats - 1;
	while ((HiCnt += (++p)->Freq) <= count)
		if (--i == 0) {
			Model->HiBitsFlag = Model->HB2Flag[Model->FoundState->Symbol];
			Model->Coder.SubRange.LowCount = HiCnt;
			Model->CharMask[p->Symbol] = Model->EscCount;
			i = (Model->NumMasked = NumStats) - 1;
			Model->FoundState = NULL;
			do {
				Model->CharMask[(--p)->Symbol] = Model->EscCount;
			} while (--i);
			Model->Coder.SubRange.HighCount = Model->Coder.SubRange.scale;
			return (true);
		}
	Model->Coder.SubRange.LowCount = (Model->Coder.SubRange.HighCount = HiCnt) - p->Freq;
	update1(Model, p);
	return (true);
}


inline void PPM_CONTEXT::update2(ModelPPM *Model, STATE *p) {
	(Model->FoundState = p)->Freq += 4;
	U.SummFreq += 4;
	if (p->Freq > MAX_FREQ)
		rescale(Model);
	Model->EscCount++;
	Model->RunLength = Model->InitRL;
}


inline SEE2_CONTEXT *PPM_CONTEXT::makeEscFreq2(ModelPPM *Model, int Diff) {
	SEE2_CONTEXT *psee2c;
	if (NumStats != 256) {
		psee2c = Model->SEE2Cont[Model->NS2Indx[Diff - 1]] +
		         (Diff < Suffix->NumStats - NumStats) +
		         2 * (U.SummFreq < 11 * NumStats) + 4 * (Model->NumMasked > Diff) +
		         Model->HiBitsFlag;
		Model->Coder.SubRange.scale = psee2c->getMean();
	} else {
		psee2c = &Model->DummySEE2Cont;
		Model->Coder.SubRange.scale = 1;
	}
	return psee2c;
}




inline bool PPM_CONTEXT::decodeSymbol2(ModelPPM *Model) {
	int count, HiCnt, i = NumStats - Model->NumMasked;
	SEE2_CONTEXT *psee2c = makeEscFreq2(Model, i);
	STATE *ps[256], ** pps = ps, * p = U.Stats - 1;
	HiCnt = 0;
	do {
		do {
			p++;
		} while (Model->CharMask[p->Symbol] == Model->EscCount);
		HiCnt += p->Freq;
		*pps++ = p;
	} while (--i);
	Model->Coder.SubRange.scale += HiCnt;
	count = Model->Coder.GetCurrentCount();
	if (count >= Model->Coder.SubRange.scale)
		return (false);
	p = *(pps = ps);
	if (count < HiCnt) {
		HiCnt = 0;
		while ((HiCnt += p->Freq) <= count)
			p = *++pps;
		Model->Coder.SubRange.LowCount = (Model->Coder.SubRange.HighCount = HiCnt) - p->Freq;
		psee2c->update();
		update2(Model, p);
	} else {
		Model->Coder.SubRange.LowCount = HiCnt;
		Model->Coder.SubRange.HighCount = Model->Coder.SubRange.scale;
		i = NumStats - Model->NumMasked;
		pps--;
		do {
			Model->CharMask[(*++pps)->Symbol] = Model->EscCount;
		} while (--i);
		psee2c->Summ += Model->Coder.SubRange.scale;
		Model->NumMasked = NumStats;
	}
	return (true);
}


inline void ModelPPM::ClearMask() {
	EscCount = 1;
	memset(CharMask, 0, sizeof(CharMask));
}




bool ModelPPM::DecodeInit(Unpack *UnpackRead, int &EscChar) {
	int MaxOrder = UnpackRead->GetChar();
	bool Reset = MaxOrder & 0x20;

	int MaxMB;
	if (Reset)
		MaxMB = UnpackRead->GetChar();
	else if (SubAlloc.GetAllocatedMemory() == 0)
		return (false);
	if (MaxOrder & 0x40)
		EscChar = UnpackRead->GetChar();
	Coder.InitDecoder(UnpackRead);
	if (Reset) {
		MaxOrder = (MaxOrder & 0x1f) + 1;
		if (MaxOrder > 16)
			MaxOrder = 16 + (MaxOrder - 16) * 3;
		if (MaxOrder == 1) {
			SubAlloc.StopSubAllocator();
			return (false);
		}
		SubAlloc.StartSubAllocator(MaxMB + 1);
		StartModelRare(MaxOrder);
	}
	return (MinContext != NULL);
}


int ModelPPM::DecodeChar() {
	if ((byte *)MinContext <= SubAlloc.pText || (byte *)MinContext > SubAlloc.HeapEnd)
		return (-1);
	if (MinContext->NumStats != 1) {
		if (!MinContext->decodeSymbol1(this))
			return (-1);
	} else
		MinContext->decodeBinSymbol(this);
	Coder.Decode();
	while (!FoundState) {
		ARI_DEC_NORMALIZE(Coder.code, Coder.low, Coder.range, Coder.UnpackRead);
		do {
			OrderFall++;
			MinContext = MinContext->Suffix;
			if ((byte *)MinContext <= SubAlloc.pText || (byte *)MinContext > SubAlloc.HeapEnd)
				return (-1);
		} while (MinContext->NumStats == NumMasked);
		if (!MinContext->decodeSymbol2(this))
			return (-1);
		Coder.Decode();
	}
	int Symbol = FoundState->Symbol;
	if (!OrderFall && (byte *) FoundState->Successor > SubAlloc.pText)
		MinContext = MaxContext = FoundState->Successor;
	else {
		UpdateModel();
		if (EscCount == 0)
			ClearMask();
	}
	ARI_DEC_NORMALIZE(Coder.code, Coder.low, Coder.range, Coder.UnpackRead);
	return (Symbol);
}


/***** File: unpack15.cpp *****/


#define STARTL1  2
static unsigned int DecL1[] = {0x8000, 0xa000, 0xc000, 0xd000, 0xe000, 0xea00,
                               0xee00, 0xf000, 0xf200, 0xf200, 0xffff
                              };
static unsigned int PosL1[] = {0, 0, 0, 2, 3, 5, 7, 11, 16, 20, 24, 32, 32};

#define STARTL2  3
static unsigned int DecL2[] = {0xa000, 0xc000, 0xd000, 0xe000, 0xea00, 0xee00,
                               0xf000, 0xf200, 0xf240, 0xffff
                              };
static unsigned int PosL2[] = {0, 0, 0, 0, 5, 7, 9, 13, 18, 22, 26, 34, 36};

#define STARTHF0  4
static unsigned int DecHf0[] = {0x8000, 0xc000, 0xe000, 0xf200, 0xf200, 0xf200,
                                0xf200, 0xf200, 0xffff
                               };
static unsigned int PosHf0[] = {0, 0, 0, 0, 0, 8, 16, 24, 33, 33, 33, 33, 33};


#define STARTHF1  5
static unsigned int DecHf1[] = {0x2000, 0xc000, 0xe000, 0xf000, 0xf200, 0xf200,
                                0xf7e0, 0xffff
                               };
static unsigned int PosHf1[] = {0, 0, 0, 0, 0, 0, 4, 44, 60, 76, 80, 80, 127};


#define STARTHF2  5
static unsigned int DecHf2[] = {0x1000, 0x2400, 0x8000, 0xc000, 0xfa00, 0xffff,
                                0xffff, 0xffff
                               };
static unsigned int PosHf2[] = {0, 0, 0, 0, 0, 0, 2, 7, 53, 117, 233, 0, 0};


#define STARTHF3  6
static unsigned int DecHf3[] = {0x800, 0x2400, 0xee00, 0xfe80, 0xffff, 0xffff,
                                0xffff
                               };
static unsigned int PosHf3[] = {0, 0, 0, 0, 0, 0, 0, 2, 16, 218, 251, 0, 0};


#define STARTHF4  8
static unsigned int DecHf4[] = {0xff00, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};
static unsigned int PosHf4[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0};


void Unpack::Unpack15(bool Solid) {
	if (Suspended)
		UnpPtr = WrPtr;
	else {
		UnpInitData(Solid);
		OldUnpInitData(Solid);
		UnpReadBuf();
		if (!Solid) {
			InitHuff();
			UnpPtr = 0;
		} else
			UnpPtr = WrPtr;
		--DestUnpSize;
	}
	if (DestUnpSize >= 0) {
		GetFlagsBuf();
		FlagsCnt = 8;
	}

	while (DestUnpSize >= 0) {
		UnpPtr &= MAXWINMASK;

		if (InAddr > ReadTop - 30 && !UnpReadBuf())
			break;
		if (((WrPtr - UnpPtr) & MAXWINMASK) < 270 && WrPtr != UnpPtr) {
			OldUnpWriteBuf();
			if (Suspended)
				return;
		}
		if (StMode) {
			HuffDecode();
			continue;
		}

		if (--FlagsCnt < 0) {
			GetFlagsBuf();
			FlagsCnt = 7;
		}

		if (FlagBuf & 0x80) {
			FlagBuf <<= 1;
			if (Nlzb > Nhfb)
				LongLZ();
			else
				HuffDecode();
		} else {
			FlagBuf <<= 1;
			if (--FlagsCnt < 0) {
				GetFlagsBuf();
				FlagsCnt = 7;
			}
			if (FlagBuf & 0x80) {
				FlagBuf <<= 1;
				if (Nlzb > Nhfb)
					HuffDecode();
				else
					LongLZ();
			} else {
				FlagBuf <<= 1;
				ShortLZ();
			}
		}
	}
	OldUnpWriteBuf();
}


void Unpack::OldUnpWriteBuf() {
	if (UnpPtr != WrPtr)
		UnpSomeRead = true;
	if (UnpPtr < WrPtr) {
		UnpIO->UnpWrite(&Window[WrPtr], -WrPtr & MAXWINMASK);
		UnpIO->UnpWrite(Window, UnpPtr);
		UnpAllBuf = true;
	} else
		UnpIO->UnpWrite(&Window[WrPtr], UnpPtr - WrPtr);
	WrPtr = UnpPtr;
}


void Unpack::ShortLZ() {
	static unsigned int ShortLen1[] = {1, 3, 4, 4, 5, 6, 7, 8, 8, 4, 4, 5, 6, 6, 4, 0};
	static unsigned int ShortXor1[] = {0, 0xa0, 0xd0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe,
	                                   0xff, 0xc0, 0x80, 0x90, 0x98, 0x9c, 0xb0
	                                  };
	static unsigned int ShortLen2[] = {2, 3, 3, 3, 4, 4, 5, 6, 6, 4, 4, 5, 6, 6, 4, 0};
	static unsigned int ShortXor2[] = {0, 0x40, 0x60, 0xa0, 0xd0, 0xe0, 0xf0, 0xf8,
	                                   0xfc, 0xc0, 0x80, 0x90, 0x98, 0x9c, 0xb0
	                                  };


	unsigned int Length, SaveLength;
	unsigned int LastDistance;
	unsigned int Distance;
	int DistancePlace;
	NumHuf = 0;

	unsigned int BitField = fgetbits();
	if (LCount == 2) {
		faddbits(1);
		if (BitField >= 0x8000) {
			OldCopyString((unsigned int)LastDist, LastLength);
			return;
		}
		BitField <<= 1;
		LCount = 0;
	}

	BitField >>= 8;

	ShortLen1[1] = ShortLen2[3] = Buf60 + 3;

	if (AvrLn1 < 37) {
		for (Length = 0;; Length++)
			if (((BitField ^ ShortXor1[Length]) & (~(0xff >> ShortLen1[Length]))) == 0)
				break;
		faddbits(ShortLen1[Length]);
	} else {
		for (Length = 0;; Length++)
			if (((BitField ^ ShortXor2[Length]) & (~(0xff >> ShortLen2[Length]))) == 0)
				break;
		faddbits(ShortLen2[Length]);
	}

	if (Length >= 9) {
		if (Length == 9) {
			LCount++;
			OldCopyString((unsigned int)LastDist, LastLength);
			return;
		}
		if (Length == 14) {
			LCount = 0;
			Length = DecodeNum(fgetbits(), STARTL2, DecL2, PosL2) + 5;
			Distance = (fgetbits() >> 1) | 0x8000;
			faddbits(15);
			LastLength = Length;
			LastDist = Distance;
			OldCopyString(Distance, Length);
			return;
		}

		LCount = 0;
		SaveLength = Length;
		Distance = OldDist[(OldDistPtr - (Length - 9)) & 3];
		Length = DecodeNum(fgetbits(), STARTL1, DecL1, PosL1) + 2;
		if (Length == 0x101 && SaveLength == 10) {
			Buf60 ^= 1;
			return;
		}
		if (Distance > 256)
			Length++;
		if (Distance >= MaxDist3)
			Length++;

		OldDist[OldDistPtr++] = Distance;
		OldDistPtr = OldDistPtr & 3;
		LastLength = Length;
		LastDist = Distance;
		OldCopyString(Distance, Length);
		return;
	}

	LCount = 0;
	AvrLn1 += Length;
	AvrLn1 -= AvrLn1 >> 4;

	DistancePlace = DecodeNum(fgetbits(), STARTHF2, DecHf2, PosHf2) & 0xff;
	Distance = ChSetA[DistancePlace];
	if (--DistancePlace != -1) {
		PlaceA[Distance]--;
		LastDistance = ChSetA[DistancePlace];
		PlaceA[LastDistance]++;
		ChSetA[DistancePlace + 1] = LastDistance;
		ChSetA[DistancePlace] = Distance;
	}
	Length += 2;
	OldDist[OldDistPtr++] = ++Distance;
	OldDistPtr = OldDistPtr & 3;
	LastLength = Length;
	LastDist = Distance;
	OldCopyString(Distance, Length);
}


void Unpack::LongLZ() {
	unsigned int Length;
	unsigned int Distance;
	unsigned int DistancePlace, NewDistancePlace;
	unsigned int OldAvr2, OldAvr3;

	NumHuf = 0;
	Nlzb += 16;
	if (Nlzb > 0xff) {
		Nlzb = 0x90;
		Nhfb >>= 1;
	}
	OldAvr2 = AvrLn2;

	unsigned int BitField = fgetbits();
	if (AvrLn2 >= 122)
		Length = DecodeNum(BitField, STARTL2, DecL2, PosL2);
	else if (AvrLn2 >= 64)
		Length = DecodeNum(BitField, STARTL1, DecL1, PosL1);
	else if (BitField < 0x100) {
		Length = BitField;
		faddbits(16);
	} else {
		for (Length = 0; ((BitField << Length) & 0x8000) == 0; Length++)
			;
		faddbits(Length + 1);
	}

	AvrLn2 += Length;
	AvrLn2 -= AvrLn2 >> 5;

	BitField = fgetbits();
	if (AvrPlcB > 0x28ff)
		DistancePlace = DecodeNum(BitField, STARTHF2, DecHf2, PosHf2);
	else if (AvrPlcB > 0x6ff)
		DistancePlace = DecodeNum(BitField, STARTHF1, DecHf1, PosHf1);
	else
		DistancePlace = DecodeNum(BitField, STARTHF0, DecHf0, PosHf0);

	AvrPlcB += DistancePlace;
	AvrPlcB -= AvrPlcB >> 8;
	while (1) {
		Distance = ChSetB[DistancePlace & 0xff];
		NewDistancePlace = NToPlB[Distance++ & 0xff]++;
		if (!(Distance & 0xff))
			CorrHuff(ChSetB, NToPlB);
		else
			break;
	}

	ChSetB[DistancePlace] = ChSetB[NewDistancePlace];
	ChSetB[NewDistancePlace] = Distance;

	Distance = ((Distance & 0xff00) | (fgetbits() >> 8)) >> 1;
	faddbits(7);

	OldAvr3 = AvrLn3;
	if (Length != 1 && Length != 4)
		if (Length == 0 && Distance <= MaxDist3) {
			AvrLn3++;
			AvrLn3 -= AvrLn3 >> 8;
		} else if (AvrLn3 > 0)
			AvrLn3--;
	Length += 3;
	if (Distance >= MaxDist3)
		Length++;
	if (Distance <= 256)
		Length += 8;
	if (OldAvr3 > 0xb0 || AvrPlc >= 0x2a00 && OldAvr2 < 0x40)
		MaxDist3 = 0x7f00;
	else
		MaxDist3 = 0x2001;
	OldDist[OldDistPtr++] = Distance;
	OldDistPtr = OldDistPtr & 3;
	LastLength = Length;
	LastDist = Distance;
	OldCopyString(Distance, Length);
}


void Unpack::HuffDecode() {
	unsigned int CurByte, NewBytePlace;
	unsigned int Length;
	unsigned int Distance;
	int BytePlace;

	unsigned int BitField = fgetbits();

	if (AvrPlc > 0x75ff)
		BytePlace = DecodeNum(BitField, STARTHF4, DecHf4, PosHf4);
	else if (AvrPlc > 0x5dff)
		BytePlace = DecodeNum(BitField, STARTHF3, DecHf3, PosHf3);
	else if (AvrPlc > 0x35ff)
		BytePlace = DecodeNum(BitField, STARTHF2, DecHf2, PosHf2);
	else if (AvrPlc > 0x0dff)
		BytePlace = DecodeNum(BitField, STARTHF1, DecHf1, PosHf1);
	else
		BytePlace = DecodeNum(BitField, STARTHF0, DecHf0, PosHf0);
	BytePlace &= 0xff;
	if (StMode) {
		if (BytePlace == 0 && BitField > 0xfff)
			BytePlace = 0x100;
		if (--BytePlace == -1) {
			BitField = fgetbits();
			faddbits(1);
			if (BitField & 0x8000) {
				NumHuf = StMode = 0;
				return;
			} else {
				Length = (BitField & 0x4000) ? 4 : 3;
				faddbits(1);
				Distance = DecodeNum(fgetbits(), STARTHF2, DecHf2, PosHf2);
				Distance = (Distance << 5) | (fgetbits() >> 11);
				faddbits(5);
				OldCopyString(Distance, Length);
				return;
			}
		}
	} else if (NumHuf++ >= 16 && FlagsCnt == 0)
		StMode = 1;
	AvrPlc += BytePlace;
	AvrPlc -= AvrPlc >> 8;
	Nhfb += 16;
	if (Nhfb > 0xff) {
		Nhfb = 0x90;
		Nlzb >>= 1;
	}

	Window[UnpPtr++] = (byte)(ChSet[BytePlace] >> 8);
	--DestUnpSize;

	while (1) {
		CurByte = ChSet[BytePlace];
		NewBytePlace = NToPl[CurByte++ & 0xff]++;
		if ((CurByte & 0xff) > 0xa1)
			CorrHuff(ChSet, NToPl);
		else
			break;
	}

	ChSet[BytePlace] = ChSet[NewBytePlace];
	ChSet[NewBytePlace] = CurByte;
}


void Unpack::GetFlagsBuf() {
	unsigned int Flags, NewFlagsPlace;
	unsigned int FlagsPlace = DecodeNum(fgetbits(), STARTHF2, DecHf2, PosHf2);

	while (1) {
		Flags = ChSetC[FlagsPlace];
		FlagBuf = Flags >> 8;
		NewFlagsPlace = NToPlC[Flags++ & 0xff]++;
		if ((Flags & 0xff) != 0)
			break;
		CorrHuff(ChSetC, NToPlC);
	}

	ChSetC[FlagsPlace] = ChSetC[NewFlagsPlace];
	ChSetC[NewFlagsPlace] = Flags;
}


void Unpack::OldUnpInitData(int Solid) {
	if (!Solid) {
		AvrPlcB = AvrLn1 = AvrLn2 = AvrLn3 = NumHuf = Buf60 = 0;
		AvrPlc = 0x3500;
		MaxDist3 = 0x2001;
		Nhfb = Nlzb = 0x80;
	}
	FlagsCnt = 0;
	FlagBuf = 0;
	StMode = 0;
	LCount = 0;
	ReadTop = 0;
}


void Unpack::InitHuff() {
	for (unsigned int I = 0; I < 256; I++) {
		Place[I] = PlaceA[I] = PlaceB[I] = I;
		PlaceC[I] = (~I + 1) & 0xff;
		ChSet[I] = ChSetB[I] = I << 8;
		ChSetA[I] = I;
		ChSetC[I] = ((~I + 1) & 0xff) << 8;
	}
	memset(NToPl, 0, sizeof(NToPl));
	memset(NToPlB, 0, sizeof(NToPlB));
	memset(NToPlC, 0, sizeof(NToPlC));
	CorrHuff(ChSetB, NToPlB);
}


void Unpack::CorrHuff(unsigned int *CharSet, unsigned int *NumToPlace) {
	int I, J;
	for (I = 7; I >= 0; I--)
		for (J = 0; J < 32; J++, CharSet++)
			*CharSet = (*CharSet & ~0xff) | I;
	memset(NumToPlace, 0, sizeof(NToPl));
	for (I = 6; I >= 0; I--)
		NumToPlace[I] = (7 - I) * 32;
}


void Unpack::OldCopyString(unsigned int Distance, unsigned int Length) {
	DestUnpSize -= Length;
	while (Length--) {
		Window[UnpPtr] = Window[(UnpPtr - Distance) & MAXWINMASK];
		UnpPtr = (UnpPtr + 1) & MAXWINMASK;
	}
}


unsigned int Unpack::DecodeNum(int Num, unsigned int StartPos,
                               unsigned int *DecTab, unsigned int *PosTab) {
	int I;
	for (Num &= 0xfff0, I = 0; DecTab[I] <= Num; I++)
		StartPos++;
	faddbits(StartPos);
	return (((Num - (I ? DecTab[I - 1] : 0)) >> (16 - StartPos)) + PosTab[StartPos]);
}


/***** File: unpack20.cpp *****/

void Unpack::CopyString20(unsigned int Length, unsigned int Distance) {
	LastDist = OldDist[OldDistPtr++ & 3] = Distance;
	LastLength = Length;
	DestUnpSize -= Length;

	unsigned int DestPtr = UnpPtr - Distance;
	if (DestPtr < MAXWINSIZE - 300 && UnpPtr < MAXWINSIZE - 300) {
		Window[UnpPtr++] = Window[DestPtr++];
		Window[UnpPtr++] = Window[DestPtr++];
		while (Length > 2) {
			Length--;
			Window[UnpPtr++] = Window[DestPtr++];
		}
	} else
		while (Length--) {
			Window[UnpPtr] = Window[DestPtr++ & MAXWINMASK];
			UnpPtr = (UnpPtr + 1) & MAXWINMASK;
		}
}


void Unpack::Unpack20(bool Solid) {
	static unsigned char LDecode[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224};
	static unsigned char LBits[] =  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5};
	static int DDecode[] = {0, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512, 768, 1024, 1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576, 32768U, 49152U, 65536, 98304, 131072, 196608, 262144, 327680, 393216, 458752, 524288, 589824, 655360, 720896, 786432, 851968, 917504, 983040};
	static unsigned char DBits[] =  {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5,  6,  6,  7,  7,  8,  8,   9,   9,  10,  10,  11,  11,  12,   12,   13,   13,    14,    14,   15,   15,    16,    16,    16,    16,    16,    16,    16,    16,    16,    16,    16,    16,    16,    16};
	static unsigned char SDDecode[] = {0, 4, 8, 16, 32, 64, 128, 192};
	static unsigned char SDBits[] =  {2, 2, 3, 4, 5, 6,  6,  6};
	unsigned int Bits;

	if (Suspended)
		UnpPtr = WrPtr;
	else {
		UnpInitData(Solid);
		if (!UnpReadBuf())
			return;
		if (!Solid)
			if (!ReadTables20())
				return;
		--DestUnpSize;
	}

	while (is64plus(DestUnpSize)) {
		UnpPtr &= MAXWINMASK;

		if (InAddr > ReadTop - 30)
			if (!UnpReadBuf())
				break;
		if (((WrPtr - UnpPtr) & MAXWINMASK) < 270 && WrPtr != UnpPtr) {
			OldUnpWriteBuf();
			if (Suspended)
				return;
		}
		if (UnpAudioBlock) {
			int AudioNumber = DecodeNumber((struct Decode *)&MD[UnpCurChannel]);

			if (AudioNumber == 256) {
				if (!ReadTables20())
					break;
				continue;
			}
			Window[UnpPtr++] = DecodeAudio(AudioNumber);
			if (++UnpCurChannel == UnpChannels)
				UnpCurChannel = 0;
			--DestUnpSize;
			continue;
		}

		int Number = DecodeNumber((struct Decode *)&LD);
		if (Number < 256) {
			Window[UnpPtr++] = (byte)Number;
			--DestUnpSize;
			continue;
		}
		if (Number > 269) {
			int Length = LDecode[Number -= 270] + 3;
			if ((Bits = LBits[Number]) > 0) {
				Length += getbits() >> (16 - Bits);
				addbits(Bits);
			}

			int DistNumber = DecodeNumber((struct Decode *)&DD);
			unsigned int Distance = DDecode[DistNumber] + 1;
			if ((Bits = DBits[DistNumber]) > 0) {
				Distance += getbits() >> (16 - Bits);
				addbits(Bits);
			}

			if (Distance >= 0x2000) {
				Length++;
				if (Distance >= 0x40000L)
					Length++;
			}

			CopyString20(Length, Distance);
			continue;
		}
		if (Number == 269) {
			if (!ReadTables20())
				break;
			continue;
		}
		if (Number == 256) {
			CopyString20(LastLength, LastDist);
			continue;
		}
		if (Number < 261) {
			unsigned int Distance = OldDist[(OldDistPtr - (Number - 256)) & 3];
			int LengthNumber = DecodeNumber((struct Decode *)&RD);
			int Length = LDecode[LengthNumber] + 2;
			if ((Bits = LBits[LengthNumber]) > 0) {
				Length += getbits() >> (16 - Bits);
				addbits(Bits);
			}
			if (Distance >= 0x101) {
				Length++;
				if (Distance >= 0x2000) {
					Length++;
					if (Distance >= 0x40000)
						Length++;
				}
			}
			CopyString20(Length, Distance);
			continue;
		}
		if (Number < 270) {
			unsigned int Distance = SDDecode[Number -= 261] + 1;
			if ((Bits = SDBits[Number]) > 0) {
				Distance += getbits() >> (16 - Bits);
				addbits(Bits);
			}
			CopyString20(2, Distance);
			continue;
		}
	}
	ReadLastTables();
	OldUnpWriteBuf();
}


bool Unpack::ReadTables20() {
	byte BitLength[BC20];
	unsigned char Table[MC20 * 4];
	int TableSize, N, I;
	if (InAddr > ReadTop - 25)
		if (!UnpReadBuf())
			return (false);
	unsigned int BitField = getbits();
	UnpAudioBlock = (BitField & 0x8000);

	if (!(BitField & 0x4000))
		memset(UnpOldTable20, 0, sizeof(UnpOldTable20));
	addbits(2);

	if (UnpAudioBlock) {
		UnpChannels = ((BitField >> 12) & 3) + 1;
		if (UnpCurChannel >= UnpChannels)
			UnpCurChannel = 0;
		addbits(2);
		TableSize = MC20 * UnpChannels;
	} else
		TableSize = NC20 + DC20 + RC20;

	for (I = 0; I < BC20; I++) {
		BitLength[I] = (byte)(getbits() >> 12);
		addbits(4);
	}
	MakeDecodeTables(BitLength, (struct Decode *)&BD, BC20);
	I = 0;
	while (I < TableSize) {
		if (InAddr > ReadTop - 5)
			if (!UnpReadBuf())
				return (false);
		int Number = DecodeNumber((struct Decode *)&BD);
		if (Number < 16) {
			Table[I] = (Number + UnpOldTable20[I]) & 0xf;
			I++;
		} else if (Number == 16) {
			N = (getbits() >> 14) + 3;
			addbits(2);
			while (N-- > 0 && I < TableSize) {
				Table[I] = Table[I - 1];
				I++;
			}
		} else {
			if (Number == 17) {
				N = (getbits() >> 13) + 3;
				addbits(3);
			} else {
				N = (getbits() >> 9) + 11;
				addbits(7);
			}
			while (N-- > 0 && I < TableSize)
				Table[I++] = 0;
		}
	}
	if (InAddr > ReadTop)
		return (true);
	if (UnpAudioBlock)
		for (I = 0; I < UnpChannels; I++)
			MakeDecodeTables(&Table[I * MC20], (struct Decode *)&MD[I], MC20);
	else {
		MakeDecodeTables(&Table[0], (struct Decode *)&LD, NC20);
		MakeDecodeTables(&Table[NC20], (struct Decode *)&DD, DC20);
		MakeDecodeTables(&Table[NC20 + DC20], (struct Decode *)&RD, RC20);
	}
	memcpy(UnpOldTable20, Table, sizeof(UnpOldTable20));
	return (true);
}


void Unpack::ReadLastTables() {
	if (ReadTop >= InAddr + 5)
		if (UnpAudioBlock) {
			if (DecodeNumber((struct Decode *)&MD[UnpCurChannel]) == 256)
				ReadTables20();
		} else if (DecodeNumber((struct Decode *)&LD) == 269)
			ReadTables20();
}


void Unpack::UnpInitData20(int Solid) {
	if (!Solid) {
		UnpChannelDelta = UnpCurChannel = 0;
		UnpChannels = 1;
		memset(AudV, 0, sizeof(AudV));
		memset(UnpOldTable20, 0, sizeof(UnpOldTable20));
	}
}


byte Unpack::DecodeAudio(int Delta) {
	struct AudioVariables *V = &AudV[UnpCurChannel];
	V->ByteCount++;
	V->D4 = V->D3;
	V->D3 = V->D2;
	V->D2 = V->LastDelta - V->D1;
	V->D1 = V->LastDelta;
	int PCh = 8 * V->LastChar + V->K1 * V->D1 + V->K2 * V->D2 + V->K3 * V->D3 + V->K4 * V->D4 + V->K5 * UnpChannelDelta;
	PCh = (PCh >> 3) & 0xFF;

	unsigned int Ch = PCh - Delta;

	int D = ((signed char)Delta) << 3;

	V->Dif[0] += abs(D);
	V->Dif[1] += abs(D - V->D1);
	V->Dif[2] += abs(D + V->D1);
	V->Dif[3] += abs(D - V->D2);
	V->Dif[4] += abs(D + V->D2);
	V->Dif[5] += abs(D - V->D3);
	V->Dif[6] += abs(D + V->D3);
	V->Dif[7] += abs(D - V->D4);
	V->Dif[8] += abs(D + V->D4);
	V->Dif[9] += abs(D - UnpChannelDelta);
	V->Dif[10] += abs(D + UnpChannelDelta);

	UnpChannelDelta = V->LastDelta = (signed char)(Ch - V->LastChar);
	V->LastChar = Ch;

	if ((V->ByteCount & 0x1F) == 0) {
		unsigned int MinDif = V->Dif[0], NumMinDif = 0;
		V->Dif[0] = 0;
		for (int I = 1; I < sizeof(V->Dif) / sizeof(V->Dif[0]); I++) {
			if (V->Dif[I] < MinDif) {
				MinDif = V->Dif[I];
				NumMinDif = I;
			}
			V->Dif[I] = 0;
		}
		switch (NumMinDif) {
		case 1:
			if (V->K1 >= -16)
				V->K1--;
			break;
		case 2:
			if (V->K1 < 16)
				V->K1++;
			break;
		case 3:
			if (V->K2 >= -16)
				V->K2--;
			break;
		case 4:
			if (V->K2 < 16)
				V->K2++;
			break;
		case 5:
			if (V->K3 >= -16)
				V->K3--;
			break;
		case 6:
			if (V->K3 < 16)
				V->K3++;
			break;
		case 7:
			if (V->K4 >= -16)
				V->K4--;
			break;
		case 8:
			if (V->K4 < 16)
				V->K4++;
			break;
		case 9:
			if (V->K5 >= -16)
				V->K5--;
			break;
		case 10:
			if (V->K5 < 16)
				V->K5++;
			break;
		}
	}
	return ((byte)Ch);
}


/***** File: unpack.cpp *****/

Unpack::Unpack(ComprDataIO *DataIO) {
	UnpIO = DataIO;
	Window = NULL;
	ExternalWindow = false;
	Suspended = false;
	UnpAllBuf = false;
	UnpSomeRead = false;
}


Unpack::~Unpack() {
	if (Window != NULL && !ExternalWindow)
		delete[] Window;
	InitFilters();
}


void Unpack::Init(byte *Window) {
	if (Window == NULL) {
		Unpack::Window = new byte[MAXWINSIZE];
		if (Unpack::Window == NULL)
			ErrHandler.MemoryError();
	} else {
		Unpack::Window = Window;
		ExternalWindow = true;
	}
	UnpInitData(false);
}


void Unpack::DoUnpack(int Method, bool Solid) {
	switch (Method) {
#ifndef SFX_MODULE
	case 15:
		Unpack15(Solid);
		break;
	case 20:
	case 26:
		Unpack20(Solid);
		break;
#endif
	case 29:
		Unpack29(Solid);
		break;
	}
}


inline void Unpack::InsertOldDist(unsigned int Distance) {
	OldDist[3] = OldDist[2];
	OldDist[2] = OldDist[1];
	OldDist[1] = OldDist[0];
	OldDist[0] = Distance;
}


inline void Unpack::InsertLastMatch(unsigned int Length, unsigned int Distance) {
	LastDist = Distance;
	LastLength = Length;
}


void Unpack::CopyString(unsigned int Length, unsigned int Distance) {
	unsigned int DestPtr = UnpPtr - Distance;
	if (DestPtr < MAXWINSIZE - 260 && UnpPtr < MAXWINSIZE - 260) {
		Window[UnpPtr++] = Window[DestPtr++];
		while (--Length > 0)
			Window[UnpPtr++] = Window[DestPtr++];
	} else
		while (Length--) {
			Window[UnpPtr] = Window[DestPtr++ & MAXWINMASK];
			UnpPtr = (UnpPtr + 1) & MAXWINMASK;
		}
}


int Unpack::DecodeNumber(struct Decode *Dec) {
	unsigned int Bits;
	unsigned int BitField = getbits() & 0xfffe;
	if (BitField < Dec->DecodeLen[8])
		if (BitField < Dec->DecodeLen[4])
			if (BitField < Dec->DecodeLen[2])
				if (BitField < Dec->DecodeLen[1])
					Bits = 1;
				else
					Bits = 2;
			else if (BitField < Dec->DecodeLen[3])
				Bits = 3;
			else
				Bits = 4;
		else if (BitField < Dec->DecodeLen[6])
			if (BitField < Dec->DecodeLen[5])
				Bits = 5;
			else
				Bits = 6;
		else if (BitField < Dec->DecodeLen[7])
			Bits = 7;
		else
			Bits = 8;
	else if (BitField < Dec->DecodeLen[12])
		if (BitField < Dec->DecodeLen[10])
			if (BitField < Dec->DecodeLen[9])
				Bits = 9;
			else
				Bits = 10;
		else if (BitField < Dec->DecodeLen[11])
			Bits = 11;
		else
			Bits = 12;
	else if (BitField < Dec->DecodeLen[14])
		if (BitField < Dec->DecodeLen[13])
			Bits = 13;
		else
			Bits = 14;
	else
		Bits = 15;

	addbits(Bits);
	unsigned int N = Dec->DecodePos[Bits] + ((BitField - Dec->DecodeLen[Bits - 1]) >> (16 - Bits));
	if (N >= Dec->MaxNum)
		N = 0;
	return (Dec->DecodeNum[N]);
}


void Unpack::Unpack29(bool Solid) {
	static unsigned char LDecode[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224};
	static unsigned char LBits[] =  {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,  4,  5,  5,  5,  5};
	static int DDecode[DC];
	static byte DBits[DC];
	static int DBitLengthCounts[] = {4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 14, 0, 12};
	static unsigned char SDDecode[] = {0, 4, 8, 16, 32, 64, 128, 192};
	static unsigned char SDBits[] =  {2, 2, 3, 4, 5, 6,  6,  6};
	unsigned int Bits;

	if (DDecode[1] == 0) {
		int Dist = 0, BitLength = 0, Slot = 0;
		for (int I = 0; I < sizeof(DBitLengthCounts) / sizeof(DBitLengthCounts[0]); I++, BitLength++)
			for (int J = 0; J < DBitLengthCounts[I]; J++, Slot++, Dist += (1 << BitLength)) {
				DDecode[Slot] = Dist;
				DBits[Slot] = BitLength;
			}
	}

	FileExtracted = true;

	if (!Suspended) {
		UnpInitData(Solid);
		if (!UnpReadBuf())
			return;
		if ((!Solid || !TablesRead) && !ReadTables())
			return;
	}

	while (true) {
		UnpPtr &= MAXWINMASK;

		if (InAddr > ReadTop - 30) {
			if (!UnpReadBuf())
				break;
		}
		if (((WrPtr - UnpPtr) & MAXWINMASK) < 260 && WrPtr != UnpPtr) {
			UnpWriteBuf();
			if (WrittenFileSize > DestUnpSize)
				return;
			if (Suspended) {
				FileExtracted = false;
				return;
			}
		}
		if (UnpBlockType == BLOCK_PPM) {
			int Ch = PPM.DecodeChar();
			if (Ch == -1)
				break;
			if (Ch == PPMEscChar) {
				int NextCh = PPM.DecodeChar();
				if (NextCh == 0) {
					if (!ReadTables())
						break;
					continue;
				}
				if (NextCh == 2 || NextCh == -1)
					break;
				if (NextCh == 3) {
					if (!ReadVMCodePPM())
						break;
					continue;
				}
				if (NextCh == 4) {
					unsigned int Distance = 0, Length;
					bool Failed = false;
					for (int I = 0; I < 4 && !Failed; I++) {
						int Ch = PPM.DecodeChar();
						if (Ch == -1)
							Failed = true;
						else if (I == 3)
							Length = (byte)Ch;
						else
							Distance = (Distance << 8) + (byte)Ch;
					}
					if (Failed)
						break;
					CopyString(Length + 32, Distance + 2);
					continue;
				}
				if (NextCh == 5) {
					int Length = PPM.DecodeChar();
					if (Length == -1)
						break;
					CopyString(Length + 4, 1);
					continue;
				}
			}
			Window[UnpPtr++] = Ch;
			continue;
		}

		int Number = DecodeNumber((struct Decode *)&LD);
		if (Number < 256) {
			Window[UnpPtr++] = (byte)Number;
			continue;
		}
		if (Number >= 271) {
			int Length = LDecode[Number -= 271] + 3;
			if ((Bits = LBits[Number]) > 0) {
				Length += getbits() >> (16 - Bits);
				addbits(Bits);
			}

			int DistNumber = DecodeNumber((struct Decode *)&DD);
			unsigned int Distance = DDecode[DistNumber] + 1;
			if ((Bits = DBits[DistNumber]) > 0) {
				if (DistNumber > 9) {
					if (Bits > 4) {
						Distance += ((getbits() >> (20 - Bits)) << 4);
						addbits(Bits - 4);
					}
					if (LowDistRepCount > 0) {
						LowDistRepCount--;
						Distance += PrevLowDist;
					} else {
						int LowDist = DecodeNumber((struct Decode *)&LDD);
						if (LowDist == 16) {
							LowDistRepCount = LOW_DIST_REP_COUNT - 1;
							Distance += PrevLowDist;
						} else {
							Distance += LowDist;
							PrevLowDist = LowDist;
						}
					}
				} else {
					Distance += getbits() >> (16 - Bits);
					addbits(Bits);
				}
			}

			if (Distance >= 0x2000) {
				Length++;
				if (Distance >= 0x40000L)
					Length++;
			}

			InsertOldDist(Distance);
			InsertLastMatch(Length, Distance);
			CopyString(Length, Distance);
			continue;
		}
		if (Number == 256) {
			if (!ReadEndOfBlock())
				break;
			continue;
		}
		if (Number == 257) {
			if (!ReadVMCode())
				break;
			continue;
		}
		if (Number == 258) {
			CopyString(LastLength, LastDist);
			continue;
		}
		if (Number < 263) {
			int DistNum = Number - 259;
			unsigned int Distance = OldDist[DistNum];
			for (int I = DistNum; I > 0; I--)
				OldDist[I] = OldDist[I - 1];
			OldDist[0] = Distance;

			int LengthNumber = DecodeNumber((struct Decode *)&RD);
			int Length = LDecode[LengthNumber] + 2;
			if ((Bits = LBits[LengthNumber]) > 0) {
				Length += getbits() >> (16 - Bits);
				addbits(Bits);
			}
			InsertLastMatch(Length, Distance);
			CopyString(Length, Distance);
			continue;
		}
		if (Number < 272) {
			unsigned int Distance = SDDecode[Number -= 263] + 1;
			if ((Bits = SDBits[Number]) > 0) {
				Distance += getbits() >> (16 - Bits);
				addbits(Bits);
			}
			InsertOldDist(Distance);
			InsertLastMatch(2, Distance);
			CopyString(2, Distance);
			continue;
		}
	}
	UnpWriteBuf();
}


bool Unpack::ReadEndOfBlock() {
	unsigned int BitField = getbits();
	bool NewTable, NewFile = false;
	if (BitField & 0x8000) {
		NewTable = true;
		addbits(1);
	} else {
		NewFile = true;
		NewTable = (BitField & 0x4000);
		addbits(2);
	}
	TablesRead = !NewTable;
	return !(NewFile || NewTable && !ReadTables());
}


bool Unpack::ReadVMCode() {
	unsigned int FirstByte = getbits() >> 8;
	addbits(8);
	int Length = (FirstByte & 7) + 1;
	if (Length == 7) {
		Length = (getbits() >> 8) + 7;
		addbits(8);
	} else if (Length == 8) {
		Length = getbits();
		addbits(16);
	}
	Array<byte> VMCode(Length);
	for (int I = 0; I < Length; I++) {
		if (InAddr >= ReadTop - 1 && !UnpReadBuf() && I < Length - 1)
			return (false);
		VMCode[I] = getbits() >> 8;
		addbits(8);
	}
	return (AddVMCode(FirstByte, &VMCode[0], Length));
}


bool Unpack::ReadVMCodePPM() {
	unsigned int FirstByte = PPM.DecodeChar();
	int Length = (FirstByte & 7) + 1;
	if (Length == 7)
		Length = PPM.DecodeChar() + 7;
	else if (Length == 8)
		Length = PPM.DecodeChar() * 256 + PPM.DecodeChar();
	Array<byte> VMCode(Length);
	for (int I = 0; I < Length; I++) {
		int Ch = PPM.DecodeChar();
		if (Ch == -1)
			return (false);
		VMCode[I] = Ch;
	}
	return (AddVMCode(FirstByte, &VMCode[0], Length));
}


bool Unpack::AddVMCode(unsigned int FirstByte, byte *Code, int CodeSize) {
	BitInput Inp;
	Inp.InitBitInput();
	memcpy(Inp.InBuf, Code, Min(BitInput::MAX_SIZE, CodeSize));
	VM.Init();

	uint FiltPos;
	if (FirstByte & 0x80) {
		FiltPos = RarVM::ReadData(Inp);
		if (FiltPos == 0)
			InitFilters();
		else
			FiltPos--;
	} else
		FiltPos = LastFilter;
	if (FiltPos > Filters.Size() || FiltPos > OldFilterLengths.Size())
		return (false);
	LastFilter = FiltPos;
	bool NewFilter = (FiltPos == Filters.Size());

	UnpackFilter *Filter;
	if (NewFilter) {
		Filters.Add(1);
		Filters[Filters.Size() - 1] = Filter = new UnpackFilter;
		OldFilterLengths.Add(1);
		Filter->ExecCount = 0;
	} else {
		Filter = Filters[FiltPos];
		Filter->ExecCount++;
	}

	UnpackFilter *StackFilter = new UnpackFilter;

	int EmptyCount = 0;
	for (int I = 0; I < PrgStack.Size(); I++) {
		PrgStack[I - EmptyCount] = PrgStack[I];
		if (PrgStack[I] == NULL)
			EmptyCount++;
		if (EmptyCount > 0)
			PrgStack[I] = NULL;
	}
	if (EmptyCount == 0) {
		PrgStack.Add(1);
		EmptyCount = 1;
	}
	int StackPos = PrgStack.Size() - EmptyCount;
	PrgStack[StackPos] = StackFilter;
	StackFilter->ExecCount = Filter->ExecCount;

	uint BlockStart = RarVM::ReadData(Inp);
	if (FirstByte & 0x40)
		BlockStart += 258;
	StackFilter->BlockStart = (BlockStart + UnpPtr)&MAXWINMASK;
	if (FirstByte & 0x20)
		StackFilter->BlockLength = RarVM::ReadData(Inp);
	else
		StackFilter->BlockLength = FiltPos < OldFilterLengths.Size() ? OldFilterLengths[FiltPos] : 0;
	StackFilter->NextWindow = WrPtr != UnpPtr && ((WrPtr - UnpPtr)&MAXWINMASK) <= BlockStart;

//  DebugLog("\nNextWindow: UnpPtr=%08x WrPtr=%08x BlockStart=%08x",UnpPtr,WrPtr,BlockStart);

	OldFilterLengths[FiltPos] = StackFilter->BlockLength;

	memset(StackFilter->Prg.InitR, 0, sizeof(StackFilter->Prg.InitR));
	StackFilter->Prg.InitR[3] = VM_GLOBALMEMADDR;
	StackFilter->Prg.InitR[4] = StackFilter->BlockLength;
	StackFilter->Prg.InitR[5] = StackFilter->ExecCount;
	if (FirstByte & 0x10) {
		unsigned int InitMask = Inp.fgetbits() >> 9;
		Inp.faddbits(7);
		for (int I = 0; I < 7; I++)
			if (InitMask & (1 << I))
				StackFilter->Prg.InitR[I] = RarVM::ReadData(Inp);
	}
	if (NewFilter) {
		uint VMCodeSize = RarVM::ReadData(Inp);
		if (VMCodeSize >= 0x10000 || VMCodeSize == 0)
			return (false);
		Array<byte> VMCode(VMCodeSize);
		for (int I = 0; I < VMCodeSize; I++) {
			VMCode[I] = Inp.fgetbits() >> 8;
			Inp.faddbits(8);
		}
		VM.Prepare(&VMCode[0], VMCodeSize, &Filter->Prg);
	}
	StackFilter->Prg.AltCmd = &Filter->Prg.Cmd[0];
	StackFilter->Prg.CmdCount = Filter->Prg.CmdCount;

	int StaticDataSize = Filter->Prg.StaticData.Size();
	if (StaticDataSize > 0 && StaticDataSize < VM_GLOBALMEMSIZE) {
		StackFilter->Prg.StaticData.Add(StaticDataSize);
		memcpy(&StackFilter->Prg.StaticData[0], &Filter->Prg.StaticData[0], StaticDataSize);
	}

	if (StackFilter->Prg.GlobalData.Size() < VM_FIXEDGLOBALSIZE) {
		StackFilter->Prg.GlobalData.Reset();
		StackFilter->Prg.GlobalData.Add(VM_FIXEDGLOBALSIZE);
	}
	byte *GlobalData = &StackFilter->Prg.GlobalData[0];
	for (int I = 0; I < 7; I++)
		VM.SetValue((uint *)&GlobalData[I * 4], StackFilter->Prg.InitR[I]);
	VM.SetValue((uint *)&GlobalData[0x1c], StackFilter->BlockLength);
	VM.SetValue((uint *)&GlobalData[0x20], 0);
	VM.SetValue((uint *)&GlobalData[0x2c], StackFilter->ExecCount);
	memset(&GlobalData[0x30], 0, 16);

	if (FirstByte & 8) {
		uint DataSize = RarVM::ReadData(Inp);
		if (DataSize >= 0x10000)
			return (false);
		unsigned int CurSize = StackFilter->Prg.GlobalData.Size();
		if (CurSize < DataSize + VM_FIXEDGLOBALSIZE)
			StackFilter->Prg.GlobalData.Add(DataSize + VM_FIXEDGLOBALSIZE - CurSize);
		byte *GlobalData = &StackFilter->Prg.GlobalData[VM_FIXEDGLOBALSIZE];
		for (int I = 0; I < DataSize; I++) {
			GlobalData[I] = Inp.fgetbits() >> 8;
			Inp.faddbits(8);
		}
	}
	return (true);
}


bool Unpack::UnpReadBuf() {
	int DataSize = ReadTop - InAddr;
	if (DataSize < 0)
		return (false);
	if (InAddr > BitInput::MAX_SIZE / 2) {
		if (DataSize > 0)
			memmove(InBuf, InBuf + InAddr, DataSize);
		InAddr = 0;
		ReadTop = DataSize;
	} else
		DataSize = InAddr;
	int ReadCode = UnpIO->UnpRead(InBuf + DataSize, (BitInput::MAX_SIZE - DataSize) & ~0xf);
	if (ReadCode > 0)
		ReadTop += ReadCode;
	return (ReadCode != -1);
}


void Unpack::UnpWriteBuf() {
	unsigned int WrittenBorder = WrPtr;
	unsigned int WriteSize = (UnpPtr - WrittenBorder)&MAXWINMASK;
	for (int I = 0; I < PrgStack.Size(); I++) {
		UnpackFilter *flt = PrgStack[I];
		if (flt == NULL)
			continue;
		if (flt->NextWindow) {
			flt->NextWindow = false;
			continue;
		}
		unsigned int BlockStart = flt->BlockStart;
		unsigned int BlockLength = flt->BlockLength;
		if (((BlockStart - WrittenBorder)&MAXWINMASK) < WriteSize) {
			if (WrittenBorder != BlockStart) {
				UnpWriteArea(WrittenBorder, BlockStart);
				WrittenBorder = BlockStart;
				WriteSize = (UnpPtr - WrittenBorder)&MAXWINMASK;
			}
			if (BlockLength <= WriteSize) {
				unsigned int BlockEnd = (BlockStart + BlockLength)&MAXWINMASK;
				if (BlockStart < BlockEnd || BlockEnd == 0)
					VM.SetMemory(0, Window + BlockStart, BlockLength);
				else {
					unsigned int FirstPartLength = MAXWINSIZE - BlockStart;
					VM.SetMemory(0, Window + BlockStart, FirstPartLength);
					VM.SetMemory(FirstPartLength, Window, BlockEnd);
				}
				VM_PreparedProgram *Prg = &flt->Prg;
				ExecuteCode(Prg);

				byte *FilteredData = Prg->FilteredData;
				unsigned int FilteredDataSize = Prg->FilteredDataSize;

				delete PrgStack[I];
				PrgStack[I] = NULL;
				while (I + 1 < PrgStack.Size()) {
					UnpackFilter *NextFilter = PrgStack[I + 1];
					if (NextFilter == NULL || NextFilter->BlockStart != BlockStart ||
					        NextFilter->BlockLength != FilteredDataSize)
						break;
					VM.SetMemory(0, FilteredData, FilteredDataSize);
					VM_PreparedProgram *NextPrg = &PrgStack[I + 1]->Prg;
					ExecuteCode(NextPrg);
					FilteredData = NextPrg->FilteredData;
					FilteredDataSize = NextPrg->FilteredDataSize;
					I++;
					delete PrgStack[I];
					PrgStack[I] = NULL;
				}
				UnpIO->UnpWrite(FilteredData, FilteredDataSize);
				UnpSomeRead = true;
				WrittenFileSize += FilteredDataSize;
				WrittenBorder = BlockEnd;
				WriteSize = (UnpPtr - WrittenBorder)&MAXWINMASK;
			} else {
				for (int J = I; J < PrgStack.Size(); J++) {
					UnpackFilter *flt = PrgStack[J];
					if (flt != NULL && flt->NextWindow)
						flt->NextWindow = false;
				}
				WrPtr = WrittenBorder;
				return;
			}
		}
	}

	UnpWriteArea(WrittenBorder, UnpPtr);
	WrPtr = UnpPtr;
}


void Unpack::ExecuteCode(VM_PreparedProgram *Prg) {
	if (Prg->GlobalData.Size() > 0) {
		Prg->InitR[6] = int64to32(WrittenFileSize);
		VM.SetValue((uint *)&Prg->GlobalData[0x24], int64to32(WrittenFileSize));
		VM.SetValue((uint *)&Prg->GlobalData[0x28], int64to32(WrittenFileSize >> 32));
		VM.Execute(Prg);
	}
}


void Unpack::UnpWriteArea(unsigned int StartPtr, unsigned int EndPtr) {
	if (EndPtr != StartPtr)
		UnpSomeRead = true;
	if (EndPtr < StartPtr) {
		UnpWriteData(&Window[StartPtr], -StartPtr & MAXWINMASK);
		UnpWriteData(Window, EndPtr);
		UnpAllBuf = true;
	} else
		UnpWriteData(&Window[StartPtr], EndPtr - StartPtr);
}


void Unpack::UnpWriteData(byte *Data, int Size) {
	if (WrittenFileSize >= DestUnpSize)
		return;
	int WriteSize = Size;
	Int64 LeftToWrite = DestUnpSize - WrittenFileSize;
	if (WriteSize > LeftToWrite)
		WriteSize = int64to32(LeftToWrite);
	UnpIO->UnpWrite(Data, WriteSize);
	WrittenFileSize += Size;
}


bool Unpack::ReadTables() {
	byte BitLength[BC];
	unsigned char Table[HUFF_TABLE_SIZE];
	if (InAddr > ReadTop - 25)
		if (!UnpReadBuf())
			return (false);
	faddbits((8 - InBit) & 7);
	unsigned int BitField = fgetbits();
	if (BitField & 0x8000) {
		UnpBlockType = BLOCK_PPM;
		return (PPM.DecodeInit(this, PPMEscChar));
	}
	UnpBlockType = BLOCK_LZ;

	PrevLowDist = 0;
	LowDistRepCount = 0;

	if (!(BitField & 0x4000))
		memset(UnpOldTable, 0, sizeof(UnpOldTable));
	faddbits(2);

	for (int I = 0; I < BC; I++) {
		int Length = (byte)(fgetbits() >> 12);
		faddbits(4);
		if (Length == 15) {
			int ZeroCount = (byte)(fgetbits() >> 12);
			faddbits(4);
			if (ZeroCount == 0)
				BitLength[I] = 15;
			else {
				ZeroCount += 2;
				while (ZeroCount-- > 0 && I < sizeof(BitLength) / sizeof(BitLength[0]))
					BitLength[I++] = 0;
				I--;
			}
		} else
			BitLength[I] = Length;
	}
	MakeDecodeTables(BitLength, (struct Decode *)&BD, BC);

	const int TableSize = HUFF_TABLE_SIZE;
	for (int I = 0; I < TableSize;) {
		if (InAddr > ReadTop - 5)
			if (!UnpReadBuf())
				return (false);
		int Number = DecodeNumber((struct Decode *)&BD);
		if (Number < 16) {
			Table[I] = (Number + UnpOldTable[I]) & 0xf;
			I++;
		} else if (Number < 18) {
			int N;
			if (Number == 16) {
				N = (fgetbits() >> 13) + 3;
				faddbits(3);
			} else {
				N = (fgetbits() >> 9) + 11;
				faddbits(7);
			}
			while (N-- > 0 && I < TableSize) {
				Table[I] = Table[I - 1];
				I++;
			}
		} else {
			int N;
			if (Number == 18) {
				N = (fgetbits() >> 13) + 3;
				faddbits(3);
			} else {
				N = (fgetbits() >> 9) + 11;
				faddbits(7);
			}
			while (N-- > 0 && I < TableSize)
				Table[I++] = 0;
		}
	}
	TablesRead = true;
	if (InAddr > ReadTop)
		return (false);
	MakeDecodeTables(&Table[0], (struct Decode *)&LD, NC);
	MakeDecodeTables(&Table[NC], (struct Decode *)&DD, DC);
	MakeDecodeTables(&Table[NC + DC], (struct Decode *)&LDD, LDC);
	MakeDecodeTables(&Table[NC + DC + LDC], (struct Decode *)&RD, RC);
	memcpy(UnpOldTable, Table, sizeof(UnpOldTable));
	return (true);
}


void Unpack::UnpInitData(int Solid) {
	if (!Solid) {
		TablesRead = false;
		memset(OldDist, 0, sizeof(OldDist));
		OldDistPtr = 0;
		LastDist = LastLength = 0;
//    memset(Window,0,MAXWINSIZE);
		memset(UnpOldTable, 0, sizeof(UnpOldTable));
		UnpPtr = WrPtr = 0;
		PPMEscChar = 2;

		InitFilters();
	}
	InitBitInput();
	WrittenFileSize = 0;
	ReadTop = 0;

#ifndef SFX_MODULE
	UnpInitData20(Solid);
#endif
}


void Unpack::InitFilters() {
	OldFilterLengths.Reset();
	LastFilter = 0;

	for (int I = 0; I < Filters.Size(); I++)
		delete Filters[I];
	Filters.Reset();
	for (int I = 0; I < PrgStack.Size(); I++)
		delete PrgStack[I];
	PrgStack.Reset();
}


void Unpack::MakeDecodeTables(unsigned char *LenTab, struct Decode *Dec, int Size) {
	int LenCount[16], TmpPos[16], I;
	long M, N;
	memset(LenCount, 0, sizeof(LenCount));
	memset(Dec->DecodeNum, 0, Size * sizeof(*Dec->DecodeNum));
	for (I = 0; I < Size; I++)
		LenCount[LenTab[I] & 0xF]++;

	LenCount[0] = 0;
	for (TmpPos[0] = Dec->DecodePos[0] = Dec->DecodeLen[0] = 0, N = 0, I = 1; I < 16; I++) {
		N = 2 * (N + LenCount[I]);
		M = N << (15 - I);
		if (M > 0xFFFF)
			M = 0xFFFF;
		Dec->DecodeLen[I] = (unsigned int)M;
		TmpPos[I] = Dec->DecodePos[I] = Dec->DecodePos[I - 1] + LenCount[I - 1];
	}

	for (I = 0; I < Size; I++)
		if (LenTab[I] != 0)
			Dec->DecodeNum[TmpPos[LenTab[I] & 0xF]++] = I;
	Dec->MaxNum = Size;
}


/***** File: cmddata.cpp *****/

CommandData::CommandData() {
	FileArgs = ExclArgs = StoreArgs = ArcNames = NULL;
	Init();
}


CommandData::~CommandData() {
	Close();
}


void CommandData::Init() {
	Close();

	*Command = 0;
	*ArcName = 0;
	*ArcNameW = 0;
	FileLists = false;
	NoMoreSwitches = false;
	TimeConverted = false;

	FileArgs = new StringList;
	ExclArgs = new StringList;
	StoreArgs = new StringList;
	ArcNames = new StringList;
}


void CommandData::Close() {
	delete FileArgs;
	delete ExclArgs;
	delete StoreArgs;
	delete ArcNames;
	FileArgs = ExclArgs = StoreArgs = ArcNames = NULL;
}


#ifndef SFX_MODULE
void CommandData::ParseArg(char *Arg) {
	if (IsSwitch(*Arg) && !NoMoreSwitches)
		if (Arg[1] == '-')
			NoMoreSwitches = true;
		else
			ProcessSwitch(&Arg[1]);
	else if (*Command == 0) {
		strncpy(Command, Arg, sizeof(Command));
		if (toupper(*Command) == 'S') {
			const char *SFXName = Command[1] ? Command + 1 : DefSFXName;
			if (PointToName(SFXName) != SFXName || FileExist(SFXName))
				strcpy(SFXModule, SFXName);
			else
				GetConfigName(SFXName, SFXModule);
		}
#ifndef GUI
		*Command = toupper(*Command);
		if (*Command != 'I' && *Command != 'S')
			strupper(Command);
#endif
	} else if (*ArcName == 0)
		strncpy(ArcName, Arg, sizeof(ArcName));
	else {
		int Length = strlen(Arg);
		char EndChar = Arg[Length - 1];
		char CmdChar = toupper(*Command);
		bool Add = strchr("AFUM", CmdChar) != NULL;
		bool Extract = CmdChar == 'X' || CmdChar == 'E';
		if ((IsDriveDiv(EndChar) || IsPathDiv(EndChar)) && !Add)
			strcpy(ExtrPath, Arg);
		else if ((Add || CmdChar == 'T') && *Arg != '@')
			FileArgs->AddString(Arg);
		else {
			struct FindData FileData;
			bool Found = FindFile::FastFind(Arg, NULL, &FileData);
			if (!Found && *Arg == '@' && !IsWildcard(Arg)) {
				ReadTextFile(Arg + 1, FileArgs, false, true, true, true, true);
				FileLists = true;
			} else if (Found && FileData.IsDir && Extract && *ExtrPath == 0) {
				strcpy(ExtrPath, Arg);
				AddEndSlash(ExtrPath);
			} else
				FileArgs->AddString(Arg);
		}
	}
}
#endif


void CommandData::ParseDone() {
	if (FileArgs->ItemsCount() == 0 && !FileLists)
		FileArgs->AddString(MASKALL);
	char CmdChar = toupper(*Command);
	bool Extract = CmdChar == 'X' || CmdChar == 'E';
	if (Test && Extract)
		Test = false;
}


#ifndef SFX_MODULE
void CommandData::ParseEnvVar() {
	char *EnvStr = getenv("RAR");
	if (EnvStr != NULL)
		ProcessSwitchesString(EnvStr);
}
#endif


#if !defined(GUI) && !defined(SFX_MODULE)
bool CommandData::IsConfigEnabled(int argc, char *argv[]) {
	for (int I = 1; I < argc; I++)
		if (IsSwitch(*argv[I]) && stricomp(&argv[I][1], "cfg-") == 0)
			return (false);
	return (true);
}
#endif


#if !defined(GUI) && !defined(SFX_MODULE)
void CommandData::ReadConfig(int argc, char *argv[]) {
	StringList List;
	if (ReadTextFile(DefConfigName, &List, true)) {
		char *Str;
		while ((Str = List.GetString()) != NULL)
			if (strnicomp(Str, "switches=", 9) == 0)
				ProcessSwitchesString(Str + 9);
	}
}
#endif


#ifndef SFX_MODULE
void CommandData::ProcessSwitchesString(char *Str) {
	while (*Str) {
		while (!IsSwitch(*Str) && *Str != 0)
			Str++;
		if (*Str == 0)
			break;
		char *Next = Str;
		while (!(Next[0] == ' ' && IsSwitch(Next[1])) && *Next != 0)
			Next++;
		char NextChar = *Next;
		*Next = 0;
		ProcessSwitch(Str + 1);
		*Next = NextChar;
		Str = Next;
	}
}
#endif


#ifndef SFX_MODULE
void CommandData::ProcessSwitch(char *Switch) {

	switch (toupper(Switch[0])) {
	case 'I':
		if (strnicomp(&Switch[1], "LOG", 3) == 0) {
			strncpy(LogName, Switch[4] ? Switch + 4 : DefLogName, sizeof(LogName));
			break;
		}
		if (stricomp(&Switch[1], "SND") == 0) {
			Sound = true;
			break;
		}
		if (stricomp(&Switch[1], "ERR") == 0) {
			MsgStream = MSG_STDERR;
			break;
		}
		if (strnicomp(&Switch[1], "EML", 3) == 0) {
			strncpy(EmailTo, Switch[4] ? Switch + 4 : "@", sizeof(EmailTo));
			EmailTo[sizeof(EmailTo) - 1] = 0;
			break;
		}
		if (stricomp(&Switch[1], "NUL") == 0) {
			MsgStream = MSG_NULL;
			break;
		}
		if (stricomp(&Switch[1], "DP") == 0) {
			DisablePercentage = true;
			break;
		}
		if (stricomp(&Switch[1], "OFF") == 0) {
			Shutdown = true;
			break;
		}
		break;
	case 'T':
		switch (toupper(Switch[1])) {
		case 'K':
			ArcTime = ARCTIME_KEEP;
			break;
		case 'L':
			ArcTime = ARCTIME_LATEST;
			break;
		case 'O':
			FileTimeOlder = TextAgeToSeconds(Switch + 2);
			break;
		case 'N':
			FileTimeNewer = TextAgeToSeconds(Switch + 2);
			break;
		case 'B':
			FileTimeBefore = IsoTextToDosTime(Switch + 2);
			break;
		case 'A':
			FileTimeAfter = IsoTextToDosTime(Switch + 2);
			break;
		case '-':
			Test = false;
			break;
		case 0:
			Test = true;
			break;
		default:
			BadSwitch(Switch);
			break;
		}
		break;
	case 'A':
		switch (toupper(Switch[1])) {
		case 'C':
			ClearArc = true;
			break;
		case 'D':
			AppendArcNameToPath = true;
			break;
		case 'G':
			if (Switch[2] == '-' && Switch[3] == 0)
				GenerateArcName = 0;
			else {
				GenerateArcName = true;
				strncpy(GenerateMask, Switch + 2, sizeof(GenerateMask));
			}
			break;
		case 'N': //reserved for archive name
			break;
		case 'O':
			AddArcOnly = true;
			break;
		case 'P':
			strcpy(ArcPath, Switch + 2);
			break;
		case 'S':
			SyncFiles = true;
			break;
		}
		break;
	case 'D':
		if (Switch[2] == 0)
			switch (toupper(Switch[1])) {
			case 'S':
				DisableSortSolid = true;
				break;
			case 'H':
				OpenShared = true;
				break;
			case 'F':
				DeleteFiles = true;
				break;
			}
		break;
	case 'O':
		switch (toupper(Switch[1])) {
		case '+':
			Overwrite = OVERWRITE_ALL;
			break;
		case '-':
			Overwrite = OVERWRITE_NONE;
			break;
		case 'W':
			ProcessOwners = true;
			break;
		default :
			BadSwitch(Switch);
			break;
		}
		break;
	case 'R':
		switch (toupper(Switch[1])) {
		case 0:
			Recurse = RECURSE_ALWAYS;
			break;
		case '-':
			Recurse = 0;
			break;
		case '0':
			Recurse = RECURSE_WILDCARDS;
			break;
		case 'R':
			Recovery = GetRecoverySize(Switch + 2, DEFAULT_RECOVERY);
			break;
		case 'V':
			RecVolNumber = GetRecoverySize(Switch + 2, DEFAULT_RECVOLUMES);
			break;
		case 'I': {
			Priority = atoi(Switch + 2);
			char *ChPtr = strchr(Switch + 2, ':');
			if (ChPtr != NULL)
				SleepTime = atoi(ChPtr + 1);
			SetPriority(Priority);
		}
		break;
		}
		break;
	case 'Y':
		AllYes = true;
		break;
	case 'X':
		if (Switch[1] != 0)
			if (Switch[1] == '@' && !IsWildcard(Switch))
				ReadTextFile(Switch + 2, ExclArgs, false, true, true, true, true);
			else
				ExclArgs->AddString(Switch + 1);
		break;
	case 'E':
		switch (toupper(Switch[1])) {
		case 'P':
			switch (Switch[2]) {
			case 0:
				ExclPath = EXCL_SKIPWHOLEPATH;
				break;
			case '1':
				ExclPath = EXCL_BASEPATH;
				break;
			case '2':
				ExclPath = EXCL_SAVEFULLPATH;
				break;
			}
			break;
		case 'D':
			ExclEmptyDir = true;
			break;
		case 'E':
			ProcessEA = false;
			break;
		case 'N':
			NoEndBlock = true;
			break;
		default:
			ExclFileAttr = GetExclAttr(&Switch[1]);
			break;
		}
		break;
	case 'P':
		if (Switch[1] == 0) {
#ifndef GUI
			GetPassword(PASSWORD_GLOBAL, NULL, Password, sizeof(Password));
			eprintf("\n");
#endif
		} else
			strncpy(Password, Switch + 1, sizeof(Password));
		break;
	case 'H':
		if (toupper(Switch[1]) == 'P') {
			EncryptHeaders = true;
			if (Switch[2] != 0)
				strncpy(Password, Switch + 2, sizeof(Password));
			else if (*Password == 0) {
#ifndef GUI
				GetPassword(PASSWORD_GLOBAL, NULL, Password, sizeof(Password));
				eprintf("\n");
#endif
			}
		}
		break;
	case 'Z':
		strncpy(CommentFile, Switch[1] != 0 ? Switch + 1 : "stdin", sizeof(CommentFile));
		break;
	case 'M':
		switch (toupper(Switch[1])) {
		case 'C': {
			char *Str = Switch + 2;
			if (*Str == '-')
				for (int I = 0; I < sizeof(FilterModes) / sizeof(FilterModes[0]); I++)
					FilterModes[I].State = FILTER_DISABLE;
			else
				while (*Str) {
					int Param1 = 0, Param2 = 0;
					FilterState State = FILTER_AUTO;
					FilterType Type = FILTER_NONE;
					if (isdigit(*Str)) {
						Param1 = atoi(Str);
						while (isdigit(*Str))
							Str++;
					}
					if (*Str == ':' && isdigit(Str[1])) {
						Param2 = atoi(++Str);
						while (isdigit(*Str))
							Str++;
					}
					switch (toupper(*(Str++))) {
					case 'T':
						Type = FILTER_PPM;
						break;
					case 'E':
						Type = FILTER_E8;
						break;
					case 'D':
						Type = FILTER_DELTA;
						break;
					case 'A':
						Type = FILTER_AUDIO;
						break;
					case 'C':
						Type = FILTER_RGB;
						break;
					case 'I':
						Type = FILTER_ITANIUM;
						break;
					case 'L':
						Type = FILTER_UPCASETOLOW;
						break;
					}
					if (*Str == '+' || *Str == '-')
						State = *(Str++) == '+' ? FILTER_FORCE : FILTER_DISABLE;
					FilterModes[Type].State = State;
					FilterModes[Type].Param1 = Param1;
					FilterModes[Type].Param2 = Param2;
				}
		}
		break;
		case 'M':
			break;
		case 'D': {
			if ((WinSize = atoi(&Switch[2])) == 0)
				WinSize = 0x10000 << (toupper(Switch[2]) - 'A');
			else
				WinSize *= 1024;
			if (!CheckWinSize())
				BadSwitch(Switch);
		}
		break;
		case 'S': {
			char *Names = Switch + 2, DefNames[512];
			if (*Names == 0) {
				strcpy(DefNames, DefaultStoreList);
				Names = DefNames;
			}
			while (*Names != 0) {
				char *End = strchr(Names, ';');
				if (End != NULL)
					*End = 0;
				if (*Names == '.')
					Names++;
				char Mask[NM];
				if (strpbrk(Names, "*?.") == NULL)
					sprintf(Mask, "*.%s", Names);
				else
					strcpy(Mask, Names);
				StoreArgs->AddString(Mask);
				if (End == NULL)
					break;
				Names = End + 1;
			}
		}
		break;
		default:
			Method = Switch[1] - '0';
			if (Method > 5 || Method < 0)
				BadSwitch(Switch);
			break;
		}
		break;
	case 'V':
		switch (toupper(Switch[1])) {
		case 'N':
			OldNumbering = true;
			break;
		case 'P':
			VolumePause = true;
			break;
		case 'E':
			if (toupper(Switch[2]) == 'R')
				VersionControl = atoi(Switch + 3) + 1;
			break;
		case '-':
			VolSize = 0;
			break;
		default:
			VolSize = atoil(&Switch[1]);

			if (VolSize == 0)
				VolSize = INT64ERR;
			else
				switch (Switch[strlen(Switch) - 1]) {
				case 'f':
				case 'F':
					switch (int64to32(VolSize)) {
					case 360:
						VolSize = 362496;
						break;
					case 720:
						VolSize = 730112;
						break;
					case 1200:
						VolSize = 1213952;
						break;
					case 1440:
						VolSize = 1457664;
						break;
					case 2880:
						VolSize = 2915328;
						break;
					}
					break;
				case 'k':
					VolSize *= 1024;
					break;
				case 'm':
					VolSize *= 1024 * 1024;
					break;
				case 'M':
					VolSize *= 1000 * 1000;
					break;
				case 'b':
				case 'B':
					break;
				default:
					VolSize *= 1000;
					break;
				}
			break;
		}
		break;
	case 'F':
		if (Switch[1] == 0)
			FreshFiles = true;
		break;
	case 'U':
		if (Switch[1] == 0)
			UpdateFiles = true;
		break;
	case 'W':
		strncpy(TempPath, &Switch[1], sizeof(TempPath) - 1);
		AddEndSlash(TempPath);
		break;
	case 'S':
		if (strnicomp(Switch, "SFX", 3) == 0) {
			const char *SFXName = Switch[3] ? Switch + 3 : DefSFXName;
			if (PointToName(SFXName) != SFXName || FileExist(SFXName))
				strcpy(SFXModule, SFXName);
			else
				GetConfigName(SFXName, SFXModule);
		}
		if (isdigit(Switch[1])) {
			Solid |= SOLID_COUNT;
			SolidCount = atoi(&Switch[1]);
		} else
			switch (toupper(Switch[1])) {
			case 0:
				Solid |= SOLID_NORMAL;
				break;
			case '-':
				Solid = SOLID_NONE;
				break;
			case 'E':
				Solid |= SOLID_FILEEXT;
				break;
			case 'V':
				Solid |= Switch[2] == '-' ? SOLID_VOLUME_DEPENDENT : SOLID_VOLUME_INDEPENDENT;
				break;
			case 'D':
				Solid |= SOLID_VOLUME_DEPENDENT;
				break;
			}
		break;
	case 'C':
		if (Switch[2] == 0)
			switch (toupper(Switch[1])) {
			case '-':
				DisableComment = true;
				break;
			case 'U':
				ConvertNames = NAMES_UPPERCASE;
				break;
			case 'L':
				ConvertNames = NAMES_LOWERCASE;
				break;
			}
		break;
	case 'K':
		switch (toupper(Switch[1])) {
		case 'B':
			KeepBroken = true;
			break;
		case 0:
			Lock = true;
			break;
		}
		break;
#ifndef GUI
	case '?' :
		OutHelp();
		break;
#endif
	default :
		BadSwitch(Switch);
		break;
	}
}
#endif


#ifndef SFX_MODULE
void CommandData::BadSwitch(char *Switch) {
	mprintf(St(MUnknownOption), Switch);
	ErrHandler.Exit(USER_ERROR);
}
#endif


#ifndef GUI
void CommandData::OutTitle() {
	mprintf(St(MCopyrightS));
}
#endif


void CommandData::OutHelp() {
#if !defined(GUI) && !defined(SILENT)
	OutTitle();
	static MSGID Help[] = {
		MUNRARTitle1, MRARTitle2, MCHelpCmd, MCHelpCmdE, MCHelpCmdL, MCHelpCmdP,
		MCHelpCmdT, MCHelpCmdV, MCHelpCmdX, MCHelpSw, MCHelpSwm, MCHelpSwAC, MCHelpSwAD,
		MCHelpSwAP, MCHelpSwAVm, MCHelpSwCm, MCHelpSwCFGm, MCHelpSwCL, MCHelpSwCU,
		MCHelpSwDH, MCHelpSwEP, MCHelpSwF, MCHelpSwIDP, MCHelpSwIERR, MCHelpSwINUL,
		MCHelpSwKB, MCHelpSwOp, MCHelpSwOm, MCHelpSwOW, MCHelpSwP, MCHelpSwPm,
		MCHelpSwR, MCHelpSwRI, MCHelpSwTA, MCHelpSwTB, MCHelpSwTN, MCHelpSwTO,
		MCHelpSwU, MCHelpSwV, MCHelpSwVER, MCHelpSwVP, MCHelpSwX, MCHelpSwXa,
		MCHelpSwXal, MCHelpSwY
	};

	for (int I = 0; I < sizeof(Help) / sizeof(Help[0]); I++) {
#ifndef SFX_MODULE
		if (Help[I] == MCHelpSwIEML || Help[I] == MCHelpSwVD || Help[I] == MCHelpSwAC ||
		        Help[I] == MCHelpSwAO || Help[I] == MCHelpSwDH || Help[I] == MCHelpSwOS)
			continue;
		if (Help[I] == MCHelpSwOL)
			continue;
		if (Help[I] == MCHelpSwEE) {
			continue;
		}
#endif
		mprintf(St(Help[I]));
	}
	mprintf("\n");
	ErrHandler.Exit(USER_ERROR);
#endif
}


bool CommandData::ExclCheck(char *CheckName, bool CheckFullPath) {
	char *Name = ConvertPath(CheckName, NULL);
	char FullName[NM], *ExclName;
	*FullName = 0;
	ExclArgs->Rewind();
	while ((ExclName = ExclArgs->GetString()) != NULL)
#ifndef SFX_MODULE
		if (CheckFullPath && IsFullPath(ExclName)) {
			if (*FullName == 0)
				ConvertNameToFull(CheckName, FullName);
			if (CmpName(ExclName, FullName, MATCH_WILDSUBPATH))
				return (true);
		} else
#endif
			if (CmpName(ConvertPath(ExclName, NULL), Name, MATCH_SUBPATH))
				return (true);
	return (false);
}




#ifndef SFX_MODULE
bool CommandData::TimeCheck(uint FileDosTime) {
	if (FileTimeBefore != 0 && FileDosTime >= FileTimeBefore)
		return (true);
	if (FileTimeAfter != 0 && FileDosTime <= FileTimeAfter)
		return (true);
	if (FileTimeOlder != 0 || FileTimeNewer != 0) {
		if (!TimeConverted) {
			if (FileTimeOlder != 0)
				FileTimeOlder = SecondsToDosTime(FileTimeOlder);
			if (FileTimeNewer != 0)
				FileTimeNewer = SecondsToDosTime(FileTimeNewer);
			TimeConverted = true;
		}
		if (FileTimeOlder != 0 && FileDosTime >= FileTimeOlder)
			return (true);
		if (FileTimeNewer != 0 && FileDosTime <= FileTimeNewer)
			return (true);
	}
	return (false);
}
#endif


bool CommandData::IsProcessFile(FileHeader &NewLhd, bool *ExactMatch) {
	if (ExclCheck(NewLhd.FileName, false))
		return (false);
#ifndef SFX_MODULE
	if (TimeCheck(NewLhd.FileTime))
		return (false);
#endif
	char *ArgName;
	wchar *ArgNameW;
	FileArgs->Rewind();
	while (FileArgs->GetString(&ArgName, &ArgNameW)) {
#ifndef SFX_MODULE
		bool Unicode = (NewLhd.Flags & LHD_UNICODE) || ArgNameW != NULL;
		if (Unicode) {
			wchar NameW[NM], ArgW[NM], *NamePtr = NewLhd.FileNameW;
			if (ArgNameW == NULL) {
				CharToWide(ArgName, ArgW);
				ArgNameW = ArgW;
			}
			if ((NewLhd.Flags & LHD_UNICODE) == 0) {
				CharToWide(NewLhd.FileName, NameW);
				NamePtr = NameW;
			}
			if (CmpName(ArgNameW, NamePtr, MATCH_WILDSUBPATH)) {
				if (ExactMatch != NULL)
					*ExactMatch = stricompcw(ArgNameW, NamePtr) == 0;
				return (true);
			}
		}
#endif
		if (CmpName(ArgName, NewLhd.FileName, MATCH_WILDSUBPATH)) {
			if (ExactMatch != NULL)
				*ExactMatch = stricompc(ArgName, NewLhd.FileName) == 0;
			return (true);
		}
	}
	return (false);
}


void CommandData::ProcessCommand() {
#ifndef SFX_MODULE
	if (Command[1] && strchr("FUADPXETK", *Command) != NULL || *ArcName == 0)
		OutHelp();

	if (GetExt(ArcName) == NULL && (!FileExist(ArcName) || IsDir(GetFileAttr(ArcName))))
		strcat(ArcName, ".rar");

	if (strchr("AFUMD", *Command) == NULL) {
		StringList ArcMasks;
		ArcMasks.AddString(ArcName);
		ScanTree Scan(&ArcMasks, Recurse, SaveLinks, SCAN_SKIPDIRS);
		FindData FindData;
		while (Scan.GetNext(&FindData) == SCAN_SUCCESS)
			AddArcName(FindData.Name, FindData.NameW);
	} else
		AddArcName(ArcName, NULL);
#endif

	switch (Command[0]) {
	case 'P':
	case 'X':
	case 'E':
	case 'T':
	case 'I': {
		CmdExtract Extract;
		Extract.DoExtract(this);
	}
	break;
#if !defined(GUI) && !defined(SILENT)
	case 'V':
	case 'L':
		ListArchive(this);
		break;
	default:
		OutHelp();
#endif
	}
#ifndef GUI
	mprintf("\n");
#endif
}


void CommandData::AddArcName(char *Name, wchar *NameW) {
	ArcNames->AddString(Name, NameW);
}


bool CommandData::GetArcName(char *Name, wchar *NameW, int MaxSize) {
	if (!ArcNames->GetString(Name, NameW, NM))
		return (false);
	return (true);
}


bool CommandData::IsSwitch(int Ch) {
	return (Ch == '-');
}


#ifndef SFX_MODULE
uint CommandData::GetExclAttr(char *Str) {
	return (strtol(Str, NULL, 0));
}
#endif


#ifndef SFX_MODULE
int CommandData::GetRecoverySize(char *Str, int DefSize) {
	if (*Str == '-')
		return (0);
	if (*Str == 0)
		return (DefSize);
	int Size = atoi(Str);
	if (Size <= 100 && strpbrk(Str, "%p") != NULL)
		Size = -Size;
	return (Size);
}
#endif


#ifndef SFX_MODULE
bool CommandData::CheckWinSize() {
	static int ValidSize[] = {
		0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000
	};
	for (int I = 0; I < sizeof(ValidSize) / sizeof(ValidSize[0]); I++)
		if (WinSize == ValidSize[I])
			return (true);
	WinSize = 0x400000;
	return (false);
}
#endif
