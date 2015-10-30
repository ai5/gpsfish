/* weightanalyzer.cc
 */

// pair のweight を解析する
// pairanalyzer と枠組みは似ているが中身が違う

#include "osl/eval/ppair/piecePairRawEval.h"
#include <boost/scoped_array.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

using namespace osl;
using namespace osl::eval;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-f weight-file-name] [-P player(0 for black, 1 for white)] [-Q viewpointPlayer] [-p position(e.g. 11)] [-t ptype(e.g. 7 for PROOK)]"
       << endl
       << "if any of -Ppt options are specified, relation of [<specified-pieace*specified-pos>,<other-pieace*other-pos>] will be shown \n"
       << "otherwise, relation of [<same-piece*same-pos>,<same-piece*same-pos>] will be shown \n"
       << endl;
  exit(1);
}

void showPieceStat(Player, Ptype);
void showPairStat(Player, Player, Square, Ptype);

boost::scoped_array<double> weights;

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *weightFileName = 0;
  int ptype = PROOK;
  Square pos(1,1);
  Player player = BLACK;
  Player viewPlayer = BLACK;
  int singleStateMode = true;
  
  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "f:p:P:Q:t:vh")) != EOF)
  {
    switch(c)
    {
    case 'f':	weightFileName = optarg;
      break;
    case 'p':	pos = Square(atoi(optarg)/10, atoi(optarg)%10);
      singleStateMode = false;
      break;
    case 'P':	player = (atoi(optarg) ? WHITE : BLACK);
      singleStateMode = false;
      break;
    case 'Q':	viewPlayer = (atoi(optarg) ? WHITE : BLACK);
      singleStateMode = false;
      break;
    case 't':	ptype = atoi(optarg);
      singleStateMode = false;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (! weightFileName))
    usage(program_name);

  weights.reset(new double[PiecePairRawTable::maxPairIndex]);
  FILE *fp = fopen(weightFileName, "r");
  assert(fp);
  for (size_t i=0; i<PiecePairRawTable::maxPairIndex; ++i)
  {
    fscanf(fp, "%lf", &weights[i]);
  }
  fclose(fp);
  
  if (singleStateMode)
  {
    for (int i=PPAWN; i<=PTYPE_MAX; ++i)
    {
      showPieceStat(BLACK,static_cast<Ptype>(i));
      showPieceStat(WHITE,static_cast<Ptype>(i));
    }
  }
  else
    showPairStat(viewPlayer,player,pos,static_cast<Ptype>(ptype));
}

void showValue(std::ostream& os, size_t i1, size_t i2)
{
  std::cout << std::fixed << std::setprecision(2) << std::setw(6) 
	    << weights[PiecePairRawTable::canonicalIndexOf(i1,i2)]*100;
}

void showPieceStat(Player player, Ptype ptype)
{
  const PtypeO ptypeo = newPtypeO(player, ptype);
  // single [piece,position]
  std::cout << player << ", " << ptype << "\n";
  for (int y=1; y<=9; ++y)
  {
    for (int x=9; x>=1; --x)
    {
      const Square pos1(x,y);
      const unsigned int index1 = PiecePairRawTable::indexOf(pos1,ptypeo);
      showValue(std::cout, index1, index1);
    }
    std::cout << "\n";
  }
  const Square pos1 = Square::STAND();
  const unsigned int index1 = PiecePairRawTable::indexOf(pos1,ptypeo);
  std::cout << pos1;
  showValue(std::cout, index1,index1);
  std::cout << "\n";
}

// この player と player2 の使い分けは obsolete ?
void showPairStatAgainst(Player player, 
			 Player player2, Ptype ptype2, unsigned int index1)
{
  const PtypeO ptypeo2 = newPtypeO(player2, ptype2);
  for (int y=1; y<=9; ++y)
  {
    for (int x=9; x>=1; --x)
    {
      const Square pos2(x,y);
      const unsigned int index2 = PiecePairRawTable::indexOf(pos2,ptypeo2);
      showValue(std::cout, index1,index2);
    }
    std::cout << "\n";
  }
  const Square pos2 = Square::STAND();
  const unsigned int index2 = PiecePairRawTable::indexOf(pos2,ptypeo2);
  std::cout << pos2;
  showValue(std::cout, index1,index2);
  std::cout << "\n";
}

void showPairStat(Player viewPlayer, Player player, Square pos1, Ptype ptype1)
{
  const PtypeO ptypeo1 = newPtypeO(player, ptype1);
  std::cout << player << ", " << pos1 << ", " << ptype1 << "\n";
  const unsigned int index1 = PiecePairRawTable::indexOf(pos1,ptypeo1);
  for (int p2=PPAWN; p2<=PTYPE_MAX; ++p2)
  {
    Ptype ptype2 = static_cast<Ptype>(p2);
    std::cout << player << ptype2 << " (<=> " << player << ptype1 << ", " << pos1 << ")\n";
    showPairStatAgainst(viewPlayer, player, ptype2, index1);
    std::cout << alt(player) << ptype2 << " (<=> " << player << ptype1 << ", " << pos1 << ")\n";
    showPairStatAgainst(viewPlayer, alt(player), ptype2, index1);
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
