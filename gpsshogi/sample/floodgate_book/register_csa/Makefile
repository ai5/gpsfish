GPSSHOGI_HOME = ../../..
ifdef PROFILE
RELEASE = true
endif

-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

#
all:
	$(MAKE) programs
programs: register_csa post kifupedia

ifdef PROFILE
PROF = $(PROFILE_FLAGS)
LDFLAGS += $(PROF)
endif

LOADLIBES += -lmysqlcppconn -lcurl -lboost_regex$(BOOST_POSTFIX_MT) -lboost_thread$(BOOST_POSTFIX_MT) -lboost_program_options$(BOOST_POSTFIX) -lpthread

## #gcc
OTHERFLAGS = -std=c++11 -pipe $(CPUOPTION) -I.
CXXOPTFLAGS = 
ifdef DEBUG
CXXOPTFLAGS += -O
OTHERFLAGS += -g -DDEBUG 
else
CXXOPTFLAGS += -O3
CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS)
WARNING_FLAGS += $(WARN_INLINE)
ifdef RELEASE
OTHERFLAGS +=  -DNDEBUG
endif
ifndef PROF
CXXOPTFLAGS += -fomit-frame-pointer
endif
endif

CXXFLAGS += $(PROF) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)

PROGRAM_SRCS = register_csa.cc post.cc threat_search_main.cc
SRCS = curl_util.cc tables.cc logging.cc threat_search.cc game.cc
OBJS = $(patsubst %.cc,%.o,$(SRCS))

CC = $(CXX)
PROGRAMS = $(PROGRAM_SRCS:.cc=)
OSL_HOME_FLAGS = -DOSL_HOME=\"$(shell dirname `dirname \`pwd\``)/osl\"

all: $(PROGRAMS)

register_csa: $(FILE_OSL_ALL) $(OBJS) register_csa.o

post: $(FILE_OSL_ALL) $(OBJS) post.o

kifupedia: $(FILE_OSL_ALL) $(OBJS) kifupedia.o 

threat_search_main: $(FILE_OSL_ALL) $(OBJS) threat_search_main.o

clean:
	-rm *.o $(PROGRAMS)
	-rm -rf .deps

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all clean light-clean run-profile run-profile-mp run-profile-usi update-gpsusione
