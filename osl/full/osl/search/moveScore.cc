/* moveScore.cc
 */
#include "osl/search/moveScore.h"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_generator/escape_.tcc"
#include "osl/move_generator/addEffectWithEffect.h"
#include "osl/move_generator/addEffectWithEffect.tcc"
#include "osl/move_generator/capture_.h"
#include "osl/move_generator/capture_.tcc"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/allMoves.tcc"
#include "osl/move_generator/drop.h"
#include "osl/move_generator/drop.tcc"
#include <algorithm>
#include <functional>

osl::search::MoveScore* osl::search::MoveScore::
sortPositive(MoveScore *first, MoveScore *last) 
{
  MoveScore *p=first-1, *q=last;
  while (p != q) {
    while (++p != q && p->score > 0)
      ;
    if (p != q) {
      while (--q != p && q->score <= 0)
	;
      std::swap(*p, *q);
    }
  } 
  std::stable_sort(first, p, std::greater<MoveScore>());
  return p;
}

namespace osl
{
  namespace search
  {
    struct Store{
      MoveScore* out;
      Store(MoveScore* o) :out(o) {}
      void simpleMove(Square /*from*/,Square /*to*/,Ptype /*ptype*/, bool /*isPromote*/,Player /*p*/,Move move){
	(*out++).move = move;
      }
      void unknownMove(Square /*from*/,Square /*to*/,Piece /*p1*/,Ptype /*ptype*/,bool /*isPromote*/,Player /*p*/,Move move)
      {
	(*out++).move = move;
      }
      void dropMove(Square /*to*/,Ptype /*ptype*/,Player /*p*/,Move move)
      {
	(*out++).move = move;
      }
      // old interfaces
      void simpleMove(Square from,Square to,Ptype ptype, 
		      bool isPromote,Player p)
      {
	simpleMove(from,to,ptype,isPromote,p,
		   Move(from,to,ptype,PTYPE_EMPTY,isPromote,p));
      }
      void unknownMove(Square from,Square to,Piece captured,
		       Ptype ptype,bool isPromote,Player p)
      {
	unknownMove(from,to,captured,ptype,isPromote,p,
		    Move(from,to,ptype,captured.ptype(),isPromote,p));
      }
      void dropMove(Square to,Ptype ptype,Player p)
      {
	dropMove(to,ptype,p,
		 Move(to,ptype,p));
      }
    };
    struct NoCaptureStore{
      MoveScore* out;
      NoCaptureStore(MoveScore* o) :out(o) {}
      void simpleMove(Square /*from*/,Square /*to*/,Ptype /*ptype*/, bool /*isPromote*/,Player /*p*/,Move move){
	(*out++).move = move;
      }
      void unknownMove(Square /*from*/,Square /*to*/,Piece p1,Ptype /*ptype*/,bool /*isPromote*/,Player /*p*/,Move move)
      {
	if(p1.isEmpty())
	  (*out++).move = move;
      }
      void dropMove(Square /*to*/,Ptype /*ptype*/,Player /*p*/,Move move)
      {
	(*out++).move = move;
      }
      // old interfaces
      void simpleMove(Square from,Square to,Ptype ptype, 
		      bool isPromote,Player p)
      {
	(*out++).move = Move(from,to,ptype,PTYPE_EMPTY,isPromote,p);
      }
      void unknownMove(Square from,Square to,Piece captured,
		       Ptype ptype,bool isPromote,Player p)
      {
	if(captured.isEmpty())
	  (*out++).move = Move(from,to,ptype,captured.ptype(),isPromote,p);
      }
      void dropMove(Square to,Ptype ptype,Player p)
      {
	(*out++).move = Move(to,ptype,p);
      }
    };
  }
}

template <osl::Player P>
osl::search::MoveScore* osl::search::MoveScore::
generateCapture(const NumEffectState& state, MoveScore *out) 
{
  Store store(out);
  for(int num=0;num<Piece::SIZE;num++){
    Piece p=state.pieceOf(num);
    if(p.isOnBoardByOwner<alt(P)>())
      move_generator::Capture<Store>::generate<P>
	(state,p.square(),store);
  }
  return store.out;
}
osl::search::MoveScore* osl::search::MoveScore::
generateCapture(const NumEffectState& state, MoveScore *out) 
{
  if (state.turn() == BLACK)
    return generateCapture<BLACK>(state, out);
  else
    return generateCapture<WHITE>(state, out);
}

osl::search::MoveScore* osl::search::MoveScore::
generateNoCapture(const NumEffectState& state, MoveScore *out) 
{
  NoCaptureStore store(out);
  if (state.turn() == BLACK)
    move_generator::AllMoves<NoCaptureStore>::generate<BLACK>(state,store);
  else
    move_generator::AllMoves<NoCaptureStore>::generate<WHITE>(state,store);
  return store.out;
}

osl::search::MoveScore* osl::search::MoveScore::
generateAll(const NumEffectState& state, MoveScore *out) 
{
  Store store(out);
  if (state.turn() == BLACK)
    move_generator::AllMoves<Store>::generate<BLACK>(state,store);
  else
    move_generator::AllMoves<Store>::generate<WHITE>(state,store);
  return store.out;
}

osl::search::MoveScore* osl::search::MoveScore::
generateCheckNoCapture(const NumEffectState& state, MoveScore *out) 
{
  const Square king = state.kingSquare(alt(state.turn()));
  NoCaptureStore store(out);
  if (state.turn() == BLACK)
    move_generator::AddEffectWithEffect<NoCaptureStore>::
      generate<BLACK,true>(state,king,store);
  else
    move_generator::AddEffectWithEffect<NoCaptureStore>::
      generate<WHITE,true>(state,king,store);
  return store.out;
}

osl::search::MoveScore* osl::search::MoveScore::
generateKingEscape(const NumEffectState& state, MoveScore *out) 
{
  Store store(out);
  if (state.turn() == BLACK)
    move_generator::Escape<Store>::generateKingEscape<BLACK,false>
      (state,store);
  else
    move_generator::Escape<Store>::generateKingEscape<WHITE,false>
      (state,store);
  return store.out;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
