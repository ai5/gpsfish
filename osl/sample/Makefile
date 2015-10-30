OSL_HOME = ..
-include makefile.local
-include $(OSL_HOME)/makefile.local
include $(OSL_HOME)/makefile.conf

LDFLAGS += -L../full/osl -L../std/osl -L../core/osl
LOADLIBES += -losl_full -losl_std -losl_core $(BOOST_LIBS) -lboost_regex -lboost_program_options $(LDLIBS)
INCLUDES += -I$(OSL_HOME)/full -I$(OSL_HOME)/std -I$(OSL_HOME)/core

PERFORMANCE_SRCS = isCheckPerf.cc \
  pin_perf.cc \
  progress_perf.cc rating-perf.cc see-perf.cc simpleMovePerf.cc
# commented out because their compilation take very long time: checkMovePerf.cc  fixedDepthSearchPerf.cc
EVAL_SRCS = show-state.cc show-progresseval.cc show-eval.cc  \
  openmidending-stat.cc
PPAIR_SRCS = pairdiff.cc pairedit.cc pairstat.cc 
DEBUG_SRCS = printPdp.cc printPosition.cc printMove.cc printPiece.cc \
	printPtypeO.cc 
OPENING_SRCS = convert.cc showOpening.cc opening-validator.cc openingStatistics.cc \
	compare-book.cc show-moves.cc
RATING_SRCS = ratingstat.cc probability.cc topn.cc
CHECKMATE_SRCS = dfpnstat.cc fixed-checkmate.cc  make-problems.cc
RECORD_SRCS = check-kisen.cc show-ipx.cc all-states.cc bad-moves.cc \
              show-ki2.cc show-ipx-result.cc csa-to-kisen.cc kisen-to-csa.cc \
	      count-win-loss.cc win-rate-openingbook-weight-level.cc \
              all-states2.cc find-states.cc check-ki2.cc showRecord.cc \
	      kakinoki-to-csa.cc csa-to-kifu.cc kifu-to-myshogi.cc showRecord.cc \
	      last-position.cc unique-csa.cc first-search-eval.cc \
              csajapanize.cc
MISC_SRCS = showCores.cc showMemory.cc convert.cc
LEARN_SRCS = show-sibling.cc learn-piece.cc

BOARD_SRCS = random_play.cc search1_play.cc legal_moves.cc \
	show_repetition.cc show-effect.cc \
	progress/show-state.cc progress/progress.cc\
	$(patsubst %.cc,eval/%.cc,$(EVAL_SRCS)) \
	$(patsubst %.cc,eval/ppair/%.cc,$(PPAIR_SRCS)) \
	$(patsubst %.cc,record/%.cc,$(RECORD_SRCS)) \
	$(patsubst %.cc,misc/%.cc,$(MISC_SRCS))
SEARCH_SRCS = 	sort.cc pass.cc  \
	find-losing-moves.cc find-uplevel-king.cc find-almost-entering.cc \
	quiescence/quiescencestat.cc quiescence/range-vs-nodes.cc \
	quiescence/quiescence2stat.cc \
	$(patsubst %.cc,checkmate/%.cc,$(CHECKMATE_SRCS)) \
	$(patsubst %.cc,debug/%.cc,$(DEBUG_SRCS)) \
	$(patsubst %.cc,opening/%.cc,$(OPENING_SRCS)) \
	$(patsubst %.cc,rating/%.cc,$(RATING_SRCS)) \
	annotate/annotate.cc \
	$(patsubst %.cc,performance/%.cc,$(PERFORMANCE_SRCS)) \
	$(patsubst %.cc,learn/%.cc,$(LEARN_SRCS)) \
	record/checkmate-kisen.cc

BOARD_PROGRAMS = $(BOARD_SRCS:.cc=) 
SEARCH_PROGRAMS = $(SEARCH_SRCS:.cc=) 
PROGRAMS = $(BOARD_PROGRAMS) $(SEARCH_PROGRAMS)
CC = $(CXX)

all: $(BOARD_PROGRAMS) $(SEARCH_PROGRAMS)
$(PROGRAMS): $(FILE_OSL_ALL)

# CC, LDFLAGS, LOADLIBES を定義しておくと
# %.o から % を勝手に作ってくれるので
# 特別なもの以外は書く必要がない
opening/convert: opening/convert.o opening/openingBookConverter.o
opening/openingBookConverter.o: opening/openingBookConverter.cc opening/openingBookConverter.h

checkmate/dfpnstatone-static: checkmate/dfpnstatone.cc checkmate/dfpnstat.cc
	$(CXX) -o $@ $(CXXFLAGS) $(OSL_HOME_FLAGS) $(LDFLAGS) $< $(LDLIBS) $(FILE_TCMALLOC)  $(LOADLIBES)


clean: 
	rm -f core *.o */*.o $(PROGRAMS) nohup.out *~
distclean: clean 
	rm -f *~

-include $(patsubst %.cc,.deps/%.cc.d,$(BOARD_SRCS))
-include $(patsubst %.cc,.deps/%.cc.d,$(SEARCH_SRCS))

# hash-dump.cc
# fixed-checkmate2.cc