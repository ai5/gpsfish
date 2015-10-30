/* see.cc
 */
#include "osl/eval/see.h"
#include "osl/eval/ptypeEval.h"
#include "osl/move_generator/effect_action.h"

struct osl::eval::See::FindEffectMore
{
  PtypeOSquareVector *direct;
  PtypeOSquareVector *more;
  Square target;
  const NumEffectState *state;
  
  template<Player P,Ptype Type>
  void doActionPtype(Piece p) { store(p); }
  template<Player P>
  void doAction(Piece p, Square) { store(p);}
  void store(Piece p);  
};

void osl::eval::See::
FindEffectMore::store(Piece p)
{
  direct->push_back(std::make_pair(p.ptypeO(), p.square()));
  findAdditionalPieces(*state, p.owner(), target, p.square(), *more);
}

template <osl::Player P>
void osl::eval::
See::findEffectPieces(const NumEffectState& state, Square effect_to,
		      PtypeOSquareVector& my_pieces, 
		      PtypeOSquareVector& op_pieces)
{
  typedef effect_action::StorePtypeOSquare store_t;
  store_t op_pieces_store(&op_pieces, effect_to);
  state.forEachEffect<alt(P),store_t>(effect_to, op_pieces_store);
  if (op_pieces.empty())
    return;
  op_pieces.sort();
  if ((int)op_pieces.size() <= state.countEffect(P, effect_to))
  {
    store_t my_pieces_store(&my_pieces, effect_to);
    state.forEachEffect<P,store_t>(effect_to, my_pieces_store);	// ignore my_pin
    my_pieces.sort();
    return;
  }
  PtypeOSquareVector my_pieces_more;
  FindEffectMore action = { &my_pieces, &my_pieces_more, effect_to, &state };
  state.forEachEffect<P,FindEffectMore>(effect_to, action); // ignore my_pin
  my_pieces.sort();
  // sort my_pieces_more ?
  my_pieces.push_back(my_pieces_more.begin(), my_pieces_more.end());

  if (op_pieces.size() <= my_pieces.size())
    return;
  my_pieces_more.clear();
  // gather shadow efect
  for (size_t i=0; i<op_pieces.size(); ++i) {
    findAdditionalPieces(state, P, effect_to, op_pieces[i].second, my_pieces_more);
  }
  my_pieces.push_back(my_pieces_more.begin(), my_pieces_more.end());
}

template <osl::Player P>
void osl::eval::
See::findEffectPiecesAfterMove(const NumEffectState& state, Move move,
			       PtypeOSquareVector& my_pieces, 
			       PtypeOSquareVector& op_pieces)
{
  const Square from=move.from();
  const Square to=move.to();

  typedef effect_action::StorePtypeOSquare store_t;
  store_t op_pieces_store(&op_pieces, to);
  state.forEachEffect<alt(P),store_t>(to, op_pieces_store);
  if (op_pieces.empty())
    return;
  op_pieces.sort();

  const Piece moved = state.pieceOnBoard(from);
  PieceMask ignore;		// here do not use my_pin to get optimistic result
  ignore.set(moved.number());
  if ((int)op_pieces.size() < state.countEffect(P, to))
  {
    store_t my_pieces_store(&my_pieces, to);
    state.forEachEffect<P,store_t>(to, my_pieces_store, ignore);
    my_pieces.sort();
    return;
  }

  PtypeOSquareVector my_pieces_more;
  findAdditionalPieces(state, move.player(), to, moved.square(), my_pieces_more);

  FindEffectMore action = { &my_pieces, &my_pieces_more, to, &state };
  state.forEachEffect<P,FindEffectMore>(to, action, ignore);
  my_pieces.sort();
  // sort my_pieces_more ?
  my_pieces.push_back(my_pieces_more.begin(), my_pieces_more.end());

  if (op_pieces.size() < my_pieces.size())
    return;
  my_pieces_more.clear();
  // gather shadow efect
  for (size_t i=0; i<op_pieces.size(); ++i) {
    findAdditionalPieces(state, P, to, op_pieces[i].second, my_pieces_more);
  }
  my_pieces.push_back(my_pieces_more.begin(), my_pieces_more.end());
}

