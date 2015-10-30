OSL_HOME = ../..
-include makefile.local
-include $(OSL_HOME)/makefile.local
include $(OSL_HOME)/makefile.conf

OSL_HOME_FLAGS = -DOSL_HOME=\"$(shell dirname $(shell dirname `pwd`))\"
CXXFLAGS += $(OSL_HOME_FLAGS)
INCLUDES += -I.. -I../../std -I../../core

ANNOTATE_SRC = analysesResult.cc  analyzer.cc  anno-facade.cc
EU_SRC = effectUtil.cc  pin.cc  virtualPin.cc
GP_SRC = \
 alphaBetaPlayer.cc cuiClient.cc     recordTracer.cc       usiResponse.cc \
 bookPlayer.cc    gameManager.cc    searchPlayer.cc       usiState.cc \
 computerPlayer.cc  gameState.cc     speculativeAllMoves.cc   weightTracer.cc \
 csaClient.cc    gnuShogiClient.cc   speculativeModel.cc     winCountTracer.cc \
 csaLogger.cc    historyToTable.cc   speculativeSearchPlayer.cc \
 csaTime.cc     openingBookTracer.cc timeKeeper.cc
SEARCH_SRC = \
 alphaBeta2.cc                   hashRejections.cc       searchMonitor.cc \
 alphaBeta2Parallel.cc           historyTable.cc         searchRecorder.cc \
 alphaBeta3.cc                   killerMoveTable.cc      searchState2.cc \
 alphaBeta4.cc                   moveGenerator.cc        searchTimer.cc \
 analyzer/categoryMoveVector.cc  moveScore.cc            simpleHashRecord.cc \
 analyzer/dotWriter.cc           moveStackRejections.cc  simpleHashTable.cc \
 analyzer/logWriter.cc           moveWithComment.cc      sortCaptureMoves.cc \
 analyzer/recordSet_.cc          quiescenceGenerator.cc  threatmateState.cc \
 bigramKillerMove.cc             quiescenceLog.cc        timeControl.cc \
 breakThreatmate.cc              quiescenceRecord.cc     usiReporter.cc \
 dualThreatmateState.cc          quiescenceSearch2.cc \
 limitToCheckCount.cc  usiProxy.cc interimReport.cc
EVAL_SRC = \
 endgame/attackDefense.cc endgame/attackKing.cc endgame/defenseKing.cc \
 endgame/kingPieceTable.cc pieceEval.cc ppair/piecePairEvalBase.cc \
 ppair/piecePairPieceTable.cc ppair/piecePairRawTable.cc  progressEval.cc \
 eval_mobilityTable.cc
MG_SRC = \
 addEffect8.cc  attackToPinned.cc additionalLance.cc addEffect8Table.cc

THREATMATE_SRC = kfendPredictor.cc mlPredictor.cc richPredictor.cc treePredictor.cc
SRCS = 	\
 $(patsubst %.cc,annotate/%.cc,$(ANNOTATE_SRC)) \
 $(patsubst %.cc,effect_util/%.cc,$(EU_SRC)) \
 $(patsubst %.cc,eval/%.cc,$(EVAL_SRC)) \
 $(patsubst %.cc,move_generator/%.cc,$(MG_SRC)) \
 $(patsubst %.cc,game_playing/%.cc,$(GP_SRC)) \
 $(patsubst %.cc,record/%.cc,$(RECORD_SRC)) \
 $(patsubst %.cc,search/%.cc,$(SEARCH_SRC)) \
 $(patsubst %.cc,threatmate/%.cc,$(THREATMATE_SRC)) \
 enter_king/simplePredictor.cc enter_king/enterKingUtil.cc \
 c/facade.cc container/pieceValues.cc state/historyState.cc \
 misc/characterEncodingConvertWin32.cc  misc/perfmon.cc \
 misc/log/htmlPerformanceLog.cc  misc/log/textPerformanceLog.cc \
 hash/hashRandom.cc

OBJS = $(patsubst %.cc,%.o,$(SRCS))
DEPS = $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

all: libosl_full.a

light-clean:
	-rm -rf .deps
	-rm -f *.a
clean: light-clean
	-rm $(OBJS)

-include $(DEPS)
libosl_full.a : $(OBJS)
