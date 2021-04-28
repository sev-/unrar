#include "rar.hpp"

static File *CreatedFiles[16];

File::File()
{
	hFile = BAD_HANDLE;
	*FileName = 0;
	*FileNameW = 0;
	NewFile = false;
	LastWrite = false;
	HandleType = FILE_HANDLENORMAL;
	SkipClose = false;
	IgnoreReadErrors = false;
	ErrorType = FILE_SUCCESS;
	OpenShared = false;
	AllowDelete = true;
}


File::~File()
{
	if (hFile != BAD_HANDLE && !SkipClose)
		if (NewFile)
			Delete();
		else
			Close();
}


void File::operator = (File &SrcFile)
{
	hFile = SrcFile.hFile;
	strcpy(FileName, SrcFile.FileName);
	NewFile = SrcFile.NewFile;
	LastWrite = SrcFile.LastWrite;
	HandleType = SrcFile.HandleType;
	SrcFile.SkipClose = true;
}


bool File::Open(const char *Name, const wchar *NameW, bool OpenShared, bool Update)
{
	ErrorType = FILE_SUCCESS;
	FileHandle hNewFile;
	int flags = Update ? O_RDWR : O_RDONLY;
#ifdef O_BINARY
	flags |= O_BINARY;
#endif
	int handle = open(Name, flags);
	hNewFile = handle == -1 ? BAD_HANDLE : fdopen(handle, Update ? UPDATEBINARY : READBINARY);
	if (hNewFile == BAD_HANDLE && errno == ENOENT)
		ErrorType = FILE_NOTFOUND;

	NewFile = false;
	HandleType = FILE_HANDLENORMAL;
	SkipClose = false;
	bool Success = hNewFile != BAD_HANDLE;
	if (Success)
	{
		hFile = hNewFile;
		if (NameW != NULL)
			strcpyw(FileNameW, NameW);
		strcpy(FileName, Name);
	}
	return (Success);
}


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void File::TOpen(const char *Name, const wchar *NameW)
{
	if (!WOpen(Name, NameW))
		ErrHandler.Exit(OPEN_ERROR);
}
#endif


bool File::WOpen(const char *Name, const wchar *NameW)
{
	if (Open(Name, NameW))
		return (true);
	ErrHandler.OpenErrorMsg(Name);
	return (false);
}


bool File::Create(const char *Name, const wchar *NameW)
{
	hFile = fopen(Name, CREATEBINARY);

	NewFile = true;
	HandleType = FILE_HANDLENORMAL;
	SkipClose = false;
	if (NameW != NULL)
		strcpyw(FileNameW, NameW);
	strcpy(FileName, Name);
	if (hFile != BAD_HANDLE)
		for (int I = 0; I < sizeof(CreatedFiles) / sizeof(CreatedFiles[0]); I++)
			if (CreatedFiles[I] == NULL)
			{
				CreatedFiles[I] = this;
				break;
			}
	return (hFile != BAD_HANDLE);
}


#if !defined(SHELL_EXT) && !defined(SFX_MODULE)
void File::TCreate(const char *Name, const wchar *NameW)
{
	if (!WCreate(Name, NameW))
		ErrHandler.Exit(FATAL_ERROR);
}
#endif


bool File::WCreate(const char *Name, const wchar *NameW)
{
	if (Create(Name, NameW))
		return (true);
	ErrHandler.CreateErrorMsg(Name);
	return (false);
}


bool File::Close()
{
	bool Success = true;
	if (HandleType != FILE_HANDLENORMAL)
		HandleType = FILE_HANDLENORMAL;
	else if (hFile != BAD_HANDLE)
	{
		if (!SkipClose)
		{
			Success = fclose(hFile) != EOF;
			for (int I = 0; I < sizeof(CreatedFiles) / sizeof(CreatedFiles[0]); I++)
				if (CreatedFiles[I] == this)
				{
					CreatedFiles[I] = NULL;
					break;
				}
		}
		hFile = BAD_HANDLE;
		if (!Success)
			ErrHandler.CloseError(FileName);
	}
	return (Success);
}


void File::Flush()
{
	fflush(hFile);
}


bool File::Delete()
{
	if (HandleType != FILE_HANDLENORMAL || !AllowDelete)
		return (false);
	if (hFile != BAD_HANDLE)
		Close();
	bool RetCode;
	RetCode = (remove(FileName) == 0);
	return (RetCode);
}


bool File::Rename(const char *NewName)
{
	bool Success = strcmp(FileName, NewName) == 0 || rename(FileName, NewName) == 0;
	if (Success)
	{
		strcpy(FileName, NewName);
		*FileNameW = 0;
	}
	return (Success);
}


