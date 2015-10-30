/* hashCollision.h
 */
#ifndef _HASHCOLLISION_H
#define _HASHCOLLISION_H

#include <stdexcept>

namespace osl
{
  namespace hash
  {
    /**
     * ハッシュの衝突を検出した時に throw するために使う.
     */
    struct HashCollision : std::runtime_error
    {
      HashCollision() : std::runtime_error("hash collision")
      {
      }
    };
  } // namespace hash
  using hash::HashCollision;
} // namespace osl


#endif /* _HASHCOLLISION_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
