// learn.cc
// 判別関数の学習
#include "osl/ppair/indexList.h"
#include "osl/ppair/pairDifference.h"
#include "osl/stat/activityCount.h"
#include "osl/ppair/discriminationInstance.h"
#include "osl/ppair/siblingUtil.h"
#include "osl/ppair/siblingUtilGenerator.h"
#include "osl/ppair/quiescenceGenerator.h"
#include "osl/stat/sparseRegressionMultiplier.h"
#include "osl/stat/diagonalPreconditioner.h"
#include "osl/stat/iterativeLinearSolver.h"
#include "osl/stat/twoDimensionalStatistics.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "gpsshogi/stat/weightRecorder.h"
#include "jobBuffer.h"
#include "moveCache.h"
#include <valarray>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-N#games] -a occurrence-filename"
       << " [-I] [-m min] [-M max] [-T threshold=20] "
       << " [-o weights filename] [-t tmp weights filename] \n"
       << " -I ignore siblings instead of using chains of siblings\n"
       << " -H ignore hot moves\n"
       << " -P multiply progress\n"
       << " -q annotation-file-name\t using quiescence search annotated in file instead of simple takeback analyses\n"
       << endl;
  exit(1);
}


using namespace gpsshogi::stat;
using namespace osl;
using namespace osl::eval;
using namespace osl::ppair;

typedef JobBuffer<const DiscriminationInstanceArray*,6> buffer_t;
buffer_t buf;

/** 
 * buffer 以上のサイズがあれば buffer にあって消費していないメモリを上
 * 書きしてしまうことはない．consumer が一つずつ消費するなら，receive 
 * して使っている途中のものを上書きしないためには，さらに一つ余裕をみ
 * ておけば良い．
 *
 * 念のため+2 に．
 */
const size_t data_pool_size = buffer_t::max_buf_size+2;
DiscriminationInstanceArray data_pool[data_pool_size];
volatile bool terminate=false;

int progressMin = 0;
int progressMax = 10000;
const char *output_filename = "weights.txt";
const char *tmp_filename = "tmp_weights.txt";
int infinite_loop = true;

typedef NumEffectState state_t;
stat::ActivityCount activities(PiecePairRawTable::maxPairIndex);

KisenFile kisenFile("../../data/kisen/01.kif");
size_t maxGames = 0;

void produce();
void consume();

SiblingPolicy policy = CHAIN;

std::unique_ptr<InstanceGenerator> generator;

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *activityFileName = 0;
  size_t threshold=20;
  const char *quiescenceSearchAnnotation = 0;
  bool ignoreHotMove = false;
  bool multiplyProgress = false;
  
  while ((c = getopt(argc, argv, "a:HIN:m:M:q:T:t:o:Pvh")) != EOF)
  {
    switch(c)
    {
    case 'H':   ignoreHotMove = true;
      break;
    case 'I':   policy = IGNORE;
      break;
    case 'N':   maxGames = atoi(optarg);
      break;
    case 'm':   progressMin = atoi(optarg);
      break;
    case 'M':   progressMax = atoi(optarg);
      break;
    case 'a':   activityFileName = optarg;
      break;
    case 'T':   threshold = atoi(optarg);
      break;
    case 't':   tmp_filename = optarg;
      break;
    case 'o':   output_filename = optarg;
      break;
    case 'P':   multiplyProgress = true;
      break;
    case 'q':   quiescenceSearchAnnotation = optarg;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! activityFileName))
    usage(program_name);

  activities.loadBinary(activityFileName, threshold);

  if (! maxGames)
    maxGames = kisenFile.size();

  if (quiescenceSearchAnnotation)
    generator.reset(new QuiescenceGenerator
		    (activities, policy, maxGames, 
		     quiescenceSearchAnnotation, ignoreHotMove, 
		     multiplyProgress));
  else				
  {
    std::cerr << "fixme! cannot control ignoreHotMove and multiplyProgress yet\n";
    generator.reset(new SiblingUtilGenerator(activities, policy));
  }
  

  boost::thread producer(produce), consumer(consume);
  producer.join();
  consumer.join();

  return 0;
}

/* ------------------------------------------------------------------------- */
// produce 関係
/* ------------------------------------------------------------------------- */

/**
 * 棋譜から学習用データを作り，キューにためる．
 */
void produce()
{
  MoveCache matches;
  matches.getAllMoves(kisenFile, maxGames, progressMin, progressMax);
  
  size_t count = 0;
  do
  {
    for (size_t i=0;i<maxGames;i++)
    {
      if (terminate)
	break;
      
      if (i % 1000 == 0)
	std::cerr << "\nprocessing " << i << "-" << i+1000 << " th record\n";
      if ((i % 100) == 0) 
	std::cerr << '.';
      NumEffectState initial((PawnMaskState(HIRATE)));
      const osl::vector<Move>& moves=matches.getMoves(initial, i);
      DiscriminationInstanceArray& data = data_pool[count % data_pool_size];
      generator->generate(i, initial, matches.getSquareID(i), moves, data);
      ++count;
      buf.send(&data);
    }
  } while ((! terminate) && infinite_loop);
  std::cerr << "produce " << count << " data\n";
  buf.send(0);
}


