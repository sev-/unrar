#ifndef _RAR_RARCOMMON_
#define _RAR_RARCOMMON_

/***** File: errhnd.hpp *****/


enum { SUCCESS, WARNING, FATAL_ERROR, CRC_ERROR, LOCK_ERROR, WRITE_ERROR,
       OPEN_ERROR, USER_ERROR, MEMORY_ERROR, USER_BREAK = 255
     };

class ErrorHandler {
private:
	void ErrMsg(char *ArcName, const char *fmt, ...);

	int ExitCode;
	int ErrCount;
	bool EnableBreak;
	bool Silent;
	bool DoShutdown;
public:
	ErrorHandler();
	void Clean();
	void MemoryError();
	void OpenError(const char *FileName);
	void CloseError(const char *FileName);
	void ReadError(const char *FileName);
	bool AskRepeatRead(const char *FileName);
	void WriteError(const char *FileName);
	void WriteErrorFAT(const char *FileName);
	bool AskRepeatWrite(const char *FileName);
	void SeekError(const char *FileName);
	void MemoryErrorMsg();
	void OpenErrorMsg(const char *FileName);
	void CreateErrorMsg(const char *FileName);
	void ReadErrorMsg(const char *FileName);
	void Exit(int ExitCode);
	void SetErrorCode(int Code);
	int GetErrorCode() {
		return (ExitCode);
	}
	int GetErrorCount() {
		return (ErrCount);
	}
	void SetSignalHandlers(bool Enable);
	void Throw(int Code);
	void SetSilent(bool Mode) {
		Silent = Mode;
	};
	void SetShutdown(bool Mode) {
		DoShutdown = Mode;
	};
};


extern ErrorHandler ErrHandler;


/***** File: os.hpp *****/


#define FALSE 0
#define TRUE  1

#define  NM  1024

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <utime.h>

#define ENABLE_ACCESS

#define DefConfigName  ".rarrc"
#define DefLogName     ".rarlog"

#define HOST_OS     UNIX

#define PATHDIVIDER  "/"
#define PATHDIVIDERW L"/"
#define CPATHDIVIDER '/'
#define MASKALL      "*"
#define MASKALLW     L"*"

#define READBINARY   "r"
#define READTEXT     "r"
#define UPDATEBINARY "r+"
#define CREATEBINARY "w+"
#define APPENDTEXT   "a"

#define _stdfunction

#ifdef _APPLE
#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif
#ifdef LITTLE_ENDIAN
#undef LITTLE_ENDIAN
#endif
#endif

#if defined(__sparc) || defined(sparc)
#ifndef BIG_ENDIAN
#define BIG_ENDIAN
#endif
#endif

typedef const char *MSGID;


/***** File: version.hpp *****/


#define RARVER_MAJOR     3
#define RARVER_MINOR    10
#define RARVER_BETA      1
#define RARVER_DAY      16
#define RARVER_MONTH    10
#define RARVER_YEAR   2002


/***** File: rartypes.hpp *****/


typedef unsigned char    byte;   //8 bits
typedef unsigned short   ushort; //preferably 16 bits, but can be more
typedef unsigned int     uint;   //32 bits or more

typedef unsigned int     uint32; //32 bits exactly
#define PRESENT_INT32

#if defined(__GNUC__)
typedef wchar_t wchar;
#else
typedef ushort wchar;
#endif

#define SHORT16(x) (sizeof(ushort)==2 ? (ushort)(x):((x)&0xffff))


/***** File: rardefs.hpp *****/


#define  Min(x,y) (((x)<(y)) ? (x):(y))
#define  Max(x,y) (((x)>(y)) ? (x):(y))

#define  MAXPASSWORD       128

#define  DefSFXName        "default.sfx"
#define  DefSortListName   "rarfiles.lst"

#ifndef FA_RDONLY
#define FA_RDONLY   0x01
#define FA_HIDDEN   0x02
#define FA_SYSTEM   0x04
#define FA_LABEL    0x08
#define FA_DIREC    0x10
#define FA_ARCH     0x20
#endif


/***** File: loclang.hpp *****/

