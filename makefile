UNRARFILES=filestr.cpp recvol.cpp rs.cpp scantree.cpp
FILES=rar.cpp strlist.cpp strfn.cpp pathfn.cpp int64.cpp savepos.cpp file.cpp filefn.cpp\
	filcreat.cpp archive.cpp arcread.cpp unicode.cpp system.cpp crc.cpp rawread.cpp\
	encname.cpp match.cpp timefn.cpp rdwrfn.cpp consio.cpp options.cpp ulinks.cpp\
	errhnd.cpp rarvm.cpp getbits.cpp sha1.cpp extinfo.cpp extract.cpp volume.cpp list.cpp\
	find.cpp unpack.cpp cmddata.cpp

makeunrar:
	c++ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DUNRAR -w -O2 -o unrar $(FILES) $(UNRARFILES)
	strip unrar

makesfx:
	c++ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DUNRAR -DSFX_MODULE -w -O2 -o default.sfx $(FILES)
	strip default.sfx
