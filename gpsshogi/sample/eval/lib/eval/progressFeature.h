/* progressFeature.h
 */
#ifndef _PROGRESSFEATURE_H
#define _PROGRESSFEATURE_H

#include "eval/eval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/bits/centering5x3.h"
#include <algorithm>
namespace gpsshogi
{
  class ProgressFeatureBase : public Eval
  {
  protected:
    boost::ptr_vector<EvalComponent> all;
    int max_progress;
    size_t dim, max_active;
    double scale;
  public:
    ProgressFeatureBase();
    ~ProgressFeatureBase();

    virtual double maxProgressConstraint() const=0;
  protected:
    /** call once in constructor of subclass, after all EvalComponents added */
    void addFinished();
  public:
    int progress(const NumEffectState&) const;
    int eval(const NumEffectState&) const;
    int newValue(const NumEffectState& new_state, Move moved, int old_value) const;

    bool load(const char *filename);
    void save(const char *filename) const;
    void setWeight(const double*);
    void setWeightScale(const double*, double);
    void saveWeight(double*) const;

    int flatValue(size_t index) const {
      for (size_t i=0; i<all.size(); ++i) {
	if (index < all[i].dimension())
	  return all[i].value(index);
	index -= all[i].dimension();
      }
      return max_progress;
    }
    size_t dimension() const { return dim; }
    size_t lambdaStart() const { return osl::PTYPE_SIZE; };
    void features(const NumEffectState&, MoveData&, int) const;
    void featuresProgress(const NumEffectState&, MoveData&) const;
    void showSummary(std::ostream&) const;
    void showAll(std::ostream&) const;
    void setRandom();
    int roundUp() const { return 1; }

    size_t maxActive() const;
    int pawnValue() const;

    int maxProgress() const { return max_progress; }
    std::tuple<std::string, int, int> findFeature(size_t index) const;

    class Stack;
    friend class Stack;
    EvalValueStack *newStack(const NumEffectState& state);
    static int compose(int progress, int progress_max, int independent, const MultiInt& stage_value);
    const std::string describe(const std::string& feature, size_t local_index) const;
    const std::string describeAll(const std::string& feature) const;
  private:
    int featuresCommon(const NumEffectState& state, 
		       MoveData& data, int offset) const;
  };
  
  class NullProgressFeatureEval : public ProgressFeatureBase
  {
  public:
    NullProgressFeatureEval();
    ~NullProgressFeatureEval();
    double maxProgressConstraint() const;
  };
  class HandProgressFeatureEval : public ProgressFeatureBase
  {
    struct HandProgress;
  public:
    HandProgressFeatureEval();
    ~HandProgressFeatureEval();
    double maxProgressConstraint() const;
  };

  class EffectProgressFeatureEval : public ProgressFeatureBase
  {
  public:
    EffectProgressFeatureEval();
    ~EffectProgressFeatureEval() { }
    double maxProgressConstraint() const;
  };

  class StableEffectProgressFeatureEval : public ProgressFeatureBase
  {
  public:
    StableEffectProgressFeatureEval();
    ~StableEffectProgressFeatureEval() { }
    double maxProgressConstraint() const;
    void setWeight(const double*);
  };

  class Null5x3Type { };
  template <class L, class R>
  struct Feature5x3List
  {
  };

  template <class FeatureList>
  struct FeatureDimension;
  template <>
  struct FeatureDimension<Null5x3Type>
  {
    enum { value = 0 };
  };
  template <class Head, class Tail>
  struct FeatureDimension<Feature5x3List<Head, Tail> >
  {
    enum { value = Head::DIM + FeatureDimension<Tail>::value };
  };

  struct Effect5x3Util
  {
    template <class Null5x3Type, Player P>
    static void featuresEach(
      Null5x3Type, Player2Type<P>,
      Square, Square,
      int, int,
      int,
      IndexCacheI<MaxActiveWithDuplication> &)
    {
    }

    template <class Head, class Tail, Player P>
    static void featuresEach(
      Feature5x3List<Head, Tail>, Player2Type<P>,
      Square king, Square target,
      int attack_count, int defense_count,
      int offset,
      IndexCacheI<MaxActiveWithDuplication> &out)
    {
      Head::template features<P>(king, target,
				       attack_count, defense_count,
				       offset, out);
      featuresEach(Tail(), Player2Type<P>(), king, target,
		   attack_count, defense_count, offset + Head::DIM, out);
    }

