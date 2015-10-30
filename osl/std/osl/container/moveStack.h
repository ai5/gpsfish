#ifndef _MOVE_STACK_H
#define _MOVE_STACK_H
#include "osl/container.h"
#include <vector>
#include <cassert>
namespace osl
{
  namespace container
  {
  /**
   * Move のstack. 主に探索で今までにさされた指手を保存するのに使用.
   * size() == 0 の時に lastMove と lastMove(2) は Move::INVALID() を返す.
   */
  class MoveStack
  {
    typedef std::vector<Move> vector_t;
    vector_t data;
  public:
    MoveStack();
    ~MoveStack();

    void reserve(size_t);
    void clear();
    void push(Move m) { data.push_back(m); }
    void pop() { data.pop_back(); }
    /** @param last lastLastMove if 2 */
    bool hasLastMove(size_t last=1) const { return size()>=last; }
    const Move lastMove(size_t last=1) const
    {
      const size_t index = data.size() - last;
      assert(index < data.size());
      return data[index];
    }
    size_t size() const { return data.size()-2; }
    /**
     * @param last_n 最後のn個を表示，0なら全て．
     */
    void dump(size_t last_n=0) const;
    void dump(std::ostream&, size_t last_n=0) const;
    bool operator==(const MoveStack& r) const
    {
      return data == r.data;
    }
  };
} // namespace container
  using container::MoveStack;
} // namespace osl
#endif // _MOVE_STACK_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
