OSL_HOME = ../..
-include makefile.local
-include $(OSL_HOME)/makefile.local
include $(OSL_HOME)/makefile.conf

OSL_HOME_FLAGS = -DOSL_HOME=\"$(shell dirname $(shell dirname `pwd`))\"
CXXFLAGS += $(OSL_HOME_FLAGS)

EVAL_SRC = openMidEndingEval.cc \
 king8.cc       minorPiece.cc         ptypeEval.cc      eval_pieceStand.cc  weights.cc \
 kingTable.cc   mobility.cc           piecePair.cc      eval_pin.cc \
 majorPiece.cc  piecePairKing.cc  eval_progress.cc see.cc
MOBILITY_SRC = mobilityTable.cc
CHECKMATE_SRC = fixedDepthSearcher.cc immediateCheckmate.cc immediateCheckmateTable.cc \
 proofDisproof.cc
MOVE_GENERATOR_SRC = capture_.cc escape_.cc allMoves.cc drop.cc pieceOnBoard.cc \
  addEffectWithEffect.cc promote_.cc
BOOK_SRC = bookInMemory.cc  compactBoard.cc  openingBook.cc
BITS_SRC = \
 align16New.cc  boardTable.cc        mask.cc             pieceStand.cc \
 binaryIO.cc    centering5x3.cc      numSimpleEffect.cc  pieceTable.cc \
 bitXmask.cc    effectedNumTable.cc           ptypeTable.cc \
 boardMask.cc   king8Info.cc         pieceMask.cc        squareCompressor.cc \
 tables.cc
SRCS = 	$(patsubst %.cc,bits/%.cc,$(BITS_SRC)) \
 $(patsubst %.cc,book/%.cc,$(BOOK_SRC)) \
 $(patsubst %.cc,eval/%.cc,$(EVAL_SRC)) \
 $(patsubst %.cc,mobility/%.cc,$(MOBILITY_SRC)) \
 $(patsubst %.cc,checkmate/%.cc,$(CHECKMATE_SRC)) \
 $(patsubst %.cc,move_generator/%.cc,$(MOVE_GENERATOR_SRC)) \
 move_classifier/kingOpenMove.cc \
 additionalEffect.cc  enterKing.cc        numEffectState.cc  progress.cc \
 basic_type.cc        hashKey.cc          oslConfig.cc       random.cc \
 csa.cc               container.cc       simpleState.cc \
 usi.cc

OBJS = $(patsubst %.cc,%.o,$(SRCS))
DEPS = $(patsubst %.cc,.deps/%.cc.d,$(SRCS))

all: libosl_core.a

light-clean:
	-rm -rf .deps
	-rm -f *.a
clean: light-clean
	-rm $(OBJS)

-include $(DEPS)
libosl_core.a : $(OBJS)
