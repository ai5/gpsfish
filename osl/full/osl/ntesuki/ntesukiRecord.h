#ifndef _NTESUKI_NTESUKI_RECORD_H
#define _NTESUKI_NTESUKI_RECORD_H
#include "osl/ntesuki/ntesukiResult.h"
#include "osl/ntesuki/ntesukiMove.h"
#include "osl/ntesuki/ntesukiMoveList.h"
#include "osl/ntesuki/rzone.h"
#include "osl/ntesuki/ntesukiExceptions.h"
#include "osl/checkmate/checkAssert.h"
#include "osl/misc/carray.h"
#include "osl/state/hashEffectState.h"
#include "osl/container/moveVector.h"
#include "osl/pathEncoding.h"
#include <iosfwd>

namespace osl
{
  namespace ntesuki
  {
    class NtesukiRecord;
    class PathEncodingList : public slist<PathEncoding>
    {
    };

    class NtesukiMoveGenerator;
    class NtesukiTable;

    /**
     * ある局面について，その局面を n手すきで探索した場合の
     * 結果を保持しておくクラス.
     */

    class NtesukiRecord
    {
    public:
      typedef slist<NtesukiRecord> RecordList;
      typedef slist<NtesukiRecord*> RecordPList;
      /**
       * 各配列のサイズ. SIZE - 1 手すきまで調べられる.
       */
      static const unsigned int SIZE = 2;
      enum IWScheme { no_iw = 0,
		      strict_iw = 1,
		      pn_iw = 2 };

      enum PSScheme { no_ps = 0,
		      pn_ps = 1 };

      enum ISScheme { no_is = 0,
		      tonshi_is = 1,
		      delay_is = 2,
		      normal_is = 3 };

      /**
       * 探索関係の色々な情報.
       */
      static unsigned int fixed_search_depth;
      static unsigned int inversion_cost;
      static bool use_dominance;
      static int pass_count;
      static bool max_for_split;
      static bool use_rzone_move_generation;
      static bool delay_lame_long;
      static bool use_9rzone;

      static NumEffectState *state;
      static NtesukiMoveGenerator *mg;
      static NtesukiTable *table;

      /* DAG関係の統計情報 */
      static unsigned int split_count /** 分流点がいくつあるか. */,
	confluence_count /** 合流点がいくつあるか. */;

      /* Visit Lock */
      class VisitLock;
      class UnVisitLock;

      /** 各プレイヤの持駒 */
      PieceStand black_stand, white_stand;

      /** root からの最短 path の距離 */
      unsigned short distance;

      /** 局面の HashKey */
      HashKey key;

      /** 盤面・手番が同じで，持駒だけ違う局面のリスト */
      RecordList *same_board_list;

      /** 親局面のリスト */
      RecordPList parents;
      int rev_refcount;/* 子から親への reference の count */

      /**
       * コンストラクタ.
       */
      NtesukiRecord(signed short distance,
		    const HashKey& key,
		    const PieceStand& white_stand,
		    RecordList* same_board_list);
      ~NtesukiRecord()
      {
      }

      /** 手番 */
      Player turn() const
      {
	return key.turn();
      }

      /** simulation によって値が決まったか */
      bool isBySimulation() const
      {
	return by_simulation;
      }

      /**
       * この局面でのプレイヤの持駒.
       * - @const P プレイヤ
       */
      template<Player P> const PieceStand&
      getPieceStand() const
      {
	return piece_stand<P>();
      }

      const PieceStand&
      getPieceStandSlow(Player P) const
      {
	if (P == BLACK)
	{
	  return piece_stand<BLACK>();
	}
	else
	{
	  return piece_stand<WHITE>();
	}
      }

      /**
       * 証明駒を計算する.
       * - @const A 攻撃側のプレイヤ
       */
      template <Player A>
      PieceStand
      calcProofPiecesOr(int pass_left,
			const NtesukiMove& m);

      template <Player A>
      PieceStand
      calcProofPiecesAnd(int pass_left);
      
