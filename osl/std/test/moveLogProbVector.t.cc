/* moveLogProbVector.t.cc
 */
#include "osl/container/moveLogProbVector.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/move_action.h"
#include "osl/csa.h"
#include "osl/numEffectState.h"

#include <boost/test/unit_test.hpp>
#include <set>

using namespace osl;

static bool sameElements(const MoveLogProbVector& l, const MoveLogProbVector& r)
{
  typedef std::set<MoveLogProb> set_t;
  set_t s1(l.begin(), l.end());
  set_t s2(r.begin(), r.end());
  BOOST_CHECK_EQUAL(s1.size(), l.size());
  BOOST_CHECK_EQUAL(s2.size(), r.size());
  return s1 == s2;
}

static bool isSorted(const MoveLogProbVector& moves) 
{
  int limit = 8000;
  for (MoveLogProbVector::const_iterator p=moves.begin(); p!=moves.end(); ++p)
  {
    if (limit < p->logProb())
      return false;
    limit = p->logProb();
  }
  return true;
}

BOOST_AUTO_TEST_CASE(MoveLogProbVectorTestSortByProbability)
{
  using namespace move_action;
  using namespace move_generator;
  NumEffectState state((CsaString(
			  "P1+NY+TO *  *  *  * -OU-KE-KY\n"
			  "P2 *  *  *  *  * -GI-KI *  *\n"
			  "P3 * +RY *  * +UM * -KI-FU-FU\n"
			  "P4 *  * +FU-FU *  *  *  *  *\n"
			  "P5 *  * -KE * +FU *  * +FU *\n"
			  "P6-KE *  * +FU+GI-FU *  * +FU\n"
			  "P7 *  * -UM *  *  *  *  *  *\n"
			  "P8 *  *  *  *  *  *  *  *  * \n"
			  "P9 * +OU * -GI *  *  *  * -NG\n"
			  "P+00HI00KI00KE00KY00FU00FU00FU00FU00FU00FU\n"
			  "P-00KI00KY00FU00FU\n"
			  "P-00AL\n"
			  "+\n").initialState()));

  MoveVector moves;
  {
    Store store(moves);
    AllMoves<Store>::generate(BLACK, state, store);
  }

  MoveLogProbVector v;
  MoveLogProbVector v2;
  for (auto p=moves.begin(); p!=moves.end(); ++p)
  {
    v.push_back(*p, 100);
    v2.push_back(*p, 100);
  }
  BOOST_CHECK(v.size() > 100);
  BOOST_CHECK_EQUAL(v.size(), v2.size());

  // ごちゃごちゃしても指手が消えたりしないことの確認
  BOOST_CHECK(sameElements(v, v2));
  v.sortByProbability();
  assert(isSorted(v));
  BOOST_CHECK(sameElements(v, v2));
  v.sortByProbability();
  BOOST_CHECK(sameElements(v, v2));
  v.sortByProbabilityReverse();
  BOOST_CHECK(sameElements(v, v2));
  v.sortByProbability();
  assert(isSorted(v));
  BOOST_CHECK(sameElements(v, v2));
  v.sortByProbabilityReverse();
  BOOST_CHECK(sameElements(v, v2));
  v2.sortByProbabilityReverse();
  BOOST_CHECK(sameElements(v, v2));
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
