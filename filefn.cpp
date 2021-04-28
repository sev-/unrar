/***** File: filefn.cpp *****/


#include "rar.hpp"

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
