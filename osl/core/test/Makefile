OSL_HOME = ../..
-include makefile.local
-include $(OSL_HOME)/makefile.local
-include $(OSL_HOME)/makefile.conf

LDFLAGS += -L../osl 
LOADLIBES += -losl_core $(BOOST_LIBS) -lboost_unit_test_framework $(LDLIBS)

PROGRAMS = testAll

MOVE_GENERATOR_SRCS = escapeMoves.t.cc capture_.t.cc addEffectWithEffect.t.cc \
 allMoves.t.cc promote_.t.cc
BOOK_SRC = bookInMemory.t.cc  compactBoard.t.cc  openingBook.t.cc
MOBILITY_SRC = bishopMobility.t.cc  lanceMobility.t.cc  rookMobility.t.cc \
 countMobility.t.cc   mobilityTable.t.cc
SRCS = $(patsubst %.cc,move_generator/%.cc,$(MOVE_GENERATOR_SRCS)) \
 $(patsubst %.cc,book/%.cc,$(BOOK_SRC)) \
 $(patsubst %.cc,mobility/%.cc,$(MOBILITY_SRC)) \
 move_classifier/check_.t.cc  move_classifier/kingOpenMove.t.cc  move_classifier/safeMove.t.cc \
 eval/openMidEndingEval.t.cc  eval/piecePair.t.cc  eval/see.t.cc \
 checkmate/immediateCheckmate.t.cc \
 board.t.cc      enterKing.t.cc  numEffectState.t.cc  player.t.cc    quadInt.t.cc \
 carray.t.cc     hashKey.t.cc    piece.t.cc           progress.t.cc  usi.t.cc \
 csa.t.cc        king8Info.t.cc  pieceMask.t.cc       psn.t.cc \
 direction.t.cc  move.t.cc       pieceStand.t.cc      ptype.t.cc \
 fixedCapacityVector.t.cc ptypeOSquareVector.t.cc

TEST_OBJS = $(patsubst %.cc,%.o,$(SRCS))
OBJS = testAll.o $(TEST_OBJS)

all : 
	$(MAKE) update-lib
	$(MAKE) test-target
test-target:
	$(MAKE) $(PROGRAMS)
update-lib:
	cd ../osl; $(MAKE)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

testAll: $(OBJS) $(FILE_OSL_CORE)
	mkdir -p `dirname $@` > /dev/null 2>&1
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

%.t: %.t.o testOne.o $(FILE_OSL_CORE)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

light-clean:
	-rm -rf .deps 
clean: light-clean
	-rm -f core *.o $(OBJS) $(PROGRAMS) nohup.out checkmate.log
