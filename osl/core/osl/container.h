/* carray.h
 */
#ifndef OSL_CONTAINER_H
#define OSL_CONTAINER_H
#include "osl/basic_type.h"
#include "osl/config.h"
#include "osl/bits/construct.h"
#include <algorithm>
#include <cstddef>
#include <cassert>
#include <array>
#include <type_traits>

#define CONSERVATIVE_PLAYER_ACCESS

namespace osl
{
  template <typename T, size_t Capacity>
  class CArray
  {
  public:
    std::array<T,Capacity> array;
    typedef typename std::remove_cv<T>::type T_simple;

    T& operator[] (size_t i) {
      assert(i < Capacity);
      return array[i];
    }
    T const& operator[] (size_t i) const {
      assert(i < Capacity);
      return array[i];
    }

    T& operator[] (Player p) {
      assert(1 < Capacity);
#ifndef CONSERVATIVE_PLAYER_ACCESS
      // equivalent to operator[](playerToIndex(p))
      return *((T*)((char *)&elements[0] + 
		    (p & ((char *)&elements[1]-(char *)&elements[0]))));
#else
      return operator[](playerToIndex(p));
#endif
    }
    const T& operator[] (Player p) const {
      assert(1 < Capacity);
#ifndef CONSERVATIVE_PLAYER_ACCESS
      return *((T*)((char *)&elements[0] + 
		    (p & ((char *)&elements[1]-(char *)&elements[0]))));
#else
      return operator[](playerToIndex(p));
#endif
    }
    T& operator[] (PtypeO ptypeo) {
      assert(PTYPEO_SIZE <= (int)Capacity);
      return operator[](ptypeOIndex(ptypeo));
    }
    const T& operator[] (PtypeO ptypeo) const {
      assert(PTYPEO_SIZE <= (int)Capacity);
      return operator[](ptypeOIndex(ptypeo));
    }

    typedef T value_type;
    typedef typename std::array<T,Capacity>::iterator iterator;
    iterator begin() { return array.begin(); }
    iterator end() { return array.end(); }

    void fill(const T_simple& value=T_simple()) {
      array.fill(value);
    }
    // for nested CArray
    template <class T2, class = typename std::enable_if<!std::is_convertible<T2,T_simple>::value>::type>
    void fill(const T2& value=T2()) {
      for (auto& a:array)
	a.fill(value);
    }
    static size_t size() { return Capacity; }
    typedef typename std::array<T,Capacity>::const_iterator const_iterator;
    const_iterator begin() const { return array.begin(); }
    const_iterator end()   const { return array.end(); }
    const_iterator cbegin() const { return array.cbegin(); }
    const_iterator cend()   const { return array.cend(); }

    bool operator==(const CArray& other) const {
      return array == other.array;
    }
    
    T& front() { return array.front(); }
    T& back() { return array.back(); }
    const T& front() const { return array.front(); }
    const T& back() const { return array.back(); }
  };

  
  template <typename T, size_t Capacity1, size_t Capacity2>
  using CArray2d = CArray<CArray<T,Capacity2>,Capacity1>;

  template <typename T, size_t Capacity1, size_t Capacity2, size_t Capacity3>
  using CArray3d = CArray<CArray2d<T,Capacity2,Capacity3>,Capacity1>;
  
  namespace detail 
  {
    template <typename T>
    class FixedCapacityVectorPushBack
    {
      T *ptr;
      T **vPtr;
#if ! (defined NDEBUG && defined MINIMAL)
      T *limit;
#endif
    public:
      FixedCapacityVectorPushBack(T** vPtr_, T* limit_)
	: ptr(*vPtr_), vPtr(vPtr_)
#if ! (defined NDEBUG && defined MINIMAL)
	,limit(limit_)
#endif
	{
	}
      ~FixedCapacityVectorPushBack() {
	assert( *vPtr == ptr );
	*vPtr = ptr;
      }
      void push_back(const T& e) {
	assert(ptr < limit);
	assert( *vPtr == ptr );
	if(misc::detail::BitCopyTraits<T>::value)
	  *ptr++ = e;
	else
	  misc::construct(ptr++,e);
#ifndef NDEBUG
	(*vPtr)++;
#endif
      }
    };
  } // namespace deteail
  template <typename T, size_t Capacity>
  class FixedCapacityVector
  {
  protected:
    struct Array : public CArray<T, Capacity> {}
#ifdef __GNUC__
    __attribute__((__may_alias__))
#endif
    ;
    typedef Array array_t;
    T* ptr;
    CArray<int64_t, (sizeof(T[Capacity])+sizeof(int64_t)-1)/sizeof(int64_t)> relements;
  private:
    const array_t &elements() const {
      return *reinterpret_cast<const array_t*>(&relements);
    }
    array_t &elements() {
      return *reinterpret_cast<array_t*>(&relements);
    }
  public:
    typedef typename array_t::value_type value_type;
    typedef typename array_t::iterator iterator;
    typedef typename array_t::const_iterator const_iterator;
    