#define   MYesNo             "_Yes_No"
#define   MYesNoAll          "_Yes_No_All"
#define   MYesNoAllRenQ      "_Yes_No_All_nEver_Rename_Quit"
#define   MContinueQuit      "_Continue_Quit"
#define   MRetryAbort        "_Retry_Abort"
#define   MCopyright         "\nRAR %s    Copyright (c) 1993-%d Eugene Roshal    %d %s %d"
#define   MRegTo             "\nRegistered to %s\n"
#define   MShare             "\nShareware version         Type RAR -? for help\n"
#define   MUCopyright        "\nUNRAR %s freeware      Copyright (c) 1993-%d Eugene Roshal\n"
#define   MBeta              "beta"
#define   MMonthJan          "Jan"
#define   MMonthFeb          "Feb"
#define   MMonthMar          "Mar"
#define   MMonthApr          "Apr"
#define   MMonthMay          "May"
#define   MMonthJun          "Jun"
#define   MMonthJul          "Jul"
#define   MMonthAug          "Aug"
#define   MMonthSep          "Sep"
#define   MMonthOct          "Oct"
#define   MMonthNov          "Nov"
#define   MMonthDec          "Dec"
#define   MRARTitle1         "\nUsage:     rar <command> -<switch 1> -<switch N> <archive> <files...>"
#define   MUNRARTitle1       "\nUsage:     unrar <command> -<switch 1> -<switch N> <archive> <files...>"
#define   MRARTitle2         "\n               <@listfiles...> <path_to_extract\\>"
#define   MCHelpCmd          "\n\n<Commands>"
#define   MCHelpCmdA         "\n  a             Add files to archive"
#define   MCHelpCmdC         "\n  c             Add archive comment"
#define   MCHelpCmdCF        "\n  cf            Add files comment"
#define   MCHelpCmdCW        "\n  cw            Write archive comment to file"
#define   MCHelpCmdD         "\n  d             Delete files from archive"
#define   MCHelpCmdE         "\n  e             Extract files to current directory"
#define   MCHelpCmdF         "\n  f             Freshen files in archive"
#define   MCHelpCmdI         "\n  i[par]=<str>  Find string in archives"
#define   MCHelpCmdK         "\n  k             Lock archive"
#define   MCHelpCmdL         "\n  l[t]          List archive [technical]"
#define   MCHelpCmdM         "\n  m[f]          Move to archive [files only]"
#define   MCHelpCmdP         "\n  p             Print file to stdout"
#define   MCHelpCmdR         "\n  r             Repair archive"
#define   MCHelpCmdRC        "\n  rc            Reconstruct missing volumes"
#define   MCHelpCmdRR        "\n  rr[N]         Add data recovery record"
#define   MCHelpCmdRV        "\n  rv[N]         Create recovery volumes"
#define   MCHelpCmdS         "\n  s[name|-]     Convert archive to or from SFX"
#define   MCHelpCmdT         "\n  t             Test archive files"
#define   MCHelpCmdU         "\n  u             Update files in archive"
#define   MCHelpCmdV         "\n  v[t]          Verbosely list archive [technical]"
#define   MCHelpCmdX         "\n  x             Extract files with full path"
#define   MCHelpSw           "\n\n<Switches>"
#define   MCHelpSwm          "\n  -             Stop switches scanning"
#define   MCHelpSwAC         "\n  ac            Clear Archive attribute after compression or extraction"
#define   MCHelpSwAD         "\n  ad            Append archive name to destination path"
#define   MCHelpSwAG         "\n  ag[format]    Generate archive name using the current date"
#define   MCHelpSwAO         "\n  ao            Add files with Archive attribute set"
#define   MCHelpSwAP         "\n  ap<path>      Set path inside archive"
#define   MCHelpSwAS         "\n  as            Synchronize archive contents"
#define   MCHelpSwAV         "\n  av            Put authenticity verification (registered versions only)"
#define   MCHelpSwAVm        "\n  av-           Disable authenticity verification check"
#define   MCHelpSwCm         "\n  c-            Disable comments show"
#define   MCHelpSwCFGm       "\n  cfg-          Disable read configuration"
#define   MCHelpSwCL         "\n  cl            Convert names to lower case"
#define   MCHelpSwCU         "\n  cu            Convert names to upper case"
#define   MCHelpSwDF         "\n  df            Delete files after archiving"
#define   MCHelpSwDH         "\n  dh            Open shared files"
#define   MCHelpSwDS         "\n  ds            Disable name sort for solid archive"
#define   MCHelpSwEa         "\n  e<attr>       Set file exclude attributes"
#define   MCHelpSwED         "\n  ed            Do not add empty directories"
#define   MCHelpSwEE         "\n  ee            Do not save and extract extended attributes"
#define   MCHelpSwEN         "\n  en            Do not put 'end of archive' block"
#define   MCHelpSwEP         "\n  ep            Exclude paths from names"
#define   MCHelpSwEP1        "\n  ep1           Exclude base directory from names"
#define   MCHelpSwEP2        "\n  ep2           Expand paths to full"
#define   MCHelpSwF          "\n  f             Freshen files"
#define   MCHelpSwHP         "\n  hp[password]  Encrypt both file data and headers"
#define   MCHelpSwIDP        "\n  idp           Disable percentage display"
#define   MCHelpSwIEML       "\n  ieml[addr]    Send archive by email"
#define   MCHelpSwIERR       "\n  ierr          Send all messages to stderr"
#define   MCHelpSwILOG       "\n  ilog[name]    Log errors to file (registered versions only)"
#define   MCHelpSwINUL       "\n  inul          Disable all messages"
#define   MCHelpSwISND       "\n  isnd          Enable sound"
#define   MCHelpSwK          "\n  k             Lock archive"
#define   MCHelpSwKB         "\n  kb            Keep broken extracted files"
#define   MCHelpSwMn         "\n  m<0..5>       Set compression level (0-store...3-default...5-maximal)"
#define   MCHelpSwMC         "\n  mc<par>       Set advanced compression parameters"
#define   MCHelpSwMD         "\n  md<size>      Dictionary size in KB (64,128,256,512,1024,2048,4096 or A-G)"
#define   MCHelpSwMS         "\n  ms[ext;ext]   Specify file types to store"
#define   MCHelpSwOp         "\n  o+            Overwrite existing files"
#define   MCHelpSwOm         "\n  o-            Do not overwrite existing files"
#define   MCHelpSwOL         "\n  ol            Save symbolic links as the link instead of the file"
#define   MCHelpSwOS         "\n  os            Save NTFS streams"
#define   MCHelpSwOW         "\n  ow            Save or restore file owner and group"
#define   MCHelpSwP          "\n  p[password]   Set password"
#define   MCHelpSwPm         "\n  p-            Do not query password"
#define   MCHelpSwR          "\n  r             Recurse subdirectories"
#define   MCHelpSwR0         "\n  r0            Recurse subdirectories for wildcard names only"
#define   MCHelpSwRI         "\n  ri<P>[:<S>]   Set priority (0-default,1-min..15-max) and sleep time in ms"
#define   MCHelpSwRR         "\n  rr[N]         Add data recovery record"
#define   MCHelpSwRV         "\n  rv[N]         Create recovery volumes"
#define   MCHelpSwS          "\n  s[<N>,v[-],e] Create solid archive"
#define   MCHelpSwSm         "\n  s-            Disable solid archiving"
#define   MCHelpSwSFX        "\n  sfx[name]     Create SFX archive"
#define   MCHelpSwT          "\n  t             Test files after archiving"
#define   MCHelpSwTK         "\n  tk            Keep original archive time"
#define   MCHelpSwTL         "\n  tl            Set archive time to latest file"
#define   MCHelpSwTN         "\n  tn<time>      Process files newer than <time>"
#define   MCHelpSwTO         "\n  to<time>      Process files older than <time>"
#define   MCHelpSwTA         "\n  ta<date>      Process files modified after <date> in YYYYMMDDHHMMSS format"
#define   MCHelpSwTB         "\n  tb<date>      Process files modified before <date> in YYYYMMDDHHMMSS format"
#define   MCHelpSwU          "\n  u             Update files"
#define   MCHelpSwV          "\n  v             Create volumes with size autodetection or list all volumes"
#define   MCHelpSwVn         "\n  v<size>[k,b]  Create volumes with size=<size>*1000 [*1024, *1]"
#define   MCHelpSwVD         "\n  vd            Erase disk contents before creating volume"
#define   MCHelpSwVER        "\n  ver[n]        File version control"
#define   MCHelpSwVN         "\n  vn            Use the old style volume naming scheme"
#define   MCHelpSwVP         "\n  vp            Pause before each volume"
#define   MCHelpSwW          "\n  w<path>       Assign work directory"
#define   MCHelpSwX          "\n  x<file>       Exclude specified file"
#define   MCHelpSwXa         "\n  x@            Read file names to exclude from stdin"
#define   MCHelpSwXal        "\n  x@<list>      Exclude files in specified list file"
#define   MCHelpSwY          "\n  y             Assume Yes on all queries"
#define   MCHelpSwZ          "\n  z<file>       Read archive comment from file"
#define   MBadArc            "\nERROR: Bad archive %s\n"
#define   MAskPsw            "Enter password (will not be echoed)"
#define   MAskPswEcho        "Enter password"
#define   MReAskPsw          "\nReenter password: "
#define   MFor               " for "
#define   MNotMatchPsw       "\nERROR: Passwords do not match\n"
#define   MErrWrite          "Write error in the file %s"
#define   MErrRead           "Read error in the file %s"
#define   MErrSeek           "Seek error in the file %s"
#define   MErrFClose         "Cannot close the file %s"
#define   MErrOutMem         "Not enough memory"
#define   MErrBrokenArc      "Corrupt archive - use 'Repair' command"
#define   MProgAborted       "Program aborted"
#define   MErrRename         "\nCannot rename %s to %s"
#define   MAbsNextVol        "\nCannot find volume %s"
#define   MBreak             "\nUser break\n"
#define   MAskCreatVol       "\nCreate next volume ?"
#define   MAskNextDisk       "\nDisk full. Insert next"
#define   MCreatVol          "\n\nCreating %sarchive %s\n"
#define   MAskNextVol        "\nInsert disk with %s"
#define   MTestVol           "\n\nTesting archive %s\n"
#define   MExtrVol           "\n\nExtracting from %s\n"
#define   MConverting        "\nConverting %s"
#define   MCvtToSFX          "\nConvert archives to SFX"
#define   MCvtFromSFX        "\nRemoving SFX module"
#define   MNotSFX            "\n%s is not SFX archive"
#define   MNotRAR            "\n%s is not RAR archive"
#define   MNotFirstVol       "\n%s is not the first volume"
#define   MCvtOldFormat      "\n%s - cannot convert to SFX archive with old format"
#define   MCannotCreate      "\nCannot create %s"
#define   MCannotOpen        "\nCannot open %s"
#define   MUnknownMeth       "\nUnknown method in %s"
#define   MVerRequired       "\nYou need RAR %d.%d to unpack it"
#define   MOk                " OK"
#define   MDone              "\nDone"
#define   MLockingArc        "\nLocking archive"
#define   MNotMdfOld         "\n\nERROR: Cannot modify old format archive"
#define   MNotMdfLock        "\n\nERROR: Locked archive"
#define   MNotMdfVol         "\n\nERROR: Cannot modify volume"
#define   MVerifyAV          "\nVerifying authenticity information ... "
#define   MFailedAV          " Failed\n"
#define   MStrAV1            "\n\nArchive %s"
#define   MStrAV2            "\ncreated at %s"
#define   MStrAV3            "\nby %s\n"
#define   MLogFailedAV       "Invalid authenticity information"
#define   MAddingAV          "\nAdding authenticity verification "
#define   MAVOldStyle        "\n\nOld style authenticity information"
#define   MPackAskReg        "\nEvaluation copy. Please register.\n"
#define   MCreateArchive     "\nCreating %sarchive %s\n"
#define   MUpdateArchive     "\nUpdating %sarchive %s\n"
#define   MAddSolid          "solid "
#define   MAddFile           "\nAdding    %-58s     "
#define   MUpdFile           "\nUpdating  %-58s     "
#define   MAddPoints         "\n...       %-58s     "
#define   MCannotUpdPswSolid "\nERROR: Cannot update solid archives with password\n"
#define   MMoveDelFiles      "\n\nDeleting files %s..."
#define   MMoveDelDirs       "and directories"
#define   MMoveDelFile       "\nDeleting %-30s"
#define   MMoveDeleted       "    deleted"
#define   MMoveNotDeleted    "    NOT DELETED"
#define   MClearAttrib       "\n\nClearing attributes..."
#define   MMoveDelDir        "\nDeleting directory %-30s"
#define   MWarErrFOpen       "\nWARNING: Cannot open %d %s"
#define   MErrOpenFiles      "files"
#define   MErrOpenFile       "file"
#define   MAddNoFiles        "\nWARNING: No files"
#define   MMdfEncrSol        "\n%s: encrypted"
#define   MCannotMdfEncrSol  "\nCannot modify solid archive containing encrypted files"
#define   MAddAnalyze        "\nAnalyzing archived files: "
#define   MRepacking         "\nRepacking archived files: "
#define   MCRCFailed         "\n%-20s - CRC failed"
#define   MExtrTest          "\n\nTesting archive %s\n"
#define   MExtracting        "\n\nExtracting from %s\n"
#define   MUseCurPsw         "\n%s - use current password ?"
#define   MCreatDir          "\nCreating    %-56s"
#define   MExtrSkipFile      "\nSkipping    %-56s"
#define   MExtrTestFile      "\nTesting     %-56s"
#define   MExtrFile          "\nExtracting  %-56s"
#define   MExtrPoints        "\n...         %-56s"
#define   MExtrErrMkDir      "\nCannot create directory %s"
#define   MExtrPrinting      "\n------ Printing %s\n\n"
#define   MEncrBadCRC        "\nEncrypted file:  CRC failed in %s (password incorrect ?)"
#define   MExtrNoFiles       "\nNo files to extract"
#define   MExtrAllOk         "\nAll OK"
#define   MExtrTotalErr      "\nTotal errors: %ld"
#define   MFileExists        "\n\n%s already exists. Overwrite it ?"
#define   MAskOverwrite      "\nOverwrite %s ?"
#define   MAskNewName        "\nEnter new name: "
#define   MLogMainHead       "\nThe archive header is corrupt"
#define   MLogFileHead       "\n%s - the file header is corrupt"
#define   MLogCommHead       "\nThe comment header is corrupt\n"
#define   MLogProtectHead    "The data recovery header is corrupt"
#define   MReadStdinCmt      "\nReading comment from stdin\n"
#define   MReadCommFrom      "\nReading comment from %s"
#define   MDelComment        "\nDeleting comment from %s"
#define   MAddComment        "\nAdding comment to %s"
#define   MFCommAdd          "\nAdding file comments"
#define   MAskFComm          "\n\nReading comment for %s : %s from stdin\n"
#define   MLogCommBrk        "\nThe archive comment is corrupt"
#define   MCommAskCont       "\nPress 'Enter' to continue or 'Q' to quit:"
#define   MLogBrokFCmt       "\nThe file comment is corrupt"
#define   MAbsDestName       "\nDestination file name required"
#define   MWriteCommTo       "\nWrite comment to %s"
#define   MCommNotPres       "\nComment is not present"
#define   MDelFrom           "\nDeleting from %s"
#define   MDeleting          "\nDeleting %s"
#define   MEraseArc          "\nErasing empty archive %s"
#define   MNoDelFiles        "\nNo files to delete"
#define   MLogTitle          "\n\n--------  %2d %s %d, archive %s\n"
#define   MPathTooLong       "\nERROR: Path too long\n"
#define   MListSolid         "Solid "
#define   MListSFX           "SFX "
#define   MListVol1          "volume"
#define   MListVol2          "Volume"
#define   MListArc1          "archive"
#define   MListArc2          "Archive"
#define   MListRecRec        "\nRecovery record is present\n"
#define   MListLock          "\nLock is present\n"
#define   MListPathComm      "\nPathname/Comment\n            "
#define   MListName          "\n Name       "
#define   MListTitle         "      Size   Packed Ratio  Date   Time     Attr      CRC   Meth Ver\n"
#define   MListTechTitle     "               Host OS    Solid   Old\n"
#define   MListEAHead        "\n   OS/2 extended attributes"
#define   MListUOHead        "\n   Unix Owner/Group data:    %-14s %-14s"
#define   MListBeEAHead      "\n   BeOS extended attributes"
#define   MListNTACLHead     "\n   NTFS security data"
#define   MListStrmHead      "\n   NTFS stream: %s"
#define   MListUnkHead       "\n   Unknown subheader type: 0x%04x"
#define   MFileComment       "\nComment: "
#define   MYes               "Yes"
#define   MNo                "No"
#define   MListNoFiles       "  0 files\n"
#define   MRprReconstr       "\nReconstructing %s"
#define   MRprBuild          "\nBuilding %s"
#define   MRprOldFormat      "\nCannot repair archive with old format"
#define   MRprFind           "\nFound  %s"
#define   MRprAskIsSol       "\nThe archive header is corrupt. Mark archive as solid ?"
#define   MRprNoFiles        "\nNo files found"
#define   MRprSuspEntry      "\n\nSuspicious entry %s"
#define   MRprDir            "\nDirectory"
#define   MRprSuspSize       "\nSize %ld  Packed %ld"
#define   MRprSuspAdd        "\nAdd it to archive ?"
#define   MLogUnexpEOF       "\nUnexpected end of archive"
#define   MRepAskReconst     "\nReconstruct archive structure ?"
#define   MRecScanning       "\nScanning..."
#define   MRecRNotFound      "\nData recovery record not found"
#define   MRecRFound         "\nData recovery record found"
#define   MRecSecDamage      "\nSector %ld (offsets %lX...%lX) damaged"
#define   MRecCorrected      " - data recovered"
#define   MRecFailed         " - cannot recover data"
#define   MAddRecRec         "\nAdding data recovery record"
#define   MEraseForVolume    "\n\nErasing contents of drive %c:\n"
#define   MGetOwnersError    "\nWARNING: Cannot get %s owner and group\n"
#define   MErrGetOwnerID     "\nWARNING: Cannot get owner %s ID\n"
#define   MErrGetGroupID     "\nWARNING: Cannot get group %s ID\n"
#define   MOwnersBroken      "\nERROR: %s group and owner data are corrupt\n"
#define   MSetOwnersError    "\nWARNING: Cannot set %s owner and group\n"
#define   MErrLnkRead        "\nWARNING: Cannot read symbolic link %s"
#define   MErrCreateLnk      "\nWARNING: Cannot create link %s"
#define   MSymLinkExists     "\nWARNING: Symbolic link %s already exists"
#define   MAskRetryCreate    "\nCannot create %s. Retry ?"
#define   MListMACHead1      "\n MacOS file type:  %c%c%c%c  ; "
#define   MListMACHead2      "file creator:  %c%c%c%c\n"
#define   MDataBadCRC        "\n%-20s : packed data CRC failed in volume %s"
#define   MFileRO            "\n%s is read-only"
#define   MACLGetError       "\nWARNING: Cannot get %s security data\n"
#define   MACLSetError       "\nWARNING: Cannot set %s security data\n"
#define   MACLBroken         "\nERROR: %s security data are corrupt\n"
#define   MACLUnknown        "\nWARNING: Unknown format of %s security data\n"
#define   MStreamBroken      "\nERROR: %s stream data are corrupt\n"
#define   MStreamUnknown     "\nWARNING: Unknown format of %s stream data\n"
#define   MInvalidName       "\nERROR: Invalid file name %s"
#define   MEABroken          "\nERROR: %s extended attributes are corrupt\n"
#define   MEAUnknHeader      "\nWARNING: %s - unknown format of extended attributes\n"
#define   MCannotSetEA       "\nWARNING: cannot set extended attributes to %s\n"
#define   MCannotGetEA       "\nERROR: Cannot get extended attributes of %s\n"
#define   MShowEA            " (+EA)"
#define   MSkipEA            "\n...skipping extended attributes"
#define   MProcessArc        "\n\nProcessing archive %s"
#define   MSyncScanError     "\nFile search errors, cannot synchronize archive"
#define   MCorrectingName    "\nWARNING: Attempting to correct the invalid file name"
#define   MUnpCannotMerge    "\nWARNING: You need to start extraction from a previous volume to unpack %s"
#define   MUnknownOption     "\nERROR: Unknown option: %s"
#define   MSubHeadCorrupt    "\nERROR: Corrupt data header found, ignored"
#define   MSubHeadUnknown    "\nWARNING: Unknown data header format, ignored"
#define   MSubHeadDataCRC    "\nERROR: Corrupt %s data block"
#define   MSubHeadType       "\nData header type: %s"
#define   MScanError         "\nCannot read contents of %s"
#define   MNotVolume         "\n%s is not volume"
#define   MRecVolDiffSets    "\nERROR: %s and %s belong to different sets"
#define   MRecVolMissing     "\n%d volumes missing"
#define   MRecVolFound       "\n%d recovery volumes found"
#define   MRecVolAllExist    "\nNothing to reconstruct"
#define   MRecVolCannotFix   "\nReconstruction impossible"
#define   MReconstructing    "\nReconstructing..."
#define   MCreating          "\nCreating %s"
#define   MRenaming          "\nRenaming %s to %s"
#define   MNTFSRequired      "\nWrite error: only NTFS file system supports files larger than 4 GB"
#define   MErrChangeAttr     "\nWARNING: Cannot change attributes of %s"
#define   MWrongSFXVer       "\nERROR: default SFX module does not support RAR %d.%d archives"
#define   MCannotEncName     "\nCannot encrypt archive already contained encrypted files"
#define   MCannotEmail       "\nCannot email the file %s"
#define   MCopyrightS        "\nRAR SFX archive"
#define   MSHelpCmd          "\n\n<Commands>"
#define   MSHelpCmdE         "\n  -x      Extract from archive (default)"
#define   MSHelpCmdT         "\n  -t      Test archive files"
#define   MSHelpCmdV         "\n  -v      Verbosely list contents of archive"

