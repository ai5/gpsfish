/* arrayCheckHashTable.h
 */
#ifndef _ARRAY_CHECK_HASHTABLE_H
#define _ARRAY_CHECK_HASHTABLE_H

#include "sameBoardList.h"
#include "twinTable.h"
#include "visitedCounter.h"
#include "osl/hash/hashKey.h"
#include <boost/scoped_array.hpp>
#include <cassert>

#define BOOST_DISABLE_ASSERTS
namespace osl
{
  class PathEncoding;
  namespace checkmate
  {
    class CheckHashRecord;
    /**
     * 詰将棋用のテーブル
     * SimpleCheckHashTable の置換
     *
     * - bucket の先頭に要素を一つ配置しているところが普通のchain hash と違う
     * - 類似局面を利用するために、まず盤上の駒で分類し、次に持駒で分類する
     */
    class ArrayCheckHashTable : public TwinTableHolder, public VisitedCounter
    {
      struct BoardEntry
      {
	hash::BoardKey board_key;
	SameBoardList colleagues;
	void setKey(const HashKey& key)
	{
	  board_key = key.boardKey();
	}
	bool unused() const { return colleagues.empty(); }
	/** value(1)は比較済であることが前提 */
	bool equalKey(const HashKey& key) const
	{
	  return board_key == key.boardKey();
	}
	CheckHashRecord *find(const HashKey& key)
	{
	  return colleagues.find(key.blackStand());
	}
	const CheckHashRecord *find(const HashKey& key) const
	{
	  return colleagues.find(key.blackStand());
	}
	template <Player Attacker>
	CheckHashRecord *allocate(const HashKey& key, 
				  const PieceStand& white_stand,
				  const PathEncoding& path, size_t& counter)
	{
	  return colleagues.template allocate<Attacker>
	    (key.blackStand(), white_stand, path, counter);
	}
      };
      typedef slist<BoardEntry> list_t;
      struct ElementList
      {
	BoardEntry elem;
	list_t list;

	template <Player Attacker>
	CheckHashRecord *allocate(const HashKey& key, 
				  const PieceStand& white_stand,
				  const PathEncoding& path, size_t& counter)
	{
	  if (elem.equalKey(key))
	    return elem.template allocate<Attacker>(key, white_stand, path, counter);
	  if (elem.unused())
	  {
	    elem.setKey(key);
	    return elem.template allocate<Attacker>(key, white_stand, path, counter);
	  }
	  for (list_t::iterator p=list.begin(); p!=list.end(); ++p)
	  {
	    if (p->equalKey(key))
	      return (*p).template allocate<Attacker>(key, white_stand, path, counter);
	  }
	  list.push_front(BoardEntry());
	  list.front().setKey(key);
	  return list.front().template allocate<Attacker>(key, white_stand, path, counter);
	}
  
	CheckHashRecord *find(const HashKey& key)
	{
	  if (elem.equalKey(key))
	    return elem.find(key);
	  for (list_t::iterator p=list.begin(); p!=list.end(); ++p)
	  {
	    if (p->equalKey(key))
	      return p->find(key);
	  }
	  return 0; 
	}
	const CheckHashRecord *find(const HashKey& key) const
	{
	  if (elem.equalKey(key))
	    return elem.find(key);
	  for (list_t::const_iterator p=list.begin(); p!=list.end(); ++p)
	  {
	    if (p->equalKey(key))
	      return p->find(key);
	  }
	  return 0; 
	}
      };
      static const size_t bucketSize = 786433ul; // 1572869ul;
      boost::scoped_array<ElementList> buckets;
      size_t numElements;
      const Player attacker;
      static unsigned int makeHash(const HashKey& key)
      {
	return key.signature() % bucketSize;
      }
      CheckHashRecord rootNode;
    public:
      explicit ArrayCheckHashTable(Player attacker);
      ~ArrayCheckHashTable();
      CheckHashRecord *find(const HashKey& key);
      CheckHashRecord *allocate(const HashKey& key,
				const PieceStand& white_stand,
				const PathEncoding& path);
      CheckHashRecord *root() { return &rootNode; }
      void clear();

      const CheckHashRecord *find(const HashKey& key) const;
      size_t size() const { return numElements; }
      Player getAttacker() const { return attacker; }
      void confirmNoVisitedRecords() const;
    };

    inline
    const CheckHashRecord* ArrayCheckHashTable::find(const HashKey& key) const
    {
      const ElementList& e = buckets[makeHash(key)];
      return e.find(key);
    }
    inline
    CheckHashRecord* ArrayCheckHashTable::find(const HashKey& key)
    {
      ElementList& e = buckets[makeHash(key)];
      return e.find(key);
    }
    inline
    CheckHashRecord* ArrayCheckHashTable::
    allocate(const HashKey& key, const PieceStand& white_stand,
	     const PathEncoding& path)
    {
      ElementList& e = buckets[makeHash(key)];
      if (attacker == BLACK)
	return e.allocate<BLACK>(key, white_stand, path, numElements);
      else
	return e.allocate<WHITE>(key, white_stand, path, numElements);
    }

  } // namespace checkmate
} // namespace osl

#endif /* _ARRAY_CHECK_HASHTABLE_H_ */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
