// akukei.cc
/** @file
 * 悪形チェック の項目を評価関数がどの程度みたしているか
 * http://plaza9.mbn.or.jp/~kfend/inside_kfend/bad_shape.html
 */
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/state/pawnMaskState.h"
#include "osl/record/csaString.h"
#include "osl/record/csa.h"
#include "osl/apply_move/doUndoMoveLock.h"
#include "osl/apply_move/applyMove.h"
#include "showRelation.h"
#include <map>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " [-v] -w weight-filename \n"
       << endl;
  exit(1);
}

using namespace osl;
using namespace osl::eval;
#define POS(x,y) Square(x,y)

bool verbose = false;
void showRelation(size_t index, int diff, Move focus)
{
  const int val = PiecePairRawTable::Table.value(index);
#if 0
  if (val == 0)
    return;
#endif
  std::cout << std::setw(3) << diff << " ";
  size_t i1, i2;
  PiecePairRawTable::meltIndex(index, i1, i2);

  PtypeO ptypeo1, ptypeo2;
  Square pos1, pos2;
  PiecePairRawTable::meltIndex(i1, pos1, ptypeo1);
  PiecePairRawTable::meltIndex(i2, pos2, ptypeo2);

  if (focus.isNormal() && (pos1 == focus.from() || pos1 == focus.to()))
  {
    showPiece(ptypeo1, pos1);
    std::cout << " ";
    showPiece(ptypeo2, pos2);
  }
  else
  {
    showPiece(ptypeo2, pos2);
    std::cout << " ";
    showPiece(ptypeo1, pos1);
  }
  std::cout << " " << val << "\n";
}

typedef std::map<size_t,int> map_t;
/** 関係の乱暴な管理 関係ID->出現数 */
struct Relations : public map_t
{
  int val(size_t index) const
  {
    const_iterator p = find(index);
    if (p == end())
      return 0;
    return p->second;
  }
  int val(bool verbose=false, Move focus=Move::INVALID()) const
  {
    int sum = 0;
    for (const_iterator p=begin(); p!=end(); ++p)
    {
      if (verbose)
	showRelation(p->first, p->second, focus);
      sum += p->second * PiecePairRawTable::Table.value(p->first);
    }
    return sum;
  }
  void subtract(const Relations& r, Relations& out) const
  {
    out = *this;
    for (const_iterator p=r.begin(); p!=r.end(); ++p)
    {
      out[p->first] -= p->second;
      if (out[p->first] == 0)
	out.erase(p->first);
    }
  }
};


template <class SquareSelect>
void relationsInArea(const PawnMaskState& state, SquareSelect selector,
		     Relations& out)
{
  for (int i=0;i<Piece::SIZE;i++)
  {
    const Piece pi = state.pieceOf(i);
    const Square posi = pi.square();
    if (! selector(posi))
      continue;
    const size_t index1 = PiecePairRawTable::indexOf(pi);
    for (int j=i;j<Piece::SIZE;j++)
    {
      const Piece pj = state.pieceOf(j);
      const Square posj = pj.square();
      if (! selector(posj))
	continue;
      const size_t index2 = PiecePairRawTable::indexOf(pj);
      const size_t index = PiecePairRawTable::canonicalIndexOf(index1,index2);
      out[index]++;
    }
  }
}

struct RangeSelector
{
  static const bool debug = false;
  int xmin, ymin, xmax, ymax;
  bool useOffBoard;
  RangeSelector(int xmin, int ymin, int xmax, int ymax, bool useOffBoard) 
    : xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax), useOffBoard(useOffBoard)
  {
    assert(xmin <= xmax);
    assert(ymin <= ymax);
  }
  bool operator()(Square pos)
  {
    if (pos.isPieceStand())
      return useOffBoard;
    const int x = pos.x(), y = pos.y();
    const bool selected =  (xmin <= x) && (x <= xmax)
      && (ymin <= y) && (y <= ymax);
    if ((! selected) && debug)
      std::cerr << "skip " << pos << "\n";
    return selected;
  }
};


