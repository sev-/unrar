#include "rar.hpp"

char* PointToName(const char *Path)
{
  const char *Found=NULL;
  for (const char *s=Path;*s!=0;s=charnext(s))
    if (IsPathDiv(*s))
      Found=(char*)(s+1);
  if (Found!=NULL)
    return((char*)Found);
  return (char*)((*Path && IsDriveDiv(Path[1]) && charnext(Path)==Path+1) ? Path+2:Path);
}


wchar* PointToName(const wchar *Path)
{
  for (int I=strlenw(Path)-1;I>=0;I--)
    if (IsPathDiv(Path[I]))
      return (wchar*)&Path[I+1];
  return (wchar*)((*Path && IsDriveDiv(Path[1])) ? Path+2:Path);
}


char* PointToLastChar(const char *Path)
{
  for (const char *s=Path,*p=Path;;p=s,s=charnext(s))
    if (*s==0)
      return((char *)p);
}


char* ConvertPath(const char *SrcPath,char *DestPath)
{
  const char *DestPtr=SrcPath;
  for (const char *s=DestPtr;*s!=0;s++)
    if (IsPathDiv(s[0]) && s[1]=='.' && s[2]=='.' && IsPathDiv(s[3]))
      DestPtr=s+4;
  while (*DestPtr)
  {
    const char *s=DestPtr;
    if (s[0] && IsDriveDiv(s[1]))
      s+=2;
    if (s[0]=='\\' && s[1]=='\\')
    {
      const char *Slash=strchr(s+2,'\\');
      if (Slash!=NULL && (Slash=strchr(Slash+1,'\\'))!=NULL)
        s=Slash+1;
    }
    for (const char *t=s;*t!=0;t++)
      if (IsPathDiv(*t))
        s=t+1;
      else
        if (*t!='.')
          break;
    if (s==DestPtr)
      break;
    DestPtr=s;
  }
  if (DestPath!=NULL)
  {
    char TmpStr[NM];
    strncpy(TmpStr,DestPtr,sizeof(TmpStr)-1);
    strcpy(DestPath,TmpStr);
  }
  return((char *)DestPtr);
}


wchar* ConvertPath(const wchar *SrcPath,wchar *DestPath)
{
  const wchar *DestPtr=*SrcPath && IsDriveDiv(SrcPath[1]) ? SrcPath+2:SrcPath;
  if (SrcPath[0]=='\\' && SrcPath[1]=='\\')
  {
    const wchar *ChPtr=strchrw(SrcPath+2,'\\');
    if (ChPtr!=NULL && (ChPtr=strchrw(ChPtr+1,'\\'))!=NULL)
      DestPtr=ChPtr+1;
  }
  for (const wchar *ChPtr=DestPtr;*ChPtr!=0;ChPtr++)
    if (IsPathDiv(ChPtr[0]) && ChPtr[1]=='.' && ChPtr[2]=='.' && IsPathDiv(ChPtr[3]))
      DestPtr=ChPtr+4;
  for (const wchar *ChPtr=DestPtr;*ChPtr!=0;ChPtr++)
#ifdef _WIN_32
    if (*ChPtr=='/' || *ChPtr=='\\')
#else
    if (*ChPtr==CPATHDIVIDER)
#endif
      DestPtr=ChPtr+1;
    else
      if (*ChPtr!='.')
        break;
  if (DestPath!=NULL)
  {
    wchar TmpStr[NM];
    strncpyw(TmpStr,DestPtr,sizeof(TmpStr)-1);
    strcpyw(DestPath,TmpStr);
  }
  return((wchar *)DestPtr);
}


void SetExt(char *Name,const char *NewExt)
{
  char *Dot=GetExt(Name);
  if (NewExt==NULL)
  {
    if (Dot!=NULL)
      *Dot=0;
  }
  else
    if (Dot==NULL)
    {
      strcat(Name,".");
      strcat(Name,NewExt);
    }
    else
      strcpy(Dot+1,NewExt);
}


#ifndef SFX_MODULE
void SetExt(wchar *Name,const wchar *NewExt)
{
  if (Name==NULL || *Name==0)
    return;
  wchar *Dot=GetExt(Name);
  if (NewExt==NULL)
  {
    if (Dot!=NULL)
      *Dot=0;
  }
  else
    if (Dot==NULL)
    {
      strcatw(Name,L".");
      strcatw(Name,NewExt);
    }
    else
      strcpyw(Dot+1,NewExt);
}
#endif


#ifndef SFX_MODULE
void SetSFXExt(char *SFXName)
{
#ifdef _UNIX
  SetExt(SFXName,"sfx");
#endif
}
#endif


