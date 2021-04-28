/***** File: filestr.cpp *****/


#include "rar.hpp"

static bool IsUnicode(byte *Data, int Size);

bool ReadTextFile(char *Name, StringList *List, bool Config, bool AbortOnError,
                  bool ConvertToAnsi, bool Unquote, bool SkipComments) {
	char FileName[NM];
	if (Config)
		GetConfigName(Name, FileName);
	else
		strcpy(FileName, Name);

	File SrcFile;
	if (*FileName) {
		bool OpenCode = AbortOnError ? SrcFile.WOpen(FileName) : SrcFile.Open(FileName);

		if (!OpenCode) {
			if (AbortOnError)
				ErrHandler.Exit(OPEN_ERROR);
			return (false);
		}
	} else
		SrcFile.SetHandleType(FILE_HANDLESTD);

	unsigned int DataSize = 0, ReadSize;
	const int ReadBlock = 1024;
	Array<char> Data(ReadBlock + 5);
	while ((ReadSize = SrcFile.Read(&Data[DataSize], ReadBlock)) != 0) {
		DataSize += ReadSize;
		Data.Add(ReadSize);
	}

	memset(&Data[DataSize], 0, 5);

	if (IsUnicode((byte *)&Data[0], DataSize)) {
		wchar *CurStr = (wchar *)&Data[2];
		Array<char> AnsiName;

		while (*CurStr != 0) {
			wchar *NextStr = CurStr, *CmtPtr = NULL;
			while (*NextStr != '\r' && *NextStr != '\n' && *NextStr != 0) {
				if (SkipComments && NextStr[0] == '/' && NextStr[1] == '/') {
					*NextStr = 0;
					CmtPtr = NextStr;
				}
				NextStr++;
			}
			*NextStr = 0;
			for (wchar *SpacePtr = (CmtPtr ? CmtPtr : NextStr) - 1; SpacePtr >= CurStr; SpacePtr--) {
				if (*SpacePtr != ' ' && *SpacePtr != '\t')
					break;
				*SpacePtr = 0;
			}
			if (*CurStr) {
				int Length = strlenw(CurStr);
				int AddSize = Length - AnsiName.Size() + 1;
				if (AddSize > 0)
					AnsiName.Add(AddSize);
				if (Unquote && *CurStr == '\"' && CurStr[Length - 1] == '\"') {
					CurStr[Length - 1] = 0;
					CurStr++;
				}
				WideToChar(CurStr, &AnsiName[0]);
				List->AddString(&AnsiName[0], CurStr);
			}
			CurStr = NextStr + 1;
			while (*CurStr == '\r' || *CurStr == '\n')
				CurStr++;
		}
	} else {
		char *CurStr = &Data[0];
		while (*CurStr != 0) {
			char *NextStr = CurStr, *CmtPtr = NULL;
			while (*NextStr != '\r' && *NextStr != '\n' && *NextStr != 0) {
				if (SkipComments && NextStr[0] == '/' && NextStr[1] == '/') {
					*NextStr = 0;
					CmtPtr = NextStr;
				}
				NextStr++;
			}
			*NextStr = 0;
			for (char *SpacePtr = (CmtPtr ? CmtPtr : NextStr) - 1; SpacePtr >= CurStr; SpacePtr--) {
				if (*SpacePtr != ' ' && *SpacePtr != '\t')
					break;
				*SpacePtr = 0;
			}
			if (*CurStr) {
				if (Unquote && *CurStr == '\"') {
					int Length = strlen(CurStr);
					if (CurStr[Length - 1] == '\"') {
						CurStr[Length - 1] = 0;
						CurStr++;
					}
				}
				List->AddString(CurStr);
			}
			CurStr = NextStr + 1;
			while (*CurStr == '\r' || *CurStr == '\n')
				CurStr++;
		}
	}
	return (true);
}


bool IsUnicode(byte *Data, int Size) {
	if (Size < 4 || Data[0] != 0xff || Data[1] != 0xfe)
		return (false);
	for (int I = 2; I < Size; I++)
		if (Data[I] < 32)
			return (true);
	return (false);
}

