// learn2.cc
// 多変量解析の学習
#include "osl/ppair/captureAnnotations.h"
#include "osl/stat/activityCount.h"
#include "osl/ppair/indexList.h"
#include "osl/ppair/pairDifference.h"
#include "osl/stat/sparseRegressionMultiplier.h"
#include "osl/stat/diagonalPreconditioner.h"
#include "osl/stat/iterativeLinearSolver.h"
#include "osl/stat/twoDimensionalStatistics.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/pieceEval.h"
#include "gpsshogi/stat/weightRecorder.h"
#include "moveCache.h"
#include <valarray>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] -a occurrence-filename"
       << " [-m min] [-M max] [-T threshold=200] "
       << " [-o weights filename] [-t tmp weights filename] [-I] \n"
       << " -q annotation-file-name\n"
       << " -I init by piece val\n"
       << endl;
  exit(1);
}


using namespace gpsshogi::stat;
using namespace osl;
using namespace osl::eval;
using namespace osl::ppair;

int progressMin = 0;
int progressMax = 10000;
const char *output_filename = "weights2.txt";
const char *tmp_filename = "tmp_weights2.txt";
int infinite_loop = true;

typedef NumEffectState state_t;
stat::ActivityCount activities(PiecePairRawTable::maxPairIndex);

KisenFile kisenFile("../../data/kisen/01.kif");
size_t maxGames = 0;
const char *quiescenceSearchAnnotation = 0;
bool initByPieceValue = false;
void learn();

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *activityFileName = 0;
  size_t threshold=200;
  
  while ((c = getopt(argc, argv, "a:IN:m:M:q:T:t:o:Pvh")) != EOF)
  {
    switch(c)
    {
    case 'a':   activityFileName = optarg;
      break;
    case 'I':   initByPieceValue = true;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'm':   progressMin = atoi(optarg);
      std::cerr << "progress control not supported yet\n";
      break;
    case 'M':   progressMax = atoi(optarg);
      std::cerr << "progress control not supported yet\n";
      break;
    case 'T':   threshold = atoi(optarg);
      break;
    case 't':   tmp_filename = optarg;
      break;
    case 'o':   output_filename = optarg;
      break;
    case 'q':   quiescenceSearchAnnotation = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! activityFileName) || (! quiescenceSearchAnnotation))
    usage(program_name);

  activities.loadBinary(activityFileName, threshold);

  if (! maxGames)
    maxGames = kisenFile.size();
  learn();

  return 0;
}

/**
 * 棋譜を再生しながら annotation の値を出力する
 */
struct RecordPlayer
{
  /** 1ループで処理する試合数 */
  const size_t numMatches;
  /** 現在処理中の試合 */
  size_t curMatch;
  /** 現在処理中の試合の局面番号 */
  size_t curSquareIndex;
  NumEffectState state;
  osl::vector<Move> moves;
  boost::scoped_array<CaptureAnnotations> annotations;
  /** 読みこんだ試合数．何周もするので numMatches を越えうる */
  mutable size_t numMatchesLoaded;
  RecordPlayer(size_t matches, const char *fileName) 
    : numMatches(matches), state((PawnMaskState(HIRATE)))

  {
    curMatch = 0;
    curSquareIndex = 0;
    numMatchesLoaded = 1;
    moves = kisenFile.getMoves(0);
    std::ifstream is(fileName);
    annotations.reset(new CaptureAnnotations[numMatches]);
    for (size_t i=0; i<numMatches; ++i)
      annotations[i].loadFrom(is);
    assert(is);
  }
  void setUpData()
  {
    while ((curSquareIndex >= moves.size())
	   || annotations[curMatch].isTerminal(curSquareIndex))
    {
      curMatch = (curMatch + 1) % numMatches;
      ++numMatchesLoaded;
      moves = kisenFile.getMoves(curMatch);
      curSquareIndex = 0;
      state = NumEffectState((PawnMaskState(HIRATE)));

      if (curMatch % 1000 == 0)
	std::cerr << "\nprocessing " << curMatch << "-" << curMatch+1000
		  << " th record\n";
      if ((curMatch % 100) == 0) 
	std::cerr << '.';
    }
  }
  void addIf(unsigned int index, double val, unsigned int& nonZeros,
	     unsigned int *non_zero_indices,
	     double *non_zero_values)
  {
    size_t i1, i2;
    PiecePairRawTable::meltIndex(index, i1, i2);
    const unsigned int canon = 
      PiecePairRawTable::canonicalIndexOf(i1, i2);
    if (activities.isActive(canon))
    {
      non_zero_indices[nonZeros] = index;
      non_zero_values[nonZeros] = val;
      ++nonZeros;
    }
  }
  size_t getVector(double& y, unsigned int *non_zero_indices,
		   double *non_zero_values)
  {
    y = annotations[curMatch].getAnnotation(curSquareIndex);
    unsigned int nonZeros = 0;
    // ここで y を差分にして， PairDifference を使えば良いはず
    IndexList added, removed;
    PairDifference::diffWithMove(state, moves[curSquareIndex], 
				 added, removed);
    for (IndexList::const_iterator p=added.begin(); p!=added.end(); ++p)
      addIf(*p,  1.0, nonZeros, non_zero_indices, non_zero_values);
    for (IndexList::const_iterator p=removed.begin(); p!=removed.end(); ++p)
      addIf(*p, -1.0, nonZeros, non_zero_indices, non_zero_values);

    ApplyMoveOfTurn::doMove(state, moves[curSquareIndex++]);
    setUpData();
    return nonZeros;
  }
};