/***** File: int64.hpp *****/


#if defined(__GNUC__)
#define NATIVE_INT64
typedef long long Int64;
#endif

#ifdef NATIVE_INT64

#define int64to32(x) ((uint)(x))
#define int32to64(high,low) ((((Int64)(high))<<32)+(low))
#define is64plus(x) (x>=0)

#else

class Int64 {
public:
	Int64();
	Int64(uint n);
	Int64(uint HighPart, uint LowPart);

//    Int64 operator = (Int64 n);
	Int64 operator << (int n);
	Int64 operator >> (int n);

	friend Int64 operator / (Int64 n1, Int64 n2);
	friend Int64 operator * (Int64 n1, Int64 n2);
	friend Int64 operator % (Int64 n1, Int64 n2);
	friend Int64 operator + (Int64 n1, Int64 n2);
	friend Int64 operator - (Int64 n1, Int64 n2);
	friend Int64 operator += (Int64 &n1, Int64 n2);
	friend Int64 operator -= (Int64 &n1, Int64 n2);
	friend Int64 operator *= (Int64 &n1, Int64 n2);
	friend Int64 operator /= (Int64 &n1, Int64 n2);
	friend Int64 operator | (Int64 n1, Int64 n2);
	inline friend void operator -= (Int64 &n1, unsigned int n2) {
		if (n1.LowPart < n2)
			n1.HighPart--;
		n1.LowPart -= n2;
	}
	inline friend void operator ++ (Int64 &n) {
		if (++n.LowPart == 0)
			++n.HighPart;
	}
	inline friend void operator -- (Int64 &n) {
		if (n.LowPart-- == 0)
			n.HighPart--;
	}
	friend bool operator == (Int64 n1, Int64 n2);
	friend bool operator > (Int64 n1, Int64 n2);
	friend bool operator < (Int64 n1, Int64 n2);
	friend bool operator != (Int64 n1, Int64 n2);
	friend bool operator >= (Int64 n1, Int64 n2);
	friend bool operator <= (Int64 n1, Int64 n2);

	void Set(uint HighPart, uint LowPart);
	uint GetLowPart() {
		return (LowPart);
	}

	uint LowPart;
	uint HighPart;
};

#define int64to32(x) ((x).GetLowPart())
#define int32to64(high,low) (Int64((high),(low)))
#define is64plus(x) ((int)(x).HighPart>=0)

#endif

#define INT64ERR int32to64(0x80000000,0)

void itoa(Int64 n, char *Str);
Int64 atoil(char *Str);


/***** File: unicode.hpp *****/


#define MBFUNCTIONS

#if defined(MBFUNCTIONS )
#define UNICODE_SUPPORTED
#endif

void WideToChar(const wchar *Src, char *Dest, int DestSize = 0x10000000);
void CharToWide(const char *Src, wchar *Dest, int DestSize = 0x10000000);
byte *WideToRaw(const wchar *Src, byte *Dest, int DestSize = 0x10000000);
wchar *RawToWide(const byte *Src, wchar *Dest, int DestSize = 0x10000000);
bool LowAscii(const wchar *Str);
int strlenw(const wchar *str);
wchar *strcpyw(wchar *dest, const wchar *src);
wchar *strncpyw(wchar *dest, const wchar *src, int n);
wchar *strcatw(wchar *dest, const wchar *src);
wchar *strncatw(wchar *dest, const wchar *src, int n);
int strcmpw(const wchar *s1, const wchar *s2);
int strncmpw(const wchar *s1, const wchar *s2, int n);
int stricmpw(const wchar *s1, const wchar *s2);
int strnicmpw(const wchar *s1, const wchar *s2, int n);
wchar *strchrw(const wchar *s, int c);
wchar *strrchrw(const wchar *s, int c);
wchar *strpbrkw(const wchar *s1, const wchar *s2);
wchar *strlowerw(wchar *Str);
wchar *strupperw(wchar *Str);
int toupperw(int ch);
int atoiw(const wchar *s);

#ifdef DBCS_SUPPORTED
class SupportDBCS {
public:
	SupportDBCS();

	char *charnext(const char *s);
	char *strchrd(const char *s, int c);
	char *strrchrd(const char *s, int c);

	bool IsLeadByte[256];
	bool DBCSMode;
};

extern SupportDBCS gdbcs;

inline char *charnext(const char *s) {
	return (char *)(gdbcs.DBCSMode ? gdbcs.charnext(s) : s + 1);
}
inline char *strchrd(const char *s, int c) {
	return (char *)(gdbcs.DBCSMode ? gdbcs.strchrd(s, c) : strchr(s, c));
}
inline char *strrchrd(const char *s, int c) {
	return (char *)(gdbcs.DBCSMode ? gdbcs.strrchrd(s, c) : strrchr(s, c));
}

#else
#define charnext(s) ((s)+1)
#define strchrd strchr
#define strrchrd strrchr
#endif


/***** File: array.hpp *****/

template <class T> class Array {
private:
	T *Buffer;
	int BufSize;
	int AllocSize;
public:
	Array();
	Array(int Size);
	~Array();
	inline void CleanData();
	inline T &operator [](int Item);
	inline int Size();
	void Add(int Items);
	void Alloc(int Items);
	void Reset();
	void operator = (Array<T> &Src);
	void Push(T Item);
};


template <class T> void Array<T>::CleanData() {
	Buffer = NULL;
	BufSize = 0;
	AllocSize = 0;
}


template <class T> Array<T>::Array() {
	CleanData();
}


template <class T> Array<T>::Array(int Size) {
	Buffer = (T *)malloc(sizeof(T) * Size);
	if (Buffer == NULL && Size != 0)
		ErrHandler.MemoryError();

	AllocSize = BufSize = Size;
}


template <class T> Array<T>::~Array() {
	if (Buffer != NULL)
		free(Buffer);
}


template <class T> inline T &Array<T>::operator [](int Item) {
	return (Buffer[Item]);
}


template <class T> inline int Array<T>::Size() {
	return (BufSize);
}


template <class T> void Array<T>::Add(int Items) {
	BufSize += Items;
	if (BufSize > AllocSize) {
		int Suggested = AllocSize + AllocSize / 4 + 32;
		int NewSize = Max(BufSize, Suggested);

		Buffer = (T *)realloc(Buffer, NewSize * sizeof(T));
		if (Buffer == NULL)
			ErrHandler.MemoryError();

		AllocSize = NewSize;
	}
}


template <class T> void Array<T>::Alloc(int Items) {
	if (Items > AllocSize)
		Add(Items - BufSize);
	else
		BufSize = Items;
}


template <class T> void Array<T>::Reset() {
	if (Buffer != NULL) {
		free(Buffer);
		Buffer = NULL;
	}
	BufSize = 0;
	AllocSize = 0;
}


template <class T> void Array<T>::operator =(Array<T> &Src) {
	Reset();
	Alloc(Src.BufSize);
//  AllocSize=Src.AllocSize;
//  BufSize=Src.BufSize;
	if (Src.BufSize != 0)
		memcpy((void *)Buffer, (void *)Src.Buffer, Src.BufSize * sizeof(T));
}


template <class T> void Array<T>::Push(T Item) {
	Add(1);
	(*this)[Size() - 1] = Item;
}

/***** File: headers.hpp *****/


