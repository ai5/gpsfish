OSL_HOME = ../..
-include makefile.local
-include $(OSL_HOME)/makefile.local
-include $(OSL_HOME)/makefile.conf

LDFLAGS += -L../osl -L../../std/osl -L../../core/osl
LOADLIBES += -losl_full -losl_std -losl_core $(BOOST_LIBS) -lboost_unit_test_framework $(LDLIBS)
INCLUDES += -I.. -I../../std -I../../core

PROGRAMS = testAll

EVAL_SRC = \
 endgame/attackDefense.t.cc endgame/attackKing.t.cc endgame/defenseKing.t.cc \
 endgame/kingPieceTable.t.cc minorPieceBonus.t.cc pieceEval.t.cc \
 ppair/piecePairIndex.t.cc ppair/piecePairPieceEval.t.cc \
 ppair/piecePairRawEval.t.cc progressEval.t.cc
SEARCH_SRC = \
 alphaBeta2.t.cc		  lRUMoves.t.cc		    quiescenceSearch.t.cc \
 bigramKillerMove.t.cc	  moveGenerator.t.cc	    sacrificeCheck.t.cc \
 breakThreatmate.t.cc	  moveStackRejections.t.cc  searchRecorder.t.cc \
 dominanceCheck.t.cc	  problems.cc		    shouldPromoteCut.t.cc \
 dualThreatmateState.t.cc  	    simpleHashRecord.t.cc \
 fixedEval.t.cc		  quiescenceGenerator.t.cc  simpleHashTable.t.cc \
 hashRejections.t.cc	  quiescenceRecord.t.cc     threatmateState.t.cc
EU_SRC = additionalEffect.t.cc  neighboring8Direct.t.cc  sendOffSquare.t.cc \
 virtualPin.t.cc effectUtil.t.cc pin.t.cc shadowEffect.t.cc
GP_SRC = alphaBetaOpenMidEndingEvalPlayer.t.cc gameState.t.cc searchPlayer.t.cc \
 alphaBetaPlayer.t.cc gnuShogiClient.t.cc weightTracer.t.cc \
 bookPlayer.t.cc historyToTable.t.cc csaClient.t.cc recordTracer.t.cc
HASH_SRC =  hashRandom.t.cc
MP_SRC = addEffect8.t.cc  attackToPinned.t.cc  safeDropMajorPiece.t.cc
MO_SRC = captureEstimation.t.cc  captureSort.t.cc  cheapPtype.t.cc  promotion.t.cc
TP_SRC = kfendPredictor.t.cc  mlPredictor.t.cc  richPredictor.t.cc  treePredictor.t.cc

SRCS = $(patsubst %.cc,eval/%.cc,$(EVAL_SRC)) \
 $(patsubst %.cc,effect_util/%.cc,$(EU_SRC)) \
 $(patsubst %.cc,game_playing/%.cc,$(GP_SRC)) \
 $(patsubst %.cc,hash/%.cc,$(HASH_SRC)) \
 $(patsubst %.cc,move_generator/%.cc,$(MP_SRC)) \
 $(patsubst %.cc,move_order/%.cc,$(MO_SRC)) \
 $(patsubst %.cc,search/%.cc,$(SEARCH_SRC)) \
 $(patsubst %.cc,threatmate/%.cc,$(TP_SRC)) \
 enter_king/simplePredictor.t.cc  stat/twoDimensionalStatistics.t.cc \
 repetitionCounter.t.cc misc/math.t.cc

TEST_OBJS = $(patsubst %.cc,%.o,$(SRCS))
OBJS = testAll.o $(TEST_OBJS)

all : 
	$(MAKE) update-lib
	$(MAKE) test-target
test-target:
	$(MAKE) $(PROGRAMS)
update-lib:
	cd ../../core/osl; $(MAKE)
	cd ../../std/osl; $(MAKE)
	cd ../osl; $(MAKE)

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

testAll: $(OBJS) $(FILE_OSL_STD) $(FILE_OSL_CORE)
	mkdir -p `dirname $@` > /dev/null 2>&1
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

%.t: %.t.o testOne.o $(FILE_OSL_STD) $(FILE_OSL_CORE)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LOADLIBES) 

light-clean:
	-rm -rf .deps 
clean: light-clean
	-rm -f core *.o $(OBJS) $(PROGRAMS) 
