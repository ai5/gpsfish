#include "osl/move_order/captureSort.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>
#include <string>
#include <fstream>
#include <iostream>
#include <set>

using namespace osl;
using namespace osl::move_generator;
using namespace osl::move_action;

static bool isSorted(const MoveVector& moves)
{
  MoveVector::const_iterator p=moves.begin();
  for (;(p!=moves.end()) && (p->capturePtype() != PTYPE_EMPTY); ++p)
    ;
  // 一度 取らない手が表れたら，もう取る手はない
  for (; p!=moves.end(); ++p)
  {
    if (p->capturePtype() != PTYPE_EMPTY)
    {
      std::cerr << moves;
      return false;
    }
  }
  return true;
}

static bool hasSameMember(const MoveVector& l, const MoveVector& r)
{
  typedef std::set<Move> set_t;
  set_t ls(l.begin(), l.end()), rs(r.begin(), r.end());
  return ls == rs;
}

static void testOrder(const std::string& filename)
{
  auto record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initialState());
  const auto& moves=record.moves;
  for (unsigned int i=0;i<moves.size();i++)
  {
    MoveVector curMoves;
    {
      Store action(curMoves);
      AllMoves<Store>::generate(state.turn(),state,action);
    }
    curMoves.unique();

    MoveVector sorted = curMoves;
    CaptureSort::sort(sorted.begin(), sorted.end());
    BOOST_CHECK(isSorted(sorted));
    BOOST_CHECK(hasSameMember(sorted, curMoves));

    const Move move=moves[i];
    state.makeMove(move);
  }
}

BOOST_AUTO_TEST_CASE(CaptureSortTestOrder){
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  std::string filename;
  while((ifs >> filename) && ++i<10){
    if(filename == "") break;
    testOrder(std::string(OslConfig::testCsaFile(filename)));
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
