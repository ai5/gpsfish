/* feature.h
 */
#ifndef GPSSHOGI_MOVE_PROBABILITY_FEATURE_H
#define GPSSHOGI_MOVE_PROBABILITY_FEATURE_H

#include "indexList.h"
#include "moveInfo.h"
#include "stateInfo.h"
#include "osl/additionalEffect.h"
#include "osl/effect_util/neighboring8Direct.h"
#include <string>

namespace gpsshogi
{
  class Feature
  {
    std::string my_name;
    int dim;
  public:
    Feature(std::string n, size_t d) : my_name(n), dim(d)
    {
      assert(dim > 0);
    }
    virtual ~Feature();
    std::string name() const { return my_name; }
    virtual void match(const StateInfo&, const MoveInfo&, int offset, index_list_t&) const=0;
    size_t dimension() const { return dim; }

    static int classifyEffect9(const NumEffectState& state, Player player, Square to)
    {
      const int a = std::min(2, state.countEffect(player, to));
      int d = std::min(2,state.countEffect(alt(player), to));
      if (a>d)
	d += AdditionalEffect::hasEffect(state, to, alt(player));
      return a*3 + d;
    }
  };

  class CheckFeature : public Feature
  {
  public:
    enum { CHECK_CLASS = 4, RELATIVE_Y_CLASS = 3 };
    CheckFeature()
      : Feature("Check", CHECK_CLASS*PTYPE_SIZE*2*RELATIVE_Y_CLASS)
    {
    }
    static int checkIndex(const MoveInfo& move) 
    {
      if (move.open_check)
	return 3;
      return (move.see > 0) ? 0 : ((move.see == 0) ? 1 : 2);
    }
    static int sign(const NumEffectState& state, Move move,
		    Player player)
    {
      const Square king = state.kingSquare(alt(player));
      int ry = (move.to().y() - king.y())*osl::sign(player);
      if (ry == 0) return 0;
      return ry > 0 ? 1 : -1;
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (! move.check && ! move.open_check)
	return;
      const Player player = move.player;
      int index = sign(state.state, move.move, player)+1;
      index = (index*2 + move.move.isDrop())*PTYPE_SIZE
	+ move.move.ptype();
      index = index*CHECK_CLASS + checkIndex(move);
      out.add(offset+index, 1);
    }
  };
  class TakeBackFeature : public Feature
  {
  public:
    TakeBackFeature() : Feature("TakeBack", 3)
    {
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (state.last_move < 0
	  || state.moves[state.last_move].to() != move.move.to())
	return;
      int match = 2;
      if (move.see >= 0) {
	--match;
	if (state.last_move >= 1
	    && state.moves[state.last_move-1].to() == move.move.to())
	  --match;
      }
      out.add(offset + match, 1);
    }
  };
  class SeeFeature : public Feature
  {
  public:
    enum { SeeClass = 23, YClass = 3, ProgressClass = 8 };
    SeeFeature() : Feature("SeeFeature", SeeClass*(3+ProgressClass))
    {
    }
    static int seeIndex(int see)
    {
      int index = see / 128;
      if (see > 0) {
	index = std::min(10, index);
	index += 12;
      } else if (see == 0) {
	index += 11;
      } else {
	index += 10;
	index = std::max(0, index);
      }
      return index;
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      const int see_index = seeIndex(move.see);
      const Player player = move.player;
      const Square to = move.move.to();
      const int promote_index = to.canPromote(player) 
	? 1 : (to.canPromote(alt(player)) ? 2 : 0);
      out.add(offset+see_index+promote_index*SeeClass, 1);
      int progress_index = YClass + state.progress8();
      out.add(offset+see_index+progress_index*SeeClass, 1);
    }
  };

  class CapturePtype : public Feature
  {
  public:
    CapturePtype() : Feature("CapturePtype", PTYPE_SIZE*PTYPE_SIZE*2*3)
    {
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      const Ptype captured = move.move.capturePtype();
      const Player player = move.player;
      int index = (move.move.ptype())*PTYPE_SIZE + captured;
      static_assert(PTYPE_EDGE == 1, "");
      if (captured != PTYPE_EMPTY
	  && captured == state.threatened[alt(player)].ptype())
	++index;
      if (move.see > 0)
	index += PTYPE_SIZE*PTYPE_SIZE;
      if (captured != PTYPE_EMPTY)
	index += std::min(2, state.state.countPiecesOnStand(player, unpromote(captured)))
	  * (2*PTYPE_SIZE*PTYPE_SIZE);
      out.add(offset+index, 1);
    }
  };

