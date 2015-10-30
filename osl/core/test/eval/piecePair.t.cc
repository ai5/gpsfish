/* piecePair.t.cc
 */
#include "osl/eval/piecePair.h"
#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace osl
{
  const CArray<int,3> operator-(const CArray<int,3>& a) 
  {
    CArray<int,3> ret = {{ -a[0], -a[1], -a[2], }};
    return ret;
  }
  std::ostream& operator<<(std::ostream& os, const CArray<int,3>& a)
  {
    return os << '[' << a[0] << ',' << a[1] << ',' << a[2] << ']';
  }
}

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::ml;

static bool tableFilledSame(const PiecePair::IndexTable& table)
{
  for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
    for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
      for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	const int pi0  = PiecePair::IndexTable::pindex(BLACK, p0), pi1  = PiecePair::IndexTable::pindex(BLACK, p1);
	const int pi0w = PiecePair::IndexTable::pindex(WHITE, p0), pi1w = PiecePair::IndexTable::pindex(WHITE, p1);
	if (! (table[d][pi0][pi1] && table[d][pi0w][pi1w])) {
	  return false;
	}
      }
    }
  }
  return true;
}
static bool tableFilled(const PiecePair::IndexTable& table)
{
  for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
    for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
      for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	const int pi0  = PiecePair::IndexTable::pindex(BLACK, p0), pi1  = PiecePair::IndexTable::pindex(BLACK, p1);
	const int pi0w = PiecePair::IndexTable::pindex(WHITE, p0), pi1w = PiecePair::IndexTable::pindex(WHITE, p1);
	if (! (table[d][pi0][pi1] && table[d][pi0w][pi1w]
	       && table[d][pi0][pi1w] && table[d][pi0w][pi1])) {
	  return false;
	}
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(PiecePairTestFilled)
{
  PiecePair::init();
  
  BOOST_CHECK(tableFilled(PiecePair::plain_table));
  for (int x=1; x<=9; ++x) {
    BOOST_CHECK(tableFilledSame(PiecePair::x_table[x]));
  }
  for (int y=1; y<=9; ++y) {
    BOOST_CHECK(tableFilledSame(PiecePair::y_table[y]));
  }
}

bool DoPiecePairTestCover(const PiecePair::IndexTable& table, bool has_different)
{
  // ptypeが違う->index違う
  for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
    for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
      for (int ip2=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
	if (ip1 == ip2)
	  continue;
	for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	  const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1), p2 = static_cast<Ptype>(ip2);
	  const int pi0  = PiecePair::IndexTable::pindex(BLACK, p0), pi1  = PiecePair::IndexTable::pindex(BLACK, p1);
	  const int pi0w = PiecePair::IndexTable::pindex(WHITE, p0), pi1w = PiecePair::IndexTable::pindex(WHITE, p1);
	  const int pi2  = PiecePair::IndexTable::pindex(BLACK, p2);
	  const int pi2w = PiecePair::IndexTable::pindex(WHITE, p2);
	  if (table[d][pi0][pi1] == table[d][pi0][pi2]
	      || table[d][pi0w][pi1w] == table[d][pi0w][pi2w])
	    return false;
	  if (has_different
	      && (table[d][pi0][pi1w] == table[d][pi0][pi2w]
		  || table[d][pi0w][pi1] == table[d][pi0w][pi2]))
	    return false;
	}
      }
    }
  }
  // direction が1or3違う
  for (int ip0=PTYPE_PIECE_MIN; ip0<=PTYPE_MAX; ++ip0) {
    for (int ip1=PTYPE_PIECE_MIN; ip1<=PTYPE_MAX; ++ip1) {
      for (size_t d=0; d<PiecePair::offsets.size(); ++d) {
	const Ptype p0 = static_cast<Ptype>(ip0), p1 = static_cast<Ptype>(ip1);
	const int pi0  = PiecePair::IndexTable::pindex(BLACK, p0), pi1  = PiecePair::IndexTable::pindex(BLACK, p1);
	const int pi0w = PiecePair::IndexTable::pindex(WHITE, p0), pi1w = PiecePair::IndexTable::pindex(WHITE, p1);
	const size_t d1 = (d+1)%12;
	const size_t d2 = (d+3)%12;
	if (table[d][pi0][pi1] == table[d1][pi0][pi1]
	    || table[d][pi0w][pi1w] == table[d1][pi0w][pi1w])
	  return false;
	if (has_different
	    && (table[d][pi0w][pi1] == table[d1][pi0w][pi1]
		|| table[d][pi0][pi1w] == table[d1][pi0][pi1w]))
	  return false;
	if (table[d][pi0][pi1] == table[d2][pi0][pi1]
	    || table[d][pi0w][pi1w] == table[d2][pi0w][pi1w])
	  return false;
	if (has_different
	    && (table[d][pi0w][pi1] == table[d2][pi0w][pi1]
		|| table[d][pi0][pi1w] == table[d2][pi0][pi1w]))
	  return false;
      }
    }
  }
  return true;
}

