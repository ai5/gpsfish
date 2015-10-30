/* eval_mobilityTable.cc
 */
#include "osl/eval/mobilityTable.h"
namespace osl
{
  const CArray<int, 9> eval::MobilityTable::rookVertical = { {
      -61,-43,-22,-9,-2,9,12,17,17,
    } };

  const CArray<int, 9> eval::MobilityTable::rookHorizontal={{
      -72,-47,-18,4,10,28,21,27,27,
    }};
  const CArray<int, 9> eval::MobilityTable::prookVertical={{
      -45,-26,-17,-13,-1,1,10,8,8,
    }};
  const CArray<int, 9> eval::MobilityTable::prookHorizontal={{
      -35,-23,-16,-10,-4,4,11,12,12,
    }};
  const CArray<int, 17> eval::MobilityTable::bishop={{
      -60,-30,-6,2,4,8,10,12,
      12,13,15,17,19,21,23,25,27
    }};
  const CArray<int, 17> eval::MobilityTable::pbishop={{
      -39,-30,-9,-6,-2,2,0,5,
      13,15,17,19,21,23,25,27,29
    }};
  const CArray<int, 9> eval::MobilityTable::lance={{
      -10,-7,2,6,18,25,27,24,24
    }};
}
