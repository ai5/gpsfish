/* checkMoveListProvider.cc
 */
#include "checkMoveListProvider.h"

osl::checkmate::
CheckMoveListProvider::CheckMoveListProvider()
  : cur(data.begin()), index(BucketSize)
{
}

osl::checkmate::
CheckMoveListProvider::~CheckMoveListProvider()
{
  clear();
}

void osl::checkmate::
CheckMoveListProvider::newBucket(size_t length) 
{
  data.push_front(new CheckMove[length]);
  cur = data.begin();
  index = 0;
}

void osl::checkmate::
CheckMoveListProvider::clear() 
{
  while (! data.empty()) {
    list_t::iterator p = data.begin();
    delete [] *p;
    data.pop_front();
  }

  index = BucketSize;
  cur = data.end();
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
