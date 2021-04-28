UNRARFILES=unrarcmd.cpp
FILES=rar.cpp

makeunrar:
	c++ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DUNRAR -w -O2 -o unrar $(FILES) $(UNRARFILES)
	strip unrar

makesfx:
	c++ -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DUNRAR -DSFX_MODULE -w -O2 -o default.sfx $(FILES)
	strip default.sfx
