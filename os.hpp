#ifndef _RAR_OS_
#define _RAR_OS_

#define FALSE 0
#define TRUE  1

#define  NM  1024

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#if defined(__FreeBSD__) || defined (__NetBSD__) || defined(__APPLE__)
  #include <sys/param.h>
  #include <sys/mount.h>
#else
  #include <sys/statfs.h>
#endif
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

typedef const char* MSGID;

#if defined(LITTLE_ENDIAN) && defined(BIG_ENDIAN)
  #if defined(BYTE_ORDER) && BYTE_ORDER == BIG_ENDIAN
    #undef LITTLE_ENDIAN
  #elif defined(BYTE_ORDER) && BYTE_ORDER == LITTLE_ENDIAN
    #undef BIG_ENDIAN
  #elif
    #error "Both LITTLE_ENDIAN and BIG_ENDIAN are defined. Undef something one"
  #endif
#endif


#endif // _RAR_OS_