#define  SIZEOF_MARKHEAD         7
#define  SIZEOF_OLDMHD           7
#define  SIZEOF_NEWMHD          13
#define  SIZEOF_OLDLHD          21
#define  SIZEOF_NEWLHD          32
#define  SIZEOF_SHORTBLOCKHEAD   7
#define  SIZEOF_LONGBLOCKHEAD   11
#define  SIZEOF_SUBBLOCKHEAD    14
#define  SIZEOF_COMMHEAD        13
#define  SIZEOF_PROTECTHEAD     26
#define  SIZEOF_AVHEAD          14
#define  SIZEOF_SIGNHEAD        15
#define  SIZEOF_UOHEAD          18
#define  SIZEOF_MACHEAD         22
#define  SIZEOF_EAHEAD          24
#define  SIZEOF_BEEAHEAD        24
#define  SIZEOF_STREAMHEAD      26

#define  PACK_VER               29
#define  PACK_CRYPT_VER         29
#define  UNP_VER                29
#define  CRYPT_VER              29
#define  AV_VER                 20
#define  PROTECT_VER            20

#define  MHD_VOLUME         0x0001
#define  MHD_COMMENT        0x0002
#define  MHD_LOCK           0x0004
#define  MHD_SOLID          0x0008
#define  MHD_PACK_COMMENT   0x0010
#define  MHD_NEWNUMBERING   0x0010
#define  MHD_AV             0x0020
#define  MHD_PROTECT        0x0040
#define  MHD_PASSWORD       0x0080
#define  MHD_FIRSTVOLUME    0x0100

#define  LHD_SPLIT_BEFORE   0x0001
#define  LHD_SPLIT_AFTER    0x0002
#define  LHD_PASSWORD       0x0004
#define  LHD_COMMENT        0x0008
#define  LHD_SOLID          0x0010

#define  LHD_WINDOWMASK     0x00e0
#define  LHD_WINDOW64       0x0000
#define  LHD_WINDOW128      0x0020
#define  LHD_WINDOW256      0x0040
#define  LHD_WINDOW512      0x0060
#define  LHD_WINDOW1024     0x0080
#define  LHD_WINDOW2048     0x00a0
#define  LHD_WINDOW4096     0x00c0
#define  LHD_DIRECTORY      0x00e0

#define  LHD_LARGE          0x0100
#define  LHD_UNICODE        0x0200
#define  LHD_SALT           0x0400
#define  LHD_VERSION        0x0800
#define  LHD_EXTTIME        0x1000
#define  LHD_EXTFLAGS       0x2000

#define  SKIP_IF_UNKNOWN    0x4000
#define  LONG_BLOCK         0x8000

#define  EARC_NEXT_VOLUME   0x0001
#define  EARC_DATACRC       0x0002
#define  EARC_REVSPACE      0x0004

enum HEADER_TYPE {
	MARK_HEAD = 0x72, MAIN_HEAD = 0x73, FILE_HEAD = 0x74, COMM_HEAD = 0x75, AV_HEAD = 0x76,
	SUB_HEAD = 0x77, PROTECT_HEAD = 0x78, SIGN_HEAD = 0x79, NEWSUB_HEAD = 0x7a,
	ENDARC_HEAD = 0x7b
};

enum { EA_HEAD = 0x100, UO_HEAD = 0x101, MAC_HEAD = 0x102, BEEA_HEAD = 0x103,
       NTACL_HEAD = 0x104, STREAM_HEAD = 0x105
     };

enum { MS_DOS = 0, OS2 = 1, WIN_32 = 2, UNIX = 3, MAC_OS = 4, BEOS = 5 };

#define SUBHEAD_TYPE_CMT      "CMT"
#define SUBHEAD_TYPE_ACL      "ACL"
#define SUBHEAD_TYPE_STREAM   "STM"
#define SUBHEAD_TYPE_UOWNER   "UOW"
#define SUBHEAD_TYPE_AV       "AV"
#define SUBHEAD_TYPE_RR       "RR"
#define SUBHEAD_TYPE_OS2EA    "EA2"
#define SUBHEAD_TYPE_BEOSEA   "EABE"

#define SUBHEAD_FLAGS_INHERITED    0x80000000

#define SUBHEAD_FLAGS_CMT_UNICODE  0x00000001

struct OldMainHeader {
	byte Mark[4];
	ushort HeadSize;
	byte Flags;
};


struct OldFileHeader {
	uint PackSize;
	uint UnpSize;
	ushort FileCRC;
	ushort HeadSize;
	uint FileTime;
	byte FileAttr;
	byte Flags;
	byte UnpVer;
	byte NameSize;
	byte Method;
};


struct MarkHeader {
	byte Mark[7];
};


struct BaseBlock {
	ushort HeadCRC;
	HEADER_TYPE HeadType;//byte
	ushort Flags;
	ushort HeadSize;

	bool IsSubBlock() {
		if (HeadType == SUB_HEAD)
			return (true);
		if (HeadType == NEWSUB_HEAD && (Flags & LHD_SOLID) != 0)
			return (true);
		return (false);
	}
};

struct BlockHeader: BaseBlock {
	union {
		uint DataSize;
		uint PackSize;
	};
};


struct MainHeader: BlockHeader {
	ushort HighPosAV;
	uint PosAV;
};


#define SALT_SIZE     8

struct FileHeader: BlockHeader {
	uint UnpSize;
	byte HostOS;
	uint FileCRC;
	uint FileTime;
	byte UnpVer;
	byte Method;
	ushort NameSize;
	union {
		uint FileAttr;
		uint SubFlags;
	};
	/* optional */
	uint HighPackSize;
	uint HighUnpSize;
	/* names */
	char FileName[NM];
	wchar FileNameW[NM];
	/* optional */
	Array<byte> SubData;
	byte Salt[SALT_SIZE];
	/* dummy */
	Int64 FullPackSize;
	Int64 FullUnpSize;

	void Clear(int SubDataSize) {
		SubData.Alloc(SubDataSize);
		Flags = LONG_BLOCK;
		SubFlags = 0;
	}

	bool CmpName(const char *Name) {
		return (strcmp(FileName, Name) == 0);
	}

	FileHeader &operator = (FileHeader &hd) {
		SubData.Reset();
		memcpy(this, &hd, sizeof(*this));
		SubData.CleanData();
		SubData = hd.SubData;
		return (*this);
	}
};


struct EndArcHeader: BaseBlock {
	uint ArcDataCRC;
};


struct SubBlockHeader: BlockHeader {
	ushort SubType;
	byte Level;
};


struct CommentHeader: BaseBlock {
	ushort UnpSize;
	byte UnpVer;
	byte Method;
	ushort CommCRC;
};


struct ProtectHeader: BlockHeader {
	byte Version;
	ushort RecSectors;
	uint TotalBlocks;
	byte Mark[8];
};


struct AVHeader: BaseBlock {
	byte UnpVer;
	byte Method;
	byte AVVer;
	uint AVInfoCRC;
};


struct SignHeader: BaseBlock {
	uint CreationTime;
	ushort ArcNameSize;
	ushort UserNameSize;
};


struct UnixOwnersHeader: SubBlockHeader {
	ushort OwnerNameSize;
	ushort GroupNameSize;
	/* dummy */
	char OwnerName[NM];
	char GroupName[NM];
};


struct EAHeader: SubBlockHeader {
	uint UnpSize;
	byte UnpVer;
	byte Method;
	uint EACRC;
};


struct StreamHeader: SubBlockHeader {
	uint UnpSize;
	byte UnpVer;
	byte Method;
	uint StreamCRC;
	ushort StreamNameSize;
	/* dummy */
	byte StreamName[NM];
};


struct MacFInfoHeader: SubBlockHeader {
	uint fileType;
	uint fileCreator;
};


/***** File: rarfn.hpp *****/


void RARInitData();


/***** File: pathfn.hpp *****/


char *PointToName(const char *Path);
wchar *PointToName(const wchar *Path);
char *PointToLastChar(const char *Path);
char *ConvertPath(const char *SrcPath, char *DestPath);
wchar *ConvertPath(const wchar *SrcPath, wchar *DestPath);
void SetExt(char *Name, const char *NewExt);
void SetExt(wchar *Name, const wchar *NewExt);
void SetSFXExt(char *SFXName);
void SetSFXExt(wchar *SFXName);
char *GetExt(const char *Name);
wchar *GetExt(const wchar *Name);
bool CmpExt(const char *Name, const char *Ext);
bool IsWildcard(const char *Str, const wchar *StrW = NULL);
bool IsPathDiv(int Ch);
bool IsDriveDiv(int Ch);
int GetPathDisk(const char *Path);
void AddEndSlash(char *Path);
void AddEndSlash(wchar *Path);
void GetFilePath(const char *FullName, char *Path);
void GetFilePath(const wchar *FullName, wchar *Path);
void RemoveNameFromPath(char *Path);
void RemoveNameFromPath(wchar *Path);
void GetConfigName(const char *Name, char *FullName);
void NextVolumeName(char *ArcName, bool OldNumbering);
bool IsNameUsable(const char *Name);
void MakeNameUsable(char *Name, bool Extended);
char *UnixSlashToDos(char *SrcName, char *DestName = NULL);
char *DosSlashToUnix(char *SrcName, char *DestName = NULL);
bool IsFullPath(const char *Path);
bool IsDiskLetter(const char *Path);
void GetPathRoot(const char *Path, char *Root);
int ParseVersionFileName(char *Name, wchar *NameW, bool Truncate);
char *VolNameToFirstName(const char *VolName, char *FirstName, bool NewNumbering);
void GenerateArcName(char *ArcName, char *GenerateMask);


/***** File: strfn.hpp *****/


const char *NullToEmpty(const char *Str);
const wchar *NullToEmpty(const wchar *Str);
char *IntNameToExt(const char *Name);
void ExtToInt(const char *Src, char *Dest);
void IntToExt(const char *Src, char *Dest);
char *strlower(char *Str);
char *strupper(char *Str);
int stricomp(const char *Str1, const char *Str2);
int strnicomp(const char *Str1, const char *Str2, int N);
char *RemoveEOL(char *Str);
char *RemoveLF(char *Str);
unsigned int loctolower(unsigned int ch);
unsigned int loctoupper(unsigned int ch);


/***** File: strlist.hpp *****/

class StringList {
private:
	Array<char> StringData;
	unsigned int CurPos;

	Array<wchar> StringDataW;
	unsigned int CurPosW;

	Array<int> PosDataW;
	uint PosDataItem;

	uint StringsCount;

	uint SaveCurPos[16], SaveCurPosW[16], SavePosDataItem[16], SavePosNumber;
public:
	StringList();
	~StringList();
	void Reset();
	unsigned int AddString(const char *Str);
	unsigned int AddString(const char *Str, const wchar *StrW);
	bool GetString(char *Str, int MaxLength);
	bool GetString(char *Str, wchar *StrW, int MaxLength);
	bool GetString(char *Str, wchar *StrW, int MaxLength, int StringNum);
	char *GetString();
	bool GetString(char **Str, wchar **StrW);
	char *GetString(unsigned int StringPos);
	void Rewind();
	unsigned int ItemsCount() {
		return (StringsCount);
	};
	int GetBufferSize();
	bool Search(char *Str, wchar *StrW, bool CaseSensitive);
	void SavePosition();
	void RestorePosition();
};


/***** File: file.hpp *****/

typedef FILE *FileHandle;
#define BAD_HANDLE NULL

