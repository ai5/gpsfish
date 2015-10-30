OSL_HOME = ../..
-include makefile.local
-include $(OSL_HOME)/makefile.local
include $(OSL_HOME)/makefile.conf

OSL_HOME_FLAGS = -DOSL_HOME=\"$(shell dirname $(shell dirname `pwd`))\"
CXXFLAGS += $(OSL_HOME_FLAGS)
INCLUDES += -I.. -I../../core

HASH_SRC = \
 hashKeyStack.cc hashRandomPair.cc
MISC_SRC = \
 base64.cc  eucToLang.cc  filePath.cc  iconvConvert.cc  sjis2euc.cc 
CHECKMATE_SRC = \
 dfpn.cc          dualDfpn.cc           pieceCost.cc         proofTreeDepthDfpn.cc \
 dfpnParallel.cc   proofNumberTable.cc checkmateIfCapture.cc fixedDepthSolverExt.cc
RECORD_SRC = \
 checkDuplicate.cc  kanjiMove.cc   kisen.cc            myshogi.cc \
 csaRecord.cc       kanjiPrint.cc        record.cc \
 kakinoki.cc        ki2.cc          usiRecord.cc
MP_SRC = feature_.cc  featureSet_.cc  moveInfo.cc  stateInfo.cc
RATING_SRC = \
 bradleyTerry.cc          feature/pattern.cc     group/king8Group.cc \
 feature.cc               feature/square.cc      group/patternGroup.cc \
 feature/bigramAttack.cc  featureSet.cc          group/squareGroup.cc \
 feature/capture.cc       group.cc               ratedMove.cc \
 feature/checkmate.cc     group/bigramGroup.cc   ratedMoveVector.cc \
 feature/countEffect2.cc  group/captureGroup.cc  ratingEnv.cc \
 feature/king8_.cc         group/escape.cc
PROGRESS_SRC = \
 effect5x3.cc  effect5x3Table.cc  effect5x3d.cc  ptypeProgress.cc
SRCS = 	$(patsubst %.cc,misc/%.cc,$(MISC_SRC)) \
 $(patsubst %.cc,hash/%.cc,$(HASH_SRC)) \
 $(patsubst %.cc,checkmate/%.cc,$(CHECKMATE_SRC)) \
 $(patsubst %.cc,record/%.cc,$(RECORD_SRC)) \
 $(patsubst %.cc,move_probability/%.cc,$(MP_SRC)) \
 $(patsubst %.cc,rating/%.cc,$(RATING_SRC)) \
 $(patsubst %.cc,progress/%.cc,$(PROGRESS_SRC)) \
 moveLogProb.cc  pathEncoding.cc  repetitionCounter.cc \
 book/miniBoard.cc  book/miniBoardChar50.cc effect_util/neighboring8Direct.cc \
 effect_util/shadowEffect.cc effect_util/sendOffSquare.cc \
 container/moveStack.cc container/square8.cc container/moveLogProbVector.cc \
 stat/histogram.cc stat/ratio.cc stat/twoDimensionalStatistics.cc \
 centering3x3.cc sennichite.cc

OBJS = $(patsubst %.cc,%.o,$(SRCS))
DEPS = $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

all: libosl_std.a

light-clean:
	-rm -rf .deps
	-rm -f *.a
clean: light-clean
	-rm $(OBJS)

-include $(DEPS)
libosl_std.a : $(OBJS) ../../core/osl/libosl_core.a
