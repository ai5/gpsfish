/* proofNumberTable.h
 */
#ifndef OSL_CHECKMATE_PROOF_NUMBER_TABLE_H
#define OSL_CHECKMATE_PROOF_NUMBER_TABLE_H
#include "osl/checkmate/proofDisproof.h"
#include "osl/numEffectState.h"
#include "osl/bits/king8Info.h"
#include "osl/additionalEffect.h"
#include "osl/bits/boardTable.h"

namespace osl
{
  namespace checkmate
  {
    class ProofNumberTable
    {
    public:
      struct Liberty
      {
	/** 玉の自由度の予測値. 空王手の場合は 0 のことがある．*/
	uint8_t liberty;
	/** false の場合は必ず空き王手 */
	bool has_effect;
	explicit Liberty(uint8_t l=0, bool e=false) : liberty(l), has_effect(e)
	{
	}
      };
    private:
      /** 一つの王手 -> Liberty:
       * long なdirection は1マスあけた王手を意味する */
      CArray2d<CArray<Liberty,DIRECTION_SIZE>,0x100u,PTYPE_SIZE> liberties;
      /** 全ての有効drop -> 最小liberty.
       * liberty 8bit + 有効drop 8bit + 最小値 -> ptype mask
       */
      CArray2d<uint8_t,0x10000u,8> drop_liberty;
      /** 龍や馬で王手をかけられる時のliberty: [liberty][move_mask] */
      CArray2d<uint8_t,0x100u,0x100u> pmajor_liberty;
      /** 王が1,2段目にいる時の移動王手によるliberty: [liberty][move_mask].
       * それ以外でUに金類が移動できる場合もこれで良いか．
       */
      CArray2d<uint8_t,0x100u,0x100u> promote_liberty;
      /** それ以外の移動liberty: [liberty][move_mask] */
      CArray2d<uint8_t,0x100u,0x100u> other_move_liberty;
    public:
      void init();

      /**
       * dir 方向からの王手をかけた時のlibertyの予想
       */
      const Liberty countLiberty(Ptype ptype, Direction d, unsigned int liberty_mask) const
      {
	assert((d != UUL) && (d != UUR));
	assert(liberty_mask <= 0xff);
	return liberties[liberty_mask][ptype][d];
      }
      /**
       * 8近傍へのdropまたは取れない移動後のlibertyの予測値を返す.
       * 玉は取り返せる時でも取り返さない値．
       * 桂馬は表をひく必要がないので呼び出し側で処理する．
       * @return 空王手の場合は 0 のことがある．
       */
      const Liberty countLibertyShortNotKnight(Player player, Square to, Ptype ptype, 
					       Square king, King8Info info) const
      {
	assert(to.isNeighboring8(king));
	assert(ptype != KNIGHT);
	const unsigned int liberty_mask = info.liberty();
	const Direction d = 
	  (player == BLACK)
	  ? Board_Table.getShort8<BLACK>(to, king)
	  : Board_Table.getShort8<WHITE>(to, king);
	return countLiberty(ptype, d, liberty_mask);
      }
      const Liberty countLibertyLong(Player player, Square to, Ptype ptype, 
				     Square king, King8Info info) const
      {
	assert(! to.isNeighboring8(king));
	const unsigned int liberty_mask = info.liberty();
	const Offset32 offset32(king,to);
	const Offset offset = Board_Table.getShortOffsetNotKnight(offset32);
	if (offset.zero())
	  return Liberty(0, false);
	if (to + offset + offset != king) // 2マス以上遠く
	{
	  if (isMajor(ptype))
	    ptype = unpromote(ptype);
	  else if (ptype != LANCE)
	    return Liberty(0, false);
	}
	const Direction d = 
	  (player == BLACK)
	  ? Board_Table.getLongDirection<BLACK>(offset32)
	  : Board_Table.getLongDirection<WHITE>(offset32);
	assert(isLong(d));
	return countLiberty(ptype, d, liberty_mask);
      }
      /**
       * move は王手である必要がある
       */
      int countLiberty(const NumEffectState& state, int liberty_count,
		       Move move, Square king, King8Info info) const
      {
	assert(liberty_count == misc::BitOp::countBit(info.liberty()));
	const Player attack = move.player();
	const Player defense = alt(attack);
	const Square to = move.to();
	const Ptype ptype = move.ptype();
	if (ptype == KNIGHT)
	  return std::max(1,liberty_count + state.countEffect(defense, to));

	const bool neighboring = to.isNeighboring8(king);
	Liberty liberty = neighboring 
	  ? countLibertyShortNotKnight(attack, to, ptype, king, info)
	  : countLibertyLong(attack, to, ptype, king, info);
	if (liberty.liberty == 0)
	  return std::max(liberty_count-1,1);
	if (! neighboring && liberty.has_effect)
	{
	  // TODO: 詰将棋と協調できるなら liberty.liberty <=
	  // liberty_count を保つように調整したい，が．
	  ++liberty.liberty;	// 合駒の分，もし両王手の場合は本来は不要
	}
	  
	liberty.liberty += state.countEffect(defense, to);
	if (move.isDrop())
	{
	  if (neighboring)
	  {
	    if (state.countEffect(attack, to))
	      --liberty.liberty;		// adjust king capture
	  }
	  assert(liberty.liberty);
	  return liberty.liberty;
	}
	// 移動: 銀のただすてなどは本当は利きをはずしたい
	if (neighboring)
	{
	  if (state.countEffect(attack, to) >= 2
	      || effect_util::AdditionalEffect::hasEffect(state, to, attack))
	    --liberty.liberty;		// adjust king capture
	}
	assert(liberty.liberty);
	return liberty.liberty;
      }
      /** テスト用 */
      int countLiberty(const NumEffectState& state, Move move) const;

      /** drop のみ */
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      libertyAfterAllDrop(const NumEffectState& state) const;
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      libertyAfterAllDrop(const NumEffectState& state, Player attack,
			  King8Info info) const;
      /** 移動 のみ */
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      libertyAfterAllMove(const NumEffectState& state) const;
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      libertyAfterAllMove(const NumEffectState& state, Player attack,
			  King8Info info, Square king) const;
      /** 全て */
      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      libertyAfterAllCheck(const NumEffectState& state) const;

      int
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      disproofAfterAllCheck(const NumEffectState&, Player, King8Info) const;
      /** 全て */
      const ProofDisproof
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      attackEstimation(const NumEffectState& state) const;
      const ProofDisproof
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      attackEstimation(const NumEffectState& state,
		       Player attack,
		       King8Info info, Square king) const;
    };
    // tables.ccに入れればconstにできる
    extern ProofNumberTable Proof_Number_Table;

    class EdgeTable
    {
      CArray2d<uint64_t, 2, Square::SIZE> edge_mask;
    public:
      void init();
      /** liberty から盤の淵(xかyが1か9)を取り除く.  libertyCount()==0になっても詰みとは限らない */
      const King8Info
#ifdef __GNUC__
	__attribute__ ((pure))
#endif
      resetEdgeFromLiberty(Player king_player, Square king, King8Info info) const
      {
	uint64_t ret = info.uint64Value();
	ret &= edge_mask[king_player][king.index()];
	const uint64_t count = misc::BitOp::countBit((ret>>8)&0xffull);
	ret |= count << 48;
	return King8Info(ret);
      }
    };
    extern EdgeTable Edge_Table;
  }
}

#endif /* _CHECKMATE_IMMEDIATE_CHECKMATE_TABLE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