class RAROptions;

enum FILE_HANDLETYPE {FILE_HANDLENORMAL, FILE_HANDLESTD, FILE_HANDLEERR};

enum FILE_ERRORTYPE {FILE_SUCCESS, FILE_NOTFOUND};

struct FileStat {
	uint FileAttr;
	uint FileTime;
	Int64 FileSize;
	bool IsDir;
};


class File {
private:
	FileHandle hFile;
	bool LastWrite;
	FILE_HANDLETYPE HandleType;
	bool SkipClose;
	bool IgnoreReadErrors;
	bool OpenShared;
	bool NewFile;
	bool AllowDelete;
public:
	char FileName[NM];
	wchar FileNameW[NM];

	FILE_ERRORTYPE ErrorType;
public:
	File();
	virtual ~File();
	void operator = (File &SrcFile);
	bool Open(const char *Name, const wchar *NameW = NULL, bool OpenShared = false, bool Update = false);
	void TOpen(const char *Name, const wchar *NameW = NULL);
	bool WOpen(const char *Name, const wchar *NameW = NULL);
	bool Create(const char *Name, const wchar *NameW = NULL);
	void TCreate(const char *Name, const wchar *NameW = NULL);
	bool WCreate(const char *Name, const wchar *NameW = NULL);
	bool Close();
	void Flush();
	bool Delete();
	bool Rename(const char *NewName);
	void Write(const void *Data, int Size);
	int Read(void *Data, int Size);
	int DirectRead(void *Data, int Size);
	void Seek(Int64 Offset, int Method);
	bool RawSeek(Int64 Offset, int Method);
	Int64 Tell();
	void Prealloc(Int64 Size);
	byte GetByte();
	void PutByte(byte Byte);
	bool Truncate();
	void SetOpenFileTime(uint ft);
	void SetCloseFileTime(uint ft);
	void SetOpenFileStat(uint FileTime);
	void SetCloseFileStat(uint FileTime, uint FileAttr);
	uint GetOpenFileTime();
	bool IsOpened() {
		return (hFile != BAD_HANDLE);
	};
	Int64 FileLength();
	void SetHandleType(FILE_HANDLETYPE Type);
	FILE_HANDLETYPE GetHandleType() {
		return (HandleType);
	};
	bool IsDevice();
	void fprintf(const char *fmt, ...);
	static void RemoveCreated();
	FileHandle GetHandle() {
		return (hFile);
	};
	void SetIgnoreReadErrors(bool Mode) {
		IgnoreReadErrors = Mode;
	};
	void SetOpenShared(bool Mode) {
		OpenShared = Mode;
	};
	char *GetName() {
		return (FileName);
	}
	long Copy(File &Dest, Int64 Length = INT64ERR);
	void SetAllowDelete(bool Allow) {
		AllowDelete = Allow;
	}
};


/***** File: sha1.hpp *****/


#define HW 5

typedef struct {
	uint32 state[5];
	uint32 count[2];
	unsigned char buffer[64];
} hash_context;

void hash_initial(hash_context *c);
void hash_process(hash_context *c, unsigned char *data, unsigned len);
void hash_final(hash_context *c, uint32[HW]);

/***** File: crc.hpp *****/


extern uint CRCTab[256];

void InitCRC();
uint CRC(uint StartCRC, void *Addr, uint Size);
ushort OldCRC(ushort StartCRC, void *Addr, uint Size);


/***** File: filefn.hpp *****/

enum MKDIR_CODE {MKDIR_SUCCESS, MKDIR_ERROR, MKDIR_BADPATH};

MKDIR_CODE MakeDir(const char *Name, const wchar *NameW, uint Attr);
void CreatePath(const char *Path, const wchar *PathW, bool SkipLastName);
void SetDirTime(const char *Name, uint ft);
bool IsRemovable(const char *FileName);
Int64 GetFreeDisk(const char *FileName);
bool FileExist(const char *FileName, const wchar *FileNameW = NULL);
bool WildFileExist(const char *FileName, const wchar *FileNameW = NULL);
bool IsDir(uint Attr);
bool IsUnreadable(uint Attr);
bool IsLabel(uint Attr);
bool IsLink(uint Attr);
void SetSFXMode(const char *FileName);
void EraseDiskContents(const char *FileName);
bool IsDeleteAllowed(uint FileAttr);
void PrepareToDelete(const char *Name, const wchar *NameW);
uint GetFileAttr(const char *Name, const wchar *NameW = NULL);
bool SetFileAttr(const char *Name, const wchar *NameW, uint Attr);
void ConvertNameToFull(const char *Src, char *Dest);
void ConvertNameToFull(const wchar *Src, wchar *Dest);
char *MkTemp(char *Name);
uint CalcFileCRC(File *SrcFile, Int64 Size = INT64ERR);


/***** File: filestr.hpp *****/

bool ReadTextFile(char *Name, StringList *List, bool Config,
                  bool AbortOnError = false, bool ConvertToAnsi = false,
                  bool Unquote = false, bool SkipComments = false);

/***** File: find.hpp *****/


struct FindData {
	char Name[NM];
	wchar NameW[NM];
	Int64 Size;
	uint FileAttr;
	uint FileTime;
	int IsDir;
	bool Error;
};

class FindFile {
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
	bool Next(struct FindData *fd, bool GetSymLink = false);
	static bool FastFind(const char *FindMask, const wchar *FindMaskW, struct FindData *fd, bool GetSymLink = false);
};


/***** File: scantree.hpp *****/


enum { RECURSE_NONE = 0, RECURSE_ALWAYS, RECURSE_WILDCARDS };
enum { SCAN_SKIPDIRS = 0, SCAN_GETDIRS, SCAN_GETDIRSTWICE, SCAN_GETCURDIRS };
enum { SCAN_SUCCESS, SCAN_DONE, SCAN_ERROR, SCAN_NEXT };

#define MAXSCANDEPTH    (NM/2)

class ScanTree {
private:
	bool PrepareMasks();
	int FindProc(FindData *FindData);

	FindFile *FindStack[MAXSCANDEPTH];
	int Depth;

	StringList *FileMasks;
	int Recurse;
	bool GetLinks;
	int GetDirs;
	int Errors;

	char CurMask[NM];
	wchar CurMaskW[NM];
	bool SearchAllInRoot;
	bool FastFindFile;
	int SpecPathLength;
	int SpecPathLengthW;
public:
	ScanTree(StringList *FileMasks, int Recurse, bool GetLinks, int GetDirs);
	~ScanTree();
	int GetNext(FindData *FindData);
	int GetSpecPathLength() {
		return (SpecPathLength);
	};
	int GetSpecPathLengthW() {
		return (SpecPathLengthW);
	};
	int GetErrors() {
		return (Errors);
	};
};


/***** File: savepos.hpp *****/

class SaveFilePos {
private:
	File *SaveFile;
	Int64 SavePos;
public:
	SaveFilePos(File &SaveFile);
	~SaveFilePos();
};


/***** File: getbits.hpp *****/

class BitInput {
public:
	enum BufferSize {MAX_SIZE = 0x8000};
protected:
	int InAddr, InBit;
public:
	BitInput();
	~BitInput();

	byte *InBuf;

	void InitBitInput() {
		InAddr = InBit = 0;
	}
	void addbits(int Bits) {
		Bits += InBit;
		InAddr += Bits >> 3;
		InBit = Bits & 7;
	}
	unsigned int getbits() {
		unsigned int BitField = (uint)InBuf[InAddr] << 16;
		BitField |= (uint)InBuf[InAddr + 1] << 8;
		BitField |= (uint)InBuf[InAddr + 2];
		BitField >>= (8 - InBit);
		return (BitField & 0xffff);
	}
	void faddbits(int Bits);
	unsigned int fgetbits();
};


/***** File: rdwrfn.hpp *****/

class CmdAdd;
class Unpack;

class ComprDataIO {
private:
	void ShowPackRead(Int64 CurSize, Int64 UnpSize);
	void ShowPackWrite();
	void ShowUnpRead(Int64 ArcPos, Int64 ArcSize);
	void ShowUnpWrite();
	uint CopyUnpData(byte *Addr, uint Size);

	Unpack *SrcUnpack;

	Array<byte> RepackUnpData;
	uint RepackUnpDataStart;
	uint RepackUnpDataEnd;

	bool PackFromMemory;
	uint PackFromMemorySize;
	byte *PackFromMemoryAddr;

	bool UnpackFromMemory;
	uint UnpackFromMemorySize;
	byte *UnpackFromMemoryAddr;

	bool UnpackToMemory;
	uint UnpackToMemorySize;
	byte *UnpackToMemoryAddr;

	uint UnpWrSize;
	byte *UnpWrAddr;

	Int64 UnpPackedSize;

	bool ShowProgress;
	bool TestMode;
	bool SkipUnpCRC;

	File *SrcFile;
	File *DestFile;

	CmdAdd *Command;

	FileHeader *SubHead;
	Int64 *SubHeadPos;

	int LastPercent;

	char CurrentCommand;

public:
	ComprDataIO(CmdAdd *Command);
	void Init();
	int UnpRead(byte *Addr, uint Count);
	void UnpWrite(byte *Addr, uint Count);
	void EnableShowProgress(bool Show) {
		ShowProgress = Show;
	}
	void GetUnpackedData(byte **Data, uint *Size);
	void SetPackedSizeToRead(Int64 Size) {
		UnpPackedSize = Size;
	}
	void SetTestMode(bool Mode) {
		TestMode = Mode;
	}
	void SetSkipUnpCRC(bool Skip) {
		SkipUnpCRC = Skip;
	}
	void SetFiles(File *SrcFile, File *DestFile);
	void SetCommand(CmdAdd *Cmd) {
		Command = Cmd;
	}
	void SetSubHeader(FileHeader *hd, Int64 *Pos) {
		SubHead = hd;
		SubHeadPos = Pos;
	}
	void SetEncryption(int Method, char *Password, byte *Salt, bool Encrypt);
	void SetUnpackToMemory(byte *Addr, uint Size);
	void SetCurrentCommand(char Cmd) {
		CurrentCommand = Cmd;
	}

	bool PackVolume;
	bool UnpVolume;
	Int64 TotalPackRead;
	Int64 UnpArcSize;
	Int64 CurPackRead, CurPackWrite, CurUnpRead, CurUnpWrite;
	Int64 ProcessedArcSize, TotalArcSize;

	uint PackFileCRC, UnpFileCRC, PackedCRC;

	int Encryption;
	int Decryption;
};


/***** File: options.hpp *****/

#define DEFAULT_RECOVERY    -1

#define DEFAULT_RECVOLUMES  -10

enum PathExclMode {
	EXCL_NONE, EXCL_BASEPATH, EXCL_SKIPWHOLEPATH, EXCL_SAVEFULLPATH, EXCL_SKIPABSPATH
};
enum {SOLID_NONE = 0, SOLID_NORMAL = 1, SOLID_COUNT = 2, SOLID_FILEEXT = 4,
      SOLID_VOLUME_DEPENDENT = 8, SOLID_VOLUME_INDEPENDENT = 16
     };
