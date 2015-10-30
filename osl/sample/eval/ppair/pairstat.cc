/* pairstat.cc
 */

#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/csa.h"
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
  cerr << "Usage: " << prog << " [-f pair-file-name] [-P player(0 for black, 1 for white)] [-p position(e.g. 11)] [-t ptype(e.g. 7 for PROOK)]"
       << endl
       << "if any of -Ppt options are specified, relation of [<specified-pieace*specified-pos>,<other-pieace*other-pos>] will be shown \n"
       << "otherwise, relation of [<same-pieace*same-pos>,<same-pieace*same-pos>] will be shown \n"
       << endl;
  exit(1);
}

void showPieceStat(Player, Ptype);
void showPairStat(Player, Square, Ptype);

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *pairFileName = 0;
  int ptype = PROOK;
  Square pos(1,1);
  Player player = BLACK;
  int singleStateMode = true;
  
  extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "f:p:P:t:vh")) != EOF)
  {
    switch(c)
    {
    case 'f':	pairFileName = optarg;
      break;
    case 'p':	pos = Square(atoi(optarg)/10, atoi(optarg)%10);
      singleStateMode = false;
      break;
    case 'P':	player = (atoi(optarg) ? WHITE : BLACK);
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

  if (error_flag || (! pairFileName))
    usage(program_name);

  PiecePairRawEval::setUp(pairFileName);

  if (singleStateMode)
  {
    for (int i=PPAWN; i<=PTYPE_MAX; ++i)
    {
      showPieceStat(BLACK,static_cast<Ptype>(i));
      showPieceStat(WHITE,static_cast<Ptype>(i));
    }
  }
  else
    showPairStat(player,pos,static_cast<Ptype>(ptype));
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
      const unsigned int index1 = PiecePairIndex::indexOf(pos1,ptypeo);
      std::cout << std::setw(4) << PiecePairRawTable::Table.valueOf(index1,index1);
    }
    std::cout << "\n";
  }
  const Square pos1 = Square::STAND();
  const unsigned int index1 = PiecePairIndex::indexOf(pos1,ptypeo);
  std::cout << pos1 << " " << std::setw(4) << PiecePairRawTable::Table.valueOf(index1,index1);
  std::cout << "\n";
}

void showPairStatAgainst(Player player2, Ptype ptype2, unsigned int index1)
{
  const PtypeO ptypeo2 = newPtypeO(player2, ptype2);
  for (int y=1; y<=9; ++y)
  {
    for (int x=9; x>=1; --x)
    {
      const Square pos2(x,y);
      const unsigned int index2 = PiecePairIndex::indexOf(pos2,ptypeo2);
      std::cout << std::setw(4) << PiecePairRawTable::Table.valueOf(index1,index2);
    }
    std::cout << "\n";
  }
  const Square pos2 = Square::STAND();
  const unsigned int index2 = PiecePairIndex::indexOf(pos2,ptypeo2);
  std::cout << pos2 << " " << std::setw(4) << PiecePairRawTable::Table.valueOf(index1,index2);
  std::cout << "\n";
}

void showPairStat(Player player, Square pos1, Ptype ptype1)
{
  const PtypeO ptypeo1 = newPtypeO(player, ptype1);
  std::cout << player << ", " << pos1 << ", " << ptype1 << "\n";
  const unsigned int index1 = PiecePairIndex::indexOf(pos1,ptypeo1);
  for (int p2=PPAWN; p2<=PTYPE_MAX; ++p2)
  {
    Ptype ptype2 = static_cast<Ptype>(p2);
    std::cout << player << ptype2 << " (<=> " << player << ptype1 << ", " << pos1 << ")\n";
    showPairStatAgainst(player, ptype2, index1);
    std::cout << alt(player) << ptype2 << " (<=> " << player << ptype1 << ", " << pos1 << ")\n";
    showPairStatAgainst(alt(player), ptype2, index1);
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
