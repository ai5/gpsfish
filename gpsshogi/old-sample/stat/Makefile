GPSSHOGI_HOME = ../..
-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

#
all: linear_solver

LOADLIBES += -lboost_thread$(BOOST_POSTFIX) -lboost_program_options$(BOOST_POSTFIX) -lpthread

## #gcc
OTHERFLAGS = -pipe 
# ifdef DEBUG
# CXXOPTFLAGS = -O
# OTHERFLAGS += -g -DDEBUG 
# else
# CXXOPTFLAGS = $(RELEASE_CXX_OPTFLAGS)
# WARNING_FLAGS += $(WARN_INLINE)
# endif
ifdef RELEASE
OTHERFLAGS +=  -DNDEBUG
endif

OTHERFLAGS += -ffast-math

CXXFLAGS = $(CPUOPTION) $(OTHERFLAGS) $(CXXOPTFLAGS) $(WARNING_FLAGS) $(INCLUDES)

PROGRAM_SRCS = linear_solver.cc
SRCS = $(PROGRAM_SRCS) 
OBJS = $(patsubst %.cc,%.o,$(SRCS))

CC = $(CXX)
PROGRAMS = $(PROGRAM_SRCS:.cc=)

$(PROGRAMS): $(FILE_GPSSHOGI) $(FILE_OSL_ALL)

clean:
	-rm *.o $(PROGRAMS)
	-rm -rf .deps

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
