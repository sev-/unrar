#include "rar.hpp"

MKDIR_CODE MakeDir(const char *Name,const wchar *NameW,uint Attr)
{
#ifdef _UNIX
  int prevmask=umask(0);
  int ErrCode=mkdir(Name,(mode_t)Attr);
  umask(prevmask);
  if (ErrCode==-1)
    return(errno==ENOENT ? MKDIR_BADPATH:MKDIR_ERROR);
  return(MKDIR_SUCCESS);
#endif
}


void CreatePath(const char *Path,const wchar *PathW,bool SkipLastName)
{
#ifdef _WIN_32
  uint DirAttr=0;
#else
  uint DirAttr=0777;
#endif
  int PosW=0;
  for (const char *s=Path;*s!=0 && PosW<NM;s=charnext(s),PosW++)
  {
    bool Wide=PathW!=NULL && *PathW!=0;
    if (Wide && PathW[PosW]==CPATHDIVIDER || !Wide && *s==CPATHDIVIDER)
    {
      wchar *DirPtrW=NULL;
      if (Wide)
      {
        wchar DirNameW[NM];
        strncpyw(DirNameW,PathW,PosW);
        DirNameW[PosW]=0;
        DirPtrW=DirNameW;
      }
      char DirName[NM];
      strncpy(DirName,Path,s-Path);
      DirName[s-Path]=0;
      if (MakeDir(DirName,DirPtrW,DirAttr)==MKDIR_SUCCESS)
      {
#ifndef GUI
        mprintf(St(MCreatDir),DirName);
        mprintf(" %s",St(MOk));
#endif
      }
    }
  }
  if (!SkipLastName)
    MakeDir(Path,PathW,DirAttr);
}


void SetDirTime(const char *Name,uint ft)
{
#ifdef _WIN_32
  if (!WinNT())
    return;
  HANDLE hFile=CreateFile(Name,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
                          NULL,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS,NULL);
  if (hFile==INVALID_HANDLE_VALUE)
    return;
  FILETIME LocalTime,FileTime;
  DosDateTimeToFileTime(HIWORD(ft),LOWORD(ft),&LocalTime);
  LocalFileTimeToFileTime(&LocalTime,&FileTime);
  SetFileTime(hFile,NULL,NULL,&FileTime);
  CloseHandle(hFile);
#endif
#if defined(_UNIX)
  struct utimbuf ut;
  ut.actime=ut.modtime=DosTimeToUnix(ft);
  utime(Name,&ut);
#endif
}


bool IsRemovable(const char *FileName)
{
  return(false);
}


#ifndef SFX_MODULE
Int64 GetFreeDisk(const char *FileName)
{
#ifdef _WIN_32
  char Root[NM];
  GetPathRoot(FileName,Root);

  typedef BOOL (WINAPI *GETDISKFREESPACEEX)(
    LPCTSTR,PULARGE_INTEGER,PULARGE_INTEGER,PULARGE_INTEGER
   );
  static GETDISKFREESPACEEX pGetDiskFreeSpaceEx=NULL;

  if (pGetDiskFreeSpaceEx==NULL)
  {
    HMODULE hKernel=GetModuleHandle("kernel32.dll");
    if (hKernel!=NULL)
      pGetDiskFreeSpaceEx=(GETDISKFREESPACEEX)GetProcAddress(hKernel,"GetDiskFreeSpaceExA");
  }
  if (pGetDiskFreeSpaceEx!=NULL)
  {
    GetFilePath(FileName,Root);
    ULARGE_INTEGER uiTotalSize,uiTotalFree,uiUserFree;
    uiUserFree.u.LowPart=uiUserFree.u.HighPart=0;
    if (pGetDiskFreeSpaceEx(*Root ? Root:NULL,&uiUserFree,&uiTotalSize,&uiTotalFree) &&
        uiUserFree.u.HighPart<=uiTotalFree.u.HighPart)
      return(int32to64(uiUserFree.u.HighPart,uiUserFree.u.LowPart));
  }

  DWORD SectorsPerCluster,BytesPerSector,FreeClusters,TotalClusters;
  if (!GetDiskFreeSpace(*Root ? Root:NULL,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&TotalClusters))
    return(1457664);
  Int64 FreeSize=SectorsPerCluster*BytesPerSector;
  FreeSize=FreeSize*FreeClusters;
  return(FreeSize);
#elif defined(_UNIX)
  return(1457664);
#else
  #define DISABLEAUTODETECT
  return(1457664);
#endif
}
#endif


bool FileExist(const char *FileName,const wchar *FileNameW)
{
#ifdef _WIN_32
  if (WinNT() && FileNameW!=NULL && *FileNameW!=0)
    return(GetFileAttributesW(FileNameW)!=0xffffffff);
  else
    return(GetFileAttributes(FileName)!=0xffffffff);
#elif defined(ENABLE_ACCESS)
  return(access(FileName,0)==0);
#else
  struct FindData FD;
  return(FindFile::FastFind(FileName,FileNameW,&FD));
#endif
}


