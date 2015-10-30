GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local


SRCS = rating/rating.cc
PROGRAMS = $(SRCS:.cc=)

all: $(PROGRAMS)
$(PROGRAMS): $(FILE_OSL_ALL)

CC = $(CXX)

clean: 
	-rm -f core *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