enum {ARCTIME_NONE, ARCTIME_KEEP, ARCTIME_LATEST};
enum {NAMES_ORIGINALCASE, NAMES_UPPERCASE, NAMES_LOWERCASE};
enum MESSAGE_TYPE {MSG_STDOUT, MSG_STDERR, MSG_NULL};
enum OVERWRITE_MODE { OVERWRITE_ASK, OVERWRITE_ALL, OVERWRITE_NONE};

#define     MAX_FILTERS           16
enum FilterState {FILTER_DEFAULT = 0, FILTER_AUTO, FILTER_FORCE, FILTER_DISABLE};

struct FilterMode {
	FilterState State;
	int Param1;
	int Param2;
};


class RAROptions {
public:
	RAROptions();
	~RAROptions();
	void Init();

	uint ExclFileAttr;
	uint WinSize;
	char TempPath[NM];
	char SFXModule[NM];
	char ExtrPath[NM];
	char CommentFile[NM];
	char ArcPath[NM];
	char Password[MAXPASSWORD];
	bool EncryptHeaders;
	char LogName[NM];
	MESSAGE_TYPE MsgStream;
	bool Sound;
	OVERWRITE_MODE Overwrite;
	int Method;
	int Recovery;
	int RecVolNumber;
	bool DisablePercentage;
	int Solid;
	int SolidCount;
	bool ClearArc;
	bool AddArcOnly;
	bool AV;
	bool DisableComment;
	bool FreshFiles;
	bool UpdateFiles;
	PathExclMode ExclPath;
	int Recurse;
	Int64 VolSize;
	bool AllYes;
	bool DisableViewAV;
	bool DisableSortSolid;
	int ArcTime;
	int ConvertNames;
	bool ProcessOwners;
	bool SaveLinks;
	int Priority;
	int SleepTime;
	bool KeepBroken;
	bool EraseDisk;
	bool OpenShared;
	bool ExclEmptyDir;
	bool DeleteFiles;
	bool SyncFiles;
	bool GenerateArcName;
	char GenerateMask[80];
	bool ProcessEA;
	bool SaveStreams;
	uint FileTimeOlder;
	uint FileTimeNewer;
	uint FileTimeBefore;
	uint FileTimeAfter;
	bool OldNumbering;
	bool Lock;
	bool Test;
	bool VolumePause;
	FilterMode FilterModes[MAX_FILTERS];
	char EmailTo[NM];
	int VersionControl;
	bool NoEndBlock;
	bool AppendArcNameToPath;
	bool Shutdown;
};


/***** File: archive.hpp *****/

class Pack;

enum { EN_LOCK = 1, EN_VOL = 2, EN_FIRSTVOL = 4 };

class Archive: public File {
private:
	bool IsSignature(byte *D);
	void UpdateLatestTime(FileHeader *CurBlock);
	void Protect(int RecSectors);
	void ConvertNameCase(char *Name);
	void ConvertNameCase(wchar *Name);
	void ConvertUnknownHeader();
	bool AddArcComment(char *NameToShow);
	int ReadOldHeader();

#ifndef SHELL_EXT
	ComprDataIO SubDataIO;
	byte SubDataSalt[SALT_SIZE];
#endif
	RAROptions *Cmd, DummyCmd;

	MarkHeader MarkHead;
	OldMainHeader OldMhd;

	int RecoverySectors;
	Int64 RecoveryPos;

	uint LatestTime;
	int LastReadBlock;
	int CurHeaderType;

	bool SilentOpen;
public:
	Archive(RAROptions *InitCmd = NULL);
	bool IsArchive(bool EnableBroken);
	int SearchBlock(int BlockType);
	int SearchSubBlock(const char *Type);
	int ReadBlock(int BlockType);
	void WriteBlock(int BlockType, BaseBlock *wb = NULL);
	int PrepareNamesToWrite(char *Name, wchar *NameW, char *DestName, byte *DestNameW);
	void SetLhdSize();
	int ReadHeader();
	void CheckArc(bool EnableBroken);
	void CheckOpen(char *Name, wchar *NameW = NULL);
	bool WCheckOpen(char *Name, wchar *NameW = NULL);
	bool TestLock(int Mode);
	void MakeTemp();
	void CopyMainHeader(Archive &Src, bool CopySFX = true, char *NameToDisplay = NULL);
	bool ProcessToFileHead(Archive &Src, bool LastBlockAdded,
	                       Pack *Pack = NULL, const char *SkipName = NULL);
	void TmpToArc(Archive &Src);
	void CloseNew(int AdjustRecovery, bool CloseVolume);
	void WriteEndBlock(bool CloseVolume);
	void CopyFileRecord(Archive &Src);
	void CopyArchiveData(Archive &Src);
	bool GetComment(Array<byte> &CmtData);
	void ViewComment();
	void ViewFileComment();
	void SetLatestTime(uint NewTime) {
		LatestTime = NewTime;
	};
	void SeekToNext();
	bool CheckAccess();
	bool IsArcDir();
	bool IsArcLabel();
	void ConvertAttributes();
	int LhdSize();
	int LhdExtraSize();
	int GetRecoverySize(bool Required);
	void VolSubtractHeaderSize(int SubSize);
	void AddSubData(byte *SrcData, int DataSize, File *SrcFile, char *Name, bool AllowSplit);
	bool ReadSubData(Array<byte> *UnpData, File *DestFile);
	int GetHeaderType() {
		return (CurHeaderType);
	};
	void WriteCommentData(byte *Data, int DataSize, bool FileComment);
	RAROptions *GetRAROptions() {
		return (Cmd);
	}
	void SetSilentOpen(bool Mode) {
		SilentOpen = Mode;
	}

	BaseBlock ShortBlock;
	MainHeader NewMhd;
	FileHeader NewLhd;
	EndArcHeader EndArcHead;
	SubBlockHeader SubBlockHead;
	FileHeader SubHead;
	CommentHeader CommHead;
	ProtectHeader ProtectHead;
	AVHeader AVHead;
	SignHeader SignHead;
	UnixOwnersHeader UOHead;
	MacFInfoHeader MACHead;
	EAHeader EAHead;
	StreamHeader StreamHead;

	Int64 CurBlockPos;
	Int64 NextBlockPos;

	bool OldFormat;
	bool Solid;
	bool Volume;
	bool MainComment;
	bool Locked;
	bool Signed;
	bool NotFirstVolume;
	bool Protected;
	bool Encrypted;
	unsigned int SFXSize;
	bool BrokenFileHeader;

	bool Splitting;

	ushort HeaderCRC;

	Int64 VolWrite;
	Int64 AddingFilesSize;
	uint AddingHeadersSize;

	bool NewArchive;

	char FirstVolumeName[NM];
	wchar FirstVolumeNameW[NM];
};


/***** File: cmddata.hpp *****/

#define DefaultStoreList "ace;arj;bz2;cab;gz;jpeg;jpg;lha;lzh;mp3;rar;zip;taz;tgz;z"

class CommandData: public RAROptions {
private:
	void ProcessSwitchesString(char *Str);
	void ProcessSwitch(char *Switch);
	void BadSwitch(char *Switch);
	uint GetExclAttr(char *Str);

	bool FileLists;
	bool NoMoreSwitches;
	bool TimeConverted;
public:
	CommandData();
	~CommandData();
	void Init();
	void Close();
	void ParseArg(char *Arg);
	void ParseDone();
	void ParseEnvVar();
	void ReadConfig(int argc, char *argv[]);
	bool IsConfigEnabled(int argc, char *argv[]);
	void OutTitle();
	void OutHelp();
	bool IsSwitch(int Ch);
	bool ExclCheck(char *CheckName, bool CheckFullPath);
	bool StoreCheck(char *CheckName);
	bool TimeCheck(uint FileDosTime);
	bool IsProcessFile(FileHeader &NewLhd, bool *ExactMatch = NULL);
	void ProcessCommand();
	void AddArcName(char *Name, wchar *NameW);
	bool GetArcName(char *Name, wchar *NameW, int MaxSize);
	bool CheckWinSize();

	int GetRecoverySize(char *Str, int DefSize);

	char Command[NM + 16];

	char ArcName[NM];
	wchar ArcNameW[NM];

	StringList *FileArgs;
	StringList *ExclArgs;
	StringList *ArcNames;
	StringList *StoreArgs;
};


/***** File: filcreat.hpp *****/

bool FileCreate(RAROptions *Cmd, File *NewFile, char *Name, wchar *NameW,
                OVERWRITE_MODE Mode, bool *UserReject, Int64 FileSize = INT64ERR,
                uint FileTime = 0);


/***** File: consio.hpp *****/

enum {ALARM_SOUND, ERROR_SOUND, QUESTION_SOUND};

enum PASSWORD_TYPE {PASSWORD_GLOBAL, PASSWORD_FILE, PASSWORD_ARCHIVE};

void InitConsoleOptions(MESSAGE_TYPE MsgStream, bool Sound);

#ifndef SILENT
void mprintf(const char *fmt, ...);
void eprintf(const char *fmt, ...);
void Alarm();
void GetPasswordText(char *Str, int MaxLength);
unsigned int GetKey();
bool GetPassword(PASSWORD_TYPE Type, const char *FileName, char *Password, int MaxLength);
int Ask(const char *AskStr);
#endif

int KbdAnsi(char *Addr, int Size);
void OutComment(char *Comment, int Size);

#ifdef SILENT
inline void mprintf(const char *fmt, const char *a = NULL, const char *b = NULL) {}
inline void eprintf(const char *fmt, const char *a = NULL, const char *b = NULL) {}
inline void mprintf(const char *fmt, int b) {}
inline void eprintf(const char *fmt, int b) {}
inline void mprintf(const char *fmt, const char *a, int b) {}
inline void eprintf(const char *fmt, const char *a, int b) {}
inline void Alarm() {}
inline void GetPasswordText(char *Str, int MaxLength) {}
inline unsigned int GetKey() {
	return (0);
}
inline bool GetPassword(PASSWORD_TYPE Type, const char *FileName, char *Password, int MaxLength) {
	return (false);
}
inline int Ask(const char *AskStr) {
	return (0);
}
#endif


/***** File: system.hpp *****/

void InitSystemOptions(int SleepTime);
void SetPriority(int Priority);
void Wait();
bool EmailFile(char *FileName, char *MailTo);
void Shutdown();


/***** File: log.hpp *****/

void InitLogOptions(char *LogName);

#ifndef SILENT
void Log(const char *ArcName, const char *Format, ...);
#endif

#ifdef SILENT
inline void Log(const char *a, const char *b, const char *c = NULL, const char *d = NULL) {}
#endif


/***** File: rawread.hpp *****/

class RawRead {
private:
	Array<byte> Data;
	File *SrcFile;
	int DataSize;
	int ReadPos;
public:
	RawRead(File *SrcFile);
	void Read(int Size);
	void Read(byte *SrcData, int Size);
	void Get(byte &Field);
	void Get(ushort &Field);
	void Get(uint &Field);
	void Get8(Int64 &Field);
	void Get(byte *Field, int Size);
	void Get(wchar *Field, int Size);
	uint GetCRC(bool ProcessedOnly);
	int Size() {
		return DataSize;
	}
	int PaddedSize() {
		return Data.Size() - DataSize;
	}
};


/***** File: encname.hpp *****/

class EncodeFileName {
private:
	void AddFlags(int Value);

