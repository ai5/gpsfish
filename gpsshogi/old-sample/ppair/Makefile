GPSSHOGI_HOME = ../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

LOADLIBES += -lboost_thread$(BOOST_POSTFIX)

ifdef DEBUG
CXXOPTFLAGS += -O
OTHERFLAGS += -pipe -Wall $(CXXOPTFLAGS) -g -DDEBUG
else
CXXOPTFLAGS += -O2 -frename-registers -funroll-loops 
OTHERFLAGS += -pipe -Wall $(CXXOPTFLAGS) $(CPUOPTION) -fomit-frame-pointer -finline-limit=2000
ifdef RELEASE
OTHERFLAGS += -DNDEBUG
endif
endif

CXXFLAGS += -ffast-math $(PROF) $(INCLUDES) $(OTHERFLAGS)

SRCS = count.cc showDifferences.cc learn.cc quantize.cc annotate.cc \
	findQuiescence.cc weightstat.cc weightanalyzer.cc \
	learn2.cc scatter.cc akukei.cc
PROGRAMS = $(SRCS:.cc=)
LIBS = $(GPSSHOGI_LIB_DIR)/libgpsshogi.a $(FILE_OSL_ALL)

all: $(PROGRAMS)
$(PROGRAMS): $(LIBS)

CC = $(CXX)

clean: 
	rm -rf .deps
	rm -f core *.o $(PROGRAMS) nohup.out *~
distclean: clean 
	rm -f *~

count200000-20.txt:
	time ./count -m 20 -N 200000 -o $@
# weights10000-20.txt:
# 	time ./learn -m 20 -N 10000 -a count200000-20.txt -o $@
# weights10000W-20.txt:
# 	time ./learn -W -m 20 -N 10000 -a count200000-20.txt -o $@
# weights80000W-20.txt:
# 	time ./learn -W -m 20 -N 80000 -a count200000-20.txt -o $@

# weights200000Iq-20.txt:
# 	time ./learn -q annotate.dat -I -m 20 -N 200000 -a count200000-20.txt -o $@
# weights200000IHq-20.txt:
# 	time ./learn -q annotate.dat -H -I -m 20 -N 200000 -a count200000-20.txt -o $@

count200000qH.txt:  # annotate.dat
	time ./count -H -q annotate.dat -N 200000 -o $@
count200000q.txt:
	time ./count -q annotate.dat -N 200000 -o $@
count100000q.txt:
	time ./count -q annotate.dat -N 100000 -o $@
weights200000IHq.txt:
	-mkdir weights200000IHq
	time ./learn -t "weights200000IHq/tmp-" -q annotate.dat -H -I -N 200000 -a count200000qH.txt -o $@
weights200000Hq.txt:
	time ./learn -q annotate.dat -H -N 200000 -a count200000qH.txt -o $@

capturew100000.txt:
	time ./learn2 -q annotate.dat -N 100000 -a count100000q.txt -o $@
capturew100000-t40.txt:
	time ./learn2 -q annotate.dat -t tmp_weights2_t40.txt -T 40 -N 100000 -a count100000q.txt -o $@
# I: ここでは initByPieceValue
capturew200000I.txt:
	time ./learn2 -I -q annotate.dat -N 200000 -a count200000q.txt -o $@


weights%.dat: weights%.txt
	./quantize -w $< -o $@

count235000-50-150.txt:
	time ./count -m 20 -M 150 -o $@
weights235000I-50-150.txt:
	time ./learn -I -m 50 -M 150 -N 235000 -a count235000-50-150.txt -o $@
weights235000W-50-150.txt:
	time ./learn -W -m 50 -M 150 -N 235000 -a count235000-50-150.txt -o $@

annotate.dat:
	time ./annotate  -o $@
annotate-c.dat:
	time ./annotate -c -o $@

-include $(patsubst %.cc,.deps/%.cc.d,$(SRCS))
