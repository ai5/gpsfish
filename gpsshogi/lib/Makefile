GPSSHOGI_HOME = ..
include $(GPSSHOGI_HOME)/makefile.conf
-include $(GPSSHOGI_HOME)/makefile.local
-include makefile.local

CXXOPTFLAGS ?= -O
OTHERFLAGS = -pipe -Wall -g $(CPUOPTION)

CXXFLAGS = -DOSL_HOME=\"`pwd`/..\" $(CXXOPTFLAGS) $(OTHERFLAGS) $(PROF) $(INCLUDES)
all : phony-mkdir libgpsshogi.a

PPAIR_OBJS = 
PROGRESS_OBJS = 
STAT_OBJS = 
EXPERIMENTAL_OBJS = 

OBJS = 	$(patsubst %.o,stat/%.o,$(STAT_OBJS)) \
	$(patsubst %.o,ppair/%.o,$(PPAIR_OBJS)) \
	$(patsubst %.o,progress/%.o,$(PROGRESS_OBJS))
SRCS = $(OBJS:.o=.cc)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

phony-mkdir:
	-mkdir stat ppair > /dev/null 2>&1 
libgpsshogi.a : $(OBJS)
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core *.o $(OBJS) $(PROGNAME) nohup.out
	-rm -rf .deps
distclean: clean 
	rm -f *~
