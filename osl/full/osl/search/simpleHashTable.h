/* simpleHashTable.h
 */
#ifndef OSL_SIMPLE_HASHTABLE_H
#define OSL_SIMPLE_HASHTABLE_H

#include "osl/search/generalSimpleHashTable.h"
#include "osl/container.h"
namespace osl
{
  namespace search
  {
    class SimpleHashRecord;

    /**
     * 基本的な hash table.
     * とりあえず g++ (SGI STL) の hash_map を使って実装
     *
     * 機能:
     * - lower bound と upper bound を登録する。
     * - もし既に登録されている局面に再び登録があったら，全て上書きする
     * - メモリがあふれたら単に無視する
     * - upperBound と lowerBound は player 依存(betterThan)なので
     *   手番の違う同一局面があると破綻する
     * - 最善手の登録
     *
     * ある程度基本的な機能を実装したら，自分で実装しなおすほうがbetter。
     * この hash_map では GCを実装することは困難と思われるため
     *
     * find, allocate で ポインタを返すため，要素を追加しても，既存の要素の
     * アドレスが変化しないデータ構造を用いる必要がある．
     */
    class SimpleHashTable : private container::GeneralSimpleHashTable<SimpleHashRecord>
    {
    private:
      int minimum_limit;
      int verbose;
    public:
      /**
       * @param capacity 表に保持する最大局面
       * @param minimumRecordLimit recordUpperBound, recordLowerBound において
       *    limit がこれ未満のものは登録要求を無視する.
       *    末端の静止探索も記録する場合はマイナスにする
       */
      explicit SimpleHashTable(size_t capacity=100000, 
			       int minimum_record_limit=0,
			       int verbose=0);
      ~SimpleHashTable();

      using GeneralSimpleHashTable<SimpleHashRecord>::clear;
      /**
       * @param new_limit recordUpperBound, recordLowerBound において 
       *   limit がこれ未満のものは登録要求を無視する
       */
      void setMinimumRecordLimit(int new_limit);

      /**
       * 表を探し，登録されてなければ新規エントリを登録する
       * @return テーブルがいっぱいだったりlimit が小さすぎると0。
       *   そうでなければ内部で確保した場所へのポインタ
       *   (間違っても delete しないこと)
       * @throw TableFull
       */
      SimpleHashRecord *allocate(const HashKey& key, int limit);

      /**
       * 表を探す．新たに登録する事はない
       * @return 存在しなければ0
       *   そうでなければ内部で確保した場所へのポインタ
       *   (間違っても delete しないこと)
       */
      using GeneralSimpleHashTable<SimpleHashRecord>::find;

      int minimumRecordLimit() const;
      using GeneralSimpleHashTable<SimpleHashRecord>::size;
      using GeneralSimpleHashTable<SimpleHashRecord>::capacity;

      void setVerbose(int verbose=1);
      int verboseLevel() const;
      bool isVerbose() const { return verboseLevel(); }

      bool isConsistent() const;
      int divSize() const;

      void getPV(const HashKey&, MoveVector&, size_t *quiesce_start=0) const;
      uint64_t memoryUse() const;
    };
    
  } // namespace search

  using search::SimpleHashTable;
} // namespace osl

#endif /* OSL_SIMPLE_HASHTABLE_H_ */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
