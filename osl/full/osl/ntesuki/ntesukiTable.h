#ifndef _PD_NTESUKI_TABLE_H
#define _PD_NTESUKI_TABLE_H
#include "osl/ntesuki/ntesukiRecord.h"
#include "osl/ntesuki/ntesukiMove.h"
#include "osl/misc/carray.h"
#include "osl/stl/hash_set.h"
#include "osl/pathEncoding.h"
#include "osl/hash/hashKey.h"
#include "osl/stl/hash_map.h"
#include "osl/stl/slist.h"
#include <boost/scoped_ptr.hpp>
#include <stdexcept>

namespace osl
{
  namespace stl
  {
    template <>
    struct hash<Square>{
      unsigned long operator()(const Square& p) const
      {
	return p.uintValue();
      }
    };
  }

  namespace ntesuki
  {
    /**
     * An exception thrown when the table is full.
     */
    struct TableFull : std::runtime_error
    {
      TableFull() : std::runtime_error("table full")
      {
      }
    };

    /**
     * An exception thrown when forEcachRecordFromRoot is called
     * althogh the root state is not set.
     */
    struct RootStateNotSet : std::runtime_error
    {
      RootStateNotSet () : std::runtime_error("root node not set")
      {
      }
    };
    /**
     * A table to hold ntesukiRecord.
     */
    class NtesukiTable
    {
    private:
      typedef hash_map<SignatureKey, NtesukiRecord::RecordList> ntesuki_hash_map;

    public:
      class Table : public ntesuki_hash_map
      {
      public:
	unsigned int capacity, default_gc_size;
	bool verbose, no_gc, gc_request;
	unsigned int numEntry, numCacheHit, gcCount;
	NtesukiRecord *root;
	boost::scoped_ptr<NumEffectState> rootState;
	static int largeGCCount;

	Table(unsigned int capacity,
	      unsigned int default_gc_size,
	      bool verbose);

	~Table();

	/**
	 * @c key に対応する Record を Table から探す.
	 * もし登録されていなかったら新たに登録する.
	 * @param key 局面の Hash値
	 * @return 対応する NtesukiRecord へのポインタ.
	 *         (外部から delete 等してはならない)
	 */
	NtesukiRecord *allocate(const HashKey& key,
				const PieceStand& white_stand,
				signed short distance);

	/**
	 * 表を探す．新たに登録する事はない
	 * @return 存在しなければ0
	 *   そうでなければ内部で確保した場所へのポインタ
	 *   (間違っても delete しないこと)
	 */
	NtesukiRecord *find(const HashKey& key);

	/**
	 * 表に登録された要素を削除する.
	 */
	void erase(const HashKey key);

	/**
	 * テーブルに登録された各 record を F で処理する.
	 */
	template <class F> void forEachRecord(F& f);
	template <class F> void forEachRecordFrom(F&,
						  NumEffectState&,
						  NtesukiRecord *);
	template <class F> void forEachRecordFromRoot(F& f);

	/**
	 * Collect garbage, until the size of the table reduces to
	 * @c gc_size
	 */
	void collectGarbage(unsigned int gc_size);
      };


    private:
      boost::scoped_ptr<Table> table;
      bool verbose;

    public:
      typedef NtesukiRecord record_t;

      struct HashPathEncoding
      {
	unsigned long operator()(PathEncoding const& pe) const
	{
	  return pe.getPath();
	}
      };
      typedef hash_set<PathEncoding, HashPathEncoding> PathSet;

      std::vector<int> depths;

      /**
       * @param capacity 表に保持する最大局面
       */
      NtesukiTable(unsigned int capacity,
		   unsigned int default_gc_size=0,
		   bool verbose=false);
      ~NtesukiTable();

      void clear()
      {
	table->clear();
      }

      Table::const_iterator begin() const
      {
	return table->begin();
      }
      Table::const_iterator end() const
      {
	return table->end();
      }

      /**
       * テーブルをひく. もし要素が見つからなかった場合，
       * テーブルの大きさを増やして良いのなら，
       * 新しい要素を allocate する.
       */
      NtesukiRecord *allocateRoot(const HashKey& key,
				  const PieceStand& white_stand,
				  signed short distance,
				  const NumEffectState* root_state = NULL)
      {
	table->root = table->allocate(key, white_stand, distance);
	if (root_state)
	{
	  table->rootState.reset(new NumEffectState(*root_state));
	}
	return table->root;
      }

