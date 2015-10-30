// categoryMoveVector.h
#ifndef CATEGORYMOVEVECTOR_H
#define CATEGORYMOVEVECTOR_H

#include "osl/container/moveLogProbVector.h"
#include <forward_list>
#include <string>

namespace osl
{
  namespace search
  {
    namespace analyzer
    {
      struct CategoryMoves
      {
	MoveLogProbVector moves;
	std::string category;

	CategoryMoves(const MoveLogProbVector&, const std::string&);
	~CategoryMoves();
      };
      class CategoryMoveVector
	: public std::forward_list<CategoryMoves>
      {
      public:
	CategoryMoveVector();
	~CategoryMoveVector();	
      };
    } // namespace analyzer
  } // namespace search
} // namespace osl

#endif /* CATEGORYMOVEVECTOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
