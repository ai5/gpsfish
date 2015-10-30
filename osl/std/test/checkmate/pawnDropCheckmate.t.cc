/* pawnDropCheckmate.t.cc
 */
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/check_.h"
#include "osl/checkmate/dfpn.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <string>
#include <iostream>
#include <fstream>

using namespace osl;
using namespace osl::move_classifier;

static
bool isPawnCheckmate(const NumEffectState& sstate, Move m)
{
  using namespace osl::checkmate;
  // m は王手であること
  NumEffectState state(sstate);
  state.makeMove(m);

  DfpnTable table(m.player());
  const PathEncoding path(alt(m.player()));
  typedef Dfpn searcher_t;
  searcher_t searcher;
  searcher.setTable(&table);
  ProofDisproof pdp=searcher.hasEscapeMove(state,HashKey(state),path,1500,m);
  return (pdp == ProofDisproof::PawnCheckmate());
}

static
bool isPawnCheckmateTest(const NumEffectState& sstate, Move m)
{
  const Ptype ptype = m.ptype();
  const Square from = m.from();
  const Square to = m.to();
  if (m.player() == BLACK)
    return PawnDropCheckmate<BLACK>::isMember(sstate, ptype, from, to);
  else
    return PawnDropCheckmate<WHITE>::isMember(sstate, ptype, from, to);
}

BOOST_AUTO_TEST_CASE(PawnDropCheckmateTestBlock)
{
  const char *problem = 
    "P1-KY *  * -OU * +KA *  * -KY\n"
    "P2 * +GI *  * +HI * -KI *  * \n"
    "P3 *  * -KE * -FU *  *  *  * \n"
    "P4-FU *  * -KE-GI * -FU * -FU\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6+FU+FU+KI-FU *  * +FU * +FU\n"
    "P7 *  * +KE *  *  *  *  *  * \n"
    "P8 * +OU+GI *  *  *  *  *  * \n"
    "P9+KY *  *  *  *  *  * +KE+KY\n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n";
  NumEffectState state((CsaString(problem).initialState()));
  // 62 に歩を打っても飛車の利きが消えるため打歩詰ではない
  const Square target = Square(6,2);
  BOOST_CHECK((! PawnDropCheckmate<BLACK>::
		  isMember(state, PAWN, Square::STAND(), target)));
  const Move m = Move(target, PAWN, BLACK);
  BOOST_CHECK(! isPawnCheckmate(state, m));
}

BOOST_AUTO_TEST_CASE(PawnDropCheckmateTestKingExposure)
{
  const char *problem = 
    "P1-OU-KI * +HI *  *  *  *  * \n"
    "P2 *  *  *  *  *  *  *  *  * \n"
    "P3 * +TO *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  *  * \n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  *  *  *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  *  *  *  *  *  *  *  * \n"
    "P9 *  *  *  *  *  *  *  * +OU\n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n";
  NumEffectState sstate((CsaString(problem).initialState()));
  // 92歩は金でとれるが打歩詰
  const Square target = Square(9,2);
  const Move m = Move(target, PAWN, BLACK);
  BOOST_CHECK(isPawnCheckmate(sstate, m));
  BOOST_CHECK((PawnDropCheckmate<BLACK>::
		  isMember(sstate, PAWN, Square::STAND(), target)));
}

BOOST_AUTO_TEST_CASE(PawnDropCheckmateTestOne)
{
  const char *problem = 
    "P1 *  *  *  *  *  *  * -KI-KY\n"
    "P2 *  *  *  *  *  *  * -KE-OU\n"
    "P3 *  *  *  *  *  *  *  *  * \n"
    "P4 *  *  *  *  *  *  *  * +GI\n"
    "P5 *  *  *  *  *  *  *  *  * \n"
    "P6 *  *  * -KI *  *  *  *  * \n"
    "P7 *  *  *  *  *  *  *  *  * \n"
    "P8 *  * +KY+OU+KE *  *  *  * \n"
    "P9-RY *  *  *  *  *  *  *  * \n"
    "P+00FU\n"
    "P-00AL\n"
    "+\n";
  NumEffectState sstate((CsaString(problem).initialState()));
  BOOST_CHECK((PawnDropCheckmate<BLACK>::
		  isMember(sstate, PAWN, Square::STAND(), Square(1,3))));
  const Move m13fu = Move(Square(1,3), PAWN, BLACK);
  BOOST_CHECK(isPawnCheckmate(sstate, m13fu));
  BOOST_CHECK((! PawnDropCheckmate<BLACK>::
		  isMember(sstate, LANCE, Square::STAND(), Square(1,3))));
  BOOST_CHECK((! PawnDropCheckmate<WHITE>::
		  isMember(sstate, PAWN, Square::STAND(), Square(1,3))));

  BOOST_CHECK((PawnDropCheckmate<WHITE>::
		  isMember(sstate, PAWN, Square::STAND(), Square(6,7))));
  BOOST_CHECK((! PawnDropCheckmate<WHITE>::
		  isMember(sstate, LANCE, Square::STAND(), Square(6,7))));
  BOOST_CHECK((! PawnDropCheckmate<BLACK>::
		  isMember(sstate, PAWN, Square::STAND(), Square(6,7))));
}

static void testFile(const std::string& filename)
{
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves = record.moves;
  for (size_t i=0; i<moves.size(); i++)
  {
    state.makeMove(moves[i]);
    const Player turn = state.turn();
    if (state.inCheck())
      break;

    const Square target = state.kingSquare(alt(turn));
    const Square drop_to 
      = Board_Table.nextSquare(turn, target, D);
    if (! drop_to.isOnBoard())
      continue;
    const Move m = Move(drop_to, PAWN, turn);
    assert(m.isValid());
    BOOST_CHECK(PlayerMoveAdaptor<Check>::isMember(state, m));
    if (state.isAlmostValidMove<false>(m))
    {
      BOOST_CHECK_EQUAL(isPawnCheckmate(state, m),
			   isPawnCheckmateTest(state, m));
    }
  }
}

BOOST_AUTO_TEST_CASE(PawnDropCheckmateTestFile)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=1000;
  if (OslConfig::inUnitTestShort()) 
    count=50;
  std::string fileName;
  while((ifs >> fileName) && (++i<count))
  {
    if(fileName == "") 
      break;
    testFile(OslConfig::testCsaFile(fileName));
    std::cerr << '.';
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