#ifndef SFX_MODULE
void SetSFXExt(wchar *SFXName)
{
  if (SFXName==NULL || *SFXName==0)
    return;

#ifdef _UNIX
  SetExt(SFXName,L"sfx");
#endif

}
#endif


char *GetExt(const char *Name)
{
  return(strrchrd(PointToName(Name),'.'));
}


wchar *GetExt(const wchar *Name)
{
  return(Name==NULL ? (wchar *)L"":strrchrw(PointToName(Name),'.'));
}


bool CmpExt(const char *Name,const char *Ext)
{
  char *NameExt=GetExt(Name);
  return(NameExt!=NULL && stricomp(NameExt+1,Ext)==0);
}


bool IsWildcard(const char *Str,const wchar *StrW)
{
  if (StrW!=NULL && *StrW!=0)
    return(strpbrkw(StrW,L"*?")!=NULL);
  return(Str==NULL ? false:strpbrk(Str,"*?")!=NULL);
}


bool IsPathDiv(int Ch)
{
#ifdef _WIN_32
  return(Ch=='\\' || Ch=='/');
#else
  return(Ch==CPATHDIVIDER);
#endif
}


bool IsDriveDiv(int Ch)
{
#ifdef _UNIX
  return(false);
#else
  return(Ch==':');
#endif
}


int GetPathDisk(const char *Path)
{
  if (IsDiskLetter(Path))
    return(toupper(*Path)-'A');
  else
    return(-1);
}


void AddEndSlash(char *Path)
{
  char *LastChar=PointToLastChar(Path);
  if (*LastChar!=0 && *LastChar!=CPATHDIVIDER)
    strcat(LastChar,PATHDIVIDER);
}


void AddEndSlash(wchar *Path)
{
  int Length=strlenw(Path);
  if (Length>0 && Path[Length-1]!=CPATHDIVIDER)
    strcatw(Path,PATHDIVIDERW);
}


void GetFilePath(const char *FullName,char *Path)
{
  int PathLength=PointToName(FullName)-FullName;
  strncpy(Path,FullName,PathLength);
  Path[PathLength]=0;
}


void GetFilePath(const wchar *FullName,wchar *Path)
{
  const wchar *PathPtr=/*(*FullName && IsDriveDiv(FullName[1])) ? FullName+2:*/FullName;
  int PathLength=PointToName(FullName)-FullName;
  strncpyw(Path,PathPtr,PathLength);
  Path[PathLength]=0;
}


void RemoveNameFromPath(char *Path)
{
  char *Name=PointToName(Path);
  if (Name>=Path+2 && (!IsDriveDiv(Path[1]) || Name>=Path+4))
    Name--;
  *Name=0;
}


#ifndef SFX_MODULE
void RemoveNameFromPath(wchar *Path)
{
  wchar *Name=PointToName(Path);
  if (Name>=Path+2 && (!IsDriveDiv(Path[1]) || Name>=Path+4))
    Name--;
  *Name=0;
}
#endif


#ifndef SFX_MODULE
void GetConfigName(const char *Name,char *FullName)
{
  char *EnvStr=getenv("HOME");
  if (EnvStr!=NULL)
  {
    strcpy(FullName,EnvStr);
    AddEndSlash(FullName);
  }
  strcat(FullName,Name);
  if (!WildFileExist(FullName))
  {
    char HomeName[NM];
    strcpy(HomeName,FullName);
    static char *AltPath[]={
      "/etc/","/usr/lib/","/usr/local/lib/","/usr/local/etc/"
    };
    for (int I=0;I<sizeof(AltPath)/sizeof(AltPath[0]);I++)
    {
      strcpy(FullName,AltPath[I]);
      strcat(FullName,Name);
      if (WildFileExist(FullName))
        break;
      strcpy(FullName,HomeName);
    }
  }
}
#endif