struct PPairMultiplier : public stat::SparseRegressionMultiplier
{
  mutable RecordPlayer player;

  /** テストに用いる試合数 */
  const size_t skipHead;

  const double *weights;
  mutable size_t iteration;
  WeightRecorder recorder;
  PPairMultiplier(size_t dimA, size_t numMatches, 
		  size_t skipHead, const double *w, const char *tmp_out)
    : stat::SparseRegressionMultiplier(dimA),
      player(numMatches, quiescenceSearchAnnotation),
      skipHead(skipHead), weights(w), iteration(0), recorder(tmp_out)
  {
  }
  /** dotProduct between sparse vector a and dense vector b */
  static double dotProduct(const unsigned int a_non_zeros, 
			   const unsigned int *a_indices, 
			   const double *a_values,
			   const double *b)
  {
    double result = 0.0;
    for (size_t i=0; i<a_non_zeros; ++i)
    {
      result += a_values[i]*b[a_indices[i]];
    }
    return result;
  }
  bool getVectorX(unsigned int& non_zeros, unsigned int *non_zero_indices,
		  double *non_zero_values) const
  {
    double y;
    non_zeros = player.getVector(y, non_zero_indices, non_zero_values);
    return (player.numMatchesLoaded % player.numMatches) != 0;
  }
  void newIteration() const
  {
    assert(player.curSquareIndex == 0);
    assert(player.numMatchesLoaded % player.numMatches == 0);

    recorder.write(iteration, dim(), weights);
    boost::scoped_array<unsigned int> indices(new unsigned int[dim()]);
    boost::scoped_array<double> values(new double[dim()]);

    stat::TwoDimensionalStatistics stat;
    do 
    {
      double y;
      const size_t non_zeros =
	player.getVector(y, &indices[0], &values[0]);
      const double prediction 
	= dotProduct(non_zeros, &indices[0], &values[0], weights);
      stat.addInstance(prediction, y);
    } while ((player.numMatchesLoaded % player.numMatches) < skipHead);
    
    const double mse = stat.meanSquaredErrorsAdjustConstant();
    std::cerr << "At " << iteration++ << " iteration\n";
    std::cerr << "matches " << player.numMatchesLoaded << "\n";
    std::cerr << "Cross Validation: " << sqrt(mse) << "\n" << std::flush;
    // std::cerr << "piece weights: " << weights[0] << "\n" << std::flush;
  }
  /**
   * @param xty is b where Ax=b to solve x
   * @param diag_inv used in order to make diagonal preconditioner of X^tX
   */
  void computeXtY(double *xty, double *diag_inv)
  {
    boost::scoped_array<unsigned int> indices(new unsigned int[dim()]);
    boost::scoped_array<double> values(new double[dim()]);
	
    std::fill(&xty[0], &xty[dim()], 0.0);
    std::fill(&diag_inv[0], &diag_inv[dim()], 0.0);

    assert(player.numMatchesLoaded == 1);
    while (player.numMatchesLoaded < skipHead)
    {
      // skip test data
      double y;
      player.getVector(y, &indices[0], &values[0]);
    }

    size_t numInstances = 0;
    while (player.numMatchesLoaded < player.numMatches)
    {
      double y;
      const size_t non_zeros = player.getVector(y, &indices[0], &values[0]);
      ++numInstances;
      for (size_t ip=0; ip<non_zeros; ++ip)
      {
	const size_t p  = indices[ip];
	const double vp = values[ip];
	assert(p < dim());
	xty[p] += vp*y;
	diag_inv[p] += vp*vp;
      }
    }
    std::cerr << "instances " << numInstances << "\n";
    for (size_t i=0; i<dim(); ++i)
    {
      diag_inv[i] = diag_inv[i] ? 1.0/diag_inv[i] : 1.0;	    
    }
  }
  void getVectorXWithID(unsigned int&, int&, unsigned int*, double*) const
  {
    assert(0);
  }
};

struct ValueInitializer
{
  double *weights;
  explicit ValueInitializer(double *w) : weights(w)
  {
  }
  void operator()(size_t i)
  {
    size_t i1, i2;
    PiecePairRawTable::meltIndex(i, i1, i2);
    if (i1 == i2)
    {
      Square pos;
      PtypeO ptypeo;
      PiecePairRawTable::meltIndex(i1, pos, ptypeo);
      weights[i] = Ptype_Eval_Table.value(ptypeo);
    }
  }
};


void learn()
{
  const size_t numMatches = maxGames;
  const size_t dimA = PiecePairRawTable::maxPairIndex;
  
  const size_t skip_head = numMatches / 128;
  const size_t max_loop = 100;
  const double eps = 0.001;
  std::valarray<double> result(0.0, dimA);
  if (initByPieceValue)
  {
    std::cerr << "initByPieceValue\n";
    ValueInitializer init(&result[0]);
    PiecePairRawTable::forEachRelation(init);
  }
  
  PPairMultiplier prodA(dimA, numMatches, skip_head, 
			&result[0], tmp_filename);
  std::valarray<double> b(dimA);
  std::valarray<double> diag_inv(dimA);
  std::cerr << "computing x^t y\n";
  prodA.computeXtY(&b[0], &diag_inv[0]);
  stat::DiagonalPreconditioner preconditioner(dimA);
  preconditioner.setInverseDiagonals(&diag_inv[0]);
  std::cerr << "preconditioner\n";

  stat::IterativeLinearSolver solver(prodA, &preconditioner, max_loop, eps);
    
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
  WeightRecorder::write(output_filename, dimA, &result[0]);

  if (err)
    std::cerr << "solver failed " <<  err << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
