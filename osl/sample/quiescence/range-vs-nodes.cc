/**
 * @file range-vs-nodes.cc
 * mtdf から QuiescenceSearch を呼ぶ時に良いrangeを求める.
 * root の静的評価値に近付けると，ノード数は増えるが値は正確に
 * - range-vs-nodes -s 30 ../data/kifdat/01haru*.csa
 * records 51
 * full width      19999994.000     457.325         61726.239         0.055
 * acc_center2     4094.000         300.855             0.000         1.000
 * acc_center4     2046.000         279.150             0.000         1.000
 * root_center2    4094.000         176.201         21992.548         0.576
 * root_center4    2046.000         147.995         22273.435         0.517
 * fixed_center0/2 4094.000          74.885         33217.950         0.477
 * fixed_center0/4 2046.000          58.941         33452.582         0.446
 * extend2c46      6581.951          71.833         33004.644         0.458
 * extend2cm46     6179.796          67.668         33085.086         0.451
 * fixed_center0/8 1022.000          50.712         33568.813         0.432
 * extend2c84      7760.072          79.849         30521.245         0.459
 * extend2cm84     7586.770          78.229         30568.973         0.455
 * - 
 */
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceSearch2.tcc"
#include "osl/search/simpleHashTable.h"
#include "osl/checkmate/dualDfpn.h"
#include "osl/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/eval/progressEval.h"
#include "osl/stat/average.h"
#include <forward_list>

#include <iostream>
#include <cstdio>
#include <fstream>
#include <cstdlib>

using namespace osl;
using namespace osl::search;
using namespace osl::misc;

void qsearch(const char *filename);

void usage(const char *program_name)
{
  std::cerr << program_name << " [-d depth] [-s skip] [-v] csafiles\n";
  exit(1);
}

int record_depth = -6;
bool verbose = false;
size_t skip_first = 0;
int center = 0;

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;

  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "c:d:s:vh")) != EOF)
  {
    switch(c)
    {
    case 'c':	center = atoi(optarg);
      break;
    case 'd':	record_depth = atoi(optarg);
      break;
    case 's':	skip_first = atoi(optarg);
      break;
    case 'v':	verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 1))
    usage(program_name);

  std::cerr << "using table record depth " << record_depth << "\n";
  try
  {
    for (int i=0; i<argc; ++i)
    {
      qsearch(argv[i]);
    }
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return 1;
  }
  catch (...)
  {
    throw;
  }
}

typedef SearchState2Core::checkmate_t checkmate_t;
typedef QuiescenceSearch2<eval::ProgressEval> qsearch_t;
typedef qsearch_t::eval_t eval_t;

class Searcher
{
protected:
  stat::Average width, nodes, diff, accuracy;
  qsearch_t **searcher;
  eval_t& eval;
public:
  Searcher(qsearch_t **q, eval_t& e) : searcher(q), eval(e)
  {
  }
  virtual ~Searcher()
  {
  }
  virtual const std::string name() const=0;
  virtual const std::pair<int,int> alphaBeta(Player turn,
					     int pawn_value, int real_value) const=0;
  /**
   * @return search result
   */
  virtual int search(Player turn, int pawn_value, int real_value,
		     Move last_move)=0;
protected:
  const std::pair<int,int>
  count(Player turn, int alpha, int beta, Move last_move)
  {
    width.add(abs(alpha-beta));
    alpha = eval::max(turn, alpha, FixedEval::winThreshold(alt(turn)));
    beta = eval::max(alt(turn), beta, FixedEval::winThreshold(turn));
    const int before = (*searcher)->nodeCount();
    int result;
    if (turn == BLACK)
      result = (*searcher)->search<BLACK>(alpha, beta, eval, last_move);
    else
      result = (*searcher)->search<WHITE>(alpha, beta, eval, last_move);
    const int after = (*searcher)->nodeCount();
    return std::make_pair(after - before, result);
  }
public:
  void report() const
  {
    const std::string n = name();    
    fprintf(stderr, "%s\t%8.3f\t%8.3f\t%10.3f\t%8.3f\n",
	    n.c_str(), width.average(), nodes.average(),
	    diff.average(), accuracy.average());
  };
};

class NormalSearcher : public Searcher
{
public:
  NormalSearcher(qsearch_t **q, eval_t& e) : Searcher(q,e)
  {
  }
  /**
   * @return search result
   */
  int search(Player turn, int pawn_value, int real_value,
	     Move last_move)
  {
    const std::pair<int,int> alpha_beta = alphaBeta(turn, pawn_value, real_value);
    const std::pair<int,int> node_result
      = count(turn, alpha_beta.first, alpha_beta.second, last_move);

    nodes.add(node_result.first);
    diff.add(abs(real_value - node_result.second));
    accuracy.add(real_value == node_result.second);
    return node_result.second;
  }
};