void NextVolumeName(char *ArcName,bool OldNumbering)
{
  char *ChPtr;
  if ((ChPtr=GetExt(ArcName))==NULL)
  {
    strcat(ArcName,".rar");
    ChPtr=GetExt(ArcName);
  }
  else
    if (ChPtr[1]==0 || stricomp(ChPtr+1,"exe")==0 || stricomp(ChPtr+1,"sfx")==0)
      strcpy(ChPtr+1,"rar");
  if (!OldNumbering)
  {
    while (!isdigit(*ChPtr) && ChPtr>ArcName)
      ChPtr--;
    while ((++(*ChPtr))=='9'+1)
    {
      *ChPtr='0';
      ChPtr--;
      if (ChPtr<ArcName || !isdigit(*ChPtr))
      {
        for (char *EndPtr=ArcName+strlen(ArcName);EndPtr!=ChPtr;EndPtr--)
          *(EndPtr+1)=*EndPtr;
        *(ChPtr+1)='1';
        break;
      }
    }
  }
  else
    if (!isdigit(*(ChPtr+2)) || !isdigit(*(ChPtr+3)))
      strcpy(ChPtr+2,"00");
    else
    {
      ChPtr+=3;
      while ((++(*ChPtr))=='9'+1)
        if (*(ChPtr-1)=='.')
        {
          *ChPtr='A';
          break;
        }
        else
        {
          *ChPtr='0';
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

bool IsNameUsable(const char *Name)
{
#ifndef _UNIX
  if (Name[0] && Name[1] && strchr(Name+2,':')!=NULL)
    return(false);
#endif
  return(*Name!=0 && strpbrk(Name,"?*<>|")==NULL);
}


void MakeNameUsable(char *Name,bool Extended)
{
  for (int I=0;Name[I]!=0;I++)
  {
    if (strchr(Extended ? "?*<>|\"":"?*",Name[I])!=NULL ||
        Extended && Name[I]<32)
      Name[I]='_';
  }
}


char* UnixSlashToDos(char *SrcName,char *DestName)
{
  if (DestName!=NULL && DestName!=SrcName)
    strcpy(DestName,SrcName);
  for (char *s=SrcName;*s!=0;s=charnext(s))
  {
    if (*s=='/')
      if (DestName==NULL)
        *s='\\';
      else
        DestName[s-SrcName]='\\';
  }
  return(DestName==NULL ? SrcName:DestName);
}


char* DosSlashToUnix(char *SrcName,char *DestName)
{
  if (DestName!=NULL && DestName!=SrcName)
    strcpy(DestName,SrcName);
  for (char *s=SrcName;*s!=0;s=charnext(s))
  {
    if (*s=='\\')
      if (DestName==NULL)
        *s='/';
      else
        DestName[s-SrcName]='/';
  }
  return(DestName==NULL ? SrcName:DestName);
}


bool IsFullPath(const char *Path)
{
  char PathOnly[NM];
  GetFilePath(Path,PathOnly);
  if (IsWildcard(PathOnly))
    return(true);
  return(IsPathDiv(Path[0]));
}


bool IsDiskLetter(const char *Path)
{
  char Letter=toupper(Path[0]);
  return(Letter>='A' && Letter<='Z' && IsDriveDiv(Path[1]));
}


void GetPathRoot(const char *Path,char *Root)
{
  *Root=0;
  if (IsDiskLetter(Path))
    sprintf(Root,"%c:\\",*Path);
  else
    if (Path[0]=='\\' && Path[1]=='\\')
    {
      const char *Slash=strchr(Path+2,'\\');
      if (Slash!=NULL)
      {
        int Length;
        if ((Slash=strchr(Slash+1,'\\'))!=NULL)
          Length=Slash-Path+1;
        else
          Length=strlen(Path);
        strncpy(Root,Path,Length);
        Root[Length]=0;
      }
    }
}


int ParseVersionFileName(char *Name,wchar *NameW,bool Truncate)
{
  int Version=0;
  char *VerText=strrchrd(Name,';');
  if (VerText!=NULL)
  {
    Version=atoi(VerText+1);
    if (Truncate)
      *VerText=0;
  }
  if (NameW!=NULL)
  {
    wchar *VerTextW=strrchrw(NameW,';');
    if (VerTextW!=NULL)
    {
      if (Version==0)
        Version=atoiw(VerTextW+1);
      if (Truncate)
        *VerTextW=0;
    }
  }
  return(Version);
}


char* VolNameToFirstName(const char *VolName,char *FirstName,bool NewNumbering)
{
  if (FirstName!=VolName)
    strcpy(FirstName,VolName);
  char *VolNumStart=FirstName;
  if (NewNumbering)
  {
    int N='1';
    for (char *ChPtr=FirstName+strlen(FirstName)-1;ChPtr>FirstName;ChPtr--)
      if (isdigit(*ChPtr))
      {
        *ChPtr=N;
        N='0';
      }
      else
        if (N=='0')
        {
          VolNumStart=ChPtr+1;
          break;
        }
  }
  else
  {
    SetExt(FirstName,"rar");
    VolNumStart=GetExt(FirstName);
  }
  if (!FileExist(FirstName))
  {
    char Mask[NM];
    strcpy(Mask,FirstName);
    SetExt(Mask,"*");
    FindFile Find;
    Find.SetMask(Mask);
    struct FindData FD;
    while (Find.Next(&FD))
    {
      Archive Arc;
      if (Arc.Open(FD.Name,FD.NameW) && Arc.IsArchive(true) && !Arc.NotFirstVolume)
      {
        strcpy(FirstName,FD.Name);
        break;
      }
    }
  }
  return(VolNumStart);
}