bool WildFileExist(const char *FileName,const wchar *FileNameW)
{
  if (IsWildcard(FileName,FileNameW))
  {
    FindFile Find;
    Find.SetMask(FileName);
    Find.SetMaskW(FileNameW);
    struct FindData fd;
    return(Find.Next(&fd));
  }
  return(FileExist(FileName,FileNameW));
}


bool IsDir(uint Attr)
{
#if defined(_UNIX)
  return((Attr & 0xF000)==0x4000);
#endif
}


bool IsUnreadable(uint Attr)
{
#if defined(_UNIX) && defined(S_ISFIFO) && defined(S_ISSOCK) && defined(S_ISCHR)
  return(S_ISFIFO(Attr) || S_ISSOCK(Attr) || S_ISCHR(Attr));
#endif
  return(false);
}


bool IsLabel(uint Attr)
{
  return(false);
}


bool IsLink(uint Attr)
{
#ifdef _UNIX
  return((Attr & 0xF000)==0xA000);
#endif
  return(false);
}






bool IsDeleteAllowed(uint FileAttr)
{
  return(false);
}


void PrepareToDelete(const char *Name,const wchar *NameW)
{
#ifdef _UNIX
  chmod(Name,S_IRUSR|S_IWUSR);
#endif
}


uint GetFileAttr(const char *Name,const wchar *NameW)
{
  struct stat st;
  if (stat(Name,&st)!=0)
    return(0);
  return(st.st_mode);
}


bool SetFileAttr(const char *Name,const wchar *NameW,uint Attr)
{
  bool Success;
  Success=chmod(Name,(mode_t)Attr)==0;
  return(Success);
}


void ConvertNameToFull(const char *Src,char *Dest)
{
#ifdef _WIN_32
  char FullName[NM],*NamePtr;
  if (GetFullPathName(Src,sizeof(FullName),FullName,&NamePtr))
    strcpy(Dest,FullName);
  else
    if (Src!=Dest)
      strcpy(Dest,Src);
#else
  char FullName[NM];
  if (IsPathDiv(*Src) || *Src!=0 && IsDriveDiv(Src[1]))
    strcpy(FullName,Src);
  else
  {
    getcwd(FullName,sizeof(FullName));
    AddEndSlash(FullName);
    strcat(FullName,Src);
  }
  strcpy(Dest,FullName);
#endif
}


#ifndef SFX_MODULE
void ConvertNameToFull(const wchar *Src,wchar *Dest)
{
  if (Src==NULL || *Src==0)
  {
    *Dest=0;
    return;
  }
#ifdef _WIN_32
  if (WinNT())
  {
    wchar FullName[NM],*NamePtr;
    if (GetFullPathNameW(Src,sizeof(FullName)/sizeof(FullName[0]),FullName,&NamePtr))
      strcpyw(Dest,FullName);
    else
      if (Src!=Dest)
        strcpyw(Dest,Src);
  }
  else
  {
    char AnsiName[NM];
    WideToChar(Src,AnsiName);
    ConvertNameToFull(AnsiName,AnsiName);
    CharToWide(AnsiName,Dest);
  }
#else
  char AnsiName[NM];
  WideToChar(Src,AnsiName);
  ConvertNameToFull(AnsiName,AnsiName);
  CharToWide(AnsiName,Dest);
#endif
}
#endif


#ifndef SFX_MODULE
char *MkTemp(char *Name)
{
  int Length=strlen(Name);
  if (Length<=6)
    return(NULL);
  for (int Random=clock(),Attempt=0;;Attempt++)
  {
    sprintf(Name+Length-6,"%06u",Random+Attempt);
    Name[Length-4]='.';
    if (!FileExist(Name))
      break;
    if (Attempt==1000)
      return(NULL);
  }
  return(Name);
}
#endif


#ifndef SFX_MODULE
uint CalcFileCRC(File *SrcFile,Int64 Size)
{
  SaveFilePos SavePos(*SrcFile);
  const int BufSize=0x10000;
  Array<byte> Data(BufSize);
  int ReadSize,BlockCount=0;
  uint DataCRC=0xffffffff;
  SrcFile->Seek(0,SEEK_SET);
  while ((ReadSize=SrcFile->Read(&Data[0],Size==INT64ERR ? BufSize:Min(BufSize,Size)))!=0)
  {
    if ((++BlockCount & 15)==0)
      Wait();
    DataCRC=CRC(DataCRC,&Data[0],ReadSize);
    if (Size!=INT64ERR)
      Size-=ReadSize;
  }
  return(DataCRC^0xffffffff);
}
#endif