/***** File: recvol.cpp *****/


#define RECVOL_BUFSIZE  0x800

RecVolumes::RecVolumes() {
	Buf.Alloc(RECVOL_BUFSIZE * 256);
	memset(SrcFile, 0, sizeof(SrcFile));
}


RecVolumes::~RecVolumes() {
	for (int I = 0; I < sizeof(SrcFile) / sizeof(SrcFile[0]); I++)
		delete SrcFile[I];
}




bool RecVolumes::Restore(RAROptions *Cmd, const char *Name,
                         const wchar *NameW, bool Silent) {
	char ArcName[NM];
	wchar ArcNameW[NM];
	strcpy(ArcName, Name);
	strcpyw(ArcNameW, NameW);
	char *Ext = GetExt(ArcName);
	bool NewStyle = false;
	bool RevName = Ext != NULL && stricomp(Ext, ".rev") == 0;
	if (RevName) {
		for (int DigitGroup = 0; Ext > ArcName && DigitGroup < 3; Ext--)
			if (!isdigit(*Ext))
				if (isdigit(*(Ext - 1)) && (*Ext == '_' || DigitGroup < 2))
					DigitGroup++;
				else if (DigitGroup < 2) {
					NewStyle = true;
					break;
				}
		while (isdigit(*Ext) && Ext > ArcName + 1)
			Ext--;
		strcpy(Ext, "*.*");
		FindFile Find;
		Find.SetMask(ArcName);
		struct FindData FD;
		while (Find.Next(&FD)) {
			Archive Arc(Cmd);
			if (Arc.WOpen(FD.Name, FD.NameW) && Arc.IsArchive(true)) {
				strcpy(ArcName, FD.Name);
				*ArcNameW = 0;
				break;
			}
		}
	}

	Archive Arc(Cmd);
	if (!Arc.WCheckOpen(ArcName, ArcNameW))
		return (false);
	if (!Arc.Volume) {
#ifndef SILENT
		Log(ArcName, St(MNotVolume), ArcName);
#endif
		return (false);
	}
	bool NewNumbering = (Arc.NewMhd.Flags & MHD_NEWNUMBERING);
	Arc.Close();
	char *VolNumStart = VolNameToFirstName(ArcName, ArcName, NewNumbering);
	char RecVolMask[NM];
	strcpy(RecVolMask, ArcName);
	int BaseNamePartLength = VolNumStart - ArcName;
	strcpy(RecVolMask + BaseNamePartLength, "*.rev");

	FindFile Find;
	Find.SetMask(RecVolMask);
	struct FindData RecData;
	int FileNumber = 0, RecVolNumber = 0, FoundRecVolumes = 0, MissingVolumes = 0;
	char PrevName[NM];
	while (Find.Next(&RecData)) {
		char *Name = RecData.Name;
		int P[3];
		if (!RevName && !NewStyle) {
			NewStyle = true;
			char *Dot = GetExt(Name);
			if (Dot != NULL) {
				int LineCount = 0;
				Dot--;
				while (Dot > Name && *Dot != '.') {
					if (*Dot == '_')
						LineCount++;
					Dot--;
				}
				if (LineCount == 2)
					NewStyle = false;
			}
		}
		if (NewStyle) {
			File CurFile;
			CurFile.TOpen(Name);
			CurFile.Seek(0, SEEK_END);
			Int64 Length = CurFile.Tell();
			CurFile.Seek(Length - 7, SEEK_SET);
			for (int I = 0; I < 3; I++)
				P[2 - I] = CurFile.GetByte() + 1;
			uint FileCRC = 0;
			for (int I = 0; I < 4; I++)
				FileCRC |= CurFile.GetByte() << (I * 8);
			if (FileCRC != CalcFileCRC(&CurFile, Length - 4)) {
#ifndef SILENT
				mprintf(St(MCRCFailed), Name);
#endif
				continue;
			}
		} else {
			char *Dot = GetExt(Name);
			if (Dot == NULL)
				continue;
			bool WrongParam = false;
			for (int I = 0; I < sizeof(P) / sizeof(P[0]); I++) {
				do {
					Dot--;
				} while (isdigit(*Dot) && Dot >= Name + BaseNamePartLength);
				P[I] = atoi(Dot + 1);
				if (P[I] == 0 || P[I] > 255)
					WrongParam = true;
			}
			if (WrongParam)
				continue;
		}
		if (P[1] + P[2] > 255)
			continue;
		if (RecVolNumber != 0 && RecVolNumber != P[1] || FileNumber != 0 && FileNumber != P[2]) {
#ifndef SILENT
			Log(NULL, St(MRecVolDiffSets), Name, PrevName);
#endif
			return (false);
		}
		RecVolNumber = P[1];
		FileNumber = P[2];
		strcpy(PrevName, Name);
		File *NewFile = new File;
		NewFile->TOpen(Name);
		SrcFile[FileNumber + P[0] - 1] = NewFile;
		FoundRecVolumes++;
	}
#ifndef SILENT
	if (!Silent || FoundRecVolumes != 0) {
		mprintf(St(MRecVolFound), FoundRecVolumes);
	}
#endif
	if (FoundRecVolumes == 0)
		return (false);

	bool WriteFlags[256];
	memset(WriteFlags, 0, sizeof(WriteFlags));

	char LastVolName[NM];
	*LastVolName = 0;

	for (int CurArcNum = 0; CurArcNum < FileNumber; CurArcNum++) {
		Archive *NewFile = new Archive;
		bool ValidVolume = FileExist(ArcName);
		if (ValidVolume) {
			NewFile->TOpen(ArcName);
			ValidVolume = NewFile->IsArchive(false);
			if (ValidVolume) {
				bool EndFound = false, EndBlockRequired = false;
				while (NewFile->ReadHeader() != 0) {
					if (NewFile->GetHeaderType() == FILE_HEAD) {
						if (NewFile->NewLhd.UnpVer >= 29)
							EndBlockRequired = true;
						if (!EndBlockRequired && (NewFile->NewLhd.Flags & LHD_SPLIT_AFTER))
							EndFound = true;
					}
					if (NewFile->GetHeaderType() == ENDARC_HEAD) {
						if ((NewFile->EndArcHead.Flags & EARC_DATACRC) != 0 &&
						        NewFile->EndArcHead.ArcDataCRC != CalcFileCRC(NewFile, NewFile->CurBlockPos)) {
							ValidVolume = false;
#ifndef SILENT
							mprintf(St(MCRCFailed), ArcName);
#endif
						}
						EndFound = true;
					}
					NewFile->SeekToNext();
				}
				if (!EndFound)
					ValidVolume = false;
			}
			if (!ValidVolume) {
				NewFile->Close();
				char NewName[NM];
				strcpy(NewName, ArcName);
				strcat(NewName, ".bad");
#ifndef SILENT
				mprintf(St(MBadArc), ArcName);
				mprintf(St(MRenaming), ArcName, NewName);
#endif
				rename(ArcName, NewName);
			}
			NewFile->Seek(0, SEEK_SET);
		}
		if (!ValidVolume) {
			NewFile->TCreate(ArcName);
			WriteFlags[CurArcNum] = true;
			MissingVolumes++;

			if (CurArcNum == FileNumber - 1)
				strcpy(LastVolName, ArcName);

#ifndef SILENT
			mprintf(St(MAbsNextVol), ArcName);
#endif
		}
		SrcFile[CurArcNum] = (File *)NewFile;
		NextVolumeName(ArcName, !NewNumbering);
	}

#ifndef SILENT
	mprintf(St(MRecVolMissing), MissingVolumes);
#endif

	if (MissingVolumes == 0) {
#ifndef SILENT
		mprintf(St(MRecVolAllExist));
#endif
		return (false);
	}

	if (MissingVolumes > FoundRecVolumes) {
#ifndef SILENT
		mprintf(St(MRecVolCannotFix));
#endif
		return (false);
	}
#ifndef SILENT
	mprintf(St(MReconstructing));
#endif

	RSCoder RSC(RecVolNumber);

	int TotalFiles = FileNumber + RecVolNumber;
	int Erasures[256], EraSize = 0;

	for (int I = 0; I < TotalFiles; I++)
		if (WriteFlags[I] || SrcFile[I] == NULL)
			Erasures[EraSize++] = I;


	while (true) {
		int MaxRead = 0;
		for (int I = 0; I < TotalFiles; I++)
			if (WriteFlags[I] || SrcFile[I] == NULL)
				memset(&Buf[I * RECVOL_BUFSIZE], 0, RECVOL_BUFSIZE);
			else {
				int ReadSize = SrcFile[I]->Read(&Buf[I * RECVOL_BUFSIZE], RECVOL_BUFSIZE);
				if (ReadSize != RECVOL_BUFSIZE)
					memset(&Buf[I * RECVOL_BUFSIZE + ReadSize], 0, RECVOL_BUFSIZE - ReadSize);
				if (ReadSize > MaxRead)
					MaxRead = ReadSize;
			}
		if (MaxRead == 0)
			break;
		for (int BufPos = 0; BufPos < MaxRead; BufPos++) {
			byte Data[256];
			for (int I = 0; I < TotalFiles; I++)
				Data[I] = Buf[I * RECVOL_BUFSIZE + BufPos];
			RSC.Decode(Data, TotalFiles, Erasures, EraSize);
			for (int I = 0; I < EraSize; I++)
				Buf[Erasures[I]*RECVOL_BUFSIZE + BufPos] = Data[Erasures[I]];
			/*
			      for (int I=0;I<FileNumber;I++)
			        Buf[I*RECVOL_BUFSIZE+BufPos]=Data[I];
			*/
		}
		for (int I = 0; I < FileNumber; I++)
			if (WriteFlags[I])
				SrcFile[I]->Write(&Buf[I * RECVOL_BUFSIZE], MaxRead);
	}
	for (int I = 0; I < RecVolNumber + FileNumber; I++)
		if (SrcFile[I] != NULL) {
			File *CurFile = SrcFile[I];
			if (NewStyle && WriteFlags[I]) {
				Int64 Length = CurFile->Tell();
				CurFile->Seek(Length - 7, SEEK_SET);
				for (int J = 0; J < 7; J++)
					CurFile->PutByte(0);
			}
			CurFile->Close();
			SrcFile[I] = NULL;
		}
	if (*LastVolName) {
		Archive Arc(Cmd);
		if (Arc.Open(LastVolName, NULL, false, true) && Arc.IsArchive(true) &&
		        Arc.SearchBlock(ENDARC_HEAD)) {
			Arc.Seek(Arc.NextBlockPos, SEEK_SET);
			char Buf[8192];
			int ReadSize = Arc.Read(Buf, sizeof(Buf));
			int ZeroCount = 0;
			while (ZeroCount < ReadSize && Buf[ZeroCount] == 0)
				ZeroCount++;
			if (ZeroCount == ReadSize) {
				Arc.Seek(Arc.NextBlockPos, SEEK_SET);
				Arc.Truncate();
			}
		}
	}
#if !defined(GUI) && !defined(SILENT)
	if (!Silent)
		mprintf(St(MDone));
#endif
	return (true);
}


