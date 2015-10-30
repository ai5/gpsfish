/* quiescenceRecord.h
 */
#ifndef _QUIESCENCERECORD_H
#define _QUIESCENCERECORD_H

#include "osl/search/dualThreatmateState.h"
#ifdef OSL_SMP
#  include "osl/misc/lightMutex.h"
#endif
#include <iosfwd>
#include <vector>

namespace osl
{
  namespace search
  {
    struct QSearchTraits
    {
      enum { 
	/** 通常探索の最大 */
	MaxDepth = 8,
	/** 即詰の深さ */
	CheckmateSpecialDepth = 127,
	/** 駒損ループの深さ */
	HistorySpecialDepth = 126,
      };
      enum { FirstThreat = 6, SecondThreat = 2 };
      enum MoveType { 
	UNKNOWN, KING_ESCAPE, CAPTURE, PROMOTE, CHECK,
	ESCAPE, ATTACK, OTHER, 
      };
    };

    /**
     * QuiescenceSearch でパスした場合の相手の有力な指手
     */
    struct QuiescenceThreat
    {
      int value;
      Move move;
      explicit QuiescenceThreat(int v=0, Move m=Move::INVALID()) 
	: value(v), move(m)
      {
      }
    };    

    struct BestMoves : public CArray<Move,4>
    {
      BestMoves()
      {
	// automaticallyfilled by invalid data by constructor
      }
      size_t capacity() const { return size(); }
      size_t sizeFilled() const
      {
	for (size_t i=0; i<capacity(); ++i)
	  if (! operator[](i).isNormal())
	    return i;
	return capacity();
      }
      void add(Move m)
      {
	if (! m.isNormal())
	  return;
	iterator p = std::find(begin(), end(), m);
	if (p != end()) {
	  std::rotate(begin(), p, p+1);
	  return;
	}
	size_t size = sizeFilled();
	if (size == capacity())
	  operator[](capacity()-1) = m;
	else
	  operator[](size++) = m;
	std::rotate(begin(), begin()+(int)size-1, begin()+(int)size);
      }
      void clear()
      {
	fill(Move());
      }
      void addSecondary(const MoveVector& v) 
      {
	CArray<Move,4> copy = *this;
	clear();
	size_t size = 0;
	if (copy[0].isNormal())
	  operator[](size++) = copy[0];
	for (size_t i=0; i<v.size() && size<capacity(); ++i) {
	  assert(v[i].isNormal());
	  if (std::find(begin(), begin()+(int)size, v[i]) == begin()+(int)size)
	    operator[](size++) = v[i];
	}
	for (size_t i=1; i<copy.size() && copy[i].isNormal() && size<capacity(); ++i)
	  if (std::find(begin(), begin()+(int)size, copy[i]) == begin()+(int)size)
	    operator[](size++) = copy[i];
      }
    };
    
    /**
     * QuiescenceRecord のデータのうちlock, public なデータ以外
     */
    struct QuiescenceRecordBase
    {
      int upper_bound, lower_bound;
      BestMoves best_moves;
      /** static_value への脅威 */
      QuiescenceThreat threat1, threat2;
      int static_value;
      int checkmate_nodes;
      DualThreatmateState threatmate;
      int threatmate_nodes;
      /** upper_bound, lower_bound, static_value を探索した深さ */
      int8_t upper_depth, lower_depth, static_value_depth;
    public:
      /**
       * 使えない深さ.
       * CAVEAT: -1 だと王手延長で depth が-1になった時にはまる
       */
      enum { InitialDepth = -128, };
      enum StaticValueType { UNKNOWN, UPPER_BOUND, EXACT };
    protected:
      QuiescenceRecordBase() 
	: checkmate_nodes(0), threatmate_nodes(0),
	  upper_depth(InitialDepth), lower_depth(InitialDepth),
	  static_value_depth(InitialDepth)
      {
      }
      ~QuiescenceRecordBase() {}
    };
    class SimpleHashRecord;
    /**
     * QuiescenceSearch で表に保存するデータ
     */
    class QuiescenceRecord : public QuiescenceRecordBase
    {
    public:
      static const char *toString(StaticValueType);
    private:
#ifdef OSL_SMP
      typedef osl::misc::LightMutexChar Mutex;
      mutable Mutex mutex;
#endif
    public:
      QuiescenceRecord()
      {
      }
      /**
       * copy constructor: copy everything except for mutex
       */
      QuiescenceRecord(const QuiescenceRecord& src)
	: QuiescenceRecordBase(src)
      {
      }
      QuiescenceRecord& operator=(const QuiescenceRecord& src)
      {
	if (this == &src)
	  return *this;
	
	QuiescenceRecordBase::operator=(src);
	return *this;
      }