	byte *EncName;
	byte Flags;
	int FlagBits;
	int FlagsPos;
	int DestSize;
public:
	EncodeFileName();
	int Encode(char *Name, wchar *NameW, byte *EncName);
	void Decode(char *Name, byte *EncName, int EncSize, wchar *NameW, int MaxDecSize);
};


/***** File: match.hpp *****/

enum {MATCH_NAMES, MATCH_PATH, MATCH_SUBPATH, MATCH_WILDSUBPATH};

bool CmpName(char *Wildcard, char *Name, int CmpPath);
bool CmpName(wchar *Wildcard, wchar *Name, int CmpPath);

int stricompc(const char *Str1, const char *Str2);
int stricompcw(const wchar *Str1, const wchar *Str2);
int strnicompc(const char *Str1, const char *Str2, int N);
int strnicompcw(const wchar *Str1, const wchar *Str2, int N);

/***** File: timefn.hpp *****/

void InitTime();
uint SecondsToDosTime(uint Seconds);
void ConvertDate(uint ft, char *DateStr, bool FullYear);
const char *GetMonthName(int Month);
uint TextAgeToSeconds(char *TimeText);
uint IsoTextToDosTime(char *TimeText);
uint UnixTimeToDos(time_t UnixTime);
time_t DosTimeToUnix(uint DosTime);

void GetCurSysTime(struct tm *T);
bool IsLeapYear(int Year);


/***** File: compress.hpp *****/

class ComprDataIO;
class PackingFileTable;

#define CODEBUFSIZE     0x4000
#define MAXWINSIZE      0x400000
#define MAXWINMASK      (MAXWINSIZE-1)

#define LOW_DIST_REP_COUNT 16

#define NC 299  /* alphabet = {0, 1, 2, ..., NC - 1} */
#define DC  60
#define LDC 17
#define RC  28
#define HUFF_TABLE_SIZE (NC+DC+RC+LDC)
#define BC  20

#define NC20 298  /* alphabet = {0, 1, 2, ..., NC - 1} */
#define DC20 48
#define RC20 28
#define BC20 19
#define MC20 257

enum {CODE_HUFFMAN, CODE_LZ, CODE_LZ2, CODE_REPEATLZ, CODE_CACHELZ,
      CODE_STARTFILE, CODE_ENDFILE, CODE_VM, CODE_VMDATA
     };


enum FilterType {
	FILTER_NONE, FILTER_PPM /*dummy*/, FILTER_E8, FILTER_E8E9,
	FILTER_UPCASETOLOW, FILTER_AUDIO, FILTER_RGB,  FILTER_DELTA,
	FILTER_ITANIUM, FILTER_E8E9V2
};


/***** File: rarvm.hpp *****/

#define VM_STANDARDFILTERS

#ifndef SFX_MODULE
#define VM_OPTIMIZE
#endif


#define VM_MEMSIZE                  0x40000
#define VM_MEMMASK           (VM_MEMSIZE-1)
#define VM_GLOBALMEMADDR            0x3C000
#define VM_GLOBALMEMSIZE             0x2000
#define VM_FIXEDGLOBALSIZE               64

enum VM_Commands {
	VM_MOV,  VM_CMP,  VM_ADD,  VM_SUB,  VM_JZ,   VM_JNZ,  VM_INC,  VM_DEC,
	VM_JMP,  VM_XOR,  VM_AND,  VM_OR,   VM_TEST, VM_JS,   VM_JNS,  VM_JB,
	VM_JBE,  VM_JA,   VM_JAE,  VM_PUSH, VM_POP,  VM_CALL, VM_RET,  VM_NOT,
	VM_SHL,  VM_SHR,  VM_SAR,  VM_NEG,  VM_PUSHA, VM_POPA, VM_PUSHF, VM_POPF,
	VM_MOVZX, VM_MOVSX, VM_XCHG, VM_MUL,  VM_DIV,  VM_ADC,  VM_SBB,  VM_PRINT,

#ifdef VM_OPTIMIZE
	VM_MOVB, VM_MOVD, VM_CMPB, VM_CMPD,

	VM_ADDB, VM_ADDD, VM_SUBB, VM_SUBD, VM_INCB, VM_INCD, VM_DECB, VM_DECD,
	VM_NEGB, VM_NEGD,
#endif

	VM_STANDARD
};

enum VM_StandardFilters {
	VMSF_NONE, VMSF_E8, VMSF_E8E9, VMSF_ITANIUM, VMSF_RGB, VMSF_AUDIO,
	VMSF_DELTA, VMSF_UPCASE
};

enum VM_Flags {VM_FC = 1, VM_FZ = 2, VM_FS = 0x80000000};

enum VM_OpType {VM_OPREG, VM_OPINT, VM_OPREGMEM, VM_OPNONE};

struct VM_PreparedOperand {
	VM_OpType Type;
	uint Data;
	uint Base;
	uint *Addr;
};

struct VM_PreparedCommand {
	VM_Commands OpCode;
	bool ByteMode;
	VM_PreparedOperand Op1, Op2;
};


struct VM_PreparedProgram {
	VM_PreparedProgram() {
		AltCmd = NULL;
	}

	Array<VM_PreparedCommand> Cmd;
	VM_PreparedCommand *AltCmd;
	int CmdCount;

	Array<byte> GlobalData;
	Array<byte> StaticData;
	uint InitR[7];

	byte *FilteredData;
	unsigned int FilteredDataSize;
};

class RarVM: BitInput {
private:
	inline uint GetValue(bool ByteMode, uint *Addr);
	inline void SetValue(bool ByteMode, uint *Addr, uint Value);
	inline uint *GetOperand(VM_PreparedOperand *CmdOp);
	void PrintState(uint IP);
	void DecodeArg(VM_PreparedOperand &Op, bool ByteMode);
#ifdef VM_OPTIMIZE
	void Optimize(VM_PreparedProgram *Prg);
#endif
	bool ExecuteCode(VM_PreparedCommand *PreparedCode, int CodeSize);
#ifdef VM_STANDARDFILTERS
	VM_StandardFilters IsStandardFilter(byte *Code, int CodeSize);
	void ExecuteStandardFilter(VM_StandardFilters FilterType);
	unsigned int FilterItanium_GetBits(byte *Data, int BitPos, int BitCount);
	void FilterItanium_SetBits(byte *Data, unsigned int BitField, int BitPos,
	                           int BitCount);
#endif

	byte *Mem;
	uint R[8];
	uint Flags;
public:
	RarVM();
	~RarVM();
	void Init();
	void Prepare(byte *Code, int CodeSize, VM_PreparedProgram *Prg);
	void Execute(VM_PreparedProgram *Prg);
	void SetValue(uint *Addr, uint Value);
	void SetMemory(unsigned int Pos, byte *Data, unsigned int DataSize);
	static uint ReadData(BitInput &Inp);
};


/***** File: coder.hpp *****/

/****************************************************************************
 *  Contents: 'Carryless rangecoder' by Dmitry Subbotin                     *
 ****************************************************************************/

const uint TOP = 1 << 24, BOT = 1 << 15;

class RangeCoder {
public:
	void InitDecoder(Unpack *UnpackRead);
	inline int GetCurrentCount();
	inline uint GetCurrentShiftCount(uint SHIFT);
	inline void Decode();
	inline void PutChar(unsigned int c);
	inline unsigned int GetChar();

	uint low, code, range;
	struct SUBRANGE {
		uint LowCount, HighCount, scale;
	} SubRange;

	Unpack *UnpackRead;
};


/***** File: suballoc.hpp *****/


/****************************************************************************
 *  This file is part of PPMd project                                       *
 *  Written and distributed to public domain by Dmitry Shkarin 1997,        *
 *  1999-2000                                                               *
 *  Contents: interface to memory allocation routines                       *
 ****************************************************************************/

const int N1 = 4, N2 = 4, N3 = 4, N4 = (128 + 3 - 1 * N1 - 2 * N2 - 3 * N3) / 4;
const int N_INDEXES = N1 + N2 + N3 + N4;

#if defined(__GNUC__)
#define _PACK_ATTR __attribute__ ((packed))
#else
#define _PACK_ATTR
#endif /* defined(__GNUC__) */

#pragma pack(1)
struct MEM_BLK {
	ushort Stamp, NU;
	MEM_BLK *next, * prev;
	void insertAt(MEM_BLK *p) {
		next = (prev = p)->next;
		p->next = next->prev = this;
	}
	void remove() {
		prev->next = next;
		next->prev = prev;
	}
} _PACK_ATTR;
#pragma pack()

struct NODE {
	NODE *next;
};

class SubAllocator {
private:
	inline void InsertNode(void *p, int indx);
	inline void *RemoveNode(int indx);
	inline uint U2B(int NU);
	inline void SplitBlock(void *pv, int OldIndx, int NewIndx);
	uint GetUsedMemory();
	inline void GlueFreeBlocks();
	void *AllocUnitsRare(int indx);

	long SubAllocatorSize;
	byte Indx2Units[N_INDEXES], Units2Indx[128], GlueCount;
	byte *HeapStart, *LoUnit, *HiUnit;
	struct NODE FreeList[N_INDEXES];
public:
	SubAllocator();
	~SubAllocator() {
		StopSubAllocator();
	}
	void Clean();
	bool StartSubAllocator(int SASize);
	void StopSubAllocator();
	void  InitSubAllocator();
	inline void *AllocContext();
	inline void *AllocUnits(int NU);
	inline void *ExpandUnits(void *ptr, int OldNU);
	inline void *ShrinkUnits(void *ptr, int OldNU, int NewNU);
	inline void  FreeUnits(void *ptr, int OldNU);
	long GetAllocatedMemory() {
		return (SubAllocatorSize);
	};

	byte *pText, *UnitsStart, *HeapEnd, *FakeUnitsStart;
};


/***** File: model.hpp *****/

const int MAX_O = 64;                 /* maximum allowed model order */

const int INT_BITS = 7, PERIOD_BITS = 7, TOT_BITS = INT_BITS + PERIOD_BITS,
          INTERVAL = 1 << INT_BITS, BIN_SCALE = 1 << TOT_BITS, MAX_FREQ = 124;

#pragma pack(1)

struct SEE2_CONTEXT {
	// SEE-contexts for PPM-contexts with masked symbols
	ushort Summ;
	byte Shift, Count;
	void init(int InitVal) {
		Summ = InitVal << (Shift = PERIOD_BITS - 4);
		Count = 4;
	}
	uint getMean() {
		uint RetVal = SHORT16(Summ) >> Shift;
		Summ -= RetVal;
		return RetVal + (RetVal == 0);
	}
	void update() {
		if (Shift < PERIOD_BITS && --Count == 0) {
			Summ += Summ;
			Count = 3 << Shift++;
		}
	}
};


class ModelPPM;
struct PPM_CONTEXT;

struct STATE {
	byte Symbol;
	byte Freq;
	PPM_CONTEXT *Successor;
};

struct PPM_CONTEXT {
	ushort NumStats;
	union {
		struct {
			ushort SummFreq;
			STATE _PACK_ATTR *Stats;
		} U;
		STATE OneState;
	};