/***** File: rs.cpp *****/


#define Clean(D,S)  {for (int I=0;I<(S);I++) (D)[I]=0;}

RSCoder::RSCoder(int ParSize) {
	RSCoder::ParSize = ParSize;
	FirstBlockDone = false;
	gfInit();
	pnInit();
}


void RSCoder::gfInit() {
	for (int I = 0, J = 1; I < MAXPAR; I++) {
		gfLog[J] = I;
		gfExp[I] = J;
		if ((J <<= 1) & 256)
			J ^= 285;
	}
	for (int I = MAXPAR; I < MAXPOL; I++)
		gfExp[I] = gfExp[I - MAXPAR];
}


inline int RSCoder::gfMult(int a, int b) {
	return (a == 0 || b == 0 ? 0 : gfExp[gfLog[a] + gfLog[b]]);
}


void RSCoder::pnInit() {
	int p1[MAXPAR + 1], p2[MAXPAR + 1];

	Clean(p2, ParSize);
	p2[0] = 1;
	for (int I = 1; I <= ParSize; I++) {
		Clean(p1, ParSize);
		p1[0] = gfExp[I];
		p1[1] = 1;
		pnMult(p1, p2, GXPol);
		for (int J = 0; J < ParSize; J++)
			p2[J] = GXPol[J];
	}
}


