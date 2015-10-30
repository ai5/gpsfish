/* repetitionCounter.h
 */
#ifndef OSL_REPETITIONCOUNTER_H
#define OSL_REPETITIONCOUNTER_H

#include "osl/numEffectState.h"
#include "osl/hash/hashKeyStack.h"
#include "osl/sennichite.h"
#include <list>
#include <vector>
#include <cassert>
#include <iosfwd>
namespace osl
{
  /**
   * 千日手の検出.
   * 連続王手の千日手(同一局面の最初と4回目の間の一方の指し手が王手のみだった場合)は、連続王手をかけていたほうが負け。
   * http://www.computer-shogi.org/wcsc14/youryou.html
   */
  class RepetitionCounter
  {
    struct Table;
    std::unique_ptr<Table> table;
    CArray<std::vector<int>, 2> continuous_check;
    HashKeyStack hash_history;
    int order() const { return hash_history.size(); }
  public:
    typedef std::list<int> list_t;

    RepetitionCounter();
    RepetitionCounter(const RepetitionCounter& c);
    explicit RepetitionCounter(const NumEffectState& initial);
    ~RepetitionCounter();

  private:
    void push(const HashKey& new_key, bool is_check);
  public:    
    /**
     * state の状態で move を(これから)指すことを記録
     */
    void push(const NumEffectState& state, Move move);
    /**
     * 指した後の局面を記録
     */
    void push(const NumEffectState& state);
    /**
     * 指した後の局面を記録
     */
    void push(const HashKey& key, const NumEffectState& state);
    void pop();
    void clear();
    
    const Sennichite isSennichite(const NumEffectState& state, Move move) const;
  private:
    const Sennichite isAlmostSennichiteUnsafe(int first_move) const
    {
      assert(first_move >= 0);
      const int duration = (order() - first_move) / 2;
      if (continuous_check[BLACK].back() >= duration)
	return Sennichite::BLACK_LOSE();
      if (continuous_check[WHITE].back() >= duration)
	return Sennichite::WHITE_LOSE();
      return Sennichite::DRAW();
    }    
  public:
    /**
     * このまま同形を繰り返したらどの結果になるかを返す
     */
    const Sennichite isAlmostSennichite(const HashKey& key) const
    {
      const int first_move = getFirstMove(key);
      if (first_move < 0)
	return Sennichite::NORMAL();
      return isAlmostSennichiteUnsafe(first_move);
    }    
    /** @return pair<isAlmostSennichite, count> */
    const std::pair<Sennichite,int> distanceToSennichite(const HashKey& key) const;
    unsigned int countRepetition(const HashKey&) const;
    const list_t getRepetitions(const HashKey&) const;
    void printMatches(const HashKey& key) const;
    /**
     * key の手を最後に登録した指手番号.
     * @return 初めての局面では-1 
     */
    int getLastMove(const HashKey& key) const;
    /**
     * key の手を最初に登録した指手番号.
     * @return 初めての局面では-1 
     */
    int getFirstMove(const HashKey& key) const;
    int checkCount(Player attack) const { 
      assert(! continuous_check[attack].empty());
      return continuous_check[attack].back(); 
    }
    const HashKeyStack& history() const { return hash_history; }
    bool isConsistent() const;

    static bool maybeEqual(const RepetitionCounter& l, const RepetitionCounter& r);
  };

}

#endif /* OSL_REPETITIONCOUNTER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
