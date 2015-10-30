GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

# type make RELEASE=yes to build optimized programs
# type make DEBUG=yes to build programs for debugging

LDFLAGS += -L$(OSL_LIB_DIR) `Magick++-config --ldflags`
LOADLIBES += `Magick++-config --libs` -lboost_program_options$(BOOST_POSTFIX) -lboost_filesystem$(BOOST_POSTFIX) -lboost_regex$(BOOST_POSTFIX) -lboost_thread$(BOOST_POSTFIX)
CXXOPTFLAGS += `Magick++-config --cppflags`

INCLUDES += -I.

ifndef RELEASE
CXXOPTFLAGS += -O -DDEBUG
else
CXXOPTFLAGS += $(RELEASE_CXX_OPTFLAGS) -DNDEBUG
endif

ifneq ($(CXX),icc)
## gcc
ifdef RELEASE
WARNING_FLAGS += -Werror $(WARN_INLINE)
else
OTHERFLAGS = $(DEBUGFLAGS)
endif
OTHERFLAGS += -pipe $(CPUOPTION)
ifndef PROF
CXXOPTFLAGS += -fomit-frame-pointer
endif
endif

CXXFLAGS =  -DOSL_HOME=\"$(shell dirname `pwd`)\" $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) \
	    $(GCH_INCLUDES) $(INCLUDES) $(ADDITIONALFLAGS)

PROGRAMS = sampleImageGenerator
CC = $(CXX)

.PHONY: all
all: $(PROGRAMS)

sampleImageGenerator: pos2img.o $(FILE_OSL_ALL)


.PHONY: test
test: sampleImageGenerator
	LANG=C ./sampleImageGenerator --checkmate-limit 100000 "sfen ln1g4+R/2sksb3/p2p2n1p/2p2gpp1/1+R7/2PP2P2/PP2P3P/2GSG4/LN1K3N+b w L3Psl2p 1" -o hoge.png -b "Bonanza_W3680_180_59" -w "Gekisashi_Xeon-X5365_8c" -t 2010-08-30 -l 2a1a

.PHONY: clean
clean: 
	-rm -f core *.o $(PROGRAMS) *~

.PHONY: distclean
distclean: clean 
	rm -f *~