void RSCoder::pnMult(int *p1, int *p2, int *r) {
	Clean(r, ParSize);
	for (int I = 0; I < ParSize; I++)
		if (p1[I] != 0)
			for (int J = 0; J < ParSize - I; J++)
				r[I + J] ^= gfMult(p1[I], p2[J]);
}


bool RSCoder::Decode(byte *Data, int DataSize, int *EraLoc, int EraSize) {
	int SynData[MAXPOL];
	bool AllZeroes = true;
	for (int I = 0; I < ParSize; I++) {
		int Sum = Data[0], J = 1, Exp = gfExp[I + 1];
		for (; J + 8 <= DataSize; J += 8) {
			Sum = Data[J] ^ gfMult(Exp, Sum);
			Sum = Data[J + 1] ^ gfMult(Exp, Sum);
			Sum = Data[J + 2] ^ gfMult(Exp, Sum);
			Sum = Data[J + 3] ^ gfMult(Exp, Sum);
			Sum = Data[J + 4] ^ gfMult(Exp, Sum);
			Sum = Data[J + 5] ^ gfMult(Exp, Sum);
			Sum = Data[J + 6] ^ gfMult(Exp, Sum);
			Sum = Data[J + 7] ^ gfMult(Exp, Sum);
		}
		for (; J < DataSize; J++)
			Sum = Data[J] ^ gfMult(Exp, Sum);
		if ((SynData[I] = Sum) != 0)
			AllZeroes = false;
	}
	if (AllZeroes)
		return (true);

	if (!FirstBlockDone) {
		FirstBlockDone = true;
		Clean(PolB, ParSize + 1);
		PolB[0] = 1;
		for (int EraPos = 0; EraPos < EraSize; EraPos++)
			for (int I = ParSize, M = gfExp[DataSize - EraLoc[EraPos] - 1]; I > 0; I--)
				PolB[I] ^= gfMult(M, PolB[I - 1]);

		ErrCount = 0;
		for (int Root = MAXPAR - DataSize; Root < MAXPAR + 1; Root++) {
			int Sum = 0;
			for (int B = 0; B < ParSize + 1; B++)
				Sum ^= gfMult(gfExp[(B * Root) % MAXPAR], PolB[B]);
			if (Sum == 0) {
				Dn[ErrCount] = 0;
				for (int I = 1; I < ParSize + 1; I += 2)
					Dn[ErrCount] ^= gfMult(PolB[I], gfExp[Root * (I - 1) % MAXPAR]);
				ErrorLocs[ErrCount++] = MAXPAR - Root;
			}
		}
	}

	int PolD[MAXPOL];
	pnMult(PolB, SynData, PolD);
	if ((ErrCount <= ParSize) && ErrCount > 0)
		for (int I = 0; I < ErrCount; I++) {
			int Loc = ErrorLocs[I], DLoc = MAXPAR - Loc, N = 0;
			for (int J = 0; J < ParSize; J++)
				N ^= gfMult(PolD[J], gfExp[DLoc * J % MAXPAR]);
			int DataPos = DataSize - Loc - 1;
			if (DataPos >= 0 && DataPos < DataSize)
				Data[DataPos] ^= gfMult(N, gfExp[MAXPAR - gfLog[Dn[I]]]);
		}
	return (ErrCount <= ParSize);
}


