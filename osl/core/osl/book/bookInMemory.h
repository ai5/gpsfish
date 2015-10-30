/* bookInMemory.h
 */
#ifndef OSL_BOOKINMEMORY_H
#define OSL_BOOKINMEMORY_H
#include "osl/hashKey.h"
#include <unordered_map>
#include <string>

namespace osl
{
  namespace book
  {
    class WeightedBook;
    class BookInMemory
    {
      typedef CArray<Move,8> moves_t;
      typedef std::unordered_map<HashKey,moves_t,std::hash<HashKey>> table_t;
      table_t table;
    public:
      ~BookInMemory();

      void find(const HashKey& key, MoveVector& out) const;
      size_t size() const { return table.size(); }
      static const BookInMemory& instance(const std::string& filename="");
    private:
      explicit BookInMemory(const std::string& filename);
      void readAll(const std::string& filename);
      int readRecursive(const HashKey& key, int index, WeightedBook& book, int, int);
    };
  }
  using book::BookInMemory;
}

#endif /* OSL_BOOKINMEMORY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