/* ------------------------------------------------------------------------- */
// consume 関係
/* ------------------------------------------------------------------------- */

struct PPairMultiplier : public stat::SparseRegressionMultiplier
{
  buffer_t& buf;
  /** 現在処理中の試合 */
  mutable const DiscriminationInstanceArray *curMatch;
  /** 現在処理中の試合の局面番号 */
  mutable size_t curSquareIndex;

  /** 読みこんだ試合数．何周もするので numMatches を越えうる */
  mutable size_t numMatchesLoaded;

  /** 1ループで処理する試合数 */
  const size_t numMatches;
  /** テストに用いる試合数 */
  const size_t skipHead;

  const double *weights;
  mutable size_t iteration;
  WeightRecorder recorder;
  PPairMultiplier(buffer_t& b, size_t dimA, size_t numMatches, 
		  size_t skipHead, const double *w, const char *tmp_out)
    : stat::SparseRegressionMultiplier(dimA),
      buf(b), curMatch(0), curSquareIndex(0), numMatchesLoaded(0),
      numMatches(numMatches), skipHead(skipHead), 
      weights(w), iteration(0), recorder(tmp_out)
  {
    setUpData();
  }
  void setUpData() const
  {
    while ((! curMatch)
	   || (curSquareIndex >= curMatch->size()))
    {
      curMatch = buf.receive(); // may block
      curSquareIndex = 0;
      ++numMatchesLoaded;
    }
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
  size_t getVector(double& y, unsigned int *non_zero_indices, 
		   double *non_zero_values) const
  {
    const DiscriminationInstance& instance = (*curMatch)[curSquareIndex++];
    
    for (size_t i=0; i<instance.data.size(); ++i)
    {
      non_zero_indices[i] = instance.data[i].id;
      non_zero_values[i] = instance.data[i].val;
    }
    y = instance.value;
    setUpData();
    return instance.data.size();
  }
  bool getVectorX(unsigned int& non_zeros, unsigned int *non_zero_indices, 
		  double *non_zero_values) const
  {
    double y;
    non_zeros = getVector(y, non_zero_indices, non_zero_values);
    return (numMatchesLoaded % numMatches) != 0;
  }
  void newIteration() const
  {
    assert(curSquareIndex == 0);
    assert(numMatchesLoaded % numMatches == 0);

    recorder.write(iteration, dim(), weights);
    std::unique_array<unsigned int> indices(new unsigned int[dim()]);
    std::unique_array<double> values(new double[dim()]);

    stat::TwoDimensionalStatistics stat;
    do 
    {
      double y;
      const unsigned int non_zeros =
	getVector(y, &indices[0], &values[0]);
      const double prediction 
	= dotProduct(non_zeros, &indices[0], &values[0], weights);
      stat.addInstance(prediction, y);
    } while ((numMatchesLoaded % numMatches) < skipHead);
    
    const double mse = stat.meanSquaredErrorsAdjustConstant();
    std::cerr << "At " << iteration++ << " iteration\n";
    std::cerr << "matches " << numMatchesLoaded << "\n";
    std::cerr << "Cross Validation: " << sqrt(mse) << "\n" << std::flush;
    // std::cerr << "piece weights: " << weights[0] << "\n" << std::flush;
  }
  /**
   * @param xty is b where Ax=b to solve x
   * @param diag_inv used in order to make diagonal preconditioner of X^tX
   */
  void computeXtY(double *xty, double *diag_inv)
  {
    std::unique_array<unsigned int> indices(new unsigned int[dim()]);
    std::unique_array<double> values(new double[dim()]);
	
    std::fill(&xty[0], &xty[dim()], 0.0);
    std::fill(&diag_inv[0], &diag_inv[dim()], 0.0);

    assert(numMatchesLoaded == 1);
    while (numMatchesLoaded < skipHead)
    {
      // skip test data
      double y;
      getVector(y, &indices[0], &values[0]);
    }

    size_t numInstances = 0;
    while (numMatchesLoaded < numMatches)
    {
      double y;
      const size_t non_zeros = getVector(y, &indices[0], &values[0]);
      ++numInstances;
      for (size_t ip=0; ip<non_zeros; ++ip)
      {
	const size_t p = indices[ip];
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

void consume()
{
  buffer_t::redundantNsleep(8000);

  const size_t numMatches = maxGames;
  const size_t dimA = PiecePairRawTable::maxPairIndex;
  
  const size_t skip_head = numMatches / 128;
  const size_t max_loop = 100;
  const double eps = 0.001;
  std::valarray<double> result(0.0, dimA);
  int iter;
  double tol;

  PPairMultiplier prodA(buf, dimA, numMatches, skip_head, 
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
  terminate = true;
  buf.receive(false);		// salvage possible blocking sender
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