/***** File: scantree.cpp *****/


ScanTree::ScanTree(StringList *FileMasks, int Recurse, bool GetLinks, int GetDirs) {
	ScanTree::FileMasks = FileMasks;
	ScanTree::Recurse = Recurse;
	ScanTree::GetLinks = GetLinks;
	ScanTree::GetDirs = GetDirs;

	*CurMask = 0;
	*CurMaskW = 0;
	memset(FindStack, 0, sizeof(FindStack));
	Depth = 0;
	Errors = 0;
	FastFindFile = false;
}


ScanTree::~ScanTree() {
	for (int I = Depth; I >= 0; I--)
		if (FindStack[I] != NULL)
			delete FindStack[I];
}


int ScanTree::GetNext(FindData *FindData) {
	if (Depth < 0)
		return (SCAN_DONE);

	int FindCode;
	while (1) {
		if ((*CurMask == 0 || FastFindFile && Depth == 0) && !PrepareMasks())
			return (SCAN_DONE);
		FindCode = FindProc(FindData);
		if (FindCode == SCAN_ERROR) {
			Errors++;
			continue;
		}
		if (FindCode == SCAN_NEXT)
			continue;
		if (FindCode == SCAN_SUCCESS && FindData->IsDir && GetDirs == SCAN_SKIPDIRS)
			continue;
		if (FindCode == SCAN_DONE && PrepareMasks())
			continue;
		break;
	}
	return (FindCode);
}


