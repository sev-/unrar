#include "rar.hpp"

#ifdef _WIN_32
#include "win32acl.cpp"
#include "win32stm.cpp"
#endif
#ifdef _UNIX
#include "uowners.cpp"
#endif



#ifndef SFX_MODULE
void SetExtraInfo(CommandData *Cmd,Archive &Arc,char *Name,wchar *NameW)
{
  switch(Arc.SubBlockHead.SubType)
  {
#ifdef _UNIX
    case UO_HEAD:
      if (Cmd->ProcessOwners)
        ExtractUnixOwner(Arc,Name);
      break;
#endif
  }
}
#endif


void SetExtraInfoNew(CommandData *Cmd,Archive &Arc,char *Name,wchar *NameW)
{
#ifdef _UNIX
  if (Cmd->ProcessOwners && Arc.SubHead.CmpName(SUBHEAD_TYPE_UOWNER))
    ExtractUnixOwnerNew(Arc,Name);
#endif
}