      template <Player Turn>
      const Square8 sendOffSquare(const NumEffectState& state) const
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	assert(Turn == state.turn());
	Square8 ret;
	const Square king_position = state.kingSquare(alt(Turn));
	if (threatmate.sendoffs == SendOffSquare::invalidData())
	  threatmate.sendoffs = SendOffSquare::find<Turn>(state, king_position, ret);
	else
	  SendOffSquare::unpack(threatmate.sendoffs, king_position, ret);
	return ret;
      }
      const Square8
      sendOffSquare(Player turn, const NumEffectState& state) const
      {
	if (turn == BLACK)
	  return sendOffSquare<BLACK>(state);
	else
	  return sendOffSquare<WHITE>(state);
      }
      /**
       * @param max このrecordで使って良いノード数
       * @return 詰将棋に使えるノード数を返す
       */
      int checkmateNodesLeft(int max)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	if (max > checkmate_nodes)
	{
	  const int left = max - checkmate_nodes;
	  checkmate_nodes = max;
	  return left;
	}
	return 0;
      }
      /**
       * @param max このrecordで使って良いノード数
       * @return 詰めろ確認の詰将棋に使えるノード数を返す
       */
      int threatmateNodesLeft(int max)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	if (max > threatmate_nodes)
	{
	  const int left = max - threatmate_nodes;
	  threatmate_nodes = max;
	  return left;
	}
	return 0;
      }
      /** 今までに詰将棋で探したノード数 */
      int checkmateNodes() const { return checkmate_nodes; }
      int threatmateNodes() const { return threatmate_nodes; }

      void clear()
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	best_moves.clear();
	upper_depth = lower_depth = static_value_depth = InitialDepth;
      }
      void setStaticValue(StaticValueType type, int value, int depth,
			  const QuiescenceThreat& t1=QuiescenceThreat(),
			  const QuiescenceThreat& t2=QuiescenceThreat())
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	assert((depth <= QSearchTraits::MaxDepth)
	       || (depth == QSearchTraits::CheckmateSpecialDepth));
	assert(value % 2 == 0);
	static_value = value;
	threat1 = t1;
	threat2 = t2;
	static_value_depth = depth;
	threatmate.flags.static_value_type = type;
      }
    public:
      void setLowerBound(int depth, int bound, Move best_move)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	assert((depth <= QSearchTraits::MaxDepth)
	       || (depth == QSearchTraits::CheckmateSpecialDepth));
	if (depth >= lower_depth)
	{
	  best_moves.add(best_move);
	  lower_bound = bound;
	  lower_depth = depth;
	}
      }
      void setUpperBound(int depth, int bound)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	assert((depth <= QSearchTraits::MaxDepth)
	       || (depth == QSearchTraits::CheckmateSpecialDepth));
	if (depth >= upper_depth)
	{
	  upper_bound = bound;
	  upper_depth = depth;
	}
      }
      void setHistoryValue(int value)
      {
	lower_bound = upper_bound = value;
	lower_depth = upper_depth = QSearchTraits::HistorySpecialDepth;
      }
      void setHistoryValue(Move best_move, int value)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	best_moves.add(best_move);
	setHistoryValue(value);
      }
    public:
      void addKillerMoves(const MoveVector& new_moves)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif	
	best_moves.addSecondary(new_moves);
      }
      StaticValueType staticValueType() const { 
	return static_cast<StaticValueType>(threatmate.flags.static_value_type); 
      }
      bool hasStaticValue() const { return staticValueType() != UNKNOWN; }
      bool hasStaticValue(int& value, int& depth, StaticValueType& type) const {
	type = staticValueType();
	if (type == UNKNOWN)
	  return false;
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif	
	type = staticValueType();	
	value = static_value;
	depth = static_value_depth;
	return type != UNKNOWN;
      }
      int staticValue() const { assert(hasStaticValue()); return static_value; }
      int staticValueDepth() const { return static_value_depth; }
      int upperDepth() const { return upper_depth; }
      int lowerDepth() const { return lower_depth; }
      int upperBound() const { return upper_bound; }
      int lowerBound() const { return lower_bound; }
      const Move bestMove() const { return best_moves[0]; }
      int movesEmpty() const { return ! best_moves[1].isNormal(); }
      int movesSizeLessThan(size_t n) const { 
	return best_moves.capacity() < n || ! best_moves[n-1].isNormal(); 
      }
      int moves_size() const { 
	return std::max(0, (int)best_moves.sizeFilled()-1); 
      }
      void loadMoves(MoveVector& dst) const{
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	dst.clear();
	for (size_t i=1; i<best_moves.capacity() && best_moves[i].isNormal(); ++i)
	  dst.push_back(best_moves[i]);
      }
      void dump(std::ostream&) const;
      const QuiescenceThreat staticThreat(int index) const
      {
	return (index == 0) ? threat1 : threat2;
      }
      void updateThreatmate(Player turn, const DualThreatmateState *parent, bool in_check)
      {
#ifdef OSL_SMP
	SCOPED_LOCK_CHAR(lk,mutex);
#endif
	threatmate.updateInLock(turn, parent, in_check);
      }
      friend class SimpleHashRecord;
    };
    
  } // namespace search
} // namespace osl

#endif /* _QUIESCENCERECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