  class ContinueCapture : public Feature
  {
  public:
    ContinueCapture() : Feature("ContinueCapture", 1) {}
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (move.move.capturePtype() == PTYPE_EMPTY
	  || state.last_move < 1
	  || state.moves[state.last_move-1].to() != move.move.from())
	return;
      out.add(offset, 1);
    }
  };

  /** 取った駒をすぐ使う */
  class DropCaptured : public Feature
  {
  public:
    DropCaptured() : Feature("DropCaptured", PTYPE_SIZE - PTYPE_BASIC_MIN) {}
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (! move.move.isDrop()
	  || state.last_move < 1
	  || state.moves[state.last_move-1].capturePtype() != move.move.ptype())
	return;
      const size_t index = move.move.ptype() - PTYPE_BASIC_MIN;
      assert(index < dimension());      
      out.add(index + offset, 1);
    }
  };

  class SquareY : public Feature
  {
  public:
    // PTYPE_SIZE, to [1,9], fromto [-3,3], drop|promote, progress8
    enum {
      DIM = 9*PTYPE_SIZE*16
    };
    SquareY() : Feature("SquareY", DIM)
    {
    }
    static int fromTo(Square to, Square from) // [-3,3]
    {
      return std::max(-3, std::min(to.y() - from.y(), 3));
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const Player P = info.player;
      const Square to = move.to().squareForBlack(P);
      size_t index = ((to.y()-1)*PTYPE_SIZE + move.ptype())*16;
      assert(index+16 <= dimension());
      const int from_to = move.isDrop() ? 0 
	: fromTo(to, move.from().squareForBlack(P));
      out.add(offset + index + from_to + 3, 1);
      if (move.isDrop() || move.isPromotion())
	out.add(offset + index + 7, 1);
      out.add(offset + index + state.progress8()+8, 1);
    }
  };

  class SquareX : public Feature
  {
  public:
    // PTYPE_SIZE, to [1,5], fromto [-3,3], drop|promote, progress8
    SquareX() : Feature("SquareX", 5*PTYPE_SIZE*16)
    {
    }
    static int fromTo(Square to, Square from) // [-3,3]
    {
      return std::max(-3, std::min(to.x() - from.x(), 3));
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      int to = move.to().x();
      int from_to = move.isDrop() ? 0 : fromTo(move.to(), move.from());
      if (to > 5) {
	to = 10 - to;
	from_to = -from_to;
      }
      size_t index = ((to-1)*PTYPE_SIZE + move.ptype())*16;
      assert(index+16 <= dimension());
      out.add(offset + index + from_to + 3, 1);
      if (move.isDrop() || move.isPromotion())
	out.add(offset + index + 7, 1);
      out.add(offset + index + state.progress8()+8, 1);
    }
  };

  class KingRelativeY : public Feature
  {
  public:
    // PTYPE_SIZE, to [-8,8], fromto [-3,3], drop|promote, progress8
    enum { ONE_DIM = 17*PTYPE_SIZE*16 };
    KingRelativeY() : Feature("KingRelativeY", ONE_DIM*2)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const Player P = info.player;
      const Ptype ptype = move.ptype();

      const Square to = move.to().squareForBlack(P);
      const Square my_king = state.state.kingSquare(P).squareForBlack(P);
      const Square op_king = state.state.kingSquare(alt(P)).squareForBlack(P);
      const int from_to = move.isDrop() ? 0 
	: SquareY::fromTo(to, move.from().squareForBlack(P));

      size_t index = ((to.y()-my_king.y()+8)*PTYPE_SIZE + ptype)*16;
      assert(index+16 <= ONE_DIM);
      out.add(offset + index + from_to + 3, 1);
      if (move.isDrop() || move.isPromotion())
	out.add(offset + index + 7, 1);
      out.add(offset + index + state.progress8()+8, 1);

      index = ((to.y()-op_king.y()+8)*PTYPE_SIZE + ptype)*16;
      assert(index+16 <= ONE_DIM);
      out.add(offset + ONE_DIM + index + from_to + 3, 1);
      if (move.isDrop() || move.isPromotion())
	out.add(offset + ONE_DIM + index + 7, 1);
      out.add(offset + ONE_DIM + index + state.progress8()+8, 1);
    }
  };
  class KingRelativeX : public Feature
  {
  public:
    // PTYPE_SIZE, to [0,8], fromto [-3,3], drop|promote, progress8
    enum { ONE_DIM = 9*PTYPE_SIZE*16 };
    KingRelativeX() : Feature("KingRelativeX", ONE_DIM*2)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const Player P = info.player;
      const Ptype ptype = move.ptype();

      const Square to = move.to();
      const Square my_king = state.state.kingSquare(P);
      const Square op_king = state.state.kingSquare(alt(P));
      const int from_to = move.isDrop() ? 0 
	: SquareY::fromTo(to, move.from());
      int dx = to.x() - my_king.x(), fx = from_to;
      if (dx < 0) {
	dx = -dx;
	fx = -fx;
      }
      size_t index = (dx*PTYPE_SIZE + ptype)*16;
      assert(index+16 <= ONE_DIM);
      out.add(offset + index + fx + 3, 1);
      if (move.isDrop() || move.isPromotion())
	out.add(offset + index + 7, 1);
      out.add(offset + index + state.progress8()+8, 1);

      dx = to.x() - op_king.x(), fx = from_to;
      if (dx < 0) {
	dx = -dx;
	fx = -fx;
      }
      index = (dx*PTYPE_SIZE + ptype)*16;
      assert(index+16 <= ONE_DIM);
      out.add(offset + ONE_DIM + index + fx + 3, 1);
      if (move.isDrop() || move.isPromotion())
	out.add(offset + ONE_DIM + index + 7, 1);
      out.add(offset + ONE_DIM + index + state.progress8()+8, 1);
    }
  };

  class FromEffect : public Feature
  {
  public:
    FromEffect() : Feature("FromEffect", PTYPE_SIZE*PTYPE_SIZE*PTYPE_SIZE)
    {
    }
    void match(const StateInfo& state_info, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      if (move.isDrop())
	return;
      const NumEffectState& state = state_info.state;
      const Ptype me = move.oldPtype();
      const Ptype support = state.findCheapAttack(info.player, move.from()).ptype();
      const Ptype attack = state.findCheapAttack(alt(info.player), move.from()).ptype();
      const size_t index = (((me * PTYPE_SIZE) + support) * PTYPE_SIZE) + attack;
      assert(index < dimension());
      out.add(index + offset, 1);
    }
  };

  class ToEffect : public Feature
  {
  public:
    ToEffect() : Feature("ToEffect", PTYPE_SIZE*PTYPE_SIZE*PTYPE_SIZE*PTYPE_SIZE)
    {
    }
    static const Piece find(const NumEffectState& state, 
			    Square to, const PieceMask& remove,
			    Player player)
    {
      assert(to.isOnBoard());
      PieceMask pieces = state.piecesOnBoard(player)
	& state.effectSetAt(to);
      pieces &= ~remove;
      return state.selectCheapPiece(pieces);
    }
    static void supportAttack(const NumEffectState& state, 
			      Square to, 
			      const PieceMask& my_pin,
			      const PieceMask& op_pin,
			      Player turn,
			      std::pair<Ptype,Ptype>& out) 
    {
      out.first  = find(state, to, my_pin, turn).ptype(); // support
      out.second = find(state, to, op_pin, alt(turn)).ptype(); // attack
    }
    static void supportAttack(const StateInfo& info, Square to, Move move,
			      std::pair<Ptype,Ptype>& out)
    {
      const Player turn = info.state.turn();
      if (move.isDrop())
	return supportAttack(info.state, to, info.pin[turn],
			     info.pin[alt(turn)], turn, out);
      PieceMask my_pin = info.pin[turn];
      my_pin.set(info.state.pieceAt(move.from()).number());
      supportAttack(info.state, to, my_pin, info.pin[alt(turn)],
		    turn, out);
    }
    static size_t supportAttack(const StateInfo& info, Square to, Move move)
    {
      std::pair<Ptype,Ptype> pair;
      supportAttack(info, to, move, pair);
      return pair.first * PTYPE_SIZE + pair.second;
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const Ptype me = move.ptype(), captured = move.capturePtype();
      const size_t position_index = supportAttack(state, move.to(), move);
      const size_t index = ((me * PTYPE_SIZE) + captured)
	* PTYPE_SIZE * PTYPE_SIZE + position_index;
      assert(index < dimension());
      out.add(index + offset, 1);
    }
  };

  class FromEffectLong : public Feature
  {
  public:
    FromEffectLong() : Feature("FromEffectLong", PTYPE_SIZE*8*8)
    {
    }
    void match(const StateInfo& state_info, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      if (move.isDrop())
	return;
      const NumEffectState& state = state_info.state;
      const Ptype ptype = move.oldPtype();
      const CArray<bool,3> me = {{
	  state.longEffectAt<LANCE>(move.from(), info.player).any(),
	  state.longEffectAt<BISHOP>(move.from(), info.player).any(),
	  state.longEffectAt<ROOK>(move.from(), info.player).any(),
	}};
      const CArray<bool,3> op = {{
	  state.longEffectAt<LANCE>(move.from(), alt(info.player)).any(),
	  state.longEffectAt<BISHOP>(move.from(), alt(info.player)).any(),
	  state.longEffectAt<ROOK>(move.from(), alt(info.player)).any(),
	}};
      size_t index = ptype;
      for (int i=0; i<3; ++i) {
	index *= 2; index += me[i];
	index *= 2; index += op[i];
      }
      assert(index < dimension());
      out.add(index + offset, 1);
    }
  };

  class ToEffectLong : public Feature
  {
  public:
    ToEffectLong() : Feature("ToEffectLong", PTYPE_SIZE*8*8)
    {
    }
    void match(const StateInfo& state_info, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const NumEffectState& state = state_info.state;
      const Ptype ptype = move.oldPtype();
      NumBitmapEffect effect=state.effectSetAt(move.to());
      if (! move.isDrop())
	effect.reset(state.pieceOnBoard(move.from()).number()+8);
      const CArray<mask_t,3> pieces = {{
	  effect.selectLong<LANCE>() >> 8, 
	  effect.selectLong<BISHOP>() >> 8, 
	  effect.selectLong<ROOK>() >> 8
	}};
      size_t index = ptype;
      for (int i=0; i<3; ++i) {
	index *= 2;
	index += (pieces[i] & state.piecesOnBoard(info.player).getMask(1)).any();
	index *= 2;
	index += (pieces[i] & state.piecesOnBoard(alt(info.player)).getMask(1)).any();
      }
      assert(index < dimension());
      out.add(index + offset, 1);
    }
  };

  class PatternCommon : public Feature
  {
  public:
    enum {
      SupportSize = PTYPE_SIZE,
      AttackSize = PTYPE_SIZE, AttackBase = SupportSize, // 32
      EffectSize = 9, EffectBase = AttackBase+AttackSize,
      OpKingSize = 4, OpKingBase = EffectBase+EffectSize,
      MyKingSize = 3, MyKingBase = OpKingBase+OpKingSize, // 48
      PromotionSize = 2, PromotionBase = MyKingBase+MyKingSize,
      PinOpenSize = 4, PinOpenBase = PromotionBase + PromotionSize,
      LastToSize = 4, LastToBase = PinOpenBase + PinOpenSize,
      LastEffectChangedSize = 6, LastEffectChangedBase = LastToBase + LastToSize,
      SquareDim = LastEffectChangedBase + LastEffectChangedSize, // 64
      PatternCacheSize = PTYPEO_SIZE*SquareDim,
      OneDim = PTYPE_SIZE*PatternCacheSize,
    };
    PatternCommon(const std::string& name, int dim) : Feature(name, dim)
    {
    }
    void addOne(const StateInfo& state, int offset, 
		index_list_t& out, Square position) const
    {
      if (! position.isOnBoardRegion() || state.state.pieceAt(position).isEdge()) {
	size_t basic = ptypeOIndex(PTYPEO_EDGE) *SquareDim;
	out.add(offset + basic, 1);
	return;
      } 
      const StateInfo::pattern_square_t& cache
	= state.pattern_cache[position.index()];
      for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i)
	out.add(offset + cache[i], 1);
    }
    static void updateCache(StateInfo& info);
  private:
    static void updateCacheOne(Square target, StateInfo& info);
  };

  template<bool TestPromotable>
  class PatternBase : public PatternCommon
  {
    int dx, black_dy;
  public:
    enum {
      PromotionSize = TestPromotable ? 3 : 1,
      DIM = PromotionSize * OneDim,
    };
    PatternBase(int x, int y)
      : PatternCommon(name(x,y), DIM), dx(x), black_dy(y)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const Ptype ptype = move.ptype();
      int basic = ptype*PatternCacheSize;
      int basic_from = (move.isPromotion() ? PTYPE_EDGE : PTYPE_EMPTY)
	*PatternCacheSize;
      const Square to = move.to();
      if (TestPromotable && to.canPromote(info.player))
	offset += OneDim;
      else if (TestPromotable && to.canPromote(alt(info.player)))
	offset += 2*OneDim;
      int dy = info.player == BLACK ? black_dy : -black_dy;
      Square target = to + Offset(dx, dy);
      if (move.from() != target)
	addOne(state, offset+basic, out, target);
      else // move from here
	addOne(state, offset+basic_from, out, target);
      if (dx == 0) 
	return;
      target = to + Offset(-dx, dy);
      if (move.from() != target)
	  addOne(state, offset+basic, out, target);
      else
	  addOne(state, offset+basic_from, out, target);
    }
    static std::string name(int x, int y)
    {
      return std::string("Pattern")
	+ (TestPromotable ? "P" : "")
	+ "X" + (char)('2'+x) + "Y"+(char)('2'+y);
    }
  };

  typedef PatternBase<false> Pattern;
  typedef PatternBase<true> PatternPromotion;

  class MoveFromOpposingSliders : public Feature
  {
  public:
    MoveFromOpposingSliders() : Feature("MoveFromOpposingSliders", 36*PTYPE_SIZE)
    {
    }
    static int longPtype(const NumEffectState& state, Square position, Player player)
    {
      const int offset = PtypeFuns<LANCE>::indexNum*32;
      mask_t p = state.longEffectAt<LANCE>(position, player);
      if (p.any()) 
	return state.hasEffectAt(player, state.pieceOf(p.takeOneBit()+offset).square());
      p = state.longEffectAt<BISHOP>(position, player);
      if (p.any()) 
	return 2 + state.hasEffectAt(player, state.pieceOf(p.takeOneBit()+offset).square());
      p = state.longEffectAt<ROOK>(position, player);
      assert(p.any());
      return 4 + state.hasEffectAt(player, state.pieceOf(p.takeOneBit()+offset).square());
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      const Square from = move.move.from();
      if (from.isPieceStand() 
	  || ! state.pinByOpposingSliders(state.state.pieceOnBoard(from)))
	return;
      const int me = longPtype(state.state, from, move.player);
      const int op = longPtype(state.state, from, alt(move.player));
      out.add(offset + (me*6+op)*PTYPE_SIZE+move.move.ptype(), 1);
    }
  };
  class AttackFromOpposingSliders : public Feature
  {
  public:
    AttackFromOpposingSliders() : Feature("AttackFromOpposingSliders", PTYPE_SIZE*PTYPE_SIZE*2)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      const Piece attack = state.state.findCheapAttack(alt(info.player), move.to());
      if (! state.pinByOpposingSliders(attack))
	return;
      if (state.state.countEffect(alt(info.player), move.to()) == 1)
	offset += dimension()/2;
      out.add(offset + PTYPE_EMPTY*PTYPE_SIZE+attack.ptype(), 1);
      out.add(offset + move.ptype()*PTYPE_SIZE+PTYPE_EMPTY, 1);
      out.add(offset + move.ptype()*PTYPE_SIZE+attack.ptype(), 1);
      if (info.see < 0)
	out.add(offset + PTYPE_EMPTY*PTYPE_SIZE+PTYPE_EMPTY, info.see/1024.0);
    }
  };
  class AttackToOpposingSliders : public Feature
  {
  public:
    AttackToOpposingSliders() : Feature("AttackToOpposingSliders", PTYPE_SIZE*PTYPE_SIZE*2)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      for (Piece piece: state.pin_by_opposing_sliders) {
	if (! state.state.hasEffectIf(move.ptypeO(), move.to(),
					  piece.square())) 
	  continue;
	int base = (piece.owner() == info.player) ? PTYPE_SIZE*PTYPE_SIZE : 0;
	out.add(offset + base + PTYPE_EMPTY*PTYPE_SIZE+piece.ptype(), 1);
	out.add(offset + base + move.ptype()*PTYPE_SIZE+PTYPE_EMPTY, 1);
	out.add(offset + base + move.ptype()*PTYPE_SIZE+piece.ptype(), 1);
	if (info.see < 0)
	  out.add(offset + base + PTYPE_EMPTY*PTYPE_SIZE+PTYPE_EMPTY, info.see/1024.0);
      }
    }
  };
  class PawnAttack : public Feature
  {
  public:
    enum { 
      PTYPE2_DIM = PTYPE_SIZE*2*PTYPE_SIZE*2*2,
      EFFECT_DIM = PTYPE_SIZE*2*8*9,
      BasicSize = PTYPE2_DIM+EFFECT_DIM,
      PawnSize = BasicSize*3,
      DIM = PawnSize*2
    };
    PawnAttack() : Feature("PawnAttack", DIM) 
    {
    }
    std::pair<int,int> squareStatus(const NumEffectState& state, Player player, Square to, Square& front) const
    {
      int u = 0, uu = 0;
      const int dy = (player == BLACK) ? -1 : 1;
      Square position = to + Offset(0, dy);
      front = position;
      Piece piece = state.pieceAt(position);
      if (piece.isPiece()) 
	u = piece.ptype() + ((piece.owner() == player) ? PTYPE_SIZE : 0);
      assert(! piece.isEdge());	// pawn move 
      piece = state.pieceAt(position + Offset(0, dy));
      if (piece.isPiece()) 
	uu = piece.ptype() + ((piece.owner() == player) ? PTYPE_SIZE : 0);
      return std::make_pair(u, uu);
    } 
    void matchPtype(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      const Player player = move.player;
      const Square to = move.move.to();

      Square front;
      const std::pair<int,int> u = squareStatus(state.state, player, to, front);

      int promotion = 0;
      if (front.canPromote(player))
	promotion = 1;
      else if (front.canPromote(alt(player)))
	promotion = 2;
      offset += BasicSize*promotion;

      bool pawn_drop = move.move.isDrop();
      const int index0 = (u.first*PTYPE_SIZE*2+u.second)*2 + pawn_drop;
      out.add(offset + index0, 1);

      const int effect = classifyEffect9(state.state, player, front);
      const int index1 = u.first*8 + move.move.to().squareForBlack(player).y()-2;
      out.add(offset + index1*9+effect, 1);
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (move.move.ptype() == PAWN)
	return matchPtype(state, move, offset, out);
      if (move.move.ptype() == LANCE
	  && state.state.canDropPawnTo(move.player, move.move.to().x()))
	return matchPtype(state, move, offset+PawnSize, out);
    }
  };

  class BlockLong : public Feature
  {
  public:
    enum {
      AttackPtype = 8,
      BasicAttack = AttackPtype*osl::PTYPEO_SIZE,
      OptionSize = 8, // King8, HasSupport, Promotable, Shadowing,...
      LongAttackSize = BasicAttack * OptionSize,
      DIM = PTYPE_SIZE * 2 * LongAttackSize
    };
    BlockLong() : Feature("BlockLong", DIM)
    {
    }
    static int longAttackIndex(osl::PtypeO ptypeo) // [0,7]?
    {
      using namespace osl;
      const Ptype ptype = getPtype(ptypeo);
      int index;
      if (ptype == LANCE) index = 0;
      else if (ptype == BISHOP) index = 1;
      else if (ptype == ROOK) index = 2;
      else {
	assert(ptype == PROOK || ptype == PBISHOP);
	index = 3;
      }
      if (getOwner(ptypeo) == WHITE) index += 4;
      return index;
    }
    static void addPiece(Player turn, const StateInfo& state, Piece piece,
			 Square to, index_list_t& out, int offset)
    {
      assert(state.state.hasEffectByPiece(piece, to));
      const Direction d
	= Board_Table.getLongDirection<BLACK>(piece.square(), to);
      const StateInfo::long_attack_t&
	cache = state.long_attack_cache[piece.number()][longToShort(d)];
      for (int index: cache) {
	assert(index < LongAttackSize);
	out.add(index+offset, 1);
      }      
    }
    static int ptypeSupport(Ptype moved, bool has_support)
    {
      return (moved*2 + has_support) * LongAttackSize;
    }
    static void findAll(const StateInfo& state, Player P,
			Square target, index_list_t& out, int offset)
    {
      mask_t m = state.state.longEffectAt(target, P);
      while (m.any()) {
	const Piece piece = state.state.pieceOf
	  (m.takeOneBit() +PtypeFuns<LANCE>::indexNum*32);
	addPiece(P, state, piece, target, out, offset);
      }
      m = state.state.longEffectAt(target, alt(P));
      while (m.any()) {
	const Piece piece = state.state.pieceOf
	  (m.takeOneBit()+PtypeFuns<LANCE>::indexNum*32);
	addPiece(P, state, piece, target, out, offset);
      }
    }
    static void findAll(const StateInfo& state, Move move, index_list_t& out, int offset=0)
    {
      const Player P = move.player();
      Square target = move.to();
      int a = state.state.countEffect(P, target);
      offset += ptypeSupport(move.ptype(), a+move.isDrop()>1);
      findAll(state, P, target, out, offset);
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      findAll(state, move.move, out, offset);
    }
    static void updateCache(StateInfo&);
  private:
    static void makeLongAttackOne(StateInfo& info,
				  Piece piece, Direction d);
  };
  class BlockLongFrom : public Feature
  {
  public:
    enum {
      DIM = BlockLong::LongAttackSize
    };
    BlockLongFrom() : Feature("BlockLongFrom", DIM)
    {
    }
    static void findAll(const StateInfo& state, Move move, index_list_t& out, int offset=0)
    {
      const Player P = move.player();
      Square target = move.from();
      BlockLong::findAll(state, P, target, out, offset);
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (move.move.isDrop())
	return;
      findAll(state, move.move, out, offset);
    }
  };
  class LongRecapture : public Feature
  {
  public:
    enum {
      DIM = BlockLong::LongAttackSize * PTYPE_SIZE
    };
    LongRecapture() : Feature("LongRecapture", DIM)
    {
    }
    void match(const StateInfo& info, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (move.see >= 0)
	return;
      const NumEffectState& state = info.state;
      const Square to = move.move.to();
      int a = state.countEffect(move.player, to)+move.move.isDrop()-1;
      int d = state.countEffect(alt(move.player), to);
      if (d == 1
	  || (d == 2 && a > 0
	      && state.hasEffectByPiece(state.kingPiece(alt(move.player)), to))) {
	out.add(offset + (PTYPE_EMPTY)*BlockLong::LongAttackSize,
		move.see/1024.0);
	offset += move.move.ptype() * BlockLong::LongAttackSize;
	const Piece opponent = state.findCheapAttack(alt(move.player), to);
	BlockLong::findAll(info, move.player, opponent.square(), out, offset);
      }
    }
  };
  class AddEffectLong : public Feature
  {
  public:
    enum {
      DIM = PTYPE_SIZE*BlockLong::LongAttackSize
    };
    AddEffectLong() : Feature("AddEffectLong", DIM)
    {
    }
    static void addOne(Direction dir, const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out)
    {
      Offset diff = Board_Table.getOffset(move.player, dir);
      Square to = move.move.to() + diff;
      if (isLong(dir)) {
	while (state.state.pieceAt(to).isEmpty())
	  to += diff;
      }
      if (! state.state.pieceAt(to).isPiece())
	return;
      BlockLong::findAll(state, move.player, to, out, offset);
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      offset += move.move.ptype()*BlockLong::LongAttackSize;
      unsigned int directions = Ptype_Table.getMoveMask(move.move.ptype());
      do {
	Direction d = (Direction)(misc::BitOp::bsf(directions));
	directions &= directions-1;
	addOne(d, state, move, offset, out);
      } while (directions);
    }
  };

  class LanceAttack : public Feature
  {
  public:
    enum {
      PatternCacheSize = PatternCommon::PatternCacheSize,
      DIM = PatternCacheSize*(8+4+4+1)
    };
    LanceAttack() : Feature("LanceAttack", DIM)
    {
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (move.move.ptype() != LANCE 
	  || (! move.move.isDrop()
	      && move.move.capturePtype() == PTYPE_EMPTY))
	return;
      const Offset up = Board_Table.getOffset(move.player, U);
      Square target = move.move.to() + up;
      while (state.state.pieceAt(target).isEmpty())
	target += up;
      if (state.state.pieceAt(target).isOnBoardByOwner(move.player)) {
	target += up;
	if (state.state.pieceAt(target).ptype() == LANCE) {
	  while (state.state.pieceAt(target).isEmpty())
	    target += up;
	}
      }
      if (state.state.pieceAt(target).isEdge()) 
	target -= up;

      int y = move.move.to().y(), x = move.move.to().x();
      if (move.player == WHITE)
	y = 10-y;
      y -= 2;
      int dx1 = abs(state.state.kingSquare(move.player).x()-x);
      int dx2 = abs(state.state.kingSquare(alt(move.player)).x()-x);
      dx1 = std::min(dx1, 3);
      dx2 = std::min(dx2, 3);
      bool pawn = state.state.canDropPawnTo(alt(move.player), x);
      assert(! state.state.pieceAt(target).isEdge());
      const StateInfo::pattern_square_t& cache
	= state.pattern_cache[target.index()];
      for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i) {
	out.add(offset + y*PatternCacheSize + cache[i], 1);
	out.add(offset + (8+dx1)*PatternCacheSize + cache[i], 1);
	out.add(offset + (12+dx1)*PatternCacheSize + cache[i], 1);
	if (! pawn)
	  out.add(offset + 16*PatternCacheSize + cache[i], 1);
      }
    }
  };
  class BishopAttack : public Feature
  {
  public:
    enum {
      PatternCacheSize = PatternCommon::PatternCacheSize,
      DIM = PatternCacheSize*2	// square:2
    };
    BishopAttack() : Feature("BishopAttack", DIM)
    {
    }
    static void addSquare(Square target, const StateInfo& info,
			  int offset, index_list_t& out)
    {
      int type = 0;
      const StateInfo::pattern_square_t& cache
	= info.pattern_cache[target.index()];
      for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i) {
	out.add(offset + type + cache[i], 1);
      }
    }
    template <Direction D,Ptype Type>
    static
    void addOne(const StateInfo& info, Square to, int offset, index_list_t& out)
    {
      const NumEffectState& state = info.state;
      const Offset diff = DirectionPlayerTraits<D,BLACK>::offset();
      Square target = to + diff;
      if (state.pieceAt(target).isEdge())
	return;
      if (state.pieceAt(target).isPiece()
	  && (state.pieceAt(target).ptype() != Type
	      || state.pieceAt(target).owner() != state.turn())) {
	; // almost included in direct pattern
      } else {
	while (state.pieceAt(target).isEmpty())
	  target += diff;
	if (state.pieceAt(target).ptype() == Type
	    && state.pieceAt(target).owner() == state.turn()) {
	  // extend additional effect by the same ptype
	  target += diff;
	  while (state.pieceAt(target).isEmpty())
	    target += diff;
	}
	if (state.pieceAt(target).isEdge()) 
	  target -= diff;
	addSquare(target, info, offset, out);
	if (! state.pieceAt(target).isPiece())
	  return;
      }
      // shadowing
      target += diff;
      if (state.pieceAt(target).isEdge())
	return;
      while (state.pieceAt(target).isEmpty())
	target += diff;
      if (state.pieceAt(target).isEdge()) 
	target -= diff;
      addSquare(target, info, offset+PatternCacheSize, out);
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (unpromote(move.move.ptype()) != BISHOP)
	return;
      addOne<UR,BISHOP>(state, move.move.to(), offset, out);
      addOne<UL,BISHOP>(state, move.move.to(), offset, out);
      addOne<DR,BISHOP>(state, move.move.to(), offset, out);
      addOne<DL,BISHOP>(state, move.move.to(), offset, out);
    }
  };
  class RookAttack : public Feature
  {
  public:
    enum {
      DirectionSize = BishopAttack::DIM,
      DIM = DirectionSize*3
    };
    RookAttack() : Feature("RookAttack", DIM)
    {
    }
    void match(const StateInfo& state, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (unpromote(move.move.ptype()) != ROOK)
	return;
      const Square to = move.move.to();
      BishopAttack::addOne<R,ROOK>(state, to, offset, out);
      BishopAttack::addOne<L,ROOK>(state, to, offset, out);
      const bool pawn_drop = state.state.canDropPawnTo(alt(move.player), to.x());
      const int scale = pawn_drop ? 1 : 2;
      BishopAttack::addOne<U,ROOK>(state, to, offset+DirectionSize*scale, out);
      BishopAttack::addOne<D,ROOK>(state, to, offset+DirectionSize*scale, out);
    }
  };
  class BreakThreatmate : public Feature
  {
  public:
    enum {
      PatternCacheSize = PatternCommon::PatternCacheSize,
      AddEffectSize = PTYPE_SIZE * PatternCacheSize,
      OpenRoadSize = PTYPE_SIZE * PatternCacheSize, OpenRoadBase = AddEffectSize,
      KingMoveSize = PatternCacheSize, KingMoveBase = OpenRoadBase + OpenRoadSize,
      CaptureSize = PTYPE_SIZE*PTYPE_SIZE, CaptureBase = KingMoveBase + KingMoveSize,
      AddEffect8Size = PTYPE_SIZE*PatternCacheSize, AddEffect8Base = CaptureBase + CaptureSize,
      OtherMoveSize = 1, OtherMoveBase = AddEffect8Base + AddEffect8Size,	// should be penalized
      DIM = OtherMoveBase + OtherMoveSize
    };
    BreakThreatmate() : Feature("BreakThreatmate", DIM)
    {
    }
    static bool isKingMove(Move move) 
    {
      return move.ptype() == KING;
    }
    static bool isOpeningKingRoad(Move move, Square king) 
    {
      return ! move.isDrop()
	&& move.from().isNeighboring8(king);
    }
    static bool isDefendingThreatmate(Move move, Move threatmate,
				      const NumEffectState& state) 
    {
      if (move.to() == threatmate.to()
	  || state.hasEffectIf(move.ptypeO(), move.to(),
				   threatmate.to()))
	return true;
      if (threatmate.isDrop())
	return false;
      Offset32 offset32=Offset32(threatmate.from(),move.to());
      EffectContent effect=Ptype_Table.getEffect(move.ptypeO(),offset32);
      if (! effect.hasEffect())
	return false;
      if (effect.offset() == threatmate.to()-threatmate.from())
	return state.isEmptyBetween(threatmate.from(), move.to());
      return false;
    }
    static bool isDefendingKing8(Move move, Square king,
				 const NumEffectState& state)
    {
      if (Neighboring8Direct::hasEffect
	  (state, move.ptypeO(), move.to(), king))
	return true;
      mask_t m = state.longEffectAt(move.to(), alt(state.turn()));
      while (m.any()) {
	const Piece piece = state.pieceOf
	  (m.takeOneBit() +PtypeFuns<LANCE>::indexNum*32);
	if (Neighboring8Direct::hasEffect
	    (state, piece.ptypeO(), piece.square(), king))
	  return true;
      }
      return false;
    }
    void match(const StateInfo& info, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (! info.threatmate_move.isNormal())
	return;
      const size_t old_size = out.size();
      const NumEffectState& state = info.state;
      const StateInfo::pattern_square_t& cache
	= info.pattern_cache[move.move.to().index()];
      const int PatternAny = ptypeOIndex(PTYPEO_EDGE)*PatternCommon::SquareDim;
      if (isKingMove(move.move)) {
	// king move
	out.add(offset + KingMoveBase + PatternAny, 1);
	for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i) {
	  out.add(offset + KingMoveBase + cache[i], 1);
	}	
      } else {
	const Square king = state.kingSquare(move.player);
	if (isOpeningKingRoad(move.move, king)) {
	  // open king road
	  int base = OpenRoadBase + move.move.ptype()*PatternCacheSize;
	  out.add(offset + base + PatternAny, 1);
	  for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i) {
	    out.add(offset + base + cache[i], 1);
	  }
	} 
	if (isDefendingThreatmate(move.move, info.threatmate_move,
				  state)) {
	  // add effect to threatmate-move-at
	  int base = move.move.ptype()*PatternCacheSize;
	  out.add(offset + base + PatternAny, 1);
	  for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i) {
	    out.add(offset + base + cache[i], 1);
	  }
	} else if (isDefendingKing8(move.move, king, state)) {
	  // add effect to king8
	  int base = move.move.ptype()*PatternCacheSize
	    + AddEffect8Base;
	  out.add(offset + base + PatternAny, 1);
	  for (size_t i=0; i<cache.size() && cache[i] >= 0; ++i) {
	    out.add(offset + base + cache[i], 1);
	  }
	}
	const Piece captured = state.pieceOnBoard(move.move.to());
	// capture
	if (captured.isPiece()) {
	  if (Neighboring8Direct::hasEffect
	      (state, captured.ptypeO(), captured.square(), king)) {
	    out.add(offset + CaptureBase
		    + captured.ptype()*PTYPE_SIZE+move.move.ptype(), 1);
	    out.add(offset + CaptureBase
		    + captured.ptype()*PTYPE_SIZE, 1);
	  }
	  else {
	    out.add(offset + CaptureBase + captured.ptype()*PTYPE_SIZE
		    + PTYPE_EDGE, 1);
	  }
	}
      }
      if (out.size() == old_size)
	out.add(offset + OtherMoveBase, 1);
    }
  };

  class SendOff : public Feature
  {
  public:
    enum {
      DIM = PTYPE_SIZE,
    };
    SendOff() : Feature("SendOff", DIM)
    {
    }
    void match(const StateInfo& info, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (! info.sendoffs.isMember(move.move.to()))
	return;
      out.add(offset + move.move.ptype(), 1);
    }
  };

  class LureDefender : public Feature
  {
  public:
    enum {
      ATTACK_DIM = PTYPE_SIZE*PTYPE_SIZE*PTYPE_SIZE,
      DIM = ATTACK_DIM*3,
    };
    LureDefender() : Feature("LureDefender", DIM)
    {
    }
    static void match(const NumEffectState& state, Move move, int see,
		      const StateInfo::pinned_gs_t& pinned_list,
		      int offset, index_list_t& out)
    {
      assert(state.turn() == move.player());
      const Square to = move.to(), king = state.kingSquare(alt(state.turn()));
      const int value = 1;
      const Offset up = Board_Table.getOffset(state.turn(), U);
      const int basic_a = move.ptype() * PTYPE_SIZE*PTYPE_SIZE;
      for (PinnedGeneral defense: pinned_list) {
	if (to != defense.attack) 
	  continue;
	assert(defense.general.owner() != move.player());
	assert(defense.covered.owner() != move.player());
	if (defense.general.square() == move.to()+up)
	  offset += ATTACK_DIM;
	else if (state.hasEffectIf(move.ptypeO(), move.to(), defense.general.square()))
	  offset += ATTACK_DIM*2;
	int a = basic_a;
	if (defense.covered.square().canPromote(state.turn())
	    && canPromote(move.ptype()))
	  a = promote(move.ptype()) * PTYPE_SIZE*PTYPE_SIZE;
	const int b = defense.general.ptype() * PTYPE_SIZE;
	const int c = defense.covered.ptype();
	out.add(offset, value);
	if (see < 0)
	  out.add(offset+1, see/1024.0*value);
	out.add(offset + a, value);
	out.add(offset + b, value);
	out.add(offset + c, value);
	out.add(offset + a + b, value);
	out.add(offset + a + c, value);
	out.add(offset + b + c, value);
	if (defense.covered.square().canPromote(state.turn())) {
	  out.add(offset + a + PTYPE_EDGE, value);
	  out.add(offset + b + PTYPE_EDGE, value);
	}
	if (defense.covered.square().isNeighboring8(king)) {
	  out.add(offset + a + KING, value);
	  out.add(offset + b + KING, value);
	}
	out.add(offset + a + b + c, value);
	return;
      }
    }
    void match(const StateInfo& info, const MoveInfo& move, int offset, index_list_t& out) const
    {
      match(info.state, move.move, move.see,
	    info.exchange_pins[alt(move.player)], offset, out);
    }
  };

  class CheckmateIfCapture : public Feature
  {
  public:
    enum {
      DIM = PTYPE_SIZE*PTYPE_SIZE,
    };
    CheckmateIfCapture() : Feature("CheckmateIfCapture", DIM)
    {
    }
    void match(const StateInfo& info, const MoveInfo& move, int offset, index_list_t& out) const
    {
      if (info.state.inCheck() || move.see > -256)
	return;
      const int value = 1;
      const Square king = info.state.kingSquare(alt(move.player));
      if (move.move.capturePtype() != PTYPE_EMPTY
	  && ! info.state.hasPieceOnStand<GOLD>(move.player)
	  && ! info.state.hasPieceOnStand<SILVER>(move.player)
	  && ! info.move_candidate_exists[alt(move.player)])
	return;
      if (! Neighboring8Direct::hasEffect
	  (info.state, move.move.ptypeO(), move.move.to(), king))
	return;
      if (hasSafeCapture(info.copy, move.move))
	return;
      int ptype_index = move.move.ptype()*PTYPE_SIZE;
      int capture_index = move.move.capturePtype();
      out.add(offset + ptype_index + capture_index, value);
      out.add(offset + capture_index, value);
      out.add(offset + ptype_index + PTYPE_EDGE, value);
      out.add(offset + PTYPE_EDGE, value);
    }
    static bool hasSafeCapture(NumEffectState& state, Move);
  };
  class AttackKing8Long : public Feature
  {
  public:
    enum {
      DIM = PTYPE_SIZE*PTYPE_SIZE,
    };
    AttackKing8Long() : Feature("AttackKing8Long", DIM)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      for (Piece piece: state.king8_long_pieces) {
	if (! state.state.hasEffectIf(move.ptypeO(), move.to(),
					  piece.square())) 
	  continue;
	out.add(offset + PTYPE_EMPTY*PTYPE_SIZE+piece.ptype(), 1);
	out.add(offset + move.ptype()*PTYPE_SIZE+PTYPE_EMPTY, 1);
	out.add(offset + move.ptype()*PTYPE_SIZE+piece.ptype(), 1);
	if (state.pin_by_opposing_sliders.isMember(piece)
	    && info.see < 0)
	  out.add(offset + PTYPE_EMPTY*PTYPE_SIZE+PTYPE_EDGE, info.see/1024.0);
      }
    }
  };
  class OpposingPawn : public Feature
  {
  public:
    enum {
      DIM = /*king-x*/9 * /* stand-pawn */ 3 * /* others */ 2,
    };
    OpposingPawn() : Feature("OpposingPawn", DIM)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      if (move.ptype() != PAWN)
	return;
      const Square front = move.to() + Board_Table.getOffset(info.player, U);
      if (state.state.pieceAt(front).ptype() != PAWN)
	return;
      int king_x = abs(state.state.kingSquare(alt(info.player)).x() - front.x());
      int stand_pawn = state.state.countPiecesOnStand<PAWN>(info.player);
      if (move.isDrop())
	--stand_pawn;
      stand_pawn = std::min(2, stand_pawn);
      bool has_other = state.state.hasPieceOnStand<LANCE>(info.player)
	|| state.state.hasPieceOnStand<KNIGHT>(info.player)
	|| state.state.hasPieceOnStand<SILVER>(info.player)
	|| state.state.hasPieceOnStand<GOLD>(info.player)
	|| state.state.hasPieceOnStand<ROOK>(info.player)
	|| state.state.hasPieceOnStand<BISHOP>(info.player);
      int index = (king_x * 3 + stand_pawn) * 2 + has_other;
      out.add(offset + index, 1);
    }
  };

  class DropAfterOpposingPawn : public Feature
  {
  public:
    enum {
      DIM = /*king-x*/9 * /* stand-pawn */ 3 * /* others */ 2 * /* capturepawn */ 2,
    };
    DropAfterOpposingPawn() : Feature("DropAfterOpposingPawn", DIM)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      const Move move = info.move;
      if (move.ptype() != PAWN || ! move.isDrop() || state.last_move < 0)
	return;
      int to_x = move.to().x();
      const Move last_move = state.moves[state.last_move];
      if (! last_move.isNormal() || last_move.isDrop()
	  || last_move.to().x() != to_x
	  || last_move.from().x() != to_x)
	return;
      
      const Square front = move.to()+Board_Table.getOffset(info.player, U);
      if (! front.canPromote(info.player))
	return;
      int king_x = abs(state.state.kingSquare(alt(info.player)).x() - to_x);
      int stand_pawn = std::min(2, state.state.countPiecesOnStand<PAWN>(info.player)-1);
      bool has_other = state.state.hasPieceOnStand<LANCE>(info.player)
	|| state.state.hasPieceOnStand<KNIGHT>(info.player)
	|| state.state.hasPieceOnStand<SILVER>(info.player)
	|| state.state.hasPieceOnStand<GOLD>(info.player)
	|| state.state.hasPieceOnStand<ROOK>(info.player)
	|| state.state.hasPieceOnStand<BISHOP>(info.player);
      bool follow_pawn_capture = last_move.capturePtype() == PAWN;
      int index = ((king_x * 3 + stand_pawn) * 2 + has_other) * 2 + follow_pawn_capture;
      out.add(offset + index, 1);
    }
  };

  class CoverPawn : public Feature
  {
  public:
    enum {
      DIM = 9 * 2 * PTYPE_SIZE * PTYPE_SIZE
    };
    CoverPawn() : Feature("CoverPawn", DIM)
    {
    }
    void match(const StateInfo& state, const MoveInfo& info, int offset, index_list_t& out) const
    {
      if (state.last_move < 0 || state.state.inCheck()
	  || info.move.isCaptureOrPromotion())
	return;
      const Move last_move = state.moves[state.last_move];
      if (last_move.ptype() != PAWN)
	return;
      const Offset diff = Board_Table.getOffset(info.player, U);
      const Square front = last_move.to()-diff, front2 = front-diff;
      if (state.state.pieceOnBoard(front).ptype() != PAWN // must be turn's pawn
	  || state.state.pieceAt(front2).isOnBoardByOwner(alt(info.player)))
	return;
      const bool cover = state.state.hasEffectIf
	(info.move.ptypeO(), info.move.to(), front);
      const Ptype moved = cover ? info.move.ptype() : PTYPE_EDGE,
	threatened = state.state.pieceAt(front2).ptype();
      const int a = std::min(2,state.state.countEffect(alt(info.player), front) - 1);
      assert(a >= 0);
      const int b = std::min(2,state.state.countEffect(info.player, front));
      const int ptype_index = moved*PTYPE_SIZE+threatened;
      const bool has_pawn = state.state.hasPieceOnStand(alt(info.player),PAWN);
      const int pawn_index = PTYPE_SIZE*PTYPE_SIZE;
      const int effect_index = (a*3+b)*2*PTYPE_SIZE*PTYPE_SIZE
	+ (has_pawn ? pawn_index : 0);
      assert(effect_index >= 0);
      out.add(offset + threatened, 1);
      out.add(offset + ptype_index, 1);
      out.add(offset + effect_index + ptype_index, 1);
    }
  };

  class SacrificeAttack : public Feature
  {
  public:
    enum {
      StandCount = 64,
      DIM = StandCount * PTYPE_SIZE * 2
    };
    SacrificeAttack() : Feature("SacrificeAttack", DIM)
    {
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      const Square king = si.state.kingSquare(alt(mi.player));
      if (mi.see >= 0
	  || (! Neighboring8Direct::hasEffect
	      (si.state, mi.move.ptypeO(), mi.move.to(), king)))
	return;
      int stand = mi.standIndex(si.state);
      int index = (stand * PTYPE_SIZE + mi.move.ptype()) * 2;
      out.add(offset + index, 1);
      if (si.last_move >= 1) {
	Move my_last_move = si.moves[si.last_move-1];
	if (Neighboring8Direct::hasEffect
	    (si.state, my_last_move.ptypeO(), my_last_move.to(), king))
	  out.add(offset + index + 1, 1);
      }
    }
  };

  class King5x5Ptype : public Feature
  {
  public:
    enum {
      ONE_DIM = 25 * 4 * PTYPE_SIZE * PTYPE_SIZE,
      DIM = 2 * ONE_DIM,
    };
    King5x5Ptype() : Feature("King5x5Ptype", DIM)
    {
    }
    static void addOne(Player king, Square center, const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out)
    {
      const Square to = mi.move.to();
      int dx = center.x() - to.x();
      const int dy = center.y() - to.y();
      if (abs(dx) >= 3 || abs(dy) >= 3)
	return;
      if ((king == BLACK && center.x() > 5)
	  || (king == WHITE && center.x() >= 5))
	dx = -dx;
      int sq_index = (dx+2)*5 + dy+2;
      bool a = mi.move.isDrop() ? si.state.hasEffectAt(mi.player, to)
	: si.state.countEffect(mi.player,to) >= 2;
      bool d = si.state.hasEffectAt(alt(mi.player), to);
      int index = (sq_index*4 + a*2+d) * PTYPE_SIZE*PTYPE_SIZE
	+ mi.move.capturePtype()*PTYPE_SIZE + mi.move.ptype();
      out.add(offset + index, 1);
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      addOne(mi.player, si.state.kingSquare(mi.player), si, mi, offset, out);
      addOne(alt(mi.player), si.state.kingSquare(alt(mi.player)), si, mi,
	     offset+ONE_DIM, out);
    }
  };

  class KingBlockade : public Feature
  {
  public:
    enum {
      StandCount = SacrificeAttack::StandCount,
      BlockLastOne = 0, BlockFront = 1, 
      BlockSideWide = 2, BlockSideOther = 3, BlockBack = 4,
      DIM = 5 * StandCount
    };
    KingBlockade() : Feature("KingBlockade", DIM)
    {
    }
    static bool blockAll(const King8Info& ki, Square king, Move move,
			 const NumEffectState& state,
			 const CArray<Direction,3>& directions)
    {
      int liberty = 0;
      for (Direction d: directions) {
	if ((ki.liberty() & (1<<d)) == 0)
	  continue;
	++liberty;
	const Square sq = king + Board_Table.getOffset(alt(state.turn()), d);
	if (! state.hasEffectIf(move.ptypeO(), move.to(), sq))
	  return false;
      }
      return liberty > 0;
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      const NumEffectState& state = si.state;
      const King8Info ki = si.king8Info(alt(mi.player));
      const Square king = state.kingSquare(alt(mi.player));
      if (ki.libertyCount() == 0
	  || (! Neighboring8Direct::hasEffect
	      (state, mi.move.ptypeO(), mi.move.to(), king)))
	return;
      int stand = mi.standIndex(state);
      offset += stand*5;
      if (ki.libertyCount() == 1) {
	const Square sq = king
	  + Board_Table.getOffset(alt(mi.player),
				  (Direction)misc::BitOp::bsf(ki.liberty()));
	if (! state.hasEffectIf(mi.move.ptypeO(), mi.move.to(), sq))
	  return;
	out.add(offset+BlockLastOne, 1);
	// fall through
      }
      const CArray<Direction,3> front3 = {{ UL, U, UR }};
      if (blockAll(ki, king, mi.move, state, front3))
	out.add(offset+BlockFront, 1);
      const CArray<Direction,3> left3 = {{ UL, L, DL }};
      if (blockAll(ki, king, mi.move, state, left3)) {
	const bool wide = (mi.player== WHITE && king.x() < 5)
	  || (mi.player== BLACK && king.x() > 5);
	out.add(offset+ (wide ? BlockSideWide : BlockSideOther), 1);
      }
      const CArray<Direction,3> right3 = {{ UR, R, DR }};
      if (blockAll(ki, king, mi.move, state, right3)) {
	const bool wide = (mi.player== BLACK && king.x() < 5)
	  || (mi.player== WHITE && king.x() > 5);
	out.add(offset+ (wide ? BlockSideWide : BlockSideOther), 1);
      }
      const CArray<Direction,3> back3 = {{ DL, D, DR }};
      if (blockAll(ki, king, mi.move, state, back3))
	out.add(offset+BlockBack, 1);
    }    
  };

  class CoverFork : public Feature
  {
  public:
    enum {
      DIM = PTYPE_SIZE * PTYPE_SIZE * PTYPE_SIZE
    };
    CoverFork() : Feature("CoverFork", DIM)
    {
    }
    static bool defending(const NumEffectState& state, Move move, Square target)
    {
      if (state.countEffect(alt(state.turn()), target) > 1)
	return false;
      if (state.hasEffectIf(move.ptypeO(), move.to(), target))
	return true;
      Piece attacking = state.findCheapAttack(alt(state.turn()), target);
      Offset o = Board_Table.getShortOffsetNotKnight(Offset32(move.to(), target));
      if (o.zero()
	  || ! Board_Table.isBetween(move.to(), attacking.square(), target))
	return false;
      return state.countEffect(state.turn(), move.to()) >= (1-move.isDrop());
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      const NumEffectState& state = si.state;
      PieceMask attacked = state.piecesOnBoard(mi.player)
	& state.effectedMask(alt(mi.player))
	& ~(state.effectedMask(mi.player));
      attacked.clearBit<PAWN>();
      offset += mi.move.ptype()*PTYPE_SIZE*PTYPE_SIZE;
      while (attacked.any()) {
	Piece a = state.pieceOf(attacked.takeOneBit());
	if (! defending(state, mi.move, a.square()))
	  continue;
	int index_a = a.ptype()*PTYPE_SIZE;
	PieceMask copy = attacked;
	while (copy.any()) {
	  Piece b = state.pieceOf(copy.takeOneBit());
	  if (! defending(state, mi.move, b.square()))
	    continue;
	  out.add(offset, 1);
	  out.add(offset+index_a, 1);
	  out.add(offset+b.ptype(), 1);
	  out.add(offset+index_a+b.ptype(), 1);
	}
      }
    }
  };

  class ThreatmateByCapture : public Feature
  {
  public:
    enum {
      DIM = PTYPE_SIZE * PTYPE_SIZE
    };
    ThreatmateByCapture() : Feature("ThreatmateByCapture", DIM)
    {
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      const Move move = mi.move;
      const Ptype captured = move.capturePtype();
      if (captured == PTYPE_EMPTY
	  || (si.possible_threatmate_ptype
	      & 1<<(captured-PTYPE_BASIC_MIN)) == 0)
	return;
      out.add(offset, 1);
      if (mi.see < 0)
	out.add(offset+1, mi.see/1024.0);
      out.add(offset+captured, 1);
      out.add(offset+move.ptype()*PTYPE_SIZE, 1);
      out.add(offset+move.ptype()*PTYPE_SIZE+captured, 1);
    }
  };

  class PromotionBySacrifice : public Feature
  {
  public:
    enum {
      DIM = 2*PTYPE_SIZE * PTYPE_SIZE
    };
    PromotionBySacrifice() : Feature("PromotionBySacrifice", DIM)
    {
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      const Move move = mi.move;
      if (mi.see >= 0 || move.isDrop()
	  || si.state.inCheck()
	  || si.threatmate_move.isNormal())
	return;
      const NumEffectState& state = si.state;
      mask_t m = state.longEffectAt(move.from(), state.turn());
      while (m.any()) {
	const Piece piece = state.pieceOf
	  (m.takeOneBit() +PtypeFuns<LANCE>::indexNum*32);
	if (piece.ptype() == LANCE || piece.isPromoted()
	    || piece.square().canPromote(mi.player))
	  continue;
	Offset o = Board_Table.getShortOffsetNotKnight(Offset32(move.from(), piece.square()));
	assert(! o.zero());
	bool can_promote = false;
	Square to = move.from()+o;
	if (to == move.to())
	  continue;
	while (state[to].isEmpty()) {
	  if (to.canPromote(mi.player)
	      && ! state.hasEffectAt(alt(mi.player), to))
	    can_promote = true;
	  to += o;
	}
	assert(state[to] != piece);
	int index = 0;
	if (piece.ptype() == ROOK)
	  index += PTYPE_SIZE*PTYPE_SIZE;
	index += move.ptype()*PTYPE_SIZE;
	if (to.canPromote(mi.player)
	    && state[to].isOnBoardByOwner(alt(mi.player))
	    && ! state.hasEffectAt(alt(mi.player), to)) {
	  out.add(offset, 1);
	  out.add(offset+1, mi.see/1024.0);
	  if (mi.check)
	    out.add(offset+2, 1);
	  if (state.hasEffectAt(alt(mi.player), piece.square())) {
	    out.add(offset+3, 1);
	    if (mi.check)
	      out.add(offset+4, 1);
	  }
	  if (state.hasEffectIf(piece.ptypeO(), to,
				state.kingSquare(alt(mi.player))))
	    out.add(offset+5, 1);
	  out.add(offset + index + state[to].ptype(), 1);
	}
	else if (can_promote) {
	  out.add(offset, 1);
	  out.add(offset+1, mi.see/1024.0);
	  if (mi.check)
	    out.add(offset+2, 1);
	  if (state.hasEffectAt(alt(mi.player), piece.square())) {
	    out.add(offset+3, 1);
	    if (mi.check)
	      out.add(offset+4, 1);
	  }
	  out.add(offset + index, 1);
	}
      }
    }
  };

  class EscapeThreatened : public Feature
  {
  public:
    enum {
      DIM =(2*PTYPE_SIZE * PTYPE_SIZE) * 3 * PTYPE_SIZE
    };
    EscapeThreatened() : Feature("EscapeThreatened", DIM)
    {
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      const NumEffectState& state = si.state;
      const Move move = mi.move;
      const Piece target = si.threatened[mi.player];
      if (mi.see > 0 || mi.check || mi.open_check
	  || ! move.isNormal()
	  || ! target.isPiece() || state.inCheck()
	  || si.threatmate_move.isNormal()
	  || Neighboring8Direct::hasEffect
	  (state, move.ptypeO(), move.to(), state.kingSquare(alt(mi.player))))
	return;
      const int t0 = target.ptype()*PTYPE_SIZE*2*PTYPE_SIZE*3;
      const int t1 = t0 + state.findCheapAttack(alt(mi.player), target.square()).ptype()*2*PTYPE_SIZE*3;
      const int t2 = t1 + state.hasEffectAt(mi.player, target.square())*PTYPE_SIZE*3;
      const int value = 1;
      if (! move.isDrop() && state[move.from()] == target) {
	out.add(offset + t0, value);
	out.add(offset + t1, value);
	out.add(offset + t2, value);
	return;
      }
      if (state.hasEffectIf(move.ptypeO(), move.to(),
				target.square())) {
	if (move.isDrop()
	    || ! state.hasEffectIf(move.oldPtypeO(), move.from(),
				   target.square())) {
	  out.add(offset + t0 + move.ptype(), value);
	  out.add(offset + t1 + move.ptype(), value);
	  out.add(offset + t2 + move.ptype(), value);
	  return;
	}
      }
      // not related move
      offset += PTYPE_SIZE;
      if (move.isDrop())
	offset += PTYPE_SIZE;
      out.add(offset + t0 + move.ptype(), value);
      out.add(offset + t1 + move.ptype(), value);
      out.add(offset + t2 + move.ptype(), value);
    }
  };

  class BookMove : public Feature
  {
  public:
    enum {
      DIM = 2
    };
    BookMove() : Feature("BookMove", DIM)
    {
    }
    void match(const StateInfo& si, const MoveInfo& mi, int offset, index_list_t& out) const
    {
      if (! si.bookmove[0].isNormal())
	return;
      if (mi.move == si.bookmove[0] || mi.move == si.bookmove[1])
	out.add(offset, 1);
      else
	out.add(offset+1, 1);
    }
  };
}

#endif /* GPSSHOGI_MOVE_PROBABILITY_FEATURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
