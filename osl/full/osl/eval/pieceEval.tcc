/* pieceEval.tcc
 */
#ifndef OSL_PIECEEVAL_TCC
#define OSL_PIECEEVAL_TCC
#include "osl/move_generator/effect_action.h"
#include "osl/move_classifier/kingOpenMove.h"
#include "osl/eval/pieceEval.h"
#include "osl/eval/see.h"
namespace osl 
{
  namespace eval
  {
  /**
   * 安全な指手を選ぶ.
   * 単純な素抜きは考慮するが正確ではない
   * @param P 指手を指すプレイヤ
   */
  template <Player P>
  struct SelectSafePieces
  {
    static void select(const NumEffectState& state, Square target,
		       const PtypeOSquareVector& src,
		       PtypeOSquareVector& out)
    {
      for (size_t i=0; i<src.size(); ++i)
      {
	assert(P == getOwner(src[i].first));
	const Ptype ptype = getPtype(src[i].first);
	const Square from = src[i].second;
	if ((ptype == KING)	// 王は無条件でいれておく
	    || (! move_classifier::KingOpenMove<P>::
		isMember(state,ptype,from,target)))
	{
	  out.push_back(src[i]);
	}
      }
    }
    /**
     * @param exceptFor ここからの利きは除外
     */
    static void select(const NumEffectState& state, Square target,
		       const PtypeOSquareVector& src,
		       PtypeOSquareVector& out, Square except_for)
    {
      for (size_t i=0; i<src.size(); ++i)
      {
	assert(P == getOwner(src[i].first));
	const Ptype ptype = getPtype(src[i].first);
	const Square from = src[i].second;
	if ((ptype == KING)	// 王は無条件でいれておく
	    || (! move_classifier::KingOpenMove<P>::
		isMember(state,ptype,from,target,except_for)))
	{
	  out.push_back(src[i]);
	}
      }
    }
  };
  
  struct TakeBackValue
  {
    /** effectTo に利きのある駒を全て集める */
    template <Player P>
    static void findEffectPieces(const NumEffectState& state, Square effect_to,
				 PtypeOSquareVector& my_pieces, 
				 PtypeOSquareVector& op_pieces)
    {
      typedef effect_action::StorePtypeOSquare store_t;
      store_t op_pieces_store(&op_pieces, effect_to);
      state.template forEachEffect<alt(P),store_t>
	(effect_to, op_pieces_store);
      if (! op_pieces.empty())
      {
	store_t my_pieces_store(&my_pieces, effect_to);
	state.template forEachEffect<P,store_t>(effect_to, my_pieces_store);
      }
    }
    /** move 後に move.to() に利きのある駒を全て集める */
    template <Player P>
    static void findEffectPiecesAfterMove(const NumEffectState& state, Move move, 
					  PtypeOSquareVector& my_pieces, 
					  PtypeOSquareVector& op_pieces)
    {
      using namespace effect_action;
      
      const Square from=move.from();
      const Square to=move.to();
      const Player Opponent = alt(P);
      StorePtypeOSquare my_pieces_store(&my_pieces, to);
      StorePtypeOSquare op_pieces_store(&op_pieces, to);
      {
	// moveの結果目的のマスの利きが変わるのを調節
	/** この部分は effectの種類によっては高速に求まるかもしれない */
	/** offsetからshortを求める */
	/** knight moveは0にしたいが，Board_Tableには対応するものはない */
	Offset shortOffset=Board_Table.getShortOffsetNotKnight(Offset32(to,from));
	// knightの場合は変わらない
	if (! shortOffset.zero()){
	  Piece p;
	  for (Square pos=from-shortOffset; (p=state.pieceAt(pos)).isEmpty();
	       pos-=shortOffset)
	    ;
	  if (p.isOnBoardByOwner<P>()){
	    // 利きあり
	    const int moveMask=Ptype_Table.getMoveMask(p.ptype());
	    Direction dir=Board_Table.getLongDirection<P>(Offset32(to,from));
	    if ((moveMask&dirToMask(dir))!=0){
	      my_pieces_store.store(p);
	    }
	  }
	  else if (p.isOnBoardByOwner<Opponent>()){
	    // 利きあり
	    const int moveMask=Ptype_Table.getMoveMask(p.ptype());
	    Direction dir=Board_Table.getLongDirection<P>(Offset32(from,to));
	    if ((moveMask&dirToMask(dir))!=0){
	      op_pieces_store.store(p);
	    }
	  }
	}
      }
      state.template forEachEffect<alt(P),StorePtypeOSquare>
	(to, op_pieces_store);
      if (! op_pieces.empty())
      {
	const Piece movePiece=state.pieceAt(from);
	state.template forEachEffectNotBy<P,StorePtypeOSquare>
	  (to, movePiece,my_pieces_store);
      }
    }
    
