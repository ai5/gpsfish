GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

INCLUDES += -I.

all: libeval.a

CC = $(CXX)

OBJS = quiesce.o analyzer.o pvFile.o pvGenerator.o \
	l1Ball.o rand012.o rotateRecord.o pvVector.o loss.o \
	eval/eval.o eval/evalFactory.o eval/kingEval.o \
	eval/ptypeAttacked.o eval/piecePair.o \
	eval/progress.o eval/pin.o eval/pieceStand.o \
	eval/mobility.o eval/minorPiece.o eval/majorPiece.o \
	eval/progressFeature.o eval/openMidEnding.o \
	eval/ptypeEval.o eval/piecePairKing.o eval/progressEval.o
SRCS = $(OBJS:.o=.cc)

libeval.a : $(OBJS)
	-rm $@
	$(ARC) $@ $(OBJS)
	-$(RANLIB) $@

clean: 
	-rm -f core $(OBJS) *.o $(PROGRAMS) nohup.out *~
	-rm -rf .deps
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
