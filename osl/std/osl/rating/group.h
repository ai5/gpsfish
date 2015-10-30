/* group.h
 */
#ifndef _GROUP_H
#define _GROUP_H

#include "osl/rating/feature.h"
#include "osl/rating/range.h"
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

namespace osl
{
  namespace rating
  {
    /** mutually exclusive set of features */
    class Group : public boost::ptr_vector<Feature> 
    {
    public:
      std::string group_name;

      Group(const std::string& name);
      Group(Feature *f) : group_name(f->name()) { push_back(f); }
      virtual ~Group();
      virtual void show(std::ostream&, int name_width, const range_t& range, 
			const std::vector<double>& weights) const;

      /** @return -1 if not found */
      virtual int findMatch(const NumEffectState& state, Move m, const RatingEnv& env) const;
      void showMinMax(std::ostream& os, int name_width, const range_t& range, 
		      const std::vector<double>& weights) const;
      void showAll(std::ostream& os, int name_width, const range_t& range, 
		   const std::vector<double>& weights) const;
      void showTopN(std::ostream& os, int name_width, const range_t& range, 
		    const std::vector<double>& weights, int n) const;
      void saveResult(const std::string& directory, const range_t& range, 
		      const std::vector<double>& weights) const;
      bool load(const std::string& directory, const range_t& range, 
		std::vector<double>& weights) const;
      virtual bool effectiveInCheck() const { return (*this)[0].effectiveInCheck(); }
    };

    struct TakeBackGroup : public Group
    {
      TakeBackGroup() : Group("TakeBack")
      {
	push_back(new TakeBack());
	push_back(new TakeBack2());
      }
#ifndef MINIMAL
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
#endif
      int findMatch(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	const Square to = move.to();
	if (! env.history.hasLastMove() || env.history.lastMove().to() != to)
	  return -1;
	if (! env.history.hasLastMove(2) || env.history.lastMove(2).to() != to)
	  return 0;
	return 1;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct CheckGroup : public Group
    {
      CheckGroup() : Group("Check")
      {
	for (int i=0; i<4; ++i)
	  for (int p=0; p<8; ++p)	// progress8
	    push_back(new Check(i));
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const bool direct = state.isDirectCheck(move);
	const bool open = state.isOpenCheck(move);
	int index = -1;
	if (direct && !open)
	  index = Check::openLong(state, move);
	else if (open)
	  index = direct + 2;
	const int progress8 = env.progress.value()/2;
	return index*8 + progress8;
      }
      bool effectiveInCheck() const { return true; }
    };

    class SendOffGroup : public Group
    {
    public:
      SendOffGroup() : Group("SendOff")
      {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new SendOff(0));
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new SendOff(1));
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState&, Move move, const RatingEnv& env) const
      {
	if (! env.sendoffs.isMember(move.to()))
	  return -1;
	const int progress8 = env.progress.value()/2;
	return (move.capturePtype() != PTYPE_EMPTY)*8 + progress8;
      }
    };

    struct BlockGroup : public Group
    {
      BlockGroup() : Group("Block")
      {
	for (int s=0; s<=3; ++s) {
	  for (int o=0; o<=3; ++o) {
	    push_back(new Block(s, o));
	  }
	}
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& ) const
      {
	const int index = Block::count(state, move.to(), state.turn())*4
	  + Block::count(state, move.to(), alt(state.turn()));
	return index;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct OpenGroup : public Group
    {
      OpenGroup() : Group("Open")
      {
	for (int i=0; i<16; ++i)
	  push_back(new Open(i));
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& ) const
      {
	const int index = Open::index(state, move);
	return index;
      }
      bool effectiveInCheck() const { return true; }
    };

    struct ChaseGroup : public Group
    {
      ChaseGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const;
    };

    struct KaranariGroup : public Group
    {
      KaranariGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv&) const;
    };

    struct ImmediateAddSupportGroup : public Group
    {
      ImmediateAddSupportGroup();
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showTopN(os, name_width, range, weights, 3);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	const int index = ImmediateAddSupport::index(state, move, env);
	if (index < 0)
	  return index;	
	const int progress8 = env.progress.value()/2;
	return index*8 + progress8;
      }
    };

    struct BadLanceGroup : public Group
    {
      BadLanceGroup() : Group("BadLance")
      {
	push_back(new BadLance(false));
	push_back(new BadLance(true));
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv&) const
      {
	const Square front = Board_Table.nextSquare(move.player(), move.to(), U);
	if (! BadLance::basicMatch(state, move, front))
	  return -1;
	const int index = state.hasEffectAt(alt(move.player()), front);
	return index;
      }
    };

    struct PawnAttackGroup : public Group
    {
      PawnAttackGroup() : Group("PawnAttack")
      {
	for (int p=0; p<8; ++p)	// progress8
	  push_back(new PawnAttack());
      }
      void show(std::ostream& os, int name_width, const range_t& range, 
		const std::vector<double>& weights) const
      {
	showAll(os, name_width, range, weights);
      }
      int findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
      {
	if (! (*this)[0].match(state, move, env))
	  return -1;
	const int progress8 = env.progress.value()/2;
	return progress8;
      }
    };
  }
}

#endif /* _GROUP_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
