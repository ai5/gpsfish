// learn.cc
// 判別関数による進行度の学習
#include "osl/progress/ptypeoSquare.h"
#include "osl/progress/ptypeRank.h"
#include "osl/stat/sparseRegressionMultiplier.h"
#include "osl/stat/diagonalPreconditioner.h"
#include "osl/stat/iterativeLinearSolver.h"
#include "osl/stat/twoDimensionalStatistics.h"
#include "osl/record/kisen.h"
#include "osl/numEffectState.h"
#include "osl/effectUtil.h"
#include <deque>
#include <valarray>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] [-g]"
       << " [-o weights filename] [-s] [-t tmp weights filename] \n"
       << " -s consider moves.size()\n"
       << " -g gekisashi simulation\n"
       << endl;
  exit(1);
}


using namespace osl;
using namespace osl::progress;
using namespace osl::stat;

typedef NumEffectState state_t;
typedef std::valarray<double> valarray_t;

template <class ProgressType>
void processRecord(osl::vector<Move> const& moves);
template <class ProgressType>
void learn(const char *output_filename, const char *tmp_filename);

bool experimental_size_consideration = false;

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  size_t maxGames = 0;
  const char *output_filename = "weights.txt";
  const char *tmp_filename = "tmp_weights.txt";
  bool use_ptype_rank = false;
  // int infinite_loop = true;

  while ((c = getopt(argc, argv, "gN:o:st:vh")) != EOF)
  {
    switch(c)
    {
    case 'g':   use_ptype_rank = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 's':   experimental_size_consideration = true;
      break;
    case 't':   tmp_filename = optarg;
      break;
    case 'o':   output_filename = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag)
    usage(program_name);

  KisenFile kisenFile("../../data/kisen/01.kif");

  if (! maxGames)
    maxGames = kisenFile.size();
  for (size_t i=0; i<maxGames; ++i)
  {
    const vector<Move> moves=kisenFile.getMoves(i);
    if (use_ptype_rank)
      processRecord<PtypeRank>(moves);
    else
      processRecord<PtypeOSquare>(moves);
  }
  if (use_ptype_rank)
    learn<PtypeRank>(output_filename, tmp_filename);
  else
    learn<PtypeOSquare>(output_filename, tmp_filename);
  return 0;
}

typedef DiscriminationInstance<4> Instance;
static std::deque<Instance> instances;
static std::deque<unsigned short> sizes;

template <class ProgressType>
void processRecord(osl::vector<Move> const& moves)
{
  NumEffectState state((SimpleState(HIRATE)));
  for (size_t i=0; i<moves.size (); ++i)
  {
    if (EffectUtil::isKingInCheck(alt(state.turn()), state))
    {
      // 自分の手番で相手の王が利きがある => 直前の手が非合法手
      std::cerr << "e"; // state;
      break;
    }

    Instance e;
    ProgressType::diff(moves[i], e);
    // std::cerr << e << "\n";
    instances.push_back(e);
    sizes.push_back(moves.size());

    state.doMove(moves[i]);
  }
}

template <class ProgressType>
struct InstanceMultiplier : public SparseRegressionMultiplier
{
  mutable size_t cur;
  InstanceMultiplier() 
    : SparseRegressionMultiplier(ProgressType::maxIndex()), cur(0)
  {
  }
  void printCurrentDate() const
  {
   struct tm *cur;
   time_t ct;
   time(&ct);
   cur=localtime(&ct);
   std::cerr << cur->tm_hour << ":" << cur->tm_min << ":" << cur->tm_sec
	     << "\n";
  }
  void newIteration() const
  {
    printCurrentDate();
  }
  bool getVectorX(size_t& num_elements, size_t *non_zero_indices, 
		  double *non_zero_values) const
  {
    num_elements = instances[cur].size;
    for (size_t i=0; i<num_elements; ++i)
    {
      non_zero_indices[i] = instances[cur].indices[i];
      assert(non_zero_indices[i] < dim());
      const double value = experimental_size_consideration 
	? 100.0/sizes[cur] : 1.0;
      non_zero_values[i]  = (instances[cur].values[i] ? value : -value);
    }
    cur = (cur+1) % instances.size();
    return cur;
  }
};


template <class ProgressType>
void learn(const char *output_filename, const char *tmp_filename)
{
  std::cerr << instances.size() << "\n";
  const size_t dim = ProgressType::maxIndex();
  valarray_t result(0.0, dim);
  valarray_t b(dim);
  valarray_t diag_inv(dim);

  InstanceMultiplier<ProgressType> prodA;

  std::cerr << "computing x^t y\n";
  DoubleConstReader y(1.0);
  prodA.computeXtY(y, &b[0], &diag_inv[0]);
  DiagonalPreconditioner preconditioner(dim);
  preconditioner.setInverseDiagonals(&diag_inv[0]);
  std::cerr << "preconditioner\n";

  IterativeLinearSolver solver(prodA, &preconditioner, 20, 0.01);
  std::cerr << "solver started ";
  int err = 0;
  int iter;
  double tol;
#if 0
  std::cerr << "using bicgstab\n";
  err = solver.solve_by_BiCGSTAB(b, result, &iter, &tol);
#else
  std::cerr << "using cg\n";
  err = solver.solve_by_CG(b, result, &iter, &tol);
#endif
  std::cerr << "tolerance achieved " << tol << "\n";
  std::ofstream os(output_filename);
  for (size_t i=0; i<dim; ++i)
    os << result[i] << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
