#include "rar.hpp"

static void GetFirstNewVolName(const char *ArcName, char *VolName,
                               Int64 VolSize, Int64 TotalSize);



bool MergeArchive(Archive &Arc, ComprDataIO *DataIO, bool ShowFileName, char Command)
{
	RAROptions *Cmd = Arc.GetRAROptions();

	int HeaderType = Arc.GetHeaderType();
	FileHeader *hd = HeaderType == NEWSUB_HEAD ? &Arc.SubHead : &Arc.NewLhd;
	bool SplitHeader = (HeaderType == FILE_HEAD || HeaderType == NEWSUB_HEAD) &&
	                   (hd->Flags & LHD_SPLIT_AFTER) != 0;

	if (DataIO != NULL && SplitHeader && hd->UnpVer >= 20 &&
	        hd->FileCRC != 0xffffffff && DataIO->PackedCRC != ~hd->FileCRC)
	{
		Log(Arc.FileName, St(MDataBadCRC), hd->FileName, Arc.FileName);
	}

	Arc.Close();

	char NextName[NM];
	strcpy(NextName, Arc.FileName);
	NextVolumeName(NextName, (Arc.NewMhd.Flags & MHD_NEWNUMBERING) == 0 || Arc.OldFormat);

#if !defined(SFX_MODULE)
	bool RecoveryDone = false;
#endif

	while (!Arc.Open(NextName))
	{
#ifndef SFX_MODULE
		if (!RecoveryDone)
		{
			RecVolumes RecVol;
			RecVol.Restore(Cmd, Arc.FileName, Arc.FileNameW, true);
			RecoveryDone = true;
			continue;
		}
#endif

#ifndef GUI
		if (!Cmd->VolumePause && !IsRemovable(NextName))
		{
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
	if (Arc.GetHeaderType() == FILE_HEAD)
	{
		Arc.ConvertAttributes();
		Arc.Seek(Arc.NextBlockPos - Arc.NewLhd.FullPackSize, SEEK_SET);
	}
#ifndef GUI
	if (ShowFileName)
	{
		mprintf(St(MExtrPoints), IntNameToExt(Arc.NewLhd.FileName));
		if (!Cmd->DisablePercentage)
			mprintf("     ");
	}
#endif
	if (DataIO != NULL)
	{
		if (HeaderType == ENDARC_HEAD)
			DataIO->UnpVolume = false;
		else
		{
			DataIO->UnpVolume = (hd->Flags & LHD_SPLIT_AFTER);
			DataIO->SetPackedSizeToRead(hd->FullPackSize);
		}
		DataIO->PackedCRC = 0xffffffff;
//    DataIO->SetFiles(&Arc,NULL);
	}
	return (true);
}






#ifndef SILENT
bool AskNextVol(char *ArcName)
{
	eprintf(St(MAskNextVol), ArcName);
	if (Ask(St(MContinueQuit)) == 2)
		return (false);
	return (true);
}
#endif