      /**
       * 証明駒を設定する.
       * - @const A 攻撃側のプレイヤ
       */
      template <Player A>
      void
      setProofPieces(int pass_left,
		     const NtesukiResult& r,
		     const NtesukiMove& m,
		     const PieceStand* ps);
      /**
       * 反証駒を設定する.
       * - @const A 攻撃側のプレイヤ
       */
      template <osl::Player A>
      void
      setDisproofPieces(int pass_left,
			const NtesukiResult& r,
			const NtesukiMove& m,
			const PieceStand* ps);
      /**
       * 探索の結果を格納する
       * - @const A 攻撃側のプレイヤ
       */
      template <Player A> void
      setResult(int i,
		const NtesukiResult& r,
		const NtesukiMove& m,
		bool bs,
		const PieceStand* ps = NULL);

      /** このノードの NtesukiResult の値を調べる
       * - @const A 攻撃側のプレイヤ
       */
      template <Player A> const NtesukiResult getValue(int i) const;
      template <Player A> const NtesukiResult getValueWithPath(int i,
							       const PathEncoding path) const;
      template <Player A> const NtesukiResult getValueOr(int i,
							 const PathEncoding path,
							 IWScheme iwscheme) const;
      template <Player A> const NtesukiResult getValueAnd(int i,
							  const PathEncoding path,
							  IWScheme iwscheme, PSScheme psscheme) const;
      const NtesukiResult getValueSlow(const Player attacker, int i) const;
      const NtesukiResult getValueOfTurn(int i) const;
      const NtesukiResult valueBeforeFinal() const;

      /* 勝ちか調べる.
       * 勝ちだったら pass_left を，
       * それ以外だったら SIZE を返す.
       */
      int isWin(const Player attacker) const
      {
	int o = 0;
	for (; o < (int)SIZE; o++)
	{
	  if (getValueSlow(attacker, o).isCheckmateSuccess())
	  {
	    break;
	  }
	}
	return o;
      }
      

      /**
       * 登録されてる最善手を返す.
       */
      template <Player A> const NtesukiMove& getBestMove(int i) const;
      const NtesukiMove& getBestMoveSlow(Player attacker, int i) const;

      /**
       * Loop になっているかチェック.
       */
      bool isVisited() const { return visited; }
      bool isFinal() {return final; }

      void setVisited()
      {
	assert (!visited);
	visited = true;
      }

      void resetVisited()
      {
	assert (visited);
	visited = false;
      }

      /**
       * Fixed Depth Searcher によって値が設定されたか.
       */
      template <Player A> bool isByFixed() const;
      bool isByFixedSlow(Player attacker) const;
      
      /**
       * このノードが n手すきになっているか.
       */
      template <Player A> bool isNtesuki(int pass_left) const;
      template <Player A> void setNtesuki(int pass_left);

      /**
       * このノードで親から来た oracle を試したか
       */
      template <Player A> bool hasTriedPropagatedOracle(int pass_left) const;
      template <Player A> void triedPropagatedOracle(int pass_left);

      /**
       * 証明駒を得る
       */
      template <Player A> PieceStand getPDPieces(int pass_left) const;
      PieceStand getPDPiecesSlow(Player attacker, int pass_left) const;
      template <Player A> void setPDPieces(int pass_left, const PieceStand p);

      /**
       * 無駄合いを読むべきか.
       */
      bool readInterpose(int pass_left) const
      {
	return read_interpose[pass_left];
      }

      void setReadInterpose(int pass_left)
      {
	read_interpose[pass_left] = true;
      }

      /**
       * defense の際に王手を読むべきか.
       */
      bool readCheckDefense(int pass_left) const
      {
	return read_check_defense[pass_left];
      }

      void setReadCheckDefense(int pass_left)
      {
	read_check_defense[pass_left] = true;
      }

      /**
       * attack の際に，ヒューリスティックに判別された攻撃手以外も読むか.
       */
      bool readNonAttack(int pass_left) const
      {
	return read_non_attack[pass_left];
      }

      void setReadNonAttack(int pass_left)
      {
	read_non_attack[pass_left] = true;
      }

