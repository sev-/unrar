#include "rar.hpp"

FindFile::FindFile()
{
  *FindMask=0;
  *FindMaskW=0;
  FirstCall=TRUE;
  dirp=NULL;
}


FindFile::~FindFile()
{
  if (dirp!=NULL)
    closedir(dirp);
}


void FindFile::SetMask(const char *FindMask)
{
  strcpy(FindFile::FindMask,FindMask);
  if (*FindMaskW==0)
    CharToWide(FindMask,FindMaskW);
  FirstCall=TRUE;
}


void FindFile::SetMaskW(const wchar *FindMaskW)
{
  if (FindMaskW==NULL)
    return;
  strcpyw(FindFile::FindMaskW,FindMaskW);
  if (*FindMask==0)
    WideToChar(FindMaskW,FindMask);
  FirstCall=TRUE;
}


bool FindFile::Next(struct FindData *fd,bool GetSymLink)
{
  fd->Error=false;
  if (*FindMask==0)
    return(false);
  if (FirstCall)
  {
    char DirName[NM];
    strcpy(DirName,FindMask);
    RemoveNameFromPath(DirName);
    if (*DirName==0)
      strcpy(DirName,".");
/*
    else
    {
      int Length=strlen(DirName);
      if (Length>1 && DirName[Length-1]==CPATHDIVIDER && (Length!=3 || !IsDriveDiv(DirName[1])))
        DirName[Length-1]=0;
    }
*/
    if ((dirp=opendir(DirName))==NULL)
    {
      fd->Error=(errno!=ENOENT);
      return(false);
    }
  }
  while (1)
  {
    struct dirent *ent=readdir(dirp);
    if (ent==NULL)
      return(false);
    if (strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
      continue;
    if (CmpName(FindMask,ent->d_name,MATCH_NAMES))
    {
      char FullName[NM];
      strcpy(FullName,FindMask);
      strcpy(PointToName(FullName),ent->d_name);
      if (!FastFind(FullName,NULL,fd,GetSymLink))
      {
        ErrHandler.OpenErrorMsg(FullName);
        continue;
      }
      strcpy(fd->Name,FullName);
      break;
    }
  }
  *fd->NameW=0;

  fd->IsDir=IsDir(fd->FileAttr);
  FirstCall=FALSE;
  char *Name=PointToName(fd->Name);
  if (strcmp(Name,".")==0 || strcmp(Name,"..")==0)
    return(Next(fd));
  return(true);
}


bool FindFile::FastFind(const char *FindMask,const wchar *FindMaskW,struct FindData *fd,bool GetSymLink)
{
  fd->Error=false;
#ifndef _UNIX
  if (IsWildcard(FindMask,FindMaskW))
    return(false);
#endif

  struct stat st;
  if (GetSymLink)
  {
#ifdef SAVE_LINKS
    if (lstat(FindMask,&st)!=0)
#else
    if (stat(FindMask,&st)!=0)
#endif
    {
      fd->Error=(errno!=ENOENT);
      return(false);
    }
  }
  else
    if (stat(FindMask,&st)!=0)
    {
      fd->Error=(errno!=ENOENT);
      return(false);
    }
  fd->FileAttr=st.st_mode;
  fd->IsDir=IsDir(st.st_mode);
  fd->Size=st.st_size;
  fd->FileTime=UnixTimeToDos(st.st_mtime);
  strcpy(fd->Name,FindMask);
  *fd->NameW=0;

  fd->IsDir=IsDir(fd->FileAttr);
  return(true);
}