bool ScanTree::PrepareMasks() {
	if (!FileMasks->GetString(CurMask, CurMaskW, sizeof(CurMask)))
		return (false);
	char *Name = PointToName(CurMask);
	if (*Name == 0)
		strcat(CurMask, MASKALL);
	if (Name[0] == '.' && (Name[1] == 0 || Name[1] == '.' && Name[2] == 0)) {
		AddEndSlash(CurMask);
		strcat(CurMask, MASKALL);
	}
	SpecPathLength = Name - CurMask;
//  if (SpecPathLength>1)
//    SpecPathLength--;

	bool WideName = (*CurMaskW != 0);

	if (WideName) {
		wchar *NameW = PointToName(CurMaskW);
		if (*NameW == 0)
			strcatw(CurMaskW, MASKALLW);
		if (NameW[0] == '.' && (NameW[1] == 0 || NameW[1] == '.' && NameW[2] == 0)) {
			AddEndSlash(CurMaskW);
			strcatw(CurMaskW, MASKALLW);
		}
		SpecPathLengthW = NameW - CurMaskW;
//    if (SpecPathLengthW>1)
//      SpecPathLengthW--;
	} else {
		wchar WideMask[NM];
		CharToWide(CurMask, WideMask);
		SpecPathLengthW = PointToName(WideMask) - WideMask;
	}
	Depth = 0;
	return (true);
}