      /**
       * old child を読むべきか.
       */
      template <Player A> bool
      useOld(int pass_left) const;
      
      template <Player A> void
      setUseOld(int pass_left,
		 bool value);
      
      /**
       * この path で loop になるか．
       */
      template <Player A> bool
      isLoopWithPath(int pass_left,
		     const PathEncoding& path) const;

      template <Player A> void
      setLoopWithPath(int pass_left,
		      const PathEncoding& path);

      template <Player A> bool
      hasLoop(int pass_left) const
      {
	return !loop_path_list<A>()[pass_left].empty();
      }

      /**
       * Fixed Depth Searcher を呼ぶ等.
       */
      template <Player P> bool
      setUpNode();

      /* 王手のかかっていない局面. */
      template <Player P> void
      setUpAttackNode();

      /* 王手のかかっている局面. */
      template <Player P> void
      setUpDefenseNode();

      /* 子ノードからの情報を使って現在の情報を更新する. */
      void
      updateWithChild(NtesukiRecord* child,
		      int pass_left);
      
      /**
       * 手の生成.
       */
      template <Player P> void
      generateMoves(NtesukiMoveList& moves,
		    int pass_left,
		    bool all_moves);


      bool operator==(const NtesukiRecord& record)
      {
	return key == record.key;
      }

      /* GC 関係 */
      unsigned int getChildCount() const
      {
	return child_count;
      }

      void addChildCount(unsigned int i)
      {
	child_count += i;
      }

      unsigned int getReadCount() const
      {
	return read_count;
      }

      unsigned int getWrittenCount() const
      {
	return written_count;
      }

      /* DAG 対策関係 */
    private:
      bool isNewParent(const NtesukiRecord* p) const
      {
	for (RecordPList::const_iterator it = parents.begin();
	     it != parents.end(); ++it)
	{
	  if (*it == p) return false;
	}
	return true;
      }

      void find_split(NtesukiRecord *rhs,
		      RecordPList& lvisited,
		      RecordPList& rvisited)
      {
	if (find_split_right(rhs, lvisited, rvisited))
	{
	  return;
	}

#if 1
	if (parents.empty())
	{
	  return;
	}
	/* 最初の一つの親だけ見る */
	RecordPList::iterator lp = parents.begin();
	if (std::find(lvisited.begin(), lvisited.end(), *lp)
	    != lvisited.end())
	{
	  return;
	}
	lvisited.push_front(*lp);
	(*lp)->find_split(rhs, lvisited, rvisited);
	lvisited.pop_front();
#else
	for (RecordPList::iterator lp = parents.begin();
	     lp != parents.end(); ++lp)
	{
	  if (std::find(lvisited.begin(), lvisited.end(), *lp)
	      == lvisited.end())
	  {
	    lvisited.push_front(*lp);
	    (*lp)->find_split(rhs, lvisited, rvisited);
	    lvisited.pop_front();
	  }
	}
#endif
      }

      /* find_split 一つでやろうとすると， O(n^2) で済むところも O(2^n) かかるので注意 */
      bool find_split_right(NtesukiRecord *rhs,
			    RecordPList& lvisited,
			    RecordPList& rvisited)
      {
	if (this == rhs)
	{
	  if (!is_split)
	  {
	    ++split_count;
	    is_split = true;
	  }
	  return true;
	}

#if 1
	if (rhs->parents.empty())
	{
	  return false;
	}
	/* 最初の一つの親だけ見る */
	RecordPList::iterator rp = rhs->parents.begin();
	if (std::find(rvisited.begin(), rvisited.end(), *rp)
	    != rvisited.end())
	{
	  return false;
	}
	rvisited.push_front(*rp);
	bool result = find_split_right(*rp, lvisited, rvisited);
	rvisited.pop_front();
	return result;
#else
	bool result = false;
	for (RecordPList::iterator rp = rhs->parents.begin();
	     rp != rhs->parents.end(); ++rp)
	{
	  if (std::find(rvisited.begin(), rvisited.end(), *rp)
	      == rvisited.end())
	  {
	    rvisited.push_front(*rp);
	    result |= find_split_right(*rp, lvisited, rvisited);
	    rvisited.pop_front();
	  }
	}
	return result;
#endif
      }

