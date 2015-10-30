/* proofNumberTable.cc
 */
#include "osl/checkmate/proofNumberTable.h"
#include "osl/checkmate/immediateCheckmate.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/bits/boardTable.h"
#include "osl/bits/ptypeTable.h"
#include "osl/oslConfig.h"

osl::checkmate::ProofNumberTable osl::checkmate::Proof_Number_Table;
osl::checkmate::EdgeTable osl::checkmate::Edge_Table;

namespace {
  namespace ProofNumberTable {
    osl::SetUpRegister _initializer([](){ 
	osl::checkmate::Proof_Number_Table.init(); 
	osl::checkmate::Edge_Table.init(); 
      });
  }
}

namespace osl
{
  namespace
  {
    using namespace osl::checkmate;
    /**
     * @return 0 は王手でない場合．それ以外は逃げ方の見積
     */
    const checkmate::ProofNumberTable::Liberty
    effectiveCheckShort(Ptype ptype,Direction dir,unsigned int mask)
    {
      assert(isShort(dir));
      // 王は8近傍に移動できない
      if (ptype==KING) 
	return ProofNumberTable::Liberty(0,false);
      // ptypeがdir方向に利きを持たない場合も空王手の場合があるので作成
      const bool has_effect 
	= (Ptype_Table.getMoveMask(ptype)
	   & (dirToMask(dir) | dirToMask(shortToLong(dir))));
      int dx=Board_Table.getDxForBlack(dir);
      int dy=Board_Table.getDyForBlack(dir);
      int count = 0;
      for (int l=0;l<8;l++) {
	if ((mask&(1<<l))==0) 
	  continue;
	Direction dir1=static_cast<Direction>(l);
	int dx1=Board_Table.getDxForBlack(dir1);
	int dy1=Board_Table.getDyForBlack(dir1);
	Offset32 o32(dx-dx1,dy-dy1);
	if ((dx != dx1 || dy != dy1)
	    && !Ptype_Table.getEffect(newPtypeO(BLACK,ptype),o32).hasEffect())
	  ++count;
      }
      return ProofNumberTable::Liberty(std::max(count,1), has_effect);
    }
    const checkmate::ProofNumberTable::Liberty
    effectiveCheckLong(Ptype ptype,Direction dir,unsigned int mask)
    {
      assert(isLong(dir));
      // ptypeがdir方向に利きを持たなくても2マス離れた空王手で8近傍に
      // 利きが増える場合があるので作成
      const bool has_effect 
	= (Ptype_Table.getMoveMask(ptype) & dirToMask(dir));
      int dx=Board_Table.getDxForBlack(dir)*2; // 1マス遠くから
      int dy=Board_Table.getDyForBlack(dir)*2;
      int count = 0;
      for (int l=0;l<8;l++) {
	if ((mask&(1<<l))==0) 
	  continue;
	Direction dir1=static_cast<Direction>(l);
	int dx1=Board_Table.getDxForBlack(dir1);
	int dy1=Board_Table.getDyForBlack(dir1);
	Offset32 o32(dx-dx1,dy-dy1);
	if (!Ptype_Table.getEffect(newPtypeO(BLACK,ptype),o32).hasEffect())
	  ++count;
      }
      return ProofNumberTable::Liberty(std::max(count,1), has_effect);
    }
  }
}

