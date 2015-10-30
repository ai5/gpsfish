GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

# type make RELEASE=yes to build optimized programs
# type make DEBUG=yes to build programs for debugging

LDFLAGS += -L$(OSL_LIB_DIR) `Magick++-config --ldflags`
LOADLIBES += -lzmq -lboost_program_options$(BOOST_POSTFIX) -lboost_filesystem$(BOOST_POSTFIX) -lboost_regex$(BOOST_POSTFIX) -lboost_thread$(BOOST_POSTFIX) `Magick++-config --libs` 
CXXOPTFLAGS += `Magick++-config --cppflags`

INCLUDES += -I.

ifndef RELEASE
CXXOPTFLAGS ?= -O -DDEBUG
else
CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS) -DNDEBUG
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

PROGRAMS = graphic_rewritemap graphic_client graphic_broker \
	   sampleImageGenerator
CC = $(CXX)

.PHONY: all
all: $(PROGRAMS)

sampleImageGenerator: pos2img.o $(FILE_OSL_ALL)

graphic_rewritemap: graphic_rewritemap.o myPublisher.o

graphic_client: graphic_client.o pos2img.o mySubscriber.o myPublisher.o $(FILE_OSL_ALL)

graphic_broker: graphic_broker.o mySubscriber.o myPublisher.o

.PHONY: test
test: sampleImageGenerator
	LANG=C ./sampleImageGenerator -d images --checkmate-limit 100000 sfen.lnsgkgsnl_1r5b1_ppppppppp_9_9_9_PPPPPPPPP_1B5R1_LNSGKGSNL.b.-.png
	LANG=C ./sampleImageGenerator -d images --checkmate-limit 100000 sfen.l4g2l_1r4sk1_p3p2pp_1p3bp2_5n1N1_3P2P2_PP2PP1PP_4K2GG_LNSG2@l1@r.w.N3Pb2sp.1.moves.B*7f.png

.PHONY: clean
clean: 
	-rm -f core *.o $(PROGRAMS) gen-rb gen-cpp nohup.out *~
	-rm images/*.png

.PHONY: distclean
distclean: clean 
	rm -f *~
