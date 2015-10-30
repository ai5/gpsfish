#include "osl/eval/endgame/kingPieceTable.h"
#include <boost/test/unit_test.hpp>

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::endgame;

class KingPieceTableA : public KingPieceTable
{
public:
  KingPieceTableA()
  {
  }
};

BOOST_AUTO_TEST_CASE(KingPieceTableTestLoad)
{
  std::unique_ptr<KingPieceTableA> t1(new KingPieceTableA),
    t2(new KingPieceTableA);
  t1->randomize();
  BOOST_CHECK(! (*t1 == *t2));
  t1->saveText("KingPieceTableTest.txt");
  t2->loadText("KingPieceTableTest.txt");
  BOOST_CHECK(*t1 == *t2);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