class FullWidth : public NormalSearcher
{
public:
  FullWidth(qsearch_t **q, eval_t& e) : NormalSearcher(q, e)
  {
  }
  const std::string name() const { return "full width"; }
  const std::pair<int,int> alphaBeta(Player turn, int /*pawn_value*/, int /*real_value*/) const
  {
    const int alpha = FixedEval::winThreshold(alt(turn));
    const int beta = FixedEval::winThreshold(turn);
    return std::make_pair(alpha, beta);
  }
};

/**
 * [center-half_range, center+half_range] で探索
 */
class FixedRange : public NormalSearcher
{
protected:
  int divider;
public:
  FixedRange(qsearch_t **q, eval_t& e, int d) : NormalSearcher(q,e), divider(d)
  {
  }
  virtual int center(int real_value) const=0;
  int halfRange(int pawn_value) const
  {
    return pawn_value/divider;
  }
  const std::pair<int,int> alphaBeta(Player turn, int pawn_value, int real_value) const
  {
    const int center=this->center(real_value);
    const int half_range=halfRange(pawn_value);
    const int alpha = center - half_range + eval::delta(turn);
    const int beta =  center + half_range - eval::delta(turn);
    return std::make_pair(alpha, beta);
  }
};

const std::string tos(int val)
{
  assert(val < 100);
  assert(val >= 0);
  std::string result = "00";
  sprintf(&result[0], "%d", val);
  return result;
}

class FixedCenter : public FixedRange
{
protected:
  const int center_value;
public:
  FixedCenter(qsearch_t **q, eval_t& e, int d, int c)
    : FixedRange(q, e, d), center_value(c)
  {
  }
  int center(int /*real_value*/) const
  {
    return center_value;
  }
  const std::string name() const { 
    return "fixedcenter" + tos(center_value) +"/"+ tos(divider); 
  }
};

class AccurateCenter : public FixedRange
{
public:
  AccurateCenter(qsearch_t **q, eval_t& e, int d)
    : FixedRange(q, e, d)
  {
  }
  int center(int real_value) const { return real_value; }
  const std::string name() const { 
    return "acc_center" + tos(divider); 
  }
};

class RootCenter : public FixedRange
{
public:
  RootCenter(qsearch_t **q, eval_t& e, int d)
    : FixedRange(q, e, d)
  {
  }
  int center(int /*real_value*/) const { return eval.value(); }
  const std::string name() const { 
    return "root_center" + tos(divider); 
  }
};

/**
 * [0-min_range, max(0+min_range, ev.value()-frontier_range] で探索.
 */
class ExtendToCenter : public FixedCenter
{
  const int extend_multiplier;
public:
  ExtendToCenter(qsearch_t **q, eval_t& e, int range_d, int c,
		   int extend_m) 
    : FixedCenter(q, e, range_d, c), extend_multiplier(extend_m)
  {
  }
  const std::string name() const { 
    return "extend2c" + tos(divider) + tos(extend_multiplier); 
  }
  const std::pair<int,int> alphaBeta(Player turn, int pawn_value, int real_value) const
  {
    const int root_value = eval.value();
    const int extend_range = pawn_value * extend_multiplier;
    const std::pair<int,int> alpha_beta
      = FixedCenter::alphaBeta(turn, pawn_value, real_value);
    const int delta = eval::delta(turn);
    int alpha = alpha_beta.first;
    int beta = alpha_beta.second;

    if (eval::betterThan(turn, center(real_value), root_value))
    {
      alpha = eval::min(turn, root_value+extend_range-delta, alpha);
    }
    else
    {
      beta = eval::max(turn, root_value-extend_range+delta, beta);
    }
    return std::make_pair(alpha, beta);
  }
};

/**
 * [0-min_range, max(0+min_range/2, ev.value()-frontier_range] で探索.
 * (root にはあまり近付かない)
 */
class ExtendToCenterModest : public FixedCenter
{
  const int extend_multiplier;
public:
  ExtendToCenterModest(qsearch_t **q, eval_t& e, int range_d, int c,
		       int extend_m) 
    : FixedCenter(q, e, range_d, c), extend_multiplier(extend_m)
  {
  }
  const std::string name() const { 
    return "extend2cm" + tos(divider) + tos(extend_multiplier); 
  }
  const std::pair<int,int> alphaBeta(Player turn, int pawn_value, int real_value) const
  {
    const int root_value = eval.value();
    const int extend_range = pawn_value * extend_multiplier;
    const int center=this->center(real_value);
    const int half_range=halfRange(pawn_value);
    const int delta = eval::delta(turn);

    int alpha = center - half_range - delta;
    int beta =  center + half_range + delta;
    if (eval::betterThan(turn, center, root_value))
    {
      alpha = eval::min(turn, root_value+extend_range-delta, 
			center - half_range/2 - delta);
    }
    else
    {
      beta = eval::max(turn, root_value-extend_range+delta, 
		       center + half_range/2 + delta);
    }
    return std::make_pair(alpha, beta);
  }
};

/**
 * [0-min_range*2, 0+min_range] 
 */