void File::Write(const void *Data, int Size)
{
	if (Size == 0)
		return;
	if (HandleType != FILE_HANDLENORMAL)
		switch (HandleType)
		{
		case FILE_HANDLESTD:
			hFile = stdout;
			break;
		case FILE_HANDLEERR:
			hFile = stderr;
			break;
		}
	while (1)
	{
		bool Success;
		Success = (fwrite(Data, 1, Size, hFile) == Size && !ferror(hFile));
		if (!Success && HandleType == FILE_HANDLENORMAL)
		{
			if (ErrHandler.AskRepeatWrite(FileName))
				continue;
			ErrHandler.WriteError(FileName);
		}
		break;
	}
	LastWrite = true;
}


int File::Read(void *Data, int Size)
{
	Int64 FilePos;
	if (IgnoreReadErrors)
		FilePos = Tell();
	int ReadSize;
	while (true)
	{
		ReadSize = DirectRead(Data, Size);
		if (ReadSize == -1)
			if (IgnoreReadErrors)
			{
				ReadSize = 0;
				for (int I = 0; I < Size; I += 512)
				{
					Seek(FilePos + I, SEEK_SET);
					int SizeToRead = Min(Size - I, 512);
					int ReadCode = DirectRead(Data, SizeToRead);
					ReadSize += (ReadCode == -1) ? 512 : ReadCode;
				}
			}
			else
			{
				if (HandleType == FILE_HANDLENORMAL && ErrHandler.AskRepeatRead(FileName))
					continue;
				ErrHandler.ReadError(FileName);
			}
		break;
	}
	return (ReadSize);
}


int File::DirectRead(void *Data, int Size)
{
	if (HandleType == FILE_HANDLESTD)
	{
		hFile = stdin;
	}
	if (LastWrite)
	{
		fflush(hFile);
		LastWrite = false;
	}
	clearerr(hFile);
	int ReadSize = fread(Data, 1, Size, hFile);
	if (ferror(hFile))
		return (-1);
	return (ReadSize);
}


void File::Seek(Int64 Offset, int Method)
{
	if (!RawSeek(Offset, Method))
		ErrHandler.SeekError(FileName);
}


bool File::RawSeek(Int64 Offset, int Method)
{
	if (hFile == BAD_HANDLE)
		return (true);
	LastWrite = false;
	if (fseek(hFile, int64to32(Offset), Method) != 0)
		return (false);
	return (true);
}


Int64 File::Tell()
{
	return (ftell(hFile));
}


void File::Prealloc(Int64 Size)
{
}


byte File::GetByte()
{
	byte Byte = 0;
	Read(&Byte, 1);
	return (Byte);
}


void File::PutByte(byte Byte)
{
	Write(&Byte, 1);
}


bool File::Truncate()
{
	return (false);
}


void File::SetOpenFileTime(uint ft)
{
}


void File::SetCloseFileTime(uint ft)
{
	struct utimbuf ut;
	ut.actime = ut.modtime = DosTimeToUnix(ft);
	utime(FileName, &ut);
}


uint File::GetOpenFileTime()
{
	struct stat st;
	fstat(fileno(hFile), &st);
	return (UnixTimeToDos(st.st_mtime));
}


void File::SetOpenFileStat(uint FileTime)
{
}


void File::SetCloseFileStat(uint FileTime, uint FileAttr)
{
	SetCloseFileTime(FileTime);
	chmod(FileName, (mode_t)FileAttr);
}


Int64 File::FileLength()
{
	SaveFilePos SavePos(*this);
	Seek(0, SEEK_END);
	return (Tell());
}


void File::SetHandleType(FILE_HANDLETYPE Type)
{
	HandleType = Type;
}


bool File::IsDevice()
{
	if (hFile == BAD_HANDLE)
		return (false);
	return (isatty(fileno(hFile)));
}


#ifndef SFX_MODULE
void File::fprintf(const char *fmt, ...)
{
	va_list argptr;
	va_start(argptr, fmt);
	char Msg[8192], OutMsg[8192];
	vsprintf(Msg, fmt, argptr);
	strcpy(OutMsg, Msg);
	Write(OutMsg, strlen(OutMsg));
	va_end(argptr);
}
#endif


void File::RemoveCreated()
{
	for (int I = 0; I < sizeof(CreatedFiles) / sizeof(CreatedFiles[0]); I++)
		if (CreatedFiles[I] != NULL)
			CreatedFiles[I]->Delete();
}


#ifndef SFX_MODULE
long File::Copy(File &Dest, Int64 Length)
{
	Array<char> Buffer(0x10000);
	long CopySize = 0;
	bool CopyAll = (Length == INT64ERR);

	while (CopyAll || Length > 0)
	{
		Wait();
		int SizeToRead = (!CopyAll && Length < Buffer.Size()) ? int64to32(Length) : Buffer.Size();
		int ReadSize = Read(&Buffer[0], SizeToRead);
		if (ReadSize == 0)
			break;
		Dest.Write(&Buffer[0], ReadSize);
		CopySize += ReadSize;
		if (!CopyAll)
			Length -= ReadSize;
	}
	return (CopySize);
}
#endif