    FixedCapacityVector() : ptr(&(elements()[0])) {}
    explicit FixedCapacityVector(size_t size) : ptr(&(elements()[0]))  {
      resize(size);
    }
    FixedCapacityVector(FixedCapacityVector const& rhs) {
      ptr= &*begin()+rhs.size();
      std::uninitialized_copy(rhs.begin(),rhs.end(),begin());
    }
    template <class RangeIterator>
    FixedCapacityVector(const RangeIterator& first, const RangeIterator& last)
      : ptr(&(elements()[0])) {
      push_back(first, last);
    }
    ~FixedCapacityVector()  {
      misc::destroy(begin(),end());
    }
    FixedCapacityVector& operator=(FixedCapacityVector const& rhs) {
      if (this == &rhs)
	return *this;
      
      if(size()>rhs.size()) {
	iterator it=std::copy(rhs.begin(),rhs.end(),begin());
	misc::destroy(it,end());
      }
      else {
	iterator it=std::copy(&(rhs.elements()[0]),
			      &(rhs.elements()[0])+size(),begin());
	std::uninitialized_copy(&(rhs.elements()[0])+size(),
				&(rhs.elements()[0])+rhs.size(),it);
      }
      ptr= &*begin()+rhs.size();
      return *this;
    }

    T& operator[] (size_t i) {
      assert(i <= size());
      return elements()[i];
    }

    iterator begin() {  return &elements()[0]; }
    iterator end() { return static_cast<iterator>(ptr); }

    T& front() { return *begin(); }
    T& back() { return *(end() - 1); }

    void push_back(const T& e) {
      assert(size() < Capacity);
      misc::construct(ptr,e);
      ++ptr;
    }
    template <class RangeIterator>
    void push_back(const RangeIterator& first, const RangeIterator& last);
    void pop_back() { 
      --ptr;
      misc::destroy(ptr+1);
    }
    void clear() { 
      size_t s=size();
      ptr= &(elements()[0]);
      // 該当する部分のdestructorを呼ぶ
      misc::destroy(begin(),begin()+(int)s);
    }
    void resize(size_t new_length) {
      while (size() < new_length)
	push_back(T());
      if (new_length < size()) {
	misc::destroy(begin()+(int)new_length,end());
	ptr= &(elements()[new_length]);
      }
    }
    void erase(const T& e) {
      const iterator new_end = std::remove(begin(), end(), e);
      ptr= &*new_end;
      misc::destroy(new_end,end());
    }

    /** 重複する要素を取り除く */
    void unique() {
      std::sort(begin(),end());
      iterator last = std::unique(begin(), end());
      ptr = &*last;
      misc::destroy(last,end());
    }

    size_t size() const { return ptr-&*begin(); }
    bool empty() const { return ptr==&*begin(); }
    size_t capacity() const { return Capacity; }

    T const& operator[] (size_t i) const {
      assert(i < size());
      return elements()[i];
    }
    const_iterator begin() const { return &elements()[0]; }
    const_iterator end()   const { return ptr; }

    const T& front() const { return *begin(); }
    const T& back() const { return *(end() - 1); }
    
    bool isMember(const T& e, const_iterator first, const_iterator last) const {
      return std::find(first, last, e) != last;
    }
    bool isMember(const T& e) const {
      return isMember(e, begin(), end());
    }
    detail::FixedCapacityVectorPushBack<T> pushBackHelper() {
      return {&ptr, &*begin()+Capacity};
    }
  };
  template <typename T, size_t C> inline
  bool operator==(const FixedCapacityVector<T,C>& l, const FixedCapacityVector<T,C>& r) 
  {
    return l.size() == r.size() && std::equal(l.begin(), l.end(), r.begin());
  }
  template <typename T, size_t C> inline
  bool operator<(const FixedCapacityVector<T,C>& l, const FixedCapacityVector<T,C>& r) 
  {
    return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
  }
  using detail::FixedCapacityVectorPushBack;
} // namespace osl

template <typename T, size_t Capacity>
template <class RangeIterator>
void osl::FixedCapacityVector<T,Capacity>::push_back(const RangeIterator& first, const RangeIterator& last)
{
  iterator insert_point = end();
  std::uninitialized_copy(first, last, insert_point);
  ptr += last-first;
  assert(size() <= Capacity);
}

namespace osl
{
  class MoveVector : public FixedCapacityVector<Move,Move::MaxUniqMoves>
  {
  public:
  };
  std::ostream& operator<<(std::ostream& os,MoveVector const& mv);
  bool operator<(const MoveVector& l, const MoveVector& r);

  enum { CheckOrEscapeMaxUniqMoves = Move::MaxUniqMoves/4 }; // 150
  class CheckMoveVector : public FixedCapacityVector<Move,CheckOrEscapeMaxUniqMoves> 
  {
  };

  class PieceVector : public FixedCapacityVector<Piece,Piece::SIZE>
  {
  public:
    /**
     * 駒の価値の小さい順に並び替える.
     * 成っているかに関わらず 歩香桂銀金角飛王
     */
    void sortByBasic();
    /**
     * 駒の価値の大きい順に並び替える. 成りを考慮.
     * 王龍馬...
     */
    void sortByPtype();
  };
  std::ostream& operator<<(std::ostream& os,const PieceVector&);

  class PtypeOSquareVector
    : public FixedCapacityVector<std::pair<PtypeO,Square>,Piece::SIZE>
  {
  public:
    /**
     * 駒の価値の小さい順に並び替える
     */
    void sort();

    struct PtypeOSquareLessThan;
  };
}

#endif /* OSL_CONTAINER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
