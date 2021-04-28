#include "rar.hpp"

ComprDataIO::ComprDataIO(CmdAdd *Command)
{
  ComprDataIO::Command=Command;
  Init();
}


void ComprDataIO::Init()
{
  SrcUnpack=NULL;
  PackFromMemory=false;
  UnpackFromMemory=false;
  UnpackToMemory=false;
  UnpPackedSize=0;
  ShowProgress=true;
  TestMode=false;
  SkipUnpCRC=false;
  PackVolume=false;
  UnpVolume=false;
  SrcFile=NULL;
  DestFile=NULL;
  UnpWrSize=0;
  ComprDataIO::Command=Command;
  Encryption=0;
  Decryption=0;
  TotalPackRead=0;
  CurPackRead=CurPackWrite=CurUnpRead=CurUnpWrite=0;
  PackFileCRC=UnpFileCRC=PackedCRC=0xffffffff;
  LastPercent=-1;
  SubHead=NULL;
  SubHeadPos=NULL;
  CurrentCommand=0;
  ProcessedArcSize=TotalArcSize=0;
}




int ComprDataIO::UnpRead(byte *Addr,uint Count)
{
  int RetCode=0,TotalRead=0;
  byte *ReadAddr;
  ReadAddr=Addr;
  while (Count > 0)
  {
    Archive *SrcArc=(Archive *)SrcFile;

    uint ReadSize=(Count>UnpPackedSize) ? int64to32(UnpPackedSize):Count;
    if (UnpackFromMemory)
    {
      memcpy(Addr,UnpackFromMemoryAddr,UnpackFromMemorySize);
      RetCode=UnpackFromMemorySize;
      UnpackFromMemorySize=0;
    }
    else
    {
      if (!SrcFile->IsOpened())
        return(-1);
      RetCode=SrcFile->Read(ReadAddr,ReadSize);
      FileHeader *hd=SubHead!=NULL ? SubHead:&SrcArc->NewLhd;
      if (hd->Flags & LHD_SPLIT_AFTER)
        PackedCRC=CRC(PackedCRC,ReadAddr,ReadSize);
    }
    CurUnpRead+=RetCode;
    ReadAddr+=RetCode;
    TotalRead+=RetCode;
    Count-=RetCode;
    UnpPackedSize-=RetCode;
    if (UnpPackedSize == 0 && UnpVolume)
    {
#ifndef NOVOLUME
      if (!MergeArchive(*SrcArc,this,true,CurrentCommand))
#endif
        return(-1);
    }
    else
      break;
  }
  Archive *SrcArc=(Archive *)SrcFile;
  if (SrcArc!=NULL)
    ShowUnpRead(SrcArc->CurBlockPos+CurUnpRead,UnpArcSize);
  if (RetCode!=-1)
  {
    RetCode=TotalRead;
  }
  Wait();
  return(RetCode);
}


void ComprDataIO::UnpWrite(byte *Addr,uint Count)
{
#ifdef RARDLL
  RAROptions *Cmd=((Archive *)SrcFile)->GetRAROptions();
  if (Cmd->Callback!=NULL &&
      Cmd->Callback(UCM_PROCESSDATA,Cmd->UserData,(LONG)Addr,Count)==-1)
    ErrHandler.Exit(USER_BREAK);
  if (Cmd->ProcessDataProc!=NULL)
  {
    _EBX=_ESP;
    int RetCode=Cmd->ProcessDataProc(Addr,Count);
    _ESP=_EBX;
    if (RetCode==0)
      ErrHandler.Exit(USER_BREAK);
  }
#endif
  UnpWrAddr=Addr;
  UnpWrSize=Count;
  if (SrcUnpack!=NULL)
  {
    int NewSize=RepackUnpDataEnd+Count;
    RepackUnpData.Alloc(NewSize);
    memcpy(&RepackUnpData[RepackUnpDataEnd],Addr,Count);
    RepackUnpDataEnd=NewSize;

    SrcUnpack->SetSuspended(true);
  }
  if (UnpackToMemory)
  {
    if (Count <= UnpackToMemorySize)
    {
      memcpy(UnpackToMemoryAddr,Addr,Count);
      UnpackToMemoryAddr+=Count;
      UnpackToMemorySize-=Count;
    }
  }
  else
    if (!TestMode)
      DestFile->Write(Addr,Count);
  CurUnpWrite+=Count;
  if (!SkipUnpCRC)
#ifndef SFX_MODULE
    if (((Archive *)SrcFile)->OldFormat)
      UnpFileCRC=OldCRC((ushort)UnpFileCRC,Addr,Count);
    else
#endif
      UnpFileCRC=CRC(UnpFileCRC,Addr,Count);
  ShowUnpWrite();
  Wait();
}


void ComprDataIO::ShowPackRead(Int64 CurSize,Int64 UnpSize)
{
  if (ShowProgress)
  {
    Archive *DestArc=(Archive *)DestFile;
    RAROptions *Cmd=DestArc->GetRAROptions();
    int CurPercent=ToPercent(TotalPackRead,DestArc->AddingFilesSize);
    if (!Cmd->DisablePercentage && CurPercent!=LastPercent)
    {
      mprintf("\b\b\b\b%3d%%",CurPercent);
      LastPercent=CurPercent;
    }
  }
}




void ComprDataIO::ShowUnpRead(Int64 ArcPos,Int64 ArcSize)
{
  if (ShowProgress && SrcUnpack==NULL && SrcFile!=NULL)
  {
    Archive *SrcArc=(Archive *)SrcFile;
    RAROptions *Cmd=SrcArc->GetRAROptions();
    if (TotalArcSize!=0)
      ArcSize=TotalArcSize;
    ArcPos+=ProcessedArcSize;
    if (!SrcArc->Volume)
    {
      int CurPercent=ToPercent(ArcPos,ArcSize);
      if (!Cmd->DisablePercentage && CurPercent!=LastPercent)
      {
        mprintf("\b\b\b\b%3d%%",CurPercent);
        LastPercent=CurPercent;
      }
    }
  }
}


void ComprDataIO::ShowUnpWrite()
{
}








void ComprDataIO::SetFiles(File *SrcFile,File *DestFile)
{
  if (SrcFile!=NULL)
    ComprDataIO::SrcFile=SrcFile;
  if (DestFile!=NULL)
    ComprDataIO::DestFile=DestFile;
  LastPercent=-1;
}


void ComprDataIO::GetUnpackedData(byte **Data,uint *Size)
{
  *Data=UnpWrAddr;
  *Size=UnpWrSize;
}


void ComprDataIO::SetEncryption(int Method,char *Password,byte *Salt,bool Encrypt)
{
  if (Encrypt)
  {
    Encryption=*Password ? Method:0;
  }
  else
  {
    Decryption=*Password ? Method:0;
  }
}



void ComprDataIO::SetUnpackToMemory(byte *Addr,uint Size)
{
  UnpackToMemory=true;
  UnpackToMemoryAddr=Addr;
  UnpackToMemorySize=Size;
}
