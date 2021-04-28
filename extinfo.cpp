#include "rar.hpp"

#ifdef _WIN_32
#include "win32acl.cpp"
#include "win32stm.cpp"
#endif
#if defined(_EMX) && !defined(_DJGPP)
#include "os2ea.cpp"
#endif
#ifdef _UNIX
#include "uowners.cpp"
#endif



#ifndef SFX_MODULE
void SetExtraInfo(CommandData *Cmd,Archive &Arc,char *Name,wchar *NameW)
{
  switch(Arc.SubBlockHead.SubType)
  {
#if defined(_EMX) && !defined(_DJGPP)
    case EA_HEAD:
      if (Cmd->ProcessEA)
        ExtractOS2EA(Arc,Name);
      break;
#endif
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
#if defined(_EMX) && !defined(_DJGPP)
  if (Cmd->ProcessEA && Arc.SubHead.CmpName(SUBHEAD_TYPE_OS2EA))
    ExtractOS2EANew(Arc,Name);
#endif
#ifdef _UNIX
  if (Cmd->ProcessOwners && Arc.SubHead.CmpName(SUBHEAD_TYPE_UOWNER))
    ExtractUnixOwnerNew(Arc,Name);
#endif
}
