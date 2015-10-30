#ifndef _OPENING_BOOK_H
#define _OPENING_BOOK_H
#include "osl/book/compactBoard.h"
#include "osl/basic_type.h"
#include "osl/numEffectState.h"
#include <fstream>
#include <functional>

namespace osl
{
  namespace book
  {
    class OMove
    {
    public:
      OMove(int i) { value = i; }
      OMove(Move m) { 
	const Square from = m.from();
	const Square to = m.to();
	const int bitFrom = (from.isPieceStand() ? 0 : 
			     (from.x() << 4 | from.y()));
	const int bitTo = (to.isPieceStand() ? 0 : 
			   (to.x() << 12 | to.y() << 8));
	value = (bitFrom | bitTo |
		 static_cast<unsigned int>(m.isPromotion()) << 19 |
		 static_cast<unsigned int>(m.capturePtype()) << 20 |
		 static_cast<unsigned int>(m.ptype()) << 24 |
		 static_cast<int>(m.player()) << 28);
      }
      Square from() {
	if ((value & 0xff) == 0)
	  return Square::STAND();
	else
	  return Square((value >> 4) & 0xf, value & 0xf);
      }
      Square to() {
	if (((value >> 8) & 0xff) == 0)
	  return Square::STAND();
	else
	  return Square((value >> 12) & 0xf, (value >> 8) & 0xf);
      }
      bool isPromotion() { return (value >> 19) & 1; }
      Ptype capturePtype() {
	return static_cast<Ptype>((value >> 20) & 0xf);
      }
      Ptype ptype() {
	return static_cast<Ptype>((value >> 24) & 0xf);
      }
      Player player() {
	return static_cast<Player>((value) >> 28);
      }
      operator Move() { return Move(from(), to(), ptype(),
				    capturePtype(), isPromotion(),
				    player()); }
      operator int() { return value; }
    private:
      int value;
    };

    struct OBMove
    {
      Move move;
      int state_index;
      int stateIndex() const { return state_index; }
    };

    /**
     * StateとOBMoveを保持する.
     * Stateはvector<OBMove>と黒から見たwinCount, loseCountを保持する
     * OBMoveはMoveとそのMoveを採用した時のStateのindex
     * ファイル形式
     * state数 - 4byte
     * State - 16byte * state数
     *   + 黒のwinCount
     *   + 白のwinCount
     *   + OBMoveの数 
     *   + OBMoveの開始index
     * OBMove - 8byte * OBMove数
     *   + Move (4byte)
     *   + Stateのindex
     */
    class WinCountBook
    {
      int nStates;
      std::ifstream ifs;
    public:
      WinCountBook(const char *filename);
      ~WinCountBook();
      int winCount(int stateIndex);
      int loseCount(int stateIndex);
      std::vector<OBMove> moves(int stateIndex);
    private:
      int readInt();
      void seek(int offset);
    };

    struct WMove
    {
      Move move;
      int state_index;
      int weight;

      int stateIndex() const { return state_index; }
      void setWeight(const int w) { weight = w; };
    };
    std::ostream& operator<<(std::ostream&, const WMove& w);
    std::istream& operator>>(std::istream&, WMove& w);

    inline bool operator==(const WMove& l, const WMove& r) 
    {
      return l.move == r.move && l.stateIndex() == r.stateIndex()
	&& l.weight == r.weight;
    }

    /**
     * WMoveのWeightによるsort
     */
    struct WMoveSort : public std::binary_function<WMove, WMove, bool>
    {
      bool operator()(const WMove& l, const WMove& r) const {
	return l.weight > r.weight;
      }
    };

    /**
     * WMoveのMoveによるsort
     */
    struct WMoveMoveSort : public std::binary_function<WMove, WMove, bool>
    {
      bool operator()(const WMove& l, const WMove& r) const  {
	return l.move.intValue() < r.move.intValue();
      }
    };

    /**
     * WMoveのWeightとMoveによるsort
     */
    struct WMoveWeightMoveSort : public std::binary_function<WMove, WMove, bool>
    {
      bool operator()(const WMove& l, const WMove& r) const {
	if (l.weight != r.weight)
	  return l.weight > r.weight;
	return l.move.intValue() < r.move.intValue();
      }
    };

    /**
     * StateとWMoveを保持する.
     * Stateはvector<WMove>を保持する
     * WMoveはMoveとそのMoveを採用した時のStateのindexと手番から見た
     * Moveの重み(0-1000)をもつ
     * ファイル形式
     * version番号 - 4byte
     * state数 - 4byte
     * move数 - 4byte
     * 開始state index - 4byte
     * State - 16byte * state数
     *   + WMoveの開始index
     *   + WMoveの数 
     *   + 先手の勝数
     *   + 後手の勝数
     * WMove - 12byte * WMove数
     *   + Move (4byte)
     *   + Stateのindex
     *   + Weight
     * CompactBoard形式の盤面 - 164byte * state数
     */
    class WeightedBook
    {
      int n_states;
      int n_moves;
      int start_state;
      std::ifstream ifs;
    public:
      typedef std::vector<WMove> WMoveContainer;

      WeightedBook(const char *filename);
      ~WeightedBook();
      /**
       * Return moves from the state of the stateIndex. If the zero_include is
       * true, all of the moves are returned. Otherwise, the moves that
       * have some weights (i.e. non-zero value) are returned.
       */
      WMoveContainer moves(int stateIndex, const bool zero_include = true);
      int whiteWinCount(int stateIndex);
      int blackWinCount(int stateIndex);
      CompactBoard compactBoard(int stateIndex);
      SimpleState board(int stateIndex);
      int totalState() const { return n_states; }
      int startState() const { return start_state; }
      void validate();
      /** 
       * As traversing the 'tree', return all state indices of the state's
       * parents. 
       * @return state indexes; empty if there is none.
       */
      std::vector<int> parents(const int stateIndex);
      /** 
       * As traversing the 'tree', find a state index of the state.  If
       * the visit_zero is true zero-weighted moves are visited (in this
       * case, player is ignored). Otherwise, the palyer's zero-weighted
       * moves are not visited.
       *
       * @param state to find
       * @param visit_zero
       * @param player
       * @return a state index of the state; if it is not found, return -1.
       */
      int stateIndex(const SimpleState& state, 
			const bool visit_zero = true, 
			const Player player = BLACK);
      /** 
       * As traversing the 'tree', find a state index of the state reached
       * by applying the moves from the initial state.
       * Note that zero-weighted moves are visited.
       *
       * @param moves to apply
       * @return state index; if it is not found, return -1.
       */
      int stateIndex(const std::vector<Move>& moves);
    private:
      void seek(int offset);
      static const int HEADER_SIZE = 16;
      static const int STATE_SIZE = 16;
      static const int MOVE_SIZE = 12;
      static const int BOARD_SIZE = 41 * 4;
    };
  } // book
  using book::CompactBoard;
  using book::WeightedBook;
} // namespace osl
#endif // _OPENING_BOOK_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
