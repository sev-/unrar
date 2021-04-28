#ifndef _RAR_RARCOMMON_
#define _RAR_RARCOMMON_

#include "errhnd.hpp"

extern ErrorHandler ErrHandler;

#include "os.hpp"


#include "version.hpp"
#include "rartypes.hpp"
#include "rardefs.hpp"
#include "rarlang.hpp"
#include "int64.hpp"
#include "unicode.hpp"
#include "errhnd.hpp"
#include "array.hpp"
#include "headers.hpp"
#include "rarfn.hpp"
#include "pathfn.hpp"
#include "strfn.hpp"
#include "strlist.hpp"
#include "file.hpp"
#include "sha1.hpp"
#include "crc.hpp"
#include "filefn.hpp"
#include "filestr.hpp"
#include "find.hpp"
#include "scantree.hpp"
#include "savepos.hpp"
#include "getbits.hpp"
#include "rdwrfn.hpp"
#include "options.hpp"
#include "archive.hpp"
#include "cmddata.hpp"
#include "filcreat.hpp"
#include "consio.hpp"
#include "system.hpp"
#include "isnt.hpp"
#include "log.hpp"
#include "rawread.hpp"
#include "encname.hpp"
#include "match.hpp"
#include "timefn.hpp"
#include "compress.hpp"


#include "rarvm.hpp"
#include "model.hpp"


#include "unpack.hpp"


#include "extinfo.hpp"
#include "extract.hpp"


#include "list.hpp"



#include "rs.hpp"
#include "recvol.hpp"
#include "volume.hpp"
#include "ulinks.hpp"


int ToPercent(Int64 N1, Int64 N2);
const char *St(MSGID StringId);

#endif
