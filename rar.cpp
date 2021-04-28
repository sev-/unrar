#include "rar.hpp"


int ToPercent(Int64 N1,Int64 N2)
{
  if (N2==0)
    return(0);
  if (N2<N1)
    return(100);
  return(int64to32(N1*100/N2));
}

int main(int argc, char *argv[])
{
#ifndef SFX_MODULE
  setbuf(stdout,NULL);

#endif

  ErrHandler.SetSignalHandlers(true);

  RARInitData();

  bool ShutdownOnClose;

#ifdef ALLOW_EXCEPTIONS
  try
#endif
  {

    CommandData Cmd;
#ifdef SFX_MODULE
    strcpy(Cmd.Command,"X");
    char *Switch=NULL;
#ifdef _SFX_RTL_
    char *CmdLine=GetCommandLine();
    if (CmdLine!=NULL && *CmdLine=='\"')
      CmdLine=strchr(CmdLine+1,'\"');
    if (CmdLine!=NULL && (CmdLine=strpbrk(CmdLine," /"))!=NULL)
    {
      while (isspace(*CmdLine))
        CmdLine++;
      Switch=CmdLine;
    }
#else
    Switch=argc>1 ? argv[1]:NULL;
#endif
    if (Switch!=NULL && Cmd.IsSwitch(Switch[0]))
    {
      int UpperCmd=toupper(Switch[1]);
      switch(UpperCmd)
      {
        case 'T':
        case 'V':
          Cmd.Command[0]=UpperCmd;
          break;
        case '?':
          Cmd.OutHelp();
          break;
      }
    }
    Cmd.AddArcName(ModuleName,NULL);
#else
    if (Cmd.IsConfigEnabled(argc,argv))
    {
      Cmd.ReadConfig(argc,argv);
      Cmd.ParseEnvVar();
    }
    for (int I=1;I<argc;I++)
      Cmd.ParseArg(argv[I]);
#endif
    Cmd.ParseDone();


    InitConsoleOptions(Cmd.MsgStream,Cmd.Sound);
    InitSystemOptions(Cmd.SleepTime);
    InitLogOptions(Cmd.LogName);
    ErrHandler.SetSilent(Cmd.AllYes || Cmd.MsgStream==MSG_NULL);
    ErrHandler.SetShutdown(Cmd.Shutdown);

    Cmd.OutTitle();
    Cmd.ProcessCommand();
  }
#ifdef ALLOW_EXCEPTIONS
  catch (int ErrCode)
  {
    ErrHandler.SetErrorCode(ErrCode);
  }
#ifdef ENABLE_BAD_ALLOC
  catch (bad_alloc)
  {
    ErrHandler.SetErrorCode(MEMORY_ERROR);
  }
#endif
  catch (...)
  {
    ErrHandler.SetErrorCode(FATAL_ERROR);
  }
#endif
  File::RemoveCreated();
  return(ErrHandler.GetErrorCode());
}


void RARInitData()
{
  InitCRC();
#ifndef SFX_MODULE
  InitTime();
#endif
  ErrHandler.Clean();
}

const char *St(MSGID StringId) {
  return(StringId);
}
