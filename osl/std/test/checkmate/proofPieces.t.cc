/* proofPieces.t.cc
 */
#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/dfpn.h"
#include "osl/checkmate/dfpnRecord.h"
#include "osl/checkmate/proofPieces.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;
using namespace osl::checkmate;
const size_t limit = 400000;

BOOST_AUTO_TEST_CASE(ProofPiecesTestLeaf) 
{
  {
    NumEffectState state(CsaString(
			    "P1 *  *  *  *  *  *  * -OU * \n"
			    "P2 *  *  *  *  *  *  * +KI * \n"
			    "P3 *  *  *  *  *  *  * +FU * \n"
			    "P4 *  *  *  *  *  *  *  *  * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 *  *  *  *  *  *  *  *  * \n"
			    "P7 *  *  *  *  *  *  *  *  * \n"
			    "P8 *  *  *  *  *  *  *  *  * \n"
			    "P9 *  * +OU *  *  *  *  *  * \n"
			    "P+00KI\n"
			    "P-00AL\n"
			    "-\n").initialState());
    const PieceStand proof_pieces
      = ProofPieces::leaf(state, BLACK, HashKey(state).blackStand());
    BOOST_CHECK_EQUAL(PieceStand(), proof_pieces);
  }
  {
    NumEffectState state(CsaString(
			    "P1 *  *  *  *  *  *  * -OU * \n"
			    "P2 *  *  *  *  *  *  * +KI * \n"
			    "P3 *  *  *  *  *  *  * +FU * \n"
			    "P4 *  *  *  *  *  *  *  *  * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 *  *  *  *  *  *  *  *  * \n"
			    "P7 *  *  *  *  *  *  *  *  * \n"
			    "P8 *  *  *  *  *  *  *  *  * \n"
			    "P9 *  * +OU *  *  *  *  *  * \n"
			    "P+00KI00KI00KI\n"
			    "P-00AL\n"
			    "-\n").initialState());
    const PieceStand proof_pieces 
      = ProofPieces::leaf(state, BLACK, HashKey(state).blackStand());
    BOOST_CHECK_EQUAL(PieceStand(), proof_pieces);
  }
  {
    // 合駒可能な状況の独占
    NumEffectState state(CsaString(
			    "P1 *  *  *  *  * +HI * -OU * \n"
			    "P2 *  *  *  *  *  *  *  *  * \n"
			    "P3 *  *  *  *  *  *  * +KI * \n"
			    "P4 *  *  *  *  *  *  *  *  * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 *  *  *  *  *  *  *  *  * \n"
			    "P7 *  *  *  *  *  *  *  *  * \n"
			    "P8 *  *  *  *  *  *  *  *  * \n"
			    "P9 *  * +OU *  *  *  *  *  * \n"
			    "P+00KI00KI00KI\n"
			    "P-00AL\n"
			    "-\n").initialState());
    const PieceStand proof_pieces 
      = ProofPieces::leaf(state, BLACK, HashKey(state).blackStand());
    PieceStand gold;
    gold.add(GOLD, 3);
    BOOST_CHECK_EQUAL(gold, proof_pieces);
  }
}

BOOST_AUTO_TEST_CASE(ProofPiecesTestAttack)
{
  NumEffectState state(CsaString(
			  "P1 *  *  *  *  *  * -KY * -KY\n"
			  "P2 *  *  *  *  *  *  * -OU * \n"
			  "P3 *  *  *  *  *  *  *  *  * \n"
			  "P4 *  *  *  *  *  *  * +FU * \n"
			  "P5 *  *  *  *  *  * +KE *  * \n"
			  "P6 *  *  *  *  *  *  *  *  * \n"
			  "P7 *  *  *  *  *  *  *  *  * \n"
			  "P8 *  *  *  *  *  *  *  *  * \n"
			  "P9 *  * +OU *  *  *  *  *  * \n"
			  "P+00KI\n"
			  "P-00AL\n"
			  "+\n").initialState());
  DualDfpn checker(limit);
  checker.setVerbose(OslConfig::verbose());
  Move check_move = Move::INVALID();
  const PathEncoding path(BLACK);
  const bool win = checker.isWinningState<BLACK>
    (100, state, HashKey(state), path, check_move);
  BOOST_CHECK_EQUAL(true, win);
  // +23{TO,NK} -21OU +22KI
  const DfpnTable& table = checker.table(BLACK);
  const DfpnRecord record = table.probe(HashKey(state), PieceStand(WHITE, state));
  PieceStand gold;
  gold.add(GOLD);
  BOOST_CHECK_EQUAL(gold, record.proofPieces());
}

BOOST_AUTO_TEST_CASE(ProofPiecesTestDefense)
{
  {
    NumEffectState state(CsaString(
			    "P1 *  *  *  *  *  *  *  *  * \n"
			    "P2 *  *  *  *  *  *  * -OU * \n"
			    "P3 *  *  *  *  *  *  * +KI * \n"
			    "P4 *  *  *  *  *  *  * +FU * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 *  *  *  *  *  *  *  *  * \n"
			    "P7 *  *  *  *  *  *  *  *  * \n"
			    "P8 *  *  *  *  *  *  *  *  * \n"
			    "P9 *  * +OU *  *  *  *  *  * \n"
			    "P+00KI\n"
			    "P-00AL\n"
			    "-\n").initialState());
    DualDfpn checker(limit);
    checker.setVerbose(OslConfig::verbose());
    const PathEncoding path(WHITE);
    const bool checkmate
      = checker.isLosingState<WHITE>(100, state, HashKey(state), path);
    BOOST_CHECK_EQUAL(true, checkmate);

    const DfpnTable& table = checker.table(BLACK);
    const DfpnRecord record = table.probe(HashKey(state), PieceStand(WHITE, state));
    PieceStand gold;
    gold.add(GOLD);
    BOOST_CHECK_EQUAL(gold, record.proofPieces());
  }
  {
    NumEffectState state(CsaString(
			    "P1 *  *  *  * -KY-OU-KI *  * \n"
			    "P2 *  *  *  *  *  * +GI *  * \n"
			    "P3 *  *  *  *  * +KI *  *  * \n"
			    "P4 *  *  *  *  *  *  *  *  * \n"
			    "P5 *  *  *  *  *  *  *  *  * \n"
			    "P6 *  *  *  *  *  *  *  *  * \n"
			    "P7 *  *  *  *  *  *  *  *  * \n"
			    "P8 *  *  *  *  *  *  *  *  * \n"
			    "P9 *  * +OU *  *  *  *  *  * \n"
			    "P+00KI00KE\n"
			    "P-00AL\n"
			    "-\n").initialState());
    DualDfpn checker(limit);
    checker.setVerbose(OslConfig::verbose());
    const PathEncoding path(WHITE);
    const bool checkmate
      = checker.isLosingState<WHITE>(100, state, HashKey(state), path);
    BOOST_CHECK_EQUAL(true, checkmate);

    const DfpnTable& table = checker.table(BLACK);
    const DfpnRecord record = table.probe(HashKey(state), PieceStand(WHITE,state));
    PieceStand gold_knight;
    gold_knight.add(GOLD);
    gold_knight.add(KNIGHT);
    BOOST_CHECK_EQUAL(gold_knight, record.proofPieces());
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
