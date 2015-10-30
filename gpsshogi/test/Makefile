GPSSHOGI_HOME = ..
-include makefile.local
-include $(GPSSHOGI_HOME)/makefile.conf

INCLUDES += -I$(OSL_HOME)/test
CXXFLAGS =  -DOSL_HOME=\"$(shell dirname `pwd`)\" $(PROF) $(GCH_INCLUDES) $(INCLUDES)
LOADLIBES += -lgpsshogi -losl_search -losl_board -lcppunit -lboost_thread$(BOOST_POSTFIX) $(LDLIBS)

Q_PPAIR_OBJS = $(patsubst %.o,ppair/%.o,$(PPAIR_OBJS))
PPAIR_OBJS = pairDifferenceTest.o

Q_STAT_OBJS = $(patsubst %.o,stat/%.o,$(STAT_OBJS))
STAT_OBJS = iterativeLinearSolver.t.o valarrayMatrix.t.o sparseRegressionMultiplier.t.o

OBJS = testAll.o $(Q_PPAIR_OBJS) $(Q_STAT_OBJS)
SRCS = $(patsubst %.o,%.cc,$(OBJS))

TEST_DEPENDS = testAll.o $(GPSSHOGI_HOME)/lib/libgpsshogi.a $(FILE_OSL_ALL)

CC = $(CXX)

all: testPPair testStat

testPPair: $(Q_PPAIR_OBJS) $(TEST_DEPENDS)
	$(CXX) $(LDFLAGS) -o $@ $(Q_PPAIR_OBJS) testAll.o $(LOADLIBES) 
testStat: $(Q_STAT_OBJS)   $(TEST_DEPENDS)
	$(CXX) $(LDFLAGS) -o $@ $(Q_STAT_OBJS) testAll.o $(LOADLIBES) 
%.t: %.t.o $(TEST_DEPENDS)
	$(CXX) $(CXXFLAGS) -o $@ testAll.o $< $(LDFLAGS) $(LOADLIBES) 

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
