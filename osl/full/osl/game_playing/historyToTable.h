/* historyToTable.h
 */
#ifndef GAME_PLAYING_HISTORYTOTABLE_H
#define GAME_PLAYING_HISTORYTOTABLE_H
namespace osl
{
  class Move;
  namespace hash
  {
    class HashKey;
  }
  namespace search
  {
    class SimpleHashTable;
    class HashRejections;
  }
  namespace game_playing
  {
    class GameState;
    struct PVHistory;
    struct HistoryToTable
    {
      /** table に書き込む深さ */
      static const int LIMIT;
      /**
       * key の局面の持駒の増減させた局面を記録
       */
      static void adjustDominance(const hash::HashKey& key, 
				  search::SimpleHashTable& table,
				  int black_win, int white_win,
				  const Move& good_move);
      /**
       * table に千日手情報，水平線対策情報を記録
       */
      static void adjustTable(const GameState&, 
			      search::SimpleHashTable& table,
			      int black_win, int draw, int white_win);
      static void setPV(const PVHistory&, const GameState&, 
			search::SimpleHashTable& table);
    };
  } // namespace game_playing
} // namespace osl

#endif /* GAME_PLAYING_HISTORYTOTABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
