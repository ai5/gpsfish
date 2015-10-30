#if !defined(POSITION_TCC_INCLUDED)
#define POSITION_TCC_INCLUDED
#include "position.h"
#include "tt.h"
#include "osl/basic_type.h"
template<typename F>
void Position::do_undo_move(Move m, StateInfo& newSt,F const& f){
  assert(is_ok());
  assert(move_is_ok(m));
  assert(&newSt != st);
  assert(move_is_legal(m));

  nodes++;
  Key key = st->key;
  struct ReducedStateInfo {
    int gamePly, pliesFromNull;
    Key key;
  };
  memcpy(&newSt, st, sizeof(ReducedStateInfo));

  newSt.previous = st;
  st = &newSt;

  // Save the current key to the history[] array, in order to be able to
  // detect repetition draws.
  history[st->gamePly++] = key;

  // Update side to move
  key ^= zobSideToMove;

  st->pliesFromNull++;


  Color us = side_to_move();
  Color them = opposite_color(us);
  Square from = move_from(m);
  Square to = move_to(m);

  PieceType pt=m.ptype();
  osl::Ptype capture = m.capturePtype();
  st->capturedType = capture;
  if(capture!=osl::PTYPE_EMPTY){
    key -= zobrist[them][(int)capture][to.index()];
    key += zobrist[us][unpromote(capture)][Square::STAND().index()];
  }
  // Update hash key
  if(move_is_promotion(m))
    key += zobrist[us][(int)pt][to.index()]-zobrist[us][(int)unpromote(pt)][from.index()];
  else
    key += zobrist[us][(int)pt][to.index()]-zobrist[us][(int)pt][from.index()];

  st->key = key;
  prefetch((char*)TT.first_entry(key));
  int old_cont=continuous_check[us];
  continuous_check[us]=(move_gives_check(m) ? old_cont+1 : 0);
  osl_state.makeUnmakeMove(m,f);
  continuous_check[us]=old_cont;
  st = st->previous;
}

template<typename F>
void Position::do_undo_null_move(StateInfo& backupSt, F const& f){
  assert(is_ok());
  backupSt.key      = st->key;
  backupSt.previous = st->previous;
  backupSt.pliesFromNull = st->pliesFromNull;
  st->previous = &backupSt;
  history[st->gamePly++] = st->key;
  st->key ^= zobSideToMove;
  prefetch((char*)TT.first_entry(st->key));
  st->pliesFromNull = 0;
  Color us = side_to_move();
  int old_cont=continuous_check[us];
  continuous_check[us]=0;
  osl_state.makeUnmakePass(f);
  continuous_check[us]=old_cont;
  st->key      = backupSt.key;
  st->previous = backupSt.previous;
  st->pliesFromNull = backupSt.pliesFromNull;

  // Update the necessary information
  st->gamePly--;
}
#endif // !defined(POSITION_TCC_INCLUDED)
