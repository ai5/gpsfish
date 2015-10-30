/* FixedCapacityVector.t.cc
 */
#include "osl/container.h"
#include <boost/test/unit_test.hpp>

#include <set>

using namespace osl;

BOOST_AUTO_TEST_CASE(FixedCapacityVectorTestPushBack)
{
  FixedCapacityVector<int,6> f123;
  f123.push_back(1); f123.push_back(2); f123.push_back(3);

  FixedCapacityVector<int,3> f456;
  f456.push_back(4); f456.push_back(5); f456.push_back(6);
  
  f123.push_back(f456.begin(), f456.end());
  BOOST_CHECK_EQUAL((size_t)6, f123.size());
  for (int i=0; i<6; ++i)
    BOOST_CHECK_EQUAL(f123[i], i+1);
}

BOOST_AUTO_TEST_CASE(FixedCapacityVectorTestUniq)
{
  std::multiset<int> s;
  FixedCapacityVector<int,1000> v;
  for (size_t i=0; i<700; ++i)
  {
    const int e = random() % 128;
    s.insert(e);
    v.push_back(e);
  }
  BOOST_CHECK_EQUAL(s.size(), v.size());
  std::set<int> u(s.begin(), s.end());
  BOOST_CHECK(u.size() < v.size());
  v.unique();
  BOOST_CHECK_EQUAL(u.size(), v.size());
  std::sort(v.begin(), v.end());
  BOOST_CHECK(std::equal(u.begin(), u.end(), v.begin()));
}

BOOST_AUTO_TEST_CASE(FixedCapacityVectorTestErase)
{
  FixedCapacityVector<int,4> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);
  v.push_back(1);
  BOOST_CHECK_EQUAL((size_t)4, v.size());
  v.erase(1);
  BOOST_CHECK_EQUAL((size_t)2, v.size());
  BOOST_CHECK_EQUAL(2, v[0]);
  BOOST_CHECK_EQUAL(3, v[1]);
  v.erase(3);
  BOOST_CHECK_EQUAL((size_t)1, v.size());
  v.erase(2);
  BOOST_CHECK_EQUAL((size_t)0, v.size());
}

struct Counting
{
  static int count;
  Counting() { ++count; }
  Counting(const Counting&) { ++count; }
  ~Counting() { --count; }
};
int Counting::count = 0;

BOOST_AUTO_TEST_CASE(FixedCapacityVectorTestDestruct)
{
  BOOST_CHECK_EQUAL(0, Counting::count);
  {
    Counting object;
    BOOST_CHECK_EQUAL(1, Counting::count);
  }
  BOOST_CHECK_EQUAL(0, Counting::count);

  {
    int64_t storage;
    BOOST_CHECK_EQUAL(0, Counting::count);
    misc::construct(reinterpret_cast<Counting*>(&storage), Counting());
    BOOST_CHECK_EQUAL(1, Counting::count);
    misc::destroy(reinterpret_cast<Counting*>(&storage));;
    BOOST_CHECK_EQUAL(0, Counting::count);
  }
  BOOST_CHECK_EQUAL(0, Counting::count);

  {
    FixedCapacityVector<Counting,4> v;
    Counting object;
    BOOST_CHECK_EQUAL(1, Counting::count);
    v.push_back(object);
    BOOST_CHECK_EQUAL(2, Counting::count);
    v.pop_back();
    BOOST_CHECK_EQUAL(1, Counting::count);
    v.push_back(object);
    v.push_back(object);
    BOOST_CHECK_EQUAL(3, Counting::count);
    v.clear();
    BOOST_CHECK_EQUAL(1, Counting::count);
  }
  BOOST_CHECK_EQUAL(0, Counting::count);
}

BOOST_AUTO_TEST_CASE(FixedCapacityVectorTestCopy)
{
  {
    FixedCapacityVector<int,2> v, v2;
    v.push_back(100);
    v.push_back(200);
    BOOST_CHECK(! (v == v2));
    v2 = v;
    BOOST_CHECK(v == v2);
  }
  {
    FixedCapacityVector<int,2> v, v2;
    v.push_back(100);
    v.push_back(200);
    v2.push_back(1);
    v2.push_back(2);
    BOOST_CHECK(! (v == v2));
    v2 = v;
    BOOST_CHECK(v == v2);
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