      void addNewParent(NtesukiRecord* p)
      {
	ntesuki_assert(isNewParent(p));
	parents.push_front(p);
	p->rev_refcount++;
      }

    public:
      void checkNewParent(NtesukiRecord *p)
      {
	if (parents.empty())
	{
	  /* allocate されたばかりなら， parent を add して終了 */
	  addNewParent(p);
	}
	else if (isNewParent(p))
	{
	  ++confluence_count;
	  if (max_for_split)
	  {
	    /* DAG の分流接点(？)を探す
	     */
	    RecordPList lvisited, rvisited;
	    lvisited.push_front(this);
	    rvisited.push_front(p);
	    find_split(p, lvisited, rvisited);
	  }
	  addNewParent(p);
	}
      }

    private:
      typedef CArray<NtesukiResult, SIZE> values_t;
      typedef CArray<NtesukiMove, SIZE> moves_t;
      typedef CArray<short, SIZE - 1> nodesread_t;
      typedef CArray<PieceStand, SIZE> pdpieces_t;
      typedef CArray<bool, SIZE> flags_t;
      typedef CArray<PathEncodingList, SIZE> pell_t;
      typedef CArray<Rzone, SIZE> rzones_t;
      values_t values_black, values_white;
      moves_t best_move_black, best_move_white;
      pdpieces_t pd_pieces_black, pd_pieces_white;
      /** 同一の局面で，Loop になっているものの Path */
      pell_t loop_path_list_black, loop_path_list_white;
      mutable unsigned int child_count, read_count, written_count;

      NtesukiResult value_before_final;/** setResult で final な値を設定される直前の result  */

      bool visited;
      bool by_simulation;
      bool by_fixed_black, by_fixed_white;
      bool already_set_up;
      bool final;

    public:
      bool is_split; /** DAG の分流点 */
      bool do_oracle_attack;
      /* AND節点について，親節点が pass_left = 0 で disproof
       * された場合，この節点からパスで到達できる節点について，
       * simulation をする.
       */
      bool do_oracle_aunt;

      /* OR 節点で，rzone を使用した手生成をする */
      bool rzone_move_generation;

    private:
      flags_t read_interpose;/* 受け方でのみ使われる */
      flags_t read_check_defense;/* 受け方でのみ使われる */
      flags_t read_non_attack;/* 攻め方でのみ使われる */
      flags_t is_ntesuki_black, is_ntesuki_white;
      flags_t propagated_oracle_black, propagated_oracle_white;
      flags_t use_old_black, use_old_white;
      rzones_t rzone_black, rzone_white;
      
      NtesukiRecord();

      template <Player P> bool& by_fixed()
      {
	if (P == BLACK)
	  return by_fixed_black;
	else
	  return by_fixed_white;
      }

      template <Player P> const bool& by_fixed() const
      {
	if (P == BLACK)
	  return by_fixed_black;
	else
	  return by_fixed_white;
      }

      template <Player P> PieceStand& piece_stand()
      {
	if (P == BLACK)
	  return black_stand;
	else
	  return white_stand;
      }

      template <Player P> const PieceStand& piece_stand() const
      {
	if (P == BLACK)
	  return black_stand;
	else
	  return white_stand;
      }

      template <Player P> values_t& values()
      {
	if (P == BLACK)
	  return values_black;
	else
	  return values_white;
      }

      template <Player P> const values_t& values() const
      {
	if (P == BLACK)
	  return values_black;
	else
	  return values_white;
      }

      template <Player P> moves_t& best_move()
      {
	if (P == BLACK)
	  return best_move_black;
	else
	  return best_move_white;
      }

      template <Player P> const moves_t& best_move() const
      {
	if (P == BLACK)
	  return best_move_black;
	else
	  return best_move_white;
      }

      template <Player P> pdpieces_t& pdpieces()
      {
	if (P == BLACK)
	  return pd_pieces_black;
	else
	  return pd_pieces_white;
      }