BOOST_AUTO_TEST_CASE(PiecePairTestCover)
{
  BOOST_CHECK(DoPiecePairTestCover(PiecePair::plain_table, true));
  for (int i=1; i<=9; ++i)
    BOOST_CHECK(DoPiecePairTestCover(PiecePair::x_table[i], false));
}

BOOST_AUTO_TEST_CASE(PiecePairTestIndex)
{
  PiecePair::init();
  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(BLACK,GOLD)),
		       PiecePair::index(3, Square(3,7), newPtypeO(BLACK,PAWN), 
					Square(2,6), newPtypeO(BLACK,GOLD)));
  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(BLACK,GOLD)),
		       PiecePair::index(9, Square(2,6), newPtypeO(BLACK,GOLD),
					Square(3,7), newPtypeO(BLACK,PAWN)));

  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(BLACK,GOLD))[1],
		       PiecePair::index(3, Square(3,7), newPtypeO(BLACK,PAWN), 
					Square(2,6), newPtypeO(BLACK,GOLD))[1]);
  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(BLACK,GOLD)),
		       -PiecePair::index(1, Square(3,3), newPtypeO(WHITE,PAWN), 
					 Square(2,4), newPtypeO(WHITE,GOLD)));
  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(BLACK,GOLD)),
		       -PiecePair::index(9, Square(7,3), newPtypeO(WHITE,PAWN), 
					 Square(8,4), newPtypeO(WHITE,GOLD)));
  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(BLACK,GOLD)),
		       -PiecePair::index(3, Square(8,4), newPtypeO(WHITE,GOLD), 
					 Square(7,3), newPtypeO(WHITE,PAWN)));

  BOOST_CHECK_EQUAL(PiecePair::index(7, Square(7,7), newPtypeO(BLACK,PAWN), 
					Square(8,6), newPtypeO(WHITE,GOLD)),
		       PiecePair::index(1, Square(8,6), newPtypeO(WHITE,GOLD),
					Square(7,7), newPtypeO(BLACK,PAWN)));

  BOOST_CHECK_EQUAL(PiecePair::index(11, Square(8,6), newPtypeO(WHITE,PAWN), 
					Square(8,7), newPtypeO(BLACK,PAWN)),
		       PiecePair::index(11, Square(2,3), newPtypeO(WHITE,PAWN),
					Square(2,4), newPtypeO(BLACK,PAWN)));
  BOOST_CHECK_EQUAL(PiecePair::index(11, Square(8,6), newPtypeO(WHITE,PAWN), 
					Square(8,7), newPtypeO(BLACK,PAWN)),
		       PiecePair::index(5, Square(2,4), newPtypeO(BLACK,PAWN),
					Square(2,3), newPtypeO(WHITE,PAWN)));

  BOOST_CHECK_EQUAL(PiecePair::index(6, Square(7,8), newPtypeO(BLACK,GOLD), 
					Square(8,6), newPtypeO(WHITE,PAWN)),
		       -PiecePair::index(6, Square(2,4), newPtypeO(BLACK,PAWN),
					Square(3,2), newPtypeO(WHITE,GOLD)));
}

