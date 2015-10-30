// quantize.cc

#include "osl/eval/progressEval.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/stat/histogram.h"
#include "showRelation.h"
#include <boost/scoped_array.hpp>
#include <map>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-a count] [-t threshold] -T #showtop -o output -w weights -s scaling"
       << endl;
  exit(1);
}

using namespace osl;
using namespace osl::eval;

void showLargeRelations(const char *activityFileName, size_t threshold,
			const double *weights, size_t showTopN);

int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *weightsFileName = 0;
  const char *outputFileName = 0;
  size_t scaling = 4096 * 4;

  // この辺は showTopN で使う
  const char *activityFileName = 0;
  size_t threshold = 0;
  size_t showTopN = 0;
  while ((c = getopt(argc, argv, "a:o:t:T:s:w:vh")) != EOF)
  {
    switch(c)
    {
    case 'a':   activityFileName = optarg;
      break;
    case 'o':   outputFileName = optarg;
      break;
    case 't':   threshold = atoi(optarg);
      break;
    case 'T':   showTopN = atoi(optarg);
      break;
    case 'w':   weightsFileName = optarg;
      break;
    case 's':   scaling = atoi(optarg);
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! (showTopN || outputFileName)) || (! weightsFileName))
    usage(program_name);

  std::cerr << "start reading\n";

  boost::scoped_array<double> 
    weights(new double[PiecePairRawTable::maxPairIndex]);
  {
    FILE *fp = fopen(weightsFileName, "r");
    assert(fp);
    for (size_t i=0; i<PiecePairRawTable::maxPairIndex; ++i)
      fscanf(fp, "%lf", &weights[i]);
    fclose(fp);
  }

  if (showTopN)
    showLargeRelations(activityFileName, threshold, 
		       &weights[0], showTopN);

  if (! outputFileName)
    return 0;
  std::cerr << "start conversion\n";

  boost::scoped_array<signed char>
    output(new signed char[PiecePairRawTable::maxPairIndex]);
  std::fill(&output[0], &output[0]+PiecePairRawTable::maxPairIndex,
	    static_cast<signed char>(0));
  output[0] = 0;
  size_t numOutOfRanges = 0;
  stat::Histogram histogram(1,256); // [0,255]
  for (size_t i=1; i<PiecePairRawTable::maxPairIndex; ++i)
  {
    if (! weights[i])
      continue;
    int result = static_cast<int>(weights[i] * scaling);
    if (result < -127)
    {
      ++numOutOfRanges;
      result = -127;
    }
    else if (result > 127)
    {
      ++numOutOfRanges;
      result = 127;
    }
    output[i] = static_cast<signed char>(result);
    histogram.add(output[i]+127);
    if (output[i])
    {
      size_t i1, i2;
      PiecePairRawTable::meltIndex(i, i1, i2);
      if (i1 != i2)
      {
	const unsigned int alternative
	  = ((i != PiecePairRawTable::indexOf(i1,i2))
	     ? PiecePairRawTable::indexOf(i1,i2)
	     : PiecePairRawTable::indexOf(i2,i1));
	assert(i != alternative);
	assert(output[alternative] == 0);
	output[alternative] = output[i];
      }
    }
  }

  std::cerr << "#out of range = " << numOutOfRanges << "\n";
  histogram.show(std::cerr);
  
  std::cerr << "start writing\n";
  FILE *fp = fopen(outputFileName, "w");
  assert(fp);
  const size_t written = fwrite(&output[0], sizeof(output[0]), 
				PiecePairRawTable::maxPairIndex, fp);
  assert(written == PiecePairRawTable::maxPairIndex);
  fclose(fp);
  return 0;
}

void showRelation(size_t index)
{
  size_t i1, i2;
  PiecePairRawTable::meltIndex(index, i1, i2);

  PtypeO ptypeo1, ptypeo2;
  Square pos1, pos2;
  PiecePairRawTable::meltIndex(i1, pos1, ptypeo1);
  PiecePairRawTable::meltIndex(i2, pos2, ptypeo2);

  showPiece(ptypeo1, pos1);
  std::cout << " ";
  showPiece(ptypeo2, pos2);
}

typedef std::multimap<double, size_t, std::greater<double> > map_t;
struct TopHolder : public map_t
{
  size_t topN;
  explicit TopHolder(size_t n) : topN(n)
  {
  }
  void add(size_t id, double w)
  {
    // w = fabs(w);
    const double threshold = 0.1;
    if (w > threshold)
    {
      insert(std::make_pair(w, id));
      if (size() > topN)
      {
	const double lowest = rbegin()->first;
	erase(lowest);
      }
    }
  }
  void show(const size_t *count)
  {
    for (const_iterator p=begin(); p!=end(); ++p)
    {
      std::cout << std::setw(9) << p->second << " ";
      std::cout << std::setw(9) << count[p->second] << " ";
      showRelation(p->second);
      std::cout << " " << p->first << "\n";
    }
  }
};

void showLargeRelations(const char *activityFileName, size_t threshold,
			const double *weights, size_t showTopN)
{
  boost::scoped_array<size_t> 
    count(new size_t[PiecePairRawTable::maxPairIndex]);
  if (activityFileName)
  {
    FILE *fp = fopen(activityFileName, "r");
    assert(fp);
    for (size_t i=0; i<PiecePairRawTable::maxPairIndex; ++i)
    {
      int occur;
      fscanf(fp, "%d", &occur);
      count[i] = occur;
    }
    fclose(fp);
  }
  else
  {
    for (size_t i=0; i<PiecePairRawTable::maxPairIndex; ++i)
      count[i] = 7777777;
  }

  TopHolder holder(showTopN);
  for (size_t i=0; i<PiecePairRawTable::maxPairIndex; ++i)
  {
    if (count[i] >= threshold)
      holder.add(i, weights[i]);
  }
  holder.show(&count[0]);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
