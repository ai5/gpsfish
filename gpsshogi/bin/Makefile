GPSSHOGI_HOME = ..

-include makefile.local
include $(GPSSHOGI_HOME)/makefile.conf

#
all:
	$(MAKE) -C $(GPSSHOGI_HOME) 
	$(MAKE) programs
programs: gpsshogi gpsusi alphabetastat 

PROGRAM_SRCS = gpsshogi.cc gpsusi.cc alphabetastat.cc brinkmate.cc
SRCS = $(PROGRAM_SRCS) 
OBJS = $(patsubst %.cc,%.o,$(SRCS))

CC = $(CXX)
PROGRAMS = $(PROGRAM_SRCS:.cc=)

gpsusi gpsshogi alphabetastat brinkmate: $(FILE_OSL_ALL)

clean:
	-rm *.o $(PROGRAMS)
	-rm -rf .deps

light-clean:
	-rm -rf .deps .objs .gch 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

.PHONY: all clean light-clean run-profile run-profile-mp run-profile-usi update-gpsusione

# gpsone: gpsone.cc gpsone.gcda gpsshogi.cc gpsoneprof
# 	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(PROFILE_USE) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC) $(LOADLIBES) -lbz2
# gpsoneprof: gpsone.cc gpsshogi.cc
# 	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(PROFILE_GENERATE) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC) $(LOADLIBES)
# gpsone-static: gpsone.cc gpsshogi.cc
# 	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC)  $(LOADLIBES) -lbz2
# 
# gpsusione: gpsusione.cc gpsusione.gcda gpsusi.cc gpsusioneprof
# 	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(PROFILE_USE) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC) $(LOADLIBES) -lbz2
# gpsusioneprof: gpsusione.cc gpsusi.cc
# 	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(PROFILE_GENERATE) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC) $(LOADLIBES)
# gpsusione-static: gpsusione.cc gpsusi.cc
# 	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC)  $(LOADLIBES) -lbz2
# 
# run-profile: gpsoneprof
# 	./gpsoneprof -e test --benchmark-more=16 -B45	
# run-profile-mp: gpsoneprof
# 	./gpsoneprof -e test --benchmark-more=16 -B45 -N1
# update-gpsone:
# 	-rm gpsoneprof gpsone gpsone.gcda 
# 	$(MAKE) gpsoneprof RELEASE=t
# 	$(MAKE) run-profile
# 	$(MAKE) gpsone RELEASE=t
# update-gpsone-mp:
# 	-rm gpsoneprof gpsone gpsone.gcda 
# 	$(MAKE) gpsoneprof RELEASE=t
# 	$(MAKE) run-profile-mp
# 	$(MAKE) gpsone RELEASE=t
# 
# run-profile-usi: gpsusioneprof
# 	./gpsusioneprof --profile --benchmark-more=16 --benchmark-seconds=45
# update-gpsusione:
# 	-rm gpsusioneprof gpsusione gpsusione.gcda 
# 	$(MAKE) gpsusioneprof RELEASE=t
# 	$(MAKE) run-profile-usi
# 	$(MAKE) gpsusione RELEASE=t