    template <osl::Player Defense, typename Features>
    static void effect5x3(const osl::NumEffectState &state,
			  int offset,
			  IndexCacheI<MaxActiveWithDuplication> &out)
    {
      const Square king = state.kingSquare<Defense>();
      const Square center = Centering5x3::adjustCenter(king);
      const int min_x = center.x() - 2;
      const int min_y = center.y() - 1;

      for (int dx=0; dx<5; ++dx)
      {
	for (int dy=0; dy<3; ++dy)
	{
	  const Square target(min_x+dx,min_y+dy);
	  const NumBitmapEffect effect = state.effectSetAt(target);
	  const int attack_count =
	    effect.countEffect(alt(Defense));
	  const int defense_count = effect.countEffect(Defense);
	  featuresEach(Features(), Player2Type<Defense>(), king, target,
		       attack_count, defense_count,
		       offset, out);
	}
      }
    }

    template <typename Features>
    static void featuresAll(
      const osl::NumEffectState &state,
      IndexCacheI<MaxActiveWithDuplication> &out)
    {
      effect5x3<BLACK, Features>(state, 0, out);
      effect5x3<WHITE, Features>(state, 0, out);
    }
  };

  class Effect5x3T
  {
  private:
    template <Player P>
    static int index(Square king, Square target)
    {
      const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return x_diff * 5 + y_diff;
    }
  public:
    enum { DIM = 25 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int attack_count, int /*defense_count*/,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      if (attack_count != 0)
      {
	out.add(index<Defense>(king, target) + offset, attack_count);
      }
    }
  };

  class Effect5x3D
  {
  private:
    template <Player P>
    static int index(Square king, Square target)
    {
      const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return x_diff * 5 + y_diff;
    }
  public:
    enum { DIM = 25 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int /*attack_count*/, int defense_count,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      if (defense_count != 0)
      {
	out.add(index<Defense>(king, target) + offset, defense_count);
      }
    }
  };

  class Effect5x3KXT
  {
  private:
    template <Player P>
    static int index(Square king, Square target)
    {
      int target_x = (king.x() > 5 ? 10 - king.x() : king.x()); // [1, 5]
      int x_diff = king.x() - target.x(); // [-4, 4]
      if (P == BLACK && king.x() >= 6)
      {
	x_diff = -x_diff;
      }
      else if (P == WHITE && king.x() >= 5)
      {
	x_diff = -x_diff;
      }
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return ((x_diff + 4) * 5 + y_diff) * 5 + target_x - 1;
    }
  public:
    enum { DIM = 225 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int attack_count, int /*defense_count*/,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      if (attack_count != 0)
      {
	out.add(index<Defense>(king, target) + offset, attack_count);
      }
    }
  };

  class Effect5x3KYT
  {
  private:
    template <Player P>
    static int index(Square king, Square target)
    {
      const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      const int king_y = (P == BLACK ? king.y() : 10 - king.y()); // [1, 9]
      return (x_diff * 5 + y_diff) * 9 + king_y - 1;
    }
  public:
    enum { DIM = 225 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int attack_count, int /*defense_count*/,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      if (attack_count != 0)
      {
	out.add(index<Defense>(king, target) + offset, attack_count);
      }
    }
  };

  class Effect5x3PerEffectT
  {
  private:
    template <Player P>
    static int index(Square king, Square target,
		     int count)
    {
      const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return x_diff * 5 + y_diff + std::min(8, count) * 25;
    }
  public:
    enum { DIM = 225 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int attack_count, int /*defense_count*/,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      out.add(index<Defense>(king, target, attack_count) + offset, 1);
    }
  };

  class Effect5x3PerEffectDT
  {
  private:
    template <Player P>
    static int index(Square king, Square target,
		     int count)
    {
      const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return x_diff * 5 + y_diff + std::min(8, count) * 25;
    }
  public:
    enum { DIM = 225 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int /*attack_count*/, int defense_count,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      out.add(index<Defense>(king, target, defense_count) + offset, 1);
    }
  };

  class Effect5x3PerEffectYT
  {
  private:
    template <Player P>
    static int index(Square king, Square target,
		     int count)
    {
      const int king_y = (P == BLACK ? king.y() : 10 - king.y());
      const int x_diff = std::abs(king.x() - target.x()); // [0, 4]
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return king_y - 1 + 9 * (x_diff * 5 + y_diff + std::min(8, count) * 25);
    }
  public:
    enum { DIM = 2025 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int attack_count, int /*defense_count*/,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      out.add(index<Defense>(king, target, attack_count) + offset, 1);
    }
  };