      NtesukiRecord *allocateWithMove(NtesukiRecord* record,
				      const NtesukiMove& move)
      {
	/* 毎回 white stand を計算しているのは無駄 */
	PieceStand white_stand = record->white_stand;
	const Move m = move.getMove();
	if (!move.isPass() && m.player())
	{
	  if (m.isDrop())
	  {
	    white_stand.sub(m.ptype());
	  }
	  else if (m.capturePtype() != PTYPE_EMPTY)
	  {
	    white_stand.add(unpromote(m.capturePtype()));
	  }
	}
	unsigned short child_distance = record->distance + 1;
	NtesukiRecord *child = table->allocate(record->key.newHashWithMove(m),
					       white_stand, child_distance);
	if (child)
	{
	  child->distance = std::min(child->distance, child_distance);
	  child->checkNewParent(record);
	}
	return child;
      }

      /**
       * テーブルの大きさを変化させずに find する.
       */
      NtesukiRecord *find(const HashKey& key)
      {
	return table->find(key);
      }

      const NtesukiRecord *find(const HashKey& key) const
      {
	return table->find(key);
      }

      /**
       * 表に登録された要素を削除する.
       */
      void erase(const HashKey key)
      {
	table->erase(key);
      }

      /**
       * 表を整理する.
       */
      void collectGarbage(unsigned int gc_size)
      {
	table->collectGarbage(gc_size);
      }

      /**
       * 与えられた @param move に格納されている手を返そうとする.
       * なかったらテーブルから引く.
       */
      NtesukiRecord *findWithMove(NtesukiRecord *record,
				  const NtesukiMove& move)
      {
	/* テーブルを調べ，あったら move に登録しておく
	 *
	 * (関係ないところから持ってきた move だと
	 *  record にない piece を drop する可能性が)
	 */
	if (move.isNormal() && move.isDrop())
	{
	  const Ptype drop_type = unpromote(move.getMove().ptype());
	  const PieceStand ps = record->getPieceStandSlow(move.getMove()
							  .player());
	  if (ps.get(drop_type) == 0)
	  {
	    return NULL;
	  }
	}
	NtesukiRecord *child = table->find(record->key.newHashWithMove(move.getMove()));
	if (child)
	{
	  child->checkNewParent(record);
	}
	return child;
      }

      NtesukiRecord *findWithMoveConst(const NtesukiRecord *record,
				       const NtesukiMove& move)
      {
	/* テーブルを調べ，あったら move に登録しておく
	 *
	 * (関係ないところから持ってきた move だと
	 *  record にない piece を drop する可能性が)
	 */
	if (move.isNormal() && move.isDrop())
	{
	  const Ptype drop_type = unpromote(move.getMove().ptype());
	  const PieceStand ps = record->getPieceStandSlow(move.getMove()
							  .player());
	  if (ps.get(drop_type) == 0)
	  {
	    return NULL;
	  }
	}
	return table->find(record->key.newHashWithMove(move.getMove()));
      }

      /**
       * テーブルに登録された各 record を F で処理する.
       */
      template <class F> void forEachRecord(F& f)
      {
	table->forEachRecord<F>(f);
      }

      /**
       * テーブルを root node から順番に調べる.
       */
      template <class F> void forEachRecordFromRoot(F& f)
      {
	table->forEachRecordFromRoot<F>(f);
      }

      /**
       * テーブルに登録された record の数.
       */
      unsigned int size() const
      {
	return table->numEntry;/* not size() */
      }

      unsigned int capacity() const
      {
	return table->capacity;
      }

      void lockGC()
      {
	table->no_gc = true;
      }

      void unlockGC()
      {
	table->no_gc = false;
	if (table->gc_request && (table->default_gc_size > 0))
	{
	  table->collectGarbage(table->default_gc_size);
	  table->gc_request = false;
	}
      }

      bool isVerbose() const;
    };
  } //ntesuki
}// osl

#endif /* _PD_NTESUKI_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