class ExtendToOther : public FixedCenter
{
  static const int extend_multiplier=2;
public:
  ExtendToOther(qsearch_t **q, eval_t& e, int range_d, int c) 
    : FixedCenter(q, e, range_d, c)
  {
  }
  const std::string name() const { 
    return "extend2o" + tos(divider) + tos(extend_multiplier); 
  }
  const std::pair<int,int> alphaBeta(Player turn, int pawn_value, int real_value) const
  {
    const int root_value = eval.value();
    const int center=this->center(real_value);
    const int half_range=halfRange(pawn_value);

    int alpha = center - half_range - eval::delta(turn);
    int beta =  center + half_range + eval::delta(turn);
    if (eval::betterThan(turn, center, root_value))
    {
      beta = center + half_range*extend_multiplier + eval::delta(turn);
    }
    else
    {
      alpha = center - half_range*extend_multiplier - eval::delta(turn);
    } 
    return std::make_pair(alpha, beta);
  }
};

class Analyzer
{
  size_t records;
  NumEffectState state;
  eval_t ev;
  checkmate_t checkmate;
  SimpleHashTable table;
  qsearch_t *qs;
  FullWidth full_searcher;
  typedef std::forward_list<Searcher*> list_t;
  list_t searchers;
public:
  Analyzer() : records(0), state(SimpleState(HIRATE)), ev(state),
	   table(100000,record_depth,verbose),
	   full_searcher(&qs, ev)
  {
    searchers.push_front(new AccurateCenter(&qs, ev, 2));
    searchers.push_front(new AccurateCenter(&qs, ev, 4));
    searchers.push_front(new RootCenter(&qs, ev, 2));
    searchers.push_front(new RootCenter(&qs, ev, 4));
    searchers.push_front(new RootCenter(&qs, ev, 8));
    searchers.push_front(new RootCenter(&qs, ev, 16));

    searchers.push_front(new FixedCenter(&qs, ev, 2, center));
#if 0
    searchers.push_front(new ExtendToCenter(&qs, ev, 2, center, 4));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 2, center, 4));
    searchers.push_front(new ExtendToCenter(&qs, ev, 2, center, 8));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 2, center, 8));
    searchers.push_front(new ExtendToOther(&qs, ev, 2, center));
#endif
    searchers.push_front(new FixedCenter(&qs, ev, 4, center));
#if 0
    searchers.push_front(new ExtendToCenter(&qs, ev, 4, center, 4));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 4, center, 4));
#endif
    searchers.push_front(new ExtendToCenter(&qs, ev, 4, center, 6));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 4, center, 6));
#if 0
    searchers.push_front(new ExtendToCenter(&qs, ev, 4, center, 8));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 4, center, 8));
    searchers.push_front(new ExtendToOther(&qs, ev, 4, center));
#endif
    searchers.push_front(new FixedCenter(&qs, ev, 8, center));
    searchers.push_front(new ExtendToCenter(&qs, ev, 8, center, 4));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 8, center, 4));
#if 0
    searchers.push_front(new ExtendToCenter(&qs, ev, 8, center, 6));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 8, center, 6));
    searchers.push_front(new ExtendToCenter(&qs, ev, 8, center, 8));
    searchers.push_front(new ExtendToCenterModest(&qs, ev, 8, center, 8));
    searchers.push_front(new ExtendToOther(&qs, ev, 8, center));
#endif
    searchers.reverse();
  }
  void report() const
  {
    std::cerr << "\nrecords " << records << "\n";
    full_searcher.report();
    for (list_t::const_iterator p=searchers.begin(); p!=searchers.end(); ++p)
    {
      (*p)->report();
    }
  }
  void search(size_t i, Move last_move)
  {
    SearchState2Core core(state, checkmate);
    qsearch_t searcher(core, table);
    qs = &searcher;
    const Player turn = state.turn();
    const int pawn_value
      = qsearch_t::eval_t::captureValue(newPtypeO(alt(turn),PAWN));
    if (verbose)
      std::cerr << i << " " << csa::show(last_move) << "\n";

    table.clear();
    const int real_value_dummy = 0;
    const int real_value
      = full_searcher.search(turn, pawn_value, real_value_dummy, last_move);

    for (list_t::iterator p=searchers.begin(); p!=searchers.end(); ++p)
    {
      table.clear();
      (*p)->search(turn, pawn_value, real_value, last_move);
    }
  }
  void search(const char *filename)
  {
    Record record=CsaFile(filename).load();
    state = NumEffectState(record.initialState());
    ev = eval_t(state);
    const auto moves=record.moves();
    size_t i=0;
    while (true)
    {
      if (i >= skip_first)
      {
	const Move last_move
	  = (i > 0) ? moves[i-1] : Move::PASS(alt(moves[0].player()));
	search(i, last_move);
      }
      if (i >= moves.size())
	break;
      const Move move = moves[i++];
      state.makeMove(move);
      ev.update(state, move);
    }
    ++records;
    report();
  }
};

Analyzer analyzer;

void qsearch(const char *filename)
{
  analyzer.search(filename);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
