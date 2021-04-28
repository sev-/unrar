#ifndef _RAR_FINDDATA_
#define _RAR_FINDDATA_

struct FindData
{
  char Name[NM];
  wchar NameW[NM];
  Int64 Size;
  uint FileAttr;
  uint FileTime;
  int IsDir;
  bool Error;
};

class FindFile
{
  private:

    char FindMask[NM];
    wchar FindMaskW[NM];
    int FirstCall;
    DIR *dirp;
  public:
    FindFile();
    ~FindFile();
    void SetMask(const char *FindMask);
    void SetMaskW(const wchar *FindMaskW);
    bool Next(struct FindData *fd,bool GetSymLink=false);
    static bool FastFind(const char *FindMask,const wchar *FindMaskW,struct FindData *fd,bool GetSymLink=false);
};

#endif