template <class SquareSelect>
int diffWithMove(PawnMaskState& state, Move m, SquareSelect selector,
		 const Relations& prev)
{
  DoUndoMoveLock lock(state, m);
  Relations next;
  relationsInArea(state, selector, next);
  
  Relations diff;
  next.subtract(prev, diff);
  return diff.val(verbose, m);
}

void showMove(const Relations& cur,
	      PawnMaskState& state, Move m, const RangeSelector& range)
{
  csaShow(std::cout, m);
  std::cout << " " << diffWithMove(state, m, range, cur) << "\n";
}

void testfig1()
{
  PawnMaskState state(CsaString(
			 "P1-OU *  *  *  *  *  * -KE-KY\n"
			 "P2 *  *  *  *  *  * -KI *  * \n"
			 "P3 *  *  *  * -FU-FU-GI-FU-FU\n"
			 "P4 *  *  *  *  *  * -FU *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  * +GI *  * +HI * \n"
			 "P7 *  *  *  * +FU+FU+FU * +FU\n"
			 "P8 *  *  *  *  *  *  *  *  * \n"
			 "P9+OU *  *  *  *  *  * +KE+KY\n"
			 "P+00AL\n"
			 "+\n").getInitialState());  
  RangeSelector range(1,1,5,9,false);
  Relations cur;
  relationsInArea(state, range, cur);
  std::cout << "\ntestfig1 " << cur.val() << "\n";

  const Move m36hi(POS(2,6),POS(3,6),ROOK,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m36hi, range);
  
  const Move m46hi(POS(2,6),POS(4,6),ROOK,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m46hi, range);

  const Move m27hi(POS(2,6),POS(2,7),ROOK,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m27hi, range);

  const Move m25fu(POS(2,5),PAWN,BLACK);
  showMove(cur, state, m25fu, range);

  const Move m16fu(POS(1,7),POS(1,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m16fu, range);

  const Move m36fu(POS(3,7),POS(3,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m36fu, range);

  const Move m46fu(POS(4,7),POS(4,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m46fu, range);
}

void testfig2()
{
  PawnMaskState state(CsaString(
			 "P1-OU *  *  *  *  *  *  *  * \n"
			 "P2 *  *  *  *  *  *  *  *  * \n"
			 "P3 *  *  *  *  *  *  *  *  * \n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  * +FU *  *  *  *  *  * \n"
			 "P7+FU+FU-UM+FU+FU *  *  *  * \n"
			 "P8 * +GI+KI *  *  *  *  *  * \n"
			 "P9+KY+KE * +OU *  *  *  *  * \n"
			 "P+00AL\n"
			 "+\n").getInitialState());  
  RangeSelector range(5,6,9,9,false);
  Relations cur;
  relationsInArea(state, range, cur);
  std::cout << "\ntestfig2 " << cur.val() << "\n";

  const Move m77gi(POS(8,8),POS(7,7),SILVER,PBISHOP,false,BLACK);
  showMove(cur, state, m77gi, range);

  const Move m77ki(POS(7,8),POS(7,7),GOLD,PBISHOP,false,BLACK);
  showMove(cur, state, m77ki, range);

  const Move m77ke(POS(8,9),POS(7,7),KNIGHT,PBISHOP,false,BLACK);
  showMove(cur, state, m77ke, range);
}

void testfig3common(const char *title, const RangeSelector& range)
{
  PawnMaskState state(HIRATE);  
  Relations cur;
  relationsInArea(state, range, cur);
  std::cout << "\n" << title << " " << cur.val() << "\n";
  const Move m18hi(POS(2,8),POS(1,8),ROOK,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m18hi, range);

  const Move m26fu(POS(2,7),POS(2,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m26fu, range);

  ApplyMoveOfTurn::doMove(state, m18hi);
  state.changeTurn();
  cur.clear();
  relationsInArea(state, range, cur);
  
  const Move m28gi(POS(3,9),POS(2,8),SILVER,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m28gi, range);
}

void testfig3()
{
  RangeSelector range(1,6,4,9,false);
  testfig3common("testfig3", range);
}

void testfig3all()
{
  RangeSelector range(1,1,9,9,false);
  testfig3common("testfig3all", range);
}

void testfig4()
{
  PawnMaskState state(CsaString(
			 "P1-OU *  *  *  *  *  *  *  * \n"
			 "P2 *  *  *  *  *  *  *  *  * \n"
			 "P3 *  *  *  *  *  *  *  *  * \n"
			 "P4 *  *  *  *  *  *  *  *  * \n"
			 "P5 *  *  *  *  *  *  *  *  * \n"
			 "P6 *  *  *  *  *  *  *  * +FU\n"
			 "P7 *  *  *  * +FU+FU+FU+FU * \n"
			 "P8 *  *  *  * +KI * +GI+OU * \n"
			 "P9 *  *  *  *  * +KI * +KE+KY\n"
			 "P+00AL\n"
			 "+\n").getInitialState());  
  RangeSelector range(1,6,5,9,false);
  Relations cur;
  relationsInArea(state, range, cur);
  std::cout << "\ntestfig4 "  << cur.val() << "\n";

  const Move m39ou(POS(2,8),POS(3,9),KING,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m39ou, range);

  const Move m26fu(POS(2,7),POS(2,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m26fu, range);

  const Move m56fu(POS(5,7),POS(5,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m56fu, range);

  const Move m46fu(POS(4,7),POS(4,6),PAWN,PTYPE_EMPTY,false,BLACK);
  showMove(cur, state, m46fu, range);
}

void test1()
{
  PawnMaskState state(CsaString(
			 "P1-KY-KE *  *  *  * -KI-KE-OU\n"
			 "P2 * -HI *  *  *  * -KI-GI-KY\n"
			 "P3 *  * +NG *  * -FU * -FU-FU\n"
			 "P4-FU *  * -FU-FU *  *  *  * \n"
			 "P5+KA-FU *  * +GI * -KA *  * \n"
			 "P6 *  * -FU+FU *  *  *  * +FU\n"
			 "P7+FU+FU *  * +FU+FU * +FU * \n"
			 "P8 *  *  * +HI * +KI * +GI+KY\n"
			 "P9+KY+KE *  *  *  * +KI+KE+OU\n"
			 "P+00FU00FU\n"
			 "P-00FU\n"
			 "-\n").getInitialState());  
  RangeSelector allRange(1,1,9,9,true);
  Relations cur;
  relationsInArea(state, allRange, cur);
  std::cout << "\ntest1 " << cur.val() << "\n";

  const Move m92hi(POS(8,2),POS(9,2),ROOK,PTYPE_EMPTY,false,WHITE);
  // 結果は同じ std::cout << PiecePairEval::diffWithMove(state, m92hi) << "\n";
  showMove(cur, state, m92hi, allRange);
  
  const Move m42hi(POS(8,2),POS(4,2),ROOK,PTYPE_EMPTY,false,WHITE);
  showMove(cur, state, m42hi, allRange);

  // hot move なので参考記録
  const Move m38fu(POS(3,8),PAWN,WHITE);
  showMove(cur, state, m38fu, allRange);
}


int main(int argc, char **argv)
{
  nice(20);

  const char *program_name = argv[0];
  bool error_flag = false;
  extern char *optarg;
  extern int optind;
  char c;

  const char *input_filename = 0;
  while ((c = getopt(argc, argv, "w:vh")) != EOF)
  {
    switch(c)
    {
    case 'w':   input_filename = optarg;
      break;
    case 'v':   verbose = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag || (! input_filename))
    usage(program_name);

  const bool success = PiecePairRawEval::setUp(input_filename);
  assert(success);
  if (! success)
    abort();
  if (! verbose)
    testfig1();
  testfig2();
  if (! verbose)
  {
    testfig3();
    testfig3all();
    testfig4();
    // test1();
  }
  return 0;
}
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
