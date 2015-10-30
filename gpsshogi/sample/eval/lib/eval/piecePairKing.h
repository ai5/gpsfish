/* piecePairKing.h
 */
#ifndef GPSSHOGI_PIECEPAIRKING_H
#define GPSSHOGI_PIECEPAIRKING_H
#include "eval/eval.h"
namespace gpsshogi
{
  /**
   * 玉が自陣9*3に居る時に盤面自陣側9*5の自分の駒二つと玉との関係を数える。成駒は見ない
   * 15(X軸対称性) * (45*7) * (45*7) = 1488375 実効としては対称性で少し減るはず
   */
  class PiecePairKing : public FeaturesOneNonUniq
  {
  public:
    PiecePairKing() : FeaturesOneNonUniq(1488375) {}
    void featuresOneNonUniq(const NumEffectState &state,
			    index_list_t &out) const;
    const std::string name() const { return "PiecePairKing"; }
    size_t maxActive() const { return 38*38; }

    static int indexWhite(Square p)
    {
      return p.x()-1 + (p.y()-1)*9;
    }
    static int indexKing(Player owner, Square king, bool& flipx)
    {
      if (owner == BLACK)
	king = king.rotate180();
      assert(king.y() <= 3);
      if (king.x() > 5)
      {
	king = king.flipHorizontal();
	flipx = true;
      }
      else
	flipx = false;
      return (king.x()-1 + (king.y()-1)*5)*45*7*45*7;
    }
    static int indexPiece(Player owner, Square position, Ptype ptype, bool flipx)
    {
      assert(! isPromoted(ptype));
      if (owner == BLACK)
	position = position.rotate180();
      if (flipx)
	position = position.flipHorizontal();
      assert(position.y() <= 5);
      return indexWhite(position)*7 + ptype-PTYPE_BASIC_MIN-1;
    }
    static int composeIndex(int king, int i0, int i1)
    {
      return i0 < i1 ? (king + i0*45*7 + i1) : (king + i1*45*7 + i0);
    }
    static int indexWhite(Square king, Piece p0, Piece p1)
    {
      assert(king.y() <= 3);
      assert(! p0.isPromoted());
      assert(! p1.isPromoted());
      bool flipx;
      const int king_index = indexKing(WHITE, king, flipx);
      const int i0 = indexPiece(WHITE, p0.square(), p0.ptype(), flipx);
      const int i1 = indexPiece(WHITE, p1.square(), p1.ptype(), flipx);
      return composeIndex(king_index, i0, i1);
    }
    static int indexBlack(Square king, Piece p0, Piece p1)
    {
      p0.setSquare(p0.square().rotate180());
      p1.setSquare(p1.square().rotate180());
      return indexWhite(king.rotate180(), p0, p1);
    }
  };

  class PiecePairKingFlat : public EvalComponent
  {
    PiecePairKing feature;
  public:
    PiecePairKingFlat() : EvalComponent(PiecePairKing().dimension())
    {
    }
    ~PiecePairKingFlat();
    const std::string name() const { return feature.name(); }
    int evalWithUpdate(const NumEffectState& state, Move moved, int last_value) const;
    int eval(const NumEffectState& state) const;
    void featuresNonUniq(const NumEffectState& state, index_list_t& out, int offset) const;
  private:
    int add(const NumEffectState& state, Player player, Square to, Ptype ptype) const;
    int sub(const NumEffectState& state, Player player, Square from, Ptype ptype) const;
    int addSub(const NumEffectState& state, Player player, Square to, Ptype ptype, Square from) const;
  };
}


#endif /* GPSSHOGI_PIECEPAIRKING_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