    /**
     * PtypeOSquareVector をもとに取り返し値を計算する
     * 
     * FIXME: 利きを延ばすコードを入れる前に PtypeOSquareVector を 
     * PtypeO,Square のベクタに変更する必要がある． 
     * computeDiffAfterMoveMulti などで駒を動かさずに，
     * move 後の取り合いを考えている時に，piece.square() が必ずしも
     * 取り合いのためのposition ではないため．
     *
     * @param P alt(P) からの取り返し
     * @param target ここに関する取り返し
     * @param ptypeo target にあると想定される駒
     */
    template <Player P>
    static int computeValue(Square target, PtypeO ptypeO, 
			    const PtypeOSquareVector& my_pieces, 
			    const PtypeOSquareVector& op_pieces)
    {
      int val = 0;
      CArray<int,Piece::SIZE> vals;
      const Player Opponent = alt(P);
      size_t i;
      for (i=0;i<op_pieces.size();i++)
      {
	vals[i*2]=val;
	// opponentMove
	val+=Ptype_Eval_Table.captureValue(ptypeO);
	{
	  ptypeO = op_pieces[i].first;
	  const bool promotable = canPromote(ptypeO) 
	    && (target.canPromote<Opponent>() 
		|| op_pieces[i].second.canPromote<Opponent>());
	  if (promotable)
	  {
	    ptypeO=promote(ptypeO);
	    val+=Ptype_Eval_Table.promoteValue(ptypeO);
	  }
	}
	vals[i*2+1]=val;
	// myMove
	if (i>=my_pieces.size()){
	  break;
	}
	val+=Ptype_Eval_Table.captureValue(ptypeO);
	{
	  ptypeO=my_pieces[i].first;
	  const bool promotable = canPromote(ptypeO) 
	    && (target.canPromote<P>() 
		|| my_pieces[i].second.canPromote<P>());
	  if (promotable)
	  {
	    ptypeO=promote(ptypeO);
	    val+=Ptype_Eval_Table.promoteValue(ptypeO);
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
  };

  /** P が PTYPE の駒を取った時の値 */
  template <Ptype PTYPE> inline int captureVal(Player P)
  {
    // unpromote(PTYPE) を定数で求められれば即値に出来る
    return Ptype_Eval_Table.captureValue(newPtypeO(alt(P),PTYPE));
  }
} // namespace eval
} // namespace osl

template<osl::Player P>
int osl::PieceEval::
computeDiffAfterMove(const NumEffectState& state, Move move)
{
  assert(P == state.turn());
  assert(state.isAlmostValidMove(move));

  /** move.to() に利きのある駒を集める */
  PtypeOSquareVector my_pieces,op_pieces;
  /**
   * moveの結果, 延びる効きがあるか
   */ 
  const Square from=move.from();
  const Square to=move.to();
  int val=0; 
  /**
   * 現状で自分を除くすべての駒
   */
  if (from.isPieceStand()) // drop moveは簡単
  {
    TakeBackValue::findEffectPieces<P>(state, to, 
				       my_pieces, op_pieces);
  }
  else
  {
    val+=Ptype_Eval_Table.diffWithMove(state,move);
    TakeBackValue::
      findEffectPiecesAfterMove<P>(state, move, my_pieces, op_pieces);
  }

  if (op_pieces.empty())
    return val;

  PtypeOSquareVector my_safe_pieces, op_safe_pieces;
  if (from.isPieceStand())
  {
    SelectSafePieces<P>::
      select(state, to, my_pieces, my_safe_pieces);
    SelectSafePieces<alt(P)>::
      select(state, to, op_pieces, op_safe_pieces);
  }
  else
  {
    SelectSafePieces<P>::
      select(state, to, my_pieces, my_safe_pieces, from);
    SelectSafePieces<alt(P)>::
      select(state, to, op_pieces, op_safe_pieces, from);
  }
  
  my_safe_pieces.sort();
  op_safe_pieces.sort();

  return val + TakeBackValue::
    computeValue<P>(to, move.ptypeO(), my_safe_pieces, op_safe_pieces);
}

#endif /* OSL_PIECEEVAL_TCC */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
