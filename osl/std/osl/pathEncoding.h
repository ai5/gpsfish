/* pathEncoding.h
 */
#ifndef OSL_PATH_ENCODING_H
#define OSL_PATH_ENCODING_H

#include "osl/basic_type.h"
#include "osl/container.h"
#include <iosfwd>
namespace osl
{
  class PathEncodingTable
  {
  public:
    static const size_t MaxEncodingLength = 256;
  private:
    typedef CArray<CArray2d<unsigned long long, Square::SIZE, PTYPE_SIZE>, 
		   MaxEncodingLength> array_t;
    array_t values;
  public:
    void init();
    unsigned long long get(size_t depth, Square pos, Ptype ptype) const
    {
      return values[depth][pos.index()][ptype-PTYPE_MIN];
    }
    /**
     * @return 必ず奇数
     */
    unsigned long long get(size_t depth, Move m) const
    {
      const Square from = m.from();
      const Square to = m.to();
      const Ptype fromPtype = m.oldPtype();
      const Ptype toPtype = m.ptype();
      depth %= 256;
      return get(depth, from, fromPtype) + get(depth, to, toPtype) + 1;
    }
  };
  extern PathEncodingTable Path_Encoding_Table;
  class PathEncoding
  {
    unsigned long long path;
    int depth;
  public:
    explicit PathEncoding(int d=0) : path(0), depth(d)
    {
    }
    explicit PathEncoding(Player turn, int d=0)
      : path((turn == BLACK) ? 0 : 1), depth(d)
    {
    }
    PathEncoding(const PathEncoding& org, Move m)
      : path(org.path), depth(org.depth)
    {
      pushMove(m);
    }
    Player turn() const { return (path % 2) ? WHITE : BLACK; }
    void pushMove(Move m)
    {
      assert(m.player() == turn());
      path += Path_Encoding_Table.get(depth, m);
      ++depth;
    }
    void popMove(Move m)
    {
      --depth;
      path -= Path_Encoding_Table.get(depth, m);
      assert(m.player() == turn());
    }
    unsigned long long getPath() const { return path; }
    int getDepth() const { return depth; }
  };

  inline bool operator==(const PathEncoding& l, const PathEncoding& r)
  {
    return l.getPath() == r.getPath();
  }
  inline bool operator!=(const PathEncoding& l, const PathEncoding& r)
  {
    return !(l == r);
  }
  std::ostream& operator<<(std::ostream&, const PathEncoding&);
} // namespace osl

#endif /* OSL_PATH_ENCODING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; coding:utf-8
// ;;; c-basic-offset:2
// ;;; End:
