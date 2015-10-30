/* generalSimpleHashTable.h
 */
#ifndef _GENERALSIMPLE_HASHTABLE_H
#define _GENERALSIMPLE_HASHTABLE_H

#include <stdexcept>
#include <memory>

namespace osl
{
  namespace hash
  {
    class HashKey;
  }
  namespace container
  {
  struct TableFull : std::runtime_error
  {
    TableFull() : std::runtime_error("table full")
    {
    }
  };

  /**
   * 基本的な hash table
   * とりあえず g++ (SGI STL) の hash_map を使って実装
   *
   * 機能:
   * - もし既に登録されている局面に再び登録があったら，全て上書きする
   * - メモリがあふれたら単に無視する
   *
   * ある程度基本的な機能を実装したら，自分で実装しなおすほうがbetter。
   * この hash_map では GCを実装することは困難と思われるため
   *
   * find, allocate で ポインタを返すため，要素を追加しても，既存の要素の
   * アドレスが変化しないデータ構造を用いる必要がある．
   */
  template <typename Record>
  class GeneralSimpleHashTable
  {
  protected:
    struct Table;
    std::unique_ptr<Table> table;
  public:
    typedef hash::HashKey HashKey;

    /**
     * @param capacity 表に保持する最大局面
     */
    explicit GeneralSimpleHashTable(size_t capacity=100000);
    ~GeneralSimpleHashTable();
    void clear();

    /**
     * 表を探し，登録されてなければ新規エントリを登録する
     * @return テーブルがいっぱい。
     *   そうでなければ内部で確保した場所へのポインタ
     *   (間違っても delete しないこと)
     * @throw TableFull
     */
    Record *allocate(const HashKey& key);
    /**
     * 表を探す．新たに登録する事はない
     * @return 存在しなければ0
     *   そうでなければ内部で確保した場所へのポインタ
     *   (間違っても delete しないこと)
     */
    Record *find(const HashKey& key);
    const Record *find(const HashKey& key) const;

    size_t size() const;
    size_t capacity() const;
    int numCacheHit() const;
    int numRecordAfterFull() const;

    bool isVerbose() const;
    /** lock contention を下げるために分割した大きさ */
    int divSize() const;
  };
} // namespace container
  using container::TableFull;
  using container::GeneralSimpleHashTable;
} // namespace osl

#endif /* _GENERALSIMPLE_HASHTABLE_H_ */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