void osl::checkmate::
ProofNumberTable::init()
{
  // liberties
  for (int i=0; i<0x100; i++) {
    for (int k=PTYPE_PIECE_MIN; k<=PTYPE_MAX; k++) {
      const Ptype ptype=static_cast<Ptype>(k);
      assert(isPiece(ptype));
      for (int j=0; j<8; j++) {
	Direction dir=static_cast<Direction>(j);
	const Liberty e = effectiveCheckShort(ptype,dir,i);
	liberties[i][k][j] = e;
      }
      int longs = 0;
      for (int j=LONG_DIRECTION_MIN; j<=LONG_DIRECTION_MAX; ++j,++longs) {
	Direction dir=static_cast<Direction>(j);
	const Liberty e = effectiveCheckLong(ptype,dir,i);
	liberties[i][k][j] = e;
      }
      assert(longs == 8);
    }
  }
  // drop_liberty
  drop_liberty.fill(0);
  for(int i=0;i<0x10000;i++){
    const unsigned int liberty = (i>>8)&0xff;
    const int liberty_count = misc::BitOp::countBit(liberty);
    if (liberty_count <= 2)
      continue;			// low enough
    
    for (int k=PTYPE_BASIC_MIN;k<=PTYPE_MAX;k++) {
      int minimum_liberty = liberty_count;
      Ptype ptype=static_cast<Ptype>(k);
      if (ptype == KING)
	continue;
      for (int j=0;j<8;j++) {
	// 有効王手でない
	if ((i&(0x1<<j))==0) 
	  continue;
	if ((i&(0x100<<j))!=0)
	  continue;
	const Direction dir=static_cast<Direction>(j);
	// ptypeがdir方向に利きを持つか
	const bool has_effect 
	  = (Ptype_Table.getMoveMask(ptype)
	     & (dirToMask(dir) | dirToMask(shortToLong(dir))));
	if (! has_effect)
	  continue;
	const int e = liberties[liberty][k][j].liberty;
	assert(e);
	minimum_liberty = std::min(minimum_liberty, e);
      }
      for (int l=minimum_liberty; l<liberty_count; ++l)
      {
	drop_liberty[i][l] |= (1<<(ptype-GOLD));
      }
    }
  }
  // pmajor_liberty
  pmajor_liberty.fill(8);
  for (int l=0; l<0x100; l++) {	// liberty
    for (int m=0; m<0x100; m++) {	// move_mask
      if (l & m)
	continue;
      int min_liberty = std::max(2,misc::BitOp::countBit(l))-1;
      if (min_liberty > 1)
      {
	for (int j=0; j<8; j++) {
	  if ((m&(0x1<<j))==0)
	    continue;
	  const int pr = liberties[l][PROOK][j].liberty;
	  const int pb = liberties[l][PBISHOP][j].liberty;
	  min_liberty = std::min(min_liberty, std::min(pr,pb));
	  assert(min_liberty);
	}
      }
      pmajor_liberty[l][m] = min_liberty;
    }
  }
  // promote_liberty
  promote_liberty.fill(8);
  for (int l=0; l<0x100; l++) {	// liberty
    for (int m=0; m<0x100; m++) {	// move_mask
      if (l & m)
	continue;
      int min_liberty = std::max(2,misc::BitOp::countBit(l))-1;
      if (min_liberty > 1)
      {
	for (int j=0; j<8; j++) {
	  if ((m&(0x1<<j))==0)
	    continue;
	  for (int k=PTYPE_BASIC_MIN;k<=PTYPE_MAX;k++) {
	    Ptype ptype=static_cast<Ptype>(k);
	    if (ptype == KING || ptype == PROOK || ptype == PBISHOP)
	      continue;
	    Liberty e = liberties[l][k][j];
	    if (! e.has_effect)
	      continue;
	    assert(e.liberty); 
	    min_liberty = std::min(min_liberty, (int)e.liberty);
	    assert(min_liberty);
	  }
	}
      }
      promote_liberty[l][m] = min_liberty;
    }
  }
  // other_move_liberty
  other_move_liberty.fill(8);
  for (int l=0; l<0x100; l++) {	// liberty
    for (int m=0; m<0x100; m++) {	// move_mask
      if (l & m)
	continue;
      int min_liberty = std::max(2,misc::BitOp::countBit(l))-1;
      if (min_liberty > 1)
      {
	for (int j=0; j<8; j++) {
	  if ((m&(0x1<<j))==0)
	    continue;
	  for (int k=PTYPE_BASIC_MIN;k<=PTYPE_MAX;k++) {
	    Ptype ptype=static_cast<Ptype>(k);
	    if (ptype == KING || ptype == PROOK || ptype == PBISHOP)
	      continue;
	    if (j == U 
		&& (ptype == GOLD || ptype == PPAWN || ptype == PLANCE
		    || ptype == PKNIGHT || ptype == PSILVER))
	      continue;
	    Liberty e = liberties[l][k][j];
	    if (! e.has_effect)
	      continue;
	    assert(e.liberty); 
	    min_liberty = std::min(min_liberty, (int)e.liberty);
	    assert(min_liberty);
	  }
	}
      }
      other_move_liberty[l][m] = min_liberty;
    }
  }
}

int osl::checkmate::
ProofNumberTable::countLiberty(const NumEffectState& state, Move move) const
{
  const Player attack = move.player();
  const Square king = state.kingSquare(alt(attack));
  const King8Info info(state.Iking8Info(alt(attack)));
  return countLiberty(state, info.libertyCount(), move, king, info);
}

int osl::checkmate::
ProofNumberTable::libertyAfterAllDrop(const NumEffectState& state, Player attack,
				      King8Info info) const
{
  assert(state.turn() == attack);
  int result = info.libertyCount()-1;
  if (result < 2)
    return 1;
  const unsigned int ld_mask = info.libertyDropMask();
  uint8_t ptype_mask = 0;
  for (int p=GOLD; p<=ROOK; ++p)
    ptype_mask |= state.hasPieceOnStand(attack, static_cast<Ptype>(p)) << (p-GOLD);
  for (; 
       result > 1
	 && (ptype_mask & drop_liberty[ld_mask][result-1]);
       --result)
  {
  }
  return result;	
}