	PPM_CONTEXT *Suffix;
	inline void encodeBinSymbol(ModelPPM *Model, int symbol); // MaxOrder:
	inline void encodeSymbol1(ModelPPM *Model, int symbol);   //  ABCD    context
	inline void encodeSymbol2(ModelPPM *Model, int symbol);   //   BCD    suffix
	inline void decodeBinSymbol(ModelPPM *Model);  //   BCDE   successor
	inline bool decodeSymbol1(ModelPPM *Model);    // other orders:
	inline bool decodeSymbol2(ModelPPM *Model);    //   BCD    context
	inline void update1(ModelPPM *Model, STATE *p); //    CD    suffix
	inline void update2(ModelPPM *Model, STATE *p); //   BCDE   successor
	void rescale(ModelPPM *Model);
	inline PPM_CONTEXT *createChild(ModelPPM *Model, STATE *pStats, STATE &FirstState);
	inline SEE2_CONTEXT *makeEscFreq2(ModelPPM *Model, int Diff);
};
#pragma pack()

const uint UNIT_SIZE = sizeof(PPM_CONTEXT);
const uint FIXED_UNIT_SIZE = 12;

/*
inline PPM_CONTEXT::PPM_CONTEXT(STATE* pStats,PPM_CONTEXT* ShorterContext):
        NumStats(1), Suffix(ShorterContext) { pStats->Successor=this; }
inline PPM_CONTEXT::PPM_CONTEXT(): NumStats(0) {}
*/

template <class T>
inline void _PPMD_SWAP(T &t1, T &t2) {
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
}


class ModelPPM {
private:
	friend struct PPM_CONTEXT;

	_PACK_ATTR SEE2_CONTEXT SEE2Cont[25][16], DummySEE2Cont;

	struct PPM_CONTEXT *MinContext, *MedContext, *MaxContext;
	STATE *FoundState;      // found next state transition
	int NumMasked, InitEsc, OrderFall, MaxOrder, RunLength, InitRL;
	byte CharMask[256], NS2Indx[256], NS2BSIndx[256], HB2Flag[256];
	byte EscCount, PrevSuccess, HiBitsFlag;
	ushort BinSumm[128][64];               // binary SEE-contexts

	RangeCoder Coder;
	SubAllocator SubAlloc;

	void RestartModelRare();
	void StartModelRare(int MaxOrder);
	inline PPM_CONTEXT *CreateSuccessors(bool Skip, STATE *p1);

	inline void UpdateModel();
	inline void ClearMask();
public:
	ModelPPM();
	bool DecodeInit(Unpack *UnpackRead, int &EscChar);
	int DecodeChar();
};


/***** File: unpack.hpp *****/

enum BLOCK_TYPES {BLOCK_LZ, BLOCK_PPM};

struct Decode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[2];
};

struct LitDecode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[NC];
};

struct DistDecode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[DC];
};

struct LowDistDecode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[LDC];
};

struct RepDecode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[RC];
};

struct BitDecode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[BC];
};

struct UnpackFilter {
	unsigned int BlockStart;
	unsigned int BlockLength;
	unsigned int ExecCount;
	bool NextWindow;
	VM_PreparedProgram Prg;
};

/***************************** Unpack v 2.0 *********************************/
struct MultDecode {
	unsigned int MaxNum;
	unsigned int DecodeLen[16];
	unsigned int DecodePos[16];
	unsigned int DecodeNum[MC20];
};

struct AudioVariables {
	int K1, K2, K3, K4, K5;
	int D1, D2, D3, D4;
	int LastDelta;
	unsigned int Dif[11];
	unsigned int ByteCount;
	int LastChar;
};
/***************************** Unpack v 2.0 *********************************/


class Unpack: BitInput {
private:
	friend class Pack;

	void Unpack29(bool Solid);
	bool UnpReadBuf();
	void UnpWriteBuf();
	void ExecuteCode(VM_PreparedProgram *Prg);
	void UnpWriteArea(unsigned int StartPtr, unsigned int EndPtr);
	void UnpWriteData(byte *Data, int Size);
	bool ReadTables();
	void MakeDecodeTables(unsigned char *LenTab, struct Decode *Dec, int Size);
	int DecodeNumber(struct Decode *Dec);
	void CopyString();
	inline void InsertOldDist(unsigned int Distance);
	inline void InsertLastMatch(unsigned int Length, unsigned int Distance);
	void UnpInitData(int Solid);
	void CopyString(unsigned int Length, unsigned int Distance);
	bool ReadEndOfBlock();
	bool ReadVMCode();
	bool ReadVMCodePPM();
	bool AddVMCode(unsigned int FirstByte, byte *Code, int CodeSize);
	void InitFilters();

	ComprDataIO *UnpIO;
	ModelPPM PPM;
	int PPMEscChar;

	RarVM VM;
	Array<UnpackFilter *> Filters;
	Array<UnpackFilter *> PrgStack;
	Array<int> OldFilterLengths;
	int LastFilter;

	bool TablesRead;
	struct LitDecode LD;
	struct DistDecode DD;
	struct LowDistDecode LDD;
	struct RepDecode RD;
	struct BitDecode BD;

	unsigned int OldDist[4], OldDistPtr;
	unsigned int LastDist, LastLength;

	unsigned int UnpPtr, WrPtr;

	int ReadTop;

	unsigned char UnpOldTable[HUFF_TABLE_SIZE];

	int UnpBlockType;

	byte *Window;
	bool ExternalWindow;

	Int64 DestUnpSize;

	bool Suspended;
	bool UnpAllBuf;
	bool UnpSomeRead;
	Int64 WrittenFileSize;
	bool FileExtracted;

	int PrevLowDist, LowDistRepCount;

	/***************************** Unpack v 1.5 *********************************/
	void Unpack15(bool Solid);
	void ShortLZ();
	void LongLZ();
	void HuffDecode();
	void GetFlagsBuf();
	void OldUnpInitData(int Solid);
	void InitHuff();
	void CorrHuff(unsigned int *CharSet, unsigned int *NumToPlace);
	void OldCopyString(unsigned int Distance, unsigned int Length);
	unsigned int DecodeNum(int Num, unsigned int StartPos,
	                       unsigned int *DecTab, unsigned int *PosTab);
	void OldUnpWriteBuf();

	unsigned int ChSet[256], ChSetA[256], ChSetB[256], ChSetC[256];
	unsigned int Place[256], PlaceA[256], PlaceB[256], PlaceC[256];
	unsigned int NToPl[256], NToPlB[256], NToPlC[256];
	unsigned int FlagBuf, AvrPlc, AvrPlcB, AvrLn1, AvrLn2, AvrLn3;
	int Buf60, NumHuf, StMode, LCount, FlagsCnt;
	unsigned int Nhfb, Nlzb, MaxDist3;
	/***************************** Unpack v 1.5 *********************************/

	/***************************** Unpack v 2.0 *********************************/
	void Unpack20(bool Solid);
	struct MultDecode MD[4];
	unsigned char UnpOldTable20[MC20 * 4];
	int UnpAudioBlock, UnpChannels, UnpCurChannel, UnpChannelDelta;
	void CopyString20(unsigned int Length, unsigned int Distance);
	bool ReadTables20();
	void UnpInitData20(int Solid);
	void ReadLastTables();
	byte DecodeAudio(int Delta);
	struct AudioVariables AudV[4];
	/***************************** Unpack v 2.0 *********************************/

public:
	Unpack(ComprDataIO *DataIO);
	~Unpack();
	void Init(byte *Window = NULL);
	void DoUnpack(int Method, bool Solid);
	bool IsFileExtracted() {
		return (FileExtracted);
	}
	void SetDestSize(Int64 DestSize) {
		DestUnpSize = DestSize;
		FileExtracted = false;
	}
	void SetSuspended(bool Suspended) {
		Unpack::Suspended = Suspended;
	}

	unsigned int GetChar() {
		if (InAddr > BitInput::MAX_SIZE - 30)
			UnpReadBuf();
		return (InBuf[InAddr++]);
	}
};


/***** File: extinfo.hpp *****/

void SetExtraInfo(CommandData *Cmd, Archive &Arc, char *Name, wchar *NameW);
void SetExtraInfoNew(CommandData *Cmd, Archive &Arc, char *Name, wchar *NameW);


/***** File: extract.hpp *****/

enum EXTRACT_ARC_CODE {EXTRACT_ARC_NEXT, EXTRACT_ARC_REPEAT};

class CmdExtract {
private:
	ComprDataIO DataIO;
	Unpack *Unp;
	long TotalFileCount;

	long FileCount;
	long MatchedArgs;
	bool FirstFile;
	bool AllMatchesExact;
	bool ReconstructDone;

	char ArcName[NM];
	wchar ArcNameW[NM];

	char Password[MAXPASSWORD];
	bool PasswordAll;
	bool PrevExtracted;
	bool SignatureFound;
	char DestFileName[NM];
	wchar DestFileNameW[NM];
public:
	CmdExtract();
	~CmdExtract();
	void DoExtract(CommandData *Cmd);
	void ExtractArchiveInit(CommandData *Cmd, Archive &Arc);
	EXTRACT_ARC_CODE ExtractArchive(CommandData *Cmd);
	bool ExtractCurrentFile(CommandData *Cmd, Archive &Arc, int HeaderSize,
	                        bool &Repeat);
	static void UnstoreFile(ComprDataIO &DataIO, Int64 DestUnpSize);
};


/***** File: list.hpp *****/

void ListArchive(CommandData *Cmd);


/***** File: rs.hpp *****/

#define MAXPAR 255
#define MAXPOL 512

class RSCoder {
private:
	void gfInit();
	int gfMult(int a, int b);
	void pnInit();
	void pnMult(int *p1, int *p2, int *r);

	int gfExp[MAXPOL];
	int gfLog[MAXPAR + 1];

	int GXPol[MAXPOL * 2];

	int ErrorLocs[MAXPAR + 1], ErrCount;
	int Dn[MAXPAR + 1];

	int ParSize;
	int PolB[MAXPOL];
	bool FirstBlockDone;
public:
	RSCoder(int ParSize);
	void Encode(byte *Data, int DataSize, byte *DestData);
	bool Decode(byte *Data, int DataSize, int *EraLoc, int EraSize);
};

/***** File: recvol.hpp *****/

class RecVolumes {
private:
	File *SrcFile[256];
	Array<byte> Buf;
public:
	RecVolumes();
	~RecVolumes();
	void Make(RAROptions *Cmd, char *ArcName, wchar *ArcNameW);
	bool Restore(RAROptions *Cmd, const char *Name, const wchar *NameW, bool Silent);
};


/***** File: volume.hpp *****/

void SplitArchive(Archive &Arc, FileHeader *fh, Int64 *HeaderPos,
                  ComprDataIO *DataIO);
bool MergeArchive(Archive &Arc, ComprDataIO *DataIO, bool ShowFileName,
                  char Command);
void SetVolWrite(Archive &Dest, Int64 VolSize);
bool AskNextVol(char *ArcName);


/***** File: ulinks.hpp *****/

void SaveLinkData(ComprDataIO &DataIO, Archive &TempArc, FileHeader &hd,
                  char *Name);
int ExtractLink(ComprDataIO &DataIO, Archive &Arc, char *DestName,
                uint &LinkCRC, bool Create);


int ToPercent(Int64 N1, Int64 N2);
const char *St(MSGID StringId);

#endif
