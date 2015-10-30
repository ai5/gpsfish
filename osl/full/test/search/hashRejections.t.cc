#include "osl/search/hashRejections.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(HashRejectionsTestProbe)
{
  HashRejections table;
  NumEffectState state;
  HashKey key(state);
  Move good_move(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, BLACK);
  Move another_move(Square(1,9), Square(1,8), LANCE, PTYPE_EMPTY, false, BLACK);

  table.addRejectionRoot(state, key, good_move);
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(good_move), key));
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(another_move), key));
  BOOST_CHECK(table.rejectionProbe(key.newHashWithMove(another_move), HashKey()));

  table.clearRejectionRoot(state, key, good_move);
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(good_move), key));
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(another_move), key));
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(another_move), HashKey()));

  table.addRejection(state, key, good_move);
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(good_move), key));
  BOOST_CHECK(table.rejectionProbe(key.newHashWithMove(another_move), key));
  BOOST_CHECK(table.rejectionProbe(key.newHashWithMove(another_move), HashKey()));

  table.clearRejection(state, key, good_move);
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(good_move), key));
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(another_move), key));
  BOOST_CHECK(! table.rejectionProbe(key.newHashWithMove(another_move), HashKey()));
}

BOOST_AUTO_TEST_CASE(HashRejectionsTestCopy)
{
  HashRejections table;
  NumEffectState state;
  HashKey key(state);
  Move good_move(Square(7,7), Square(7,6), PAWN, PTYPE_EMPTY, false, BLACK);
  Move another_move(Square(1,9), Square(1,8), LANCE, PTYPE_EMPTY, false, BLACK);
  table.addRejectionRoot(state, key, good_move);

  HashRejections table2 = table;
  BOOST_CHECK(table2.rejectionProbe(key.newHashWithMove(another_move), HashKey()));

  table.addRejection(state, key, good_move);

  HashRejections table3;
  table3 = table;
  BOOST_CHECK(table3.rejectionProbe(key.newHashWithMove(another_move), key));
  BOOST_CHECK(table3.rejectionProbe(key.newHashWithMove(another_move), HashKey()));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