int osl::checkmate::
ProofNumberTable::libertyAfterAllDrop(const NumEffectState& state) const
{
  const Player attack = state.turn();
  const King8Info info(state.Iking8Info(alt(attack)));
  return libertyAfterAllDrop(state, attack, info);
}

int osl::checkmate::
ProofNumberTable::libertyAfterAllMove(const NumEffectState& state,
				      Player attack,
				      King8Info info, Square king) const
{
  bool has_pmajor = false;
  {
    for (int i = PtypeTraits<BISHOP>::indexMin;
	 i < PtypeTraits<ROOK>::indexLimit; i++)
    {
      // move maskを見て8マス調べる方が良いか?
      const Piece p = state.pieceOf(i);
      assert(isMajor(p.ptype()));
      if (! p.isOnBoardByOwner(attack))
	continue;
      if (king.squareForBlack(attack).y() > 3 // 1段おまけ
	  && ! isPromoted(p.ptype()))
      {
	if (! p.square().canPromote(attack))
	  continue;
      }
      if (Neighboring8Direct::hasEffect(state, p.ptypeO(), p.square(),
					king))
      {
	// 本当はそこにbitが立っているかを判定したい．
	has_pmajor = true;
	break;
      }
    }
  }
  int moveCandidate;
  if(attack==BLACK)
    moveCandidate = info.moveCandidateMask<BLACK>(state);
  else
    moveCandidate = info.moveCandidateMask<WHITE>(state);
  if (has_pmajor)
  {
    int result = pmajor_liberty[info.liberty()][moveCandidate];
    assert(result);
    return result;
  }
  bool promoted_area = king.squareForBlack(attack).y() < 3;
  if (! promoted_area)
  {
    const Square u = king + Board_Table.getOffset(alt(attack), U);
    promoted_area = state.hasEffectByPtype<GOLD>(attack, u);
  }
  if (promoted_area)
  {
    int result = promote_liberty[info.liberty()][moveCandidate];
    assert(result);
    return result;
  }
  int result = other_move_liberty[info.liberty()][moveCandidate];
  assert(result);
  return result;
}

int osl::checkmate::
ProofNumberTable::libertyAfterAllMove(const NumEffectState& state) const
{
  const Player attack = state.turn();
  const Square king = state.kingSquare(alt(attack));
  const King8Info info(state.Iking8Info(alt(attack)));
  return libertyAfterAllMove(state, attack, info, king);
}

int osl::checkmate::
ProofNumberTable::disproofAfterAllCheck(const NumEffectState& state, 
					Player attack,
					King8Info info) const
{
  int num_checks;
  num_checks=info.countMoveCandidate(attack,state);
  int drop_scale = state.hasPieceOnStand<GOLD>(attack) 
    + state.hasPieceOnStand<SILVER>(attack);  
  if (drop_scale)
    num_checks += misc::BitOp::countBit(info.dropCandidate()) * drop_scale;
  return std::max(1, num_checks);
}

const osl::checkmate::ProofDisproof osl::checkmate::
ProofNumberTable::attackEstimation(const NumEffectState& state,
				   Player attack,
				   King8Info info, Square king) const
{
  int p = libertyAfterAllDrop(state, attack, info);
  if (p >= 2)
  {
    p = std::min(p, libertyAfterAllMove(state, attack, info, king));
  }
  return ProofDisproof(p, disproofAfterAllCheck(state, attack, info));
}

const osl::checkmate::ProofDisproof osl::checkmate::
ProofNumberTable::attackEstimation(const NumEffectState& state) const
{
  const Player attack = state.turn();
  const Square king = state.kingSquare(alt(attack));
  const King8Info info(state.Iking8Info(alt(attack)));
  return attackEstimation(state, attack, info, king);
}

int osl::checkmate::
 ProofNumberTable::libertyAfterAllCheck(const NumEffectState& state) const
{
  return attackEstimation(state).proof();
}



void osl::checkmate::EdgeTable::init()
{
  edge_mask.fill(~(0xfull << 48));
  for (int x=1; x<=9; ++x) {
    for (int y=1; y<=9; ++y) {
      Square king(x,y);
      for (int d=DIRECTION_MIN; d<=SHORT8_DIRECTION_MAX; ++d) {
	Square target = king+Board_Table.getOffset(BLACK, Direction(d));
	if (target.x() <= 1 || target.x() >= 9 || target.y() <=1 || target.y() >=9)
	  edge_mask[BLACK][king.index()] &= ~(0x100ull<<d);
	target = king+Board_Table.getOffset(WHITE, Direction(d));
	if (target.x() <= 1 || target.x() >= 9 || target.y() <=1 || target.y() >=9)
	  edge_mask[WHITE][king.index()] &= ~(0x100ull<<d);
      }
    }
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:


