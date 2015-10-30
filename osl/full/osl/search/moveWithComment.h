/* moveWithComment.h
 */
#ifndef _MOVEWITHCOMMENT_H
#define _MOVEWITHCOMMENT_H

#include "osl/basic_type.h"
#include "osl/hashKey.h"
#include <vector>
namespace osl
{
  namespace search
  {
    struct MoveWithComment
    {
      Move move;
      int value;
      std::vector<Move> moves;
      HashKey root;
      uint64_t node_count;
      double elapsed;
      int root_limit;
      explicit MoveWithComment(Move m=Move::INVALID(), int v=0)
	: move(m), value(v), node_count(0), elapsed(0), root_limit(0)
      {
      }
      ~MoveWithComment();
    };
  } // namespace search
  using search::MoveWithComment;
} // namespace osl

#endif /* _MOVEWITHCOMMENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
