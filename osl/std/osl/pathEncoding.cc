/* pathEncoding.cc
 */
#include "osl/pathEncoding.h"
#include "osl/random.h"
#include "osl/oslConfig.h"
#include <iostream>

osl::PathEncodingTable osl::Path_Encoding_Table;

namespace
{
  namespace PathEncoding {
    osl::SetUpRegister _initializer([](){ osl::Path_Encoding_Table.init(); });
  }
}

void osl::PathEncodingTable::
init()
{
  for (size_t i=0; i<MaxEncodingLength; ++i)
  {
    for (size_t j=0; j<Square::SIZE; ++j)
    {
      for (int k=0; k<PTYPE_SIZE; ++k)
      {
	const unsigned long long h = random();
	const unsigned int l = random();
	assert(l);
	assert(h << 32);
	// 手番を表現するため下位1bitをあけておく
	values[i][j][k] = (h << 32) + (l & (~1u));
      }
    }
  }
}

#if (!defined MINIMAL ) || (defined DFPNSTATONE)
std::ostream& osl::operator<<(std::ostream& os, const osl::PathEncoding& path)
{
  os << std::hex << path.getPath() << std::dec << " " << path.getDepth();
  return os;
}
#endif
/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