template <osl::Player P>
int osl::eval::
See::computeValue(const NumEffectState& state, Move move,
		  PtypeOSquareVector& my_pieces, 
		  PtypeOSquareVector& op_pieces,
		  const PieceMask& my_pin, const PieceMask& op_pin,
		  const eval::PtypeEvalTable& table)
{
  Square target=move.to(), move_from=move.from();
  PtypeO ptypeO=move.ptypeO();

  int val = 0;
  CArray<int,Piece::SIZE> vals;
  const Player Opponent = alt(P);
  size_t i;
  int c=0;
  bool op_deleted=false, my_deleted=false;
  for (i=0;i<op_pieces.size();i++,c++)
  {
    if(c>10) break; // avoid infinite loop
      {
	Square from=op_pieces[i].second;
	Piece p=state.pieceAt(from);
	int num=p.number();
	if(num==KingTraits<Opponent>::index && my_deleted) break;
	assert(p.owner()==Opponent);
	if(op_pin.test(num) && !state.pinnedCanMoveTo<Opponent>(p,target) &&
	   ptypeO!=newPtypeO(P,KING)){
	  Piece attacker=state.pinAttacker<Opponent>(p);
	  assert(attacker.owner()==P);
	  Square attacker_sq=attacker.square();
	  if(attacker_sq != move_from){
	    size_t j=0;
	    for(;j<my_pieces.size();j++) if(my_pieces[j].second==attacker_sq) break;
	    if(i<=j){
	      if(j==my_pieces.size() || op_pieces.size()<=j+1 ){
		for(size_t k=i;k<op_pieces.size()-1;k++)
		  op_pieces[k]=op_pieces[k+1];
		op_pieces.pop_back();
		op_deleted=true;
	      }
	      else{
		std::pair<PtypeO,Square> v=op_pieces[i];
		for(size_t k=i;k<=j;k++)
		  op_pieces[k]=op_pieces[k+1];
		op_pieces[j+1]=v;
	      }
	      i--;
	      continue;
	    }
	  }
	  // pin move?
	}
      }
    vals[i*2]=val;
    // opponent moves
    val+=table.captureValue(ptypeO);
    {
      ptypeO = op_pieces[i].first;
      const bool promotable = canPromote(ptypeO) 
	&& (target.canPromote<Opponent>() 
	    || op_pieces[i].second.canPromote<Opponent>());
      if (promotable)
      {
	ptypeO=promote(ptypeO);
	val+=table.promoteValue(ptypeO);
      }
    }
    vals[i*2+1]=val;
    // my moves
  retry:
    if (i>=my_pieces.size()){
      break;
    }
      {
	Square from=my_pieces[i].second;
	Piece p=state.pieceAt(from);
	int num=p.number();
	assert(p.owner()==P);
	if(num==KingTraits<P>::index && op_deleted) break;
	if(my_pin.test(num) && !state.pinnedCanMoveTo<P>(p,target) &&
	   ptypeO!=newPtypeO(Opponent,KING)){
	  Piece attacker=state.pinAttacker<P>(p);
	  assert(attacker.owner()==Opponent);
	  Square attacker_sq=attacker.square();
	  size_t j=0;
	  for(;j<op_pieces.size();j++) if(op_pieces[j].second==attacker_sq) break;
	  if(i<j){
	    if(j==op_pieces.size() || my_pieces.size()<=j ){
	      for(size_t k=i;k<my_pieces.size()-1;k++)
		my_pieces[k]=my_pieces[k+1];
	      my_pieces.pop_back();
	      my_deleted=true;
	    }
	    else{
	      std::pair<PtypeO,Square> v=my_pieces[i];
	      for(size_t k=i;k<j;k++)
		my_pieces[k]=my_pieces[k+1];
	      my_pieces[j]=v;
	    }
	    goto retry;
	  }
	  // pin move?
	}
      }
    val+=table.captureValue(ptypeO);
    {
      ptypeO=my_pieces[i].first;
      const bool promotable = canPromote(ptypeO) 
	&& (target.canPromote<P>() 
	    || my_pieces[i].second.canPromote<P>());
      if (promotable)
      {
	ptypeO=promote(ptypeO);
	val+=table.promoteValue(ptypeO);
      }
    }
  }
  for (int j=i-1;j>=0;j--)
  {
    val=EvalTraits<P>::max(val,vals[j*2+1]);
    val=EvalTraits<Opponent>::max(val,vals[j*2]);
  }
  return val;
}

template <osl::Player P>
int osl::eval::See::seeInternal(const NumEffectState& state, Move move,
				const PieceMask& my_pin, const PieceMask& op_pin,
				const eval::PtypeEvalTable& table)
{
  assert(state.isAlmostValidMove(move));
  
  const Square from=move.from();
  const Square to=move.to();
  PtypeOSquareVector my_pieces, op_pieces;
  int val=0; 
  if (from.isPieceStand())
  {
    findEffectPieces<P>(state, to, my_pieces, op_pieces);
  }
  else
  {
    val = Ptype_Eval_Table.diffWithMove(state,move);
    findEffectPiecesAfterMove<P>(state, move, my_pieces, op_pieces);
  }
  if (op_pieces.empty())
    return val;
  return val + computeValue<P>(state, move, my_pieces, op_pieces, my_pin, op_pin, table);
}

int osl::eval::See::see(const NumEffectState& state, Move move,
			const PieceMask& my_pin, const PieceMask& op_pin,
			const eval::PtypeEvalTable *table)
{
  if (! table)
    table = &Ptype_Eval_Table;
  if (move.player() == BLACK)
    return seeInternal<BLACK>(state, move, my_pin, op_pin, *table);
  else
    return -seeInternal<WHITE>(state, move, my_pin, op_pin, *table);
}

void osl::eval::
See::findAdditionalPieces(const NumEffectState& state, Player attack, 
			  Square target,
			  Square from,
			  PtypeOSquareVector& out)
{
  const Offset32 diff32 = Offset32(from, target);
  const Offset step = Board_Table.getShortOffsetNotKnight(diff32);
  if (step.zero())
    return;
  // 利きが8方向の場合
  Piece candidate=state.nextPiece(from, step);
  if (! candidate.isPiece())
    return;
  const Offset32 diff_reverse = Offset32(target,candidate.square());
  for (; candidate.isPiece(); 
       candidate=state.nextPiece(candidate.square(), step))
  {
    if (candidate.owner() != attack)
      return;
    const EffectContent effect 
      = Ptype_Table.getEffect(candidate.ptypeO(), diff_reverse);
    if (! effect.hasEffect())
      return;
    out.push_back(std::make_pair(candidate.ptypeO(), candidate.square()));
  } 
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