BOOST_AUTO_TEST_CASE(PiecePairTestSymmetry)
{
  PiecePair::init();
  Weights values(PiecePair::DIM);
  values.setRandom();

  PiecePair::sanitize(values);  
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU * -FU-FU-FU-FU * -FU-FU\n"
			   "P4 * -FU *  *  *  * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  * +FU * \n"
			   "P7+FU+FU * +FU+FU+FU+FU * +FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "-\n").initialState());
    BOOST_CHECK_EQUAL(PiecePair::pieceValue(state, state.pieceAt(Square(7,6)), values),
			 - PiecePair::pieceValue(state, state.pieceAt(Square(3,4)), values));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			   "P2 * -HI *  *  *  * -KI-KA * \n"
			   "P3-FU * -FU-FU-FU-FU * -FU-FU\n"
			   "P4 *  *  *  *  *  * -FU+FU * \n"
			   "P5 * -FU *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU+FU * +FU+FU+FU+FU * +FU\n"
			   "P8 * +KA+KI *  *  *  * +HI * \n"
			   "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
			   "-\n").initialState());
    NumEffectState state_h(state.flipHorizontal());
    BOOST_CHECK_EQUAL(PiecePair::pieceValue(state, state.pieceAt(Square(2,4)), values),
			 PiecePair::pieceValue(state_h, state_h.pieceAt(Square(8,4)), values));    
  }

  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			   "P2 * -HI *  *  *  *  * -KA * \n"
			   "P3-FU-FU-FU-FU-FU-FU * -FU-FU\n"
			   "P4 *  *  *  *  *  * -FU *  * \n"
			   "P5 *  *  *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  * +FU * \n"
			   "P7+FU+FU * +FU+FU+FU+FU * +FU\n"
			   "P8 * +KA *  *  *  *  * +HI * \n"
			   "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			   "-\n").initialState());
    NumEffectState state_r(CsaString(
			     "P1-KY-KE-GI-KI-OU-KI-GI-KE-KY\n"
			     "P2 * -HI *  *  *  *  * -KA * \n"
			     "P3-FU * -FU-FU-FU-FU * -FU-FU\n"
			     "P4 * -FU *  *  *  * -FU *  * \n"
			     "P5 *  *  *  *  *  *  *  *  * \n"
			     "P6 *  * +FU *  *  *  *  *  * \n"
			     "P7+FU+FU * +FU+FU+FU+FU+FU+FU\n"
			     "P8 * +KA *  *  *  *  * +HI * \n"
			     "P9+KY+KE+GI+KI+OU+KI+GI+KE+KY\n"
			     "+\n").initialState());
    BOOST_CHECK_EQUAL(PiecePair::pieceValue(state, state.pieceAt(Square(3,4)), values),
			 - PiecePair::pieceValue(state_r, state.pieceAt(Square(7,6)), values));
  }
  {
    NumEffectState state(CsaString(
			   "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			   "P2 * -HI *  *  *  * -KI-KA * \n"
			   "P3-FU * -FU-FU-FU-FU * -FU-FU\n"
			   "P4 *  *  *  *  *  * -FU+FU * \n"
			   "P5 * -FU *  *  *  *  *  *  * \n"
			   "P6 *  * +FU *  *  *  *  *  * \n"
			   "P7+FU+FU * +FU+FU+FU+FU * +FU\n"
			   "P8 * +KA+KI *  *  *  * +HI * \n"
			   "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
			   "-\n").initialState());
    NumEffectState state_r(CsaString(
			     "P1-KY-KE-GI-KI-OU * -GI-KE-KY\n"
			     "P2 * -HI *  *  *  * -KI-KA * \n"
			     "P3-FU * -FU-FU-FU-FU * -FU-FU\n"
			     "P4 *  *  *  *  *  * -FU *  * \n"
			     "P5 *  *  *  *  *  *  * +FU * \n"
			     "P6 * -FU+FU *  *  *  *  *  * \n"
			     "P7+FU+FU * +FU+FU+FU+FU * +FU\n"
			     "P8 * +KA+KI *  *  *  * +HI * \n"
			     "P9+KY+KE+GI * +OU+KI+GI+KE+KY\n"
			     "+\n").initialState());
    BOOST_CHECK_EQUAL(PiecePair::pieceValue(state, state.pieceAt(Square(2,4)), values),
			 - PiecePair::pieceValue(state_r, state_r.pieceAt(Square(8,6)), values));
  }
  
}

void showEvalSummary(const NumEffectState& state, const Weights& values)
{
  for (int y=1; y<=9; ++y) {
    for (int x=9; x>=1; --x) {
      const Square position(x,y);
      const Piece piece = state.pieceOnBoard(position);
      std::cerr << std::setw(4) << PiecePair::pieceValue(state, piece, values);
    }
    std::cerr << "\n";
  }
}

BOOST_AUTO_TEST_CASE(PiecePairTestSymmetryFile)
{
  PiecePair::init();
  Weights values(PiecePair::DIM);
  values.setRandom();

  PiecePair::sanitize(values);

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 200) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();

    NumEffectState state(record.initial_state);
    for (auto m:record.moves) {
      state.makeMove(m);

      NumEffectState state_r(state.rotate180());
      if (PiecePair::eval(state, values) != -PiecePair::eval(state_r, values))
      {
	std::cerr << state; showEvalSummary(state, values);
	std::cerr << state_r; showEvalSummary(state_r, values);
	std::cerr << "\n";
      }
      BOOST_CHECK(PiecePair::eval(state, values) == -PiecePair::eval(state_r, values));
#if 1
      NumEffectState state_h(state.flipHorizontal());
      if (PiecePair::eval(state, values) != PiecePair::eval(state_h, values))
      {
	std::cerr << state; showEvalSummary(state, values);
	std::cerr << state_h; showEvalSummary(state_h, values);
	std::cerr << "\n";
      }
      BOOST_CHECK(PiecePair::eval(state, values) == PiecePair::eval(state_h, values));
#endif
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
