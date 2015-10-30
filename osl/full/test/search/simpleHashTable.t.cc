#include "osl/search/simpleHashTable.h"
#include "osl/search/simpleHashRecord.h"
#include "osl/moveLogProb.h"
#include "osl/hashKey.h"
#include <boost/test/unit_test.hpp>
using namespace osl;
using namespace osl::search;

BOOST_AUTO_TEST_CASE(SimpleHashTableTestCreation) {
  SimpleHashTable table(1,1,false);
  BOOST_CHECK_EQUAL(1, table.minimumRecordLimit());

  SimpleHashTable table2(3,-4,false);
  BOOST_CHECK_EQUAL(-4, table2.minimumRecordLimit());
}

BOOST_AUTO_TEST_CASE(SimpleHashTableTestSize) {
  SimpleHashTable table(1,1,false);
  BOOST_CHECK_EQUAL((size_t)0, table.size());
}

BOOST_AUTO_TEST_CASE(SimpleHashTableTestAllocate) {
  SimpleHashTable table(100,1,false);
  HashKey k;
  BOOST_CHECK_EQUAL((size_t)0, table.size());

  SimpleHashRecord *record = table.allocate(k, 100);
  BOOST_CHECK_EQUAL((size_t)1, table.size());
  BOOST_CHECK(! record->hasLowerBound(0));
  BOOST_CHECK(! record->hasUpperBound(0));
}

BOOST_AUTO_TEST_CASE(SimpleHashTableTestIsRecordedUpper) {
  SimpleHashTable table(100,1,false);
  MoveLogProb m1;
  const HashKey k;
  const int limit = 100;
  const int value = 8;
  {
    SimpleHashRecord *record = table.allocate(k, limit);
    record->setUpperBound(BLACK, limit, m1, value);
  }
  const SimpleHashRecord *record = table.find(k);
  BOOST_CHECK(record);
  BOOST_CHECK_EQUAL(value, record->upperBound());
}

BOOST_AUTO_TEST_CASE(SimpleHashTableTestIsRecordedLower) {
  SimpleHashTable table(100,1,false);
  MoveLogProb m1;
  const HashKey k;
  const int limit = 100;
  const int value = 8;
  {
    SimpleHashRecord *record = table.allocate(k, limit);
    record->setLowerBound(BLACK, limit, m1, value);
  }
  const SimpleHashRecord *record = table.find(k);
  BOOST_CHECK(record);
  BOOST_CHECK_EQUAL(value, record->lowerBound());
}

/** 
 * 2回登録したら深いほうの結果が残る 
 */
BOOST_AUTO_TEST_CASE(SimpleHashTableTestOverWrite) {
  const HashKey k;
  MoveLogProb m1;
  const int shallow_limit = 100;
  const int shallow_value = 8;
  const int deep_limit = 200;
  const int deep_value = 888;
  {
    // shallow -> deep
    SimpleHashTable table(100,1,false);
    SimpleHashRecord *record = table.allocate(k, shallow_limit);
    record->setLowerBound(BLACK, shallow_limit, m1, shallow_value);
    record->setLowerBound(BLACK, deep_limit, m1, deep_value);
    
    BOOST_CHECK(record->hasLowerBound(deep_limit));
    BOOST_CHECK_EQUAL(deep_value, record->lowerBound());
  }
  {
    // deep -> shallow
    SimpleHashTable table(100,1,false);
    SimpleHashRecord *record = table.allocate(k, shallow_limit);
    record->setLowerBound(BLACK, deep_limit, m1, deep_value);
    record->setLowerBound(BLACK, shallow_limit, m1, shallow_value);

    BOOST_CHECK(record->hasLowerBound(deep_limit));
    BOOST_CHECK_EQUAL(deep_value, record->lowerBound());
  }
}

/** capacity を越えて insert しない */
BOOST_AUTO_TEST_CASE(SimpleHashTableTestCapacity) {
  const size_t capacity = 128;
  SimpleHashTable table(capacity,1,false);
  BOOST_CHECK_EQUAL(capacity, table.capacity());
  int table_full = 0;
  
  for (unsigned int i=1; i<=capacity*5; ++i)
  {
    HashKey k;
    k.setRandom();
    const int limit = i;
    // const int value = i*2;
    if (table.size() < capacity)
    {
      try 
      {
	SimpleHashRecord *r = table.allocate(k, limit);
	BOOST_CHECK_EQUAL((size_t)i, table.size()+table_full);
	const SimpleHashRecord *record = table.find(k);
	// std::cerr << r << " " << record << "\n";
	BOOST_CHECK_EQUAL(const_cast<const SimpleHashRecord*>(r), record);
      }
      catch (TableFull&)
      {
	BOOST_CHECK(table.size() >= (capacity / table.divSize()));
	++table_full;
      }
    }
    else 
    {
      if (table.find(k) || (limit < table.minimumRecordLimit()))
      {
	// already recorded
	BOOST_CHECK_EQUAL(capacity, table.size());
	table.allocate(k, limit);
	BOOST_CHECK_EQUAL(capacity, table.size());
      }
      else
      {
	try
	{
	  BOOST_CHECK_EQUAL(capacity, table.size());
	  table.allocate(k, limit);
	  BOOST_CHECK(! table.find(k));
	  BOOST_CHECK_EQUAL(capacity, table.size());
	}
	catch (TableFull&)
	{
	  BOOST_CHECK(! table.find(k));
	  BOOST_CHECK_EQUAL(capacity, table.size());
	}
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(SimpleHashTableTestClear) {
  SimpleHashTable table(100,1,false);
  const HashKey k;
  BOOST_CHECK_EQUAL((size_t)0, table.size());
  table.allocate(k, 100);
  BOOST_CHECK_EQUAL((size_t)1, table.size());
  table.clear();
  BOOST_CHECK_EQUAL((size_t)0, table.size());
}

BOOST_AUTO_TEST_CASE(SimpleHashTableTestSetMinimumRecordLimit) {
  SimpleHashTable table(100,10000,false);
  const HashKey k;
  table.setMinimumRecordLimit(1000);
  table.allocate(k, 100);
  BOOST_CHECK_EQUAL((size_t)0, table.size());
  table.allocate(k, 10000);
  BOOST_CHECK_EQUAL((size_t)1, table.size());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
