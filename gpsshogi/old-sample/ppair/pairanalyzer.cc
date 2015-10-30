/* pairanalyzer.cc
 */

// ppair::Table を解析する

#include "osl/ppair/table.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>

using namespace osl;
using namespace osl::ppair;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-NDFL] [-f table-file-name] [-P player(0 for black, 1 for white)] [-Q viewpointPlayer] [-p position(e.g. 11)] [-t ptype(e.g. 7 for PROOK)]"
       << endl
       << "if any of -Ppt options are specified, relation of [<specified-pieace*specified-pos>,<other-pieace*other-pos>] will be shown \n"
       << "otherwise, relation of [<same-piece*same-pos>,<same-piece*same-pos>] will be shown \n"
       << "-N show numerator\n"
       << "-D show denominator\n"
       << "-F show fraction\n"
       << "-L show scaled -log fraction\n"
       << endl;
  exit(1);
}

void showPieceStat(Player, Ptype);
void showPairStat(Player, Player, Square, Ptype);

Table table;
bool showNumerator=false;
bool showDenominator=false;
bool showFraction=false;
bool showLogFraction=false;

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *tableFileName = 0;
  int ptype = PROOK;
  Square pos = newSquare(1,1);
  Player player = BLACK;
  Player viewPlayer = BLACK;
  int singleStateMode = true;
  
  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "DNFLf:p:P:Q:t:vh")) != EOF)
  {
    switch(c)
    {
    case 'D':	showDenominator = true;
      break;
    case 'N':	showNumerator = true;
      break;
    case 'F':	showFraction = true;
      break;
    case 'L':	showLogFraction = true;
      break;
    case 'f':	tableFileName = optarg;
      break;
    case 'p':	pos = newSquare(atoi(optarg)/10, atoi(optarg)%10);
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

  if (error_flag || (! tableFileName))
    usage(program_name);

  std::ifstream is(tableFileName);
  table.binaryLoad(is);

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

void showProbability(std::ostream& os, const Probability& p)
{
  if (showNumerator)
    std::cout << std::setw(8) << p.numerator;
  if (showDenominator)
    std::cout << std::setw(8) << p.denominator;
  if (showFraction)
    std::cout << std::fixed << std::setprecision(2) << std::setw(6) 
	      << p.probability()*100;
  if (showLogFraction)
    std::cout << std::fixed << std::setprecision(2) << std::setw(6) 
	      << -log(p.badShapeProbability())/log(2.0)*50;
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
      const Square pos1 = newSquare(x,y);
      const unsigned int index1 = PiecePairRawTable::indexOf(pos1,ptypeo);
      const Probability& prob = table.probability(player, index1, index1);
      showProbability(std::cout, prob);
    }
    std::cout << "\n";
  }
  const Square pos1 = OFFBOARD;
  const unsigned int index1 = PiecePairRawTable::indexOf(pos1,ptypeo);
  std::cout << pos1;
  showProbability(std::cout, table.probability(player, index1,index1));
  std::cout << "\n";
}

void showPairStatAgainst(Player player, 
			 Player player2, Ptype ptype2, unsigned int index1)
{
  const PtypeO ptypeo2 = newPtypeO(player2, ptype2);
  for (int y=1; y<=9; ++y)
  {
    for (int x=9; x>=1; --x)
    {
      const Square pos2 = newSquare(x,y);
      const unsigned int index2 = PiecePairRawTable::indexOf(pos2,ptypeo2);
      showProbability(std::cout, table.probability(player, index1,index2));
    }
    std::cout << "\n";
  }
  const Square pos2 = OFFBOARD;
  const unsigned int index2 = PiecePairRawTable::indexOf(pos2,ptypeo2);
  std::cout << pos2;
  showProbability(std::cout, table.probability(player, index1,index2));
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