  class Effect5x3PerEffectXT
  {
  private:
    template <Player P>
    static int index(Square king, Square target,
		     int count)
    {
      const int king_x = (king.x() > 5 ? 10 - king.x() : king.x());
      int x_diff = king.x() - target.x(); // [-4, 4]
      if ((P == BLACK && (king.x() > 5)) ||
	  (P == WHITE && (king.x() >= 5)))
	x_diff = -x_diff;
      const int y_diff = (P == BLACK ? king.y() - target.y() :
			  target.y() - king.y()) + 2; // [-2, 2] + 2
      return king_x - 1 + 5 * (x_diff + 4 +
			       9 * (y_diff + 5 *  std::min(8, count)));
    }
  public:
    enum { DIM = 2025 };
    template <Player Defense>
    static void features(Square king, Square target,
			 int attack_count, int /*defense_count*/,
			 int offset,
			 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      out.add(index<Defense>(king, target, attack_count) + offset, 1);
    }
  };

  typedef Feature5x3List<
    Effect5x3T,
    Feature5x3List<
      Effect5x3KXT,
      Feature5x3List<
	Effect5x3KYT,
	Feature5x3List<
	  Effect5x3D,
	  Feature5x3List<
	    Effect5x3PerEffectT,
	    Feature5x3List<
	      Effect5x3PerEffectDT,
	      Feature5x3List<
		Effect5x3PerEffectYT,
		Feature5x3List<
		  Effect5x3PerEffectXT, Null5x3Type> > > > > > > >
  Effect5x3List;
  class Effect5x3Features : public EvalComponent
  {
  public:
    Effect5x3Features() :
      EvalComponent(FeatureDimension<Effect5x3List>::value) { } 
    int eval(const osl::NumEffectState& state) const
    {
      IndexCacheI<MaxActiveWithDuplication> values;
      Effect5x3Util::featuresAll<Effect5x3List>(state, values);
      int result = 0;
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += values[i].second * value(values[i].first);
      }
      return result;
    }
    void featuresNonUniq(const osl::NumEffectState& state,
			 index_list_t& out,
			 int offset) const
    {
      assert(offset == 0);
      Effect5x3Util::featuresAll<Effect5x3List>(state, out);
    }
    const std::string name() const
    {
      return "Effect5x3Features";
    }
  };

  class Null5x5Type { };
  template <class L, class R>
  struct Feature5x5List
  {
  };

  template <class FeatureList>
  struct FeatureDimension;
  template <>
  struct FeatureDimension<Null5x5Type>
  {
    enum { value = 0 };
  };
  template <class Head, class Tail>
  struct FeatureDimension<Feature5x5List<Head, Tail> >
  {
    enum { value = Head::DIM + FeatureDimension<Tail>::value };
  };
  struct Effect5x5Util
  {
    template <class Null5x5Type, Player P>
    static void featuresEach(
      Null5x5Type, Player2Type<P>,
      Square /*king*/, Square /*target*/,
      int /*attack_count*/, int /*defense_count*/,
      int /*offset*/,
      IndexCacheI<MaxActiveWithDuplication> &/*out*/)
    {
    }

    template <class Head, class Tail, Player P>
    static void featuresEach(
      Feature5x5List<Head, Tail>, Player2Type<P>,
      Square king, Square target,
      int attack_count, int defense_count,
      int offset,
      IndexCacheI<MaxActiveWithDuplication> &out)
    {
      Head::template featuresSquare<P>(king, target,
					 attack_count, defense_count,
					 offset, out);
      featuresEach(Tail(), Player2Type<P>(), king, target,
		   attack_count, defense_count, offset + Head::DIM, out);
    }

    template <class Null5x5Type, Player Attack>
    static void featuresMask(
      Null5x5Type, Player2Type<Attack>,
      const NumEffectState &,
      Square ,
      const PieceMask &,
      int ,
      IndexCacheI<MaxActiveWithDuplication> &)
    {
    }

    template <class Head, class Tail, Player P>
    static void featuresMask(
      Feature5x5List<Head, Tail>, Player2Type<P>,
      const NumEffectState &state,
      Square king,
      const PieceMask &king25,
      int offset,
      IndexCacheI<MaxActiveWithDuplication> &out)
    {
      Head::template featuresMask<P>(state, king, king25, offset, out);
      featuresMask(Tail(), Player2Type<P>(),
		   state, king, king25, offset + Head::DIM, out);
    }

    template <osl::Player Attack, typename Features>
    static void effect5x5(const osl::NumEffectState &state,
			  int offset,
			  IndexCacheI<MaxActiveWithDuplication> &out)
    {
      const Square king =
	state.kingSquare<alt(Attack)>();
      const int min_x = std::max(1, king.x() - 2);
      const int max_x = std::min(9, king.x() + 2);
      const int min_y = std::max(1, king.y() - 2);
      const int max_y = std::min(9, king.y() + 2);

      PieceMask mask;
      for (int y = min_y; y <= max_y; ++y)
      {
	for (int x = min_x; x <= max_x; ++x)
	{
	  const Square target(x, y);
	  const NumBitmapEffect effect = state.effectSetAt(target);
	  mask = mask | effect;
	  const int attack_count = effect.countEffect(Attack);
	  const int defense_count =
	    effect.countEffect(alt(Attack));
	  featuresEach(Features(), Player2Type<Attack>(), king, target,
		       attack_count, defense_count,
		       offset, out);
	}
      }
      mask = mask & state.piecesOnBoard(Attack);
      featuresMask(Features(), Player2Type<Attack>(),
		   state, king, mask, offset, out);
    }

    template <typename Features>
    static void featuresAll(
      const osl::NumEffectState &state, int offset,
      IndexCacheI<MaxActiveWithDuplication> &out)
    {
      effect5x5<BLACK, Features>(state, offset, out);
      effect5x5<WHITE, Features>(state, offset, out);
    }
  };
  class Feature5x5Base
  {
  public:
    template <Player Attack>
    static void featuresSquare(Square /*king*/, Square /*target*/,
				 int /*attack_count*/, int /*defense_count*/,
				 int /*offset*/,
				 IndexCacheI<MaxActiveWithDuplication> &/*out*/)
    {
    }
    template <Player Attack>
    static void featuresMask(const NumEffectState &,
			     Square ,
			     const PieceMask &,
			     int ,
			     IndexCacheI<MaxActiveWithDuplication> &) { }
  };

  class EffectState5x5T : public Feature5x5Base
  {
  public:
    enum { DIM = 75 };
    template <Player Attack>
    static void featuresSquare(Square king, Square target,
				 int attack_count, int defense_count,
				 int offset,
				 IndexCacheI<MaxActiveWithDuplication> &out)
    {
      const int effect_diff = attack_count - defense_count;
      const int x_diff = std::abs(target.x() - king.x()); // [0, 2]
      // [-2, 2]
      const int y_diff = (Attack == BLACK ? king.y() - target.y() :
			  target.y() - king.y());
      int index = std::max(std::min(effect_diff, 2), -2) + 2 + 5 * x_diff +
	5 * 3 * (y_diff + 2);
      out.add(index + offset, 1);
    }
  };

  class AttackPieces5x5T : public Feature5x5Base
  {
  public:
    enum { DIM = 1125 };
    template <Player Attack>
    static void featuresMask(const NumEffectState &state,
			     Square /*king*/,
			     const PieceMask &king25,
			     int offset,
			     IndexCacheI<MaxActiveWithDuplication> &out)
    {
      PieceMask mask = king25;
      const int rook = mask.selectBit<ROOK>().countBit();
      const int bishop = mask.selectBit<BISHOP>().countBit();
      const int gold = mask.selectBit<GOLD>().countBit();
      const int silver = (mask & ~state.promotedPieces()).selectBit<SILVER>().countBit();
      PieceMask promoted_pieces = mask & state.promotedPieces();
      promoted_pieces.clearBit<ROOK>();
      promoted_pieces.clearBit<BISHOP>();
      const int promoted = std::min(promoted_pieces.countBit(), 4);
      out.add(index(rook + state.countPiecesOnStand<ROOK>(Attack),
		    bishop + state.countPiecesOnStand<BISHOP>(Attack),
		    gold + state.countPiecesOnStand<GOLD>(Attack),
		    silver + state.countPiecesOnStand<SILVER>(Attack),
		    promoted) + offset, 1);
    }
  private:
    static int index(int rook, int bishop, int gold, int silver, int promoted)
    {
      assert(0 <= promoted && promoted <= 4);
      return promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook)));
    }
  };

  class AttackPieces5x5XT : public Feature5x5Base
  {
  public:
    enum { DIM = 5625 };
    template <Player Attack>
    static void featuresMask(const NumEffectState &state,
			     Square king,
			     const PieceMask &king25,
			     int offset,
			     IndexCacheI<MaxActiveWithDuplication> &out)
    {
      PieceMask mask = king25;
      const int rook = mask.selectBit<ROOK>().countBit();
      const int bishop = mask.selectBit<BISHOP>().countBit();
      const int gold = mask.selectBit<GOLD>().countBit();
      const int silver = (mask & ~state.promotedPieces()).selectBit<SILVER>().countBit();
      PieceMask promoted_pieces = mask & state.promotedPieces();
      promoted_pieces.clearBit<ROOK>();
      promoted_pieces.clearBit<BISHOP>();
      const int promoted = std::min(promoted_pieces.countBit(), 4);
      out.add(index(rook + state.countPiecesOnStand<ROOK>(Attack),
		    bishop + state.countPiecesOnStand<BISHOP>(Attack),
		    gold + state.countPiecesOnStand<GOLD>(Attack),
		    silver + state.countPiecesOnStand<SILVER>(Attack),
		    promoted,
		    king.x()) + offset, 1);
    }
  private:
    static int index(int rook, int bishop, int gold, int silver, int promoted,
		     int king_x)
    {
      assert(0 <= promoted && promoted <= 4);
      if (king_x > 5)
	king_x = 10 - king_x;
      return king_x - 1 +
	5 * (promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook))));
    }
  };

  class AttackPieces5x5YT : public Feature5x5Base
  {
  public:
    enum { DIM = 10125 };
    template <Player Attack>
    static void featuresMask(const NumEffectState &state,
			     Square king,
			     const PieceMask &king25,
			     int offset,
			     IndexCacheI<MaxActiveWithDuplication> &out)
    {
      PieceMask mask = king25;
      const int rook = mask.selectBit<ROOK>().countBit();
      const int bishop = mask.selectBit<BISHOP>().countBit();
      const int gold = mask.selectBit<GOLD>().countBit();
      const int silver = (mask & ~state.promotedPieces()).selectBit<SILVER>().countBit();
      PieceMask promoted_pieces = mask & state.promotedPieces();
      promoted_pieces.clearBit<ROOK>();
      promoted_pieces.clearBit<BISHOP>();
      const int promoted = std::min(promoted_pieces.countBit(), 4);
      out.add(index(rook + state.countPiecesOnStand<ROOK>(Attack),
		    bishop + state.countPiecesOnStand<BISHOP>(Attack),
		    gold + state.countPiecesOnStand<GOLD>(Attack),
		    silver + state.countPiecesOnStand<SILVER>(Attack),
		    promoted,
		    Attack == BLACK ? 10 - king.y() : king.y()) + offset, 1);
    }
  private:
    static int index(int rook, int bishop, int gold, int silver, int promoted,
		     int king_y)
    {
      assert(0 <= promoted && promoted <= 4);
      return king_y - 1 +
	9 * (promoted + 5 * (silver + 5 * (gold + 5 * (bishop + 3 * rook))));
    }
  };
  typedef Feature5x5List<
    AttackPieces5x5T,
    Feature5x5List<
      EffectState5x5T,
      Feature5x5List<
	AttackPieces5x5XT,
	Feature5x5List<AttackPieces5x5YT, Null5x5Type> > > >
  Effect5x5List;
  class Effect5x5Features : public EvalComponent
  {
  public:
    Effect5x5Features() :
      EvalComponent(FeatureDimension<Effect5x5List>::value) {
    } 
    int eval(const osl::NumEffectState& state) const
    {
      IndexCacheI<MaxActiveWithDuplication> values;
      Effect5x5Util::featuresAll<Effect5x5List>(state, 0, values);
      int result = 0;
      for (size_t i = 0; i < values.size(); ++i)
      {
	result += values[i].second * value(values[i].first);
      }
      return result;
    }
    void featuresNonUniq(const osl::NumEffectState& state,
			 index_list_t& out,
			 int offset) const
    {
      Effect5x5Util::featuresAll<Effect5x5List>(state, offset, out);
    }
    const std::string name() const
    {
      return "Effect5x5Features";
    }
  };
}

#endif /* _PROGRESSFEATURE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