      template <Player P> const pdpieces_t& pdpieces() const
      {
	if (P == BLACK)
	  return pd_pieces_black;
	else
	  return pd_pieces_white;
      }

      template <Player P> flags_t& is_ntesuki()
      {
	if (P == BLACK)
	  return is_ntesuki_black;
	else
	  return is_ntesuki_white;
      }

      template <Player P> const flags_t& is_ntesuki() const
      {
	if (P == BLACK)
	  return is_ntesuki_black;
	else
	  return is_ntesuki_white;
      }

      template <Player P> flags_t& propagated_oracle()
      {
	if (P == BLACK)
	  return propagated_oracle_black;
	else
	  return propagated_oracle_white;
      }

      template <Player P> const flags_t propagated_oracle() const
      {
	if (P == BLACK)
	  return propagated_oracle_black;
	else
	  return propagated_oracle_white;
      }

      template <Player P> flags_t& use_old()
      {
	if (P == BLACK)
	  return use_old_black;
	else
	  return use_old_white;
      }

      template <Player P> const flags_t use_old() const
      {
	if (P == BLACK)
	  return use_old_black;
	else
	  return use_old_white;
      }

      template <Player P> pell_t& loop_path_list()
      {
	if (P == BLACK)
	  return loop_path_list_black;
	else
	  return loop_path_list_white;
      }

      template <Player P> const pell_t& loop_path_list() const
      {
	if (P == BLACK)
	  return loop_path_list_black;
	else
	  return loop_path_list_white;
      }

    public:
      template <osl::Player P> rzones_t& rzone()
      {
	if (P == BLACK)
	  return rzone_black;
	else
	  return rzone_white;
      }
    private:
      template <Player P> void
      setFinal(int i,
	       const NtesukiResult& r,
	       const NtesukiMove& m,
	       const PieceStand* ps);

      /**
       * Dominace の伝播用.
       */
      void lookup_same_board_list();

      template <Player P>
      void propagate_proof(int pass_left);

      template <Player P>
      void propagate_disproof(int pass_left);

    public:
      template <Player P>
      bool isDominatedByProofPieces(const NtesukiRecord* record,
				    int pass_left) const;

      template <Player P>
      bool isDominatedByDisproofPieces(const NtesukiRecord* record,
				       int pass_left) const;

      template <Player P>
      bool isBetterFor(NtesukiRecord* record);
    };
    std::ostream& operator<<(std::ostream&, const osl::ntesuki::NtesukiRecord&);

    std::ostream& operator<<(std::ostream&,
			     const osl::ntesuki::NtesukiRecord::IWScheme&);
    std::istream& operator>>(std::istream&,
			     osl::ntesuki::NtesukiRecord::IWScheme&);

    std::ostream& operator<<(std::ostream&,
			     const osl::ntesuki::NtesukiRecord::PSScheme&);
    std::istream& operator>>(std::istream&,
			     osl::ntesuki::NtesukiRecord::PSScheme&);

    std::ostream& operator<<(std::ostream&,
			     const osl::ntesuki::NtesukiRecord::ISScheme&);
    std::istream& operator>>(std::istream&,
			     osl::ntesuki::NtesukiRecord::ISScheme&);

    class NtesukiRecord::VisitLock
    {
      NtesukiRecord* record;
    public:
      VisitLock(NtesukiRecord* record)
	: record(record)
      {
	assert(!record->isVisited());
	record->setVisited();
      }
      ~VisitLock()
      {
	assert(record->isVisited());
	record->resetVisited();
      }
    };

    class NtesukiRecord::UnVisitLock
    {
      NtesukiRecord* record;
    public:
      UnVisitLock(NtesukiRecord* record)
	: record(record)
      {
	assert(record->isVisited());
	record->resetVisited();
      }
      ~UnVisitLock()
      {
	assert(!record->isVisited());
	record->setVisited();
      }
    };
  }//ntesuki
}//osl

#endif /* _NTESUKI_NTESUKI_RECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
