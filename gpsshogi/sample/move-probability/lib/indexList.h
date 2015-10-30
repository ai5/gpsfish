/* indexList.h
 */
#ifndef GPSSHOGI_MOVE_PROBABILITY_INDEXLIST_H
#define GPSSHOGI_MOVE_PROBABILITY_INDEXLIST_H

#include "osl/container.h"
namespace gpsshogi
{
  typedef double weight_t;
  using namespace osl;
  struct IndexList
    : public FixedCapacityVector<std::pair<int,weight_t>,8000>
  {
  public:
    void add(int id, weight_t value) 
    {
      push_back(std::make_pair(id, value));
    }
  };
  typedef IndexList index_list_t;
}

#endif /* GPSSHOGI_MOVE_PROBABILITY_INDEXLIST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
