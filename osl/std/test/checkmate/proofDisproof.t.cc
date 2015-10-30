#include "osl/checkmate/proofDisproof.h"
#include <boost/test/unit_test.hpp>
using namespace osl;
using namespace osl::checkmate;

BOOST_AUTO_TEST_CASE(ProofDisproofTestOrder)
{
  BOOST_CHECK(ProofDisproof::PawnCheckmate().isBetterForAttack(ProofDisproof::NoCheckmate()));
  BOOST_CHECK(ProofDisproof::LoopDetection().isBetterForAttack(ProofDisproof::PawnCheckmate()));
  BOOST_CHECK(ProofDisproof::Unknown ().isBetterForAttack(ProofDisproof::LoopDetection()));
  BOOST_CHECK(ProofDisproof::NoEscape     ().isBetterForAttack(ProofDisproof::Unknown()));
  // (1) 下の(2)との両立が難しいのでアルゴリズムで対処する
  // SafeMoveFilter 導入したので大丈夫のはず?
  // BOOST_CHECK(isBetterForAttack(Checkmate, NoEscape));

  BOOST_CHECK(ProofDisproof::NoCheckmate  ().isBetterForDefense(ProofDisproof::PawnCheckmate()));
  BOOST_CHECK(ProofDisproof::PawnCheckmate().isBetterForDefense(ProofDisproof::LoopDetection()));
  BOOST_CHECK(ProofDisproof::LoopDetection().isBetterForDefense(ProofDisproof::Unknown()));
  BOOST_CHECK(ProofDisproof::Unknown      ().isBetterForDefense(ProofDisproof::Checkmate()));
  // (2) NoEscape の手はルール上指せないので Checkmate の方が好ましいとしておく
  BOOST_CHECK(ProofDisproof::Checkmate().isBetterForDefense(ProofDisproof::NoEscape()));

  BOOST_CHECK(ProofDisproof::NoCheckmate().isBetterForAttack(ProofDisproof::Bottom()));
  BOOST_CHECK(ProofDisproof::Checkmate  ().isBetterForAttack(ProofDisproof::Bottom()));

  BOOST_CHECK(ProofDisproof::NoCheckmate().isBetterForDefense(ProofDisproof::Bottom()));
  BOOST_CHECK(ProofDisproof::Checkmate  ().isBetterForDefense(ProofDisproof::Bottom()));
}

BOOST_AUTO_TEST_CASE(ProofDisproofTestFinal)
{
  BOOST_CHECK(ProofDisproof::NoCheckmate().isFinal());
  BOOST_CHECK(ProofDisproof::Checkmate  ().isFinal());

  BOOST_CHECK(ProofDisproof::PawnCheckmate().proof() > 0);
  BOOST_CHECK(ProofDisproof::PawnCheckmate().disproof() == 0);
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
