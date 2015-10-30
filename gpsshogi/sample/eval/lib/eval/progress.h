#ifndef _PROGRESS_H
#define _PROGRESS_H

#include "eval/pieceFeature.h"
#include "osl/numEffectState.h"
#include "osl/progress.h"
#include <vector>

namespace gpsshogi
{
  class ProgressBonus : public EvalComponent
  {
  private:
    int effectValue(const osl::NumEffectState &state) const;
  public:
    ProgressBonus() : EvalComponent(1) {
    }
    int eval(const osl::NumEffectState &state) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 index_list_t&diffs,
			 int offset) const;
    void showSummary(std::ostream &os) const;
    const std::string name() const { return "ProgressBonus"; };
  };

  class EvalProgress 
  {
  public:
    typedef const osl::progress::ml::NewProgress progress_t;
    virtual ~EvalProgress();
    virtual int eval(const osl::NumEffectState&, const osl::progress::ml::NewProgress&) const=0;
    virtual void featuresNonUniq(const osl::NumEffectState &state, 
				 const progress_t& progress,
				 index_list_t &diffs,
				 int offset) const=0;
  };
  

  class ProgressBonus2 : public EvalComponent, public EvalProgress
  {
  private:
    int index(Progress16 black, Progress16 white) const
    {
      return black.value() * 16 + white.value();
    }
  public:
    ProgressBonus2() : EvalComponent(256) { }
    int eval(const osl::NumEffectState &state) const;
    int eval(const osl::NumEffectState&, const progress_t&) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
		  index_list_t &diffs,
		  int offset) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 const progress_t& progress,
			 index_list_t &diffs,
			 int offset) const;
    void showSummary(std::ostream &os) const;
    const std::string name() const { return "ProgressBonus2"; };
    size_t maxActive() const { return 1; }
  };

  class ProgressBonusAttackDefense : public EvalComponent, public EvalProgress
  {
  private:
    int index(Progress16 attack, Progress16 defense) const
    {
      return attack.value() * 16 + defense.value();
    }
  public:
    ProgressBonusAttackDefense() : EvalComponent(256) { }
    int eval(const osl::NumEffectState &state) const;
    int eval(const osl::NumEffectState&, const progress_t&) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 index_list_t&diffs,
			 int offset) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 const progress_t& progress,
			 index_list_t &diffs,
			 int offset) const;
    const std::string name() const { return "ProgressBonusAttackDefense"; };
    size_t maxActive() const { return 2; }
  };

  class ProgressBonusAttackDefenseAll : public EvalComponent, public EvalProgress
  {
  private:
    int index(Progress16 black_attack, Progress16 white_defense,
	      Progress16 white_attack, Progress16 black_defense) const
    {
      return white_attack.value() + 16 * (black_defense.value() +
					  16 * (black_attack.value() * 16 + white_defense.value()));
    }
  public:
    ProgressBonusAttackDefenseAll() : EvalComponent(65536) { }
    int eval(const osl::NumEffectState &state) const;
    int eval(const osl::NumEffectState&, const progress_t&) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 index_list_t&diffs,
			 int offset) const;
    void featuresNonUniq(const osl::NumEffectState &state, 
			 const progress_t& progress,
			 index_list_t &diffs,
			 int offset) const;
    const std::string name() const { return "ProgressBonusAttackDefenseAll"; };
    size_t maxActive() const { return 1; }
  };
}

#endif // _PROGRESS_H
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