int ScanTree::FindProc(FindData *FindData) {
	if (*CurMask == 0)
		return (SCAN_NEXT);
	FastFindFile = false;
	if (FindStack[Depth] == NULL) {
		bool Wildcards = IsWildcard(CurMask, CurMaskW);
		bool FindCode = !Wildcards && FindFile::FastFind(CurMask, CurMaskW, FindData, GetLinks);
		bool IsDir = FindCode && FindData->IsDir;
		bool SearchAll = !IsDir && (Depth > 0 || Recurse == RECURSE_ALWAYS ||
		                            Wildcards && Recurse == RECURSE_WILDCARDS);
		if (Depth == 0)
			SearchAllInRoot = SearchAll;
		if (SearchAll || Wildcards) {
			FindStack[Depth] = new FindFile;
			char SearchMask[NM];
			strcpy(SearchMask, CurMask);
			if (SearchAll)
				strcpy(PointToName(SearchMask), MASKALL);
			FindStack[Depth]->SetMask(SearchMask);
			if (*CurMaskW) {
				wchar SearchMaskW[NM];
				strcpyw(SearchMaskW, CurMaskW);
				if (SearchAll)
					strcpyw(PointToName(SearchMaskW), MASKALLW);
				FindStack[Depth]->SetMaskW(SearchMaskW);
			}
		} else {
			FastFindFile = true;
			if (!FindCode) {
				ErrHandler.OpenErrorMsg(CurMask);
				return (FindData->Error ? SCAN_ERROR : SCAN_NEXT);
			}
		}
	}

	if (!FastFindFile && !FindStack[Depth]->Next(FindData, GetLinks)) {
		bool Error = FindData->Error;

#ifndef SILENT
		if (Error) {
			Log(NULL, St(MScanError), CurMask);
		}
#endif

		char DirName[NM];
		wchar DirNameW[NM];
		*DirName = 0;
		*DirNameW = 0;

		delete FindStack[Depth];
		FindStack[Depth--] = NULL;
		while (Depth >= 0 && FindStack[Depth] == NULL)
			Depth--;
		if (Depth < 0)
			return (SCAN_DONE);
		char *Slash = strrchrd(CurMask, CPATHDIVIDER);
		if (Slash != NULL) {
			char Mask[NM];
			strcpy(Mask, Slash);
			*Slash = 0;
			strcpy(DirName, CurMask);
			char *PrevSlash = strrchrd(CurMask, CPATHDIVIDER);
			if (PrevSlash == NULL)
				strcpy(CurMask, Mask + 1);
			else
				strcpy(PrevSlash, Mask);
		}

		if (*CurMaskW != 0) {
			wchar *Slash = strrchrw(CurMaskW, CPATHDIVIDER);
			if (Slash != NULL) {
				wchar Mask[NM];
				strcpyw(Mask, Slash);
				*Slash = 0;
				strcpyw(DirNameW, CurMaskW);
				wchar *PrevSlash = strrchrw(CurMaskW, CPATHDIVIDER);
				if (PrevSlash == NULL)
					strcpyw(CurMaskW, Mask + 1);
				else
					strcpyw(PrevSlash, Mask);
			}
			if (LowAscii(CurMaskW))
				*CurMaskW = 0;
		}
		if (GetDirs == SCAN_GETDIRSTWICE &&
		        FindFile::FastFind(DirName, DirNameW, FindData, GetLinks) && FindData->IsDir)
			return (Error ? SCAN_ERROR : SCAN_SUCCESS);
		return (Error ? SCAN_ERROR : SCAN_NEXT);
	}

	if (FindData->IsDir) {
		if (!FastFindFile && Depth == 0 && !SearchAllInRoot)
			return (GetDirs == SCAN_GETCURDIRS ? SCAN_SUCCESS : SCAN_NEXT);

		char Mask[NM];
		strcpy(Mask, FastFindFile ? MASKALL : PointToName(CurMask));
		strcpy(CurMask, FindData->Name);

		if (strlen(CurMask) + strlen(Mask) + 1 >= NM || Depth >= MAXSCANDEPTH - 1) {
#ifndef SILENT
			Log(NULL, "\n%s%c%s", CurMask, CPATHDIVIDER, Mask);
			Log(NULL, St(MPathTooLong));
#endif
			return (SCAN_ERROR);
		}

		AddEndSlash(CurMask);
		strcat(CurMask, Mask);

		if (*CurMaskW && *FindData->NameW == 0)
			CharToWide(FindData->Name, FindData->NameW);
		if (*FindData->NameW != 0) {
			wchar Mask[NM];
			if (FastFindFile)
				strcpyw(Mask, MASKALLW);
			else if (*CurMaskW)
				strcpyw(Mask, PointToName(CurMaskW));
			else
				CharToWide(PointToName(CurMask), Mask);
			strcpyw(CurMaskW, FindData->NameW);
			AddEndSlash(CurMaskW);
			strcatw(CurMaskW, Mask);
		}
		Depth++;
	}
	if (!FastFindFile && !CmpName(CurMask, FindData->Name, MATCH_NAMES))
		return (SCAN_NEXT);
	return (SCAN_SUCCESS);
}
