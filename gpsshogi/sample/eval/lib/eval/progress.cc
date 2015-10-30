#include "eval/progress.h"
#include "eval/pieceStand.h"
#include "eval/pin.h"
#include "eval/piecePair.h"
#include "eval/mobility.h"
#include "eval/piecePair.h"
#include "eval/kingEval.h"
#include "eval/minorPiece.h"
#include "eval/majorPiece.h"
#include "eval/ptypeAttacked.h"
#include "eval/pieceEvalComponent.h"

#include "osl/progress/effect5x3.h"
#include "osl/progress/effect5x3d.h"
#include "osl/progress.h"

#include <iomanip>
#include <fstream>
#include <iostream>

int gpsshogi::ProgressBonus::effectValue(
  const osl::NumEffectState &state) const
{
  osl::progress::Effect5x3WithBonus progress(state);
  osl::progress::Effect5x3d defense(state);

  return (progress.progress16bonus(osl::WHITE).value() * 2 - defense.progress16(osl::WHITE).value() - progress.progress16bonus(osl::BLACK).value() * 2 + defense.progress16(osl::BLACK).value()) / 2/* * progress.progress16().value() / 16*/;
}
int gpsshogi::ProgressBonus::eval(
  const osl::NumEffectState &state) const
{
  return value(0) * effectValue(state);
}

void gpsshogi::ProgressBonus::featuresNonUniq(
  const osl::NumEffectState &state, 
	      index_list_t&diffs,
	      int offset) const
{
  const int effect = effectValue(state);
  diffs.add(offset, effect);
}

void gpsshogi::ProgressBonus::showSummary(std::ostream &os) const
{
  os << "Progress Bonus " << value(0) << std::endl;
}


gpsshogi::EvalProgress::~EvalProgress()
{
}

int gpsshogi::ProgressBonus2::eval(
  const NumEffectState& /*state*/, const progress::ml::NewProgress& progress
  ) const
{
  const osl::Progress16 black = progress.progress16(BLACK), white = progress.progress16(WHITE);
  if (black.value() == white.value())
  {
    return 0;
  }
  else if (black.value() > white.value())
  {
    return value(index(black, white));
  }
  else
  {
    return -value(index(white, black));
  }
}

int gpsshogi::ProgressBonus2::eval(
  const osl::NumEffectState &state) const
{
  progress_t progress(state);
  return eval(state, progress);
}

void gpsshogi::ProgressBonus2::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t&diffs,
  int offset) const
{
  progress_t progress(state);
  return featuresNonUniq(state, progress, diffs, offset);
}

void gpsshogi::ProgressBonus2::featuresNonUniq(
  const osl::NumEffectState &, 
  const progress_t& progress,
  index_list_t&diffs,
  int offset) const
{
  const Progress16 black = progress.progress16(BLACK);
  const Progress16 white = progress.progress16(WHITE);
  if (black.value() == white.value())
  {
  }
  else if (black.value() > white.value())
  {
    diffs.add(index(black, white) + offset, 1);
  }
  else
  {
    diffs.add(index(white, black) + offset, -1);
  }
}

void gpsshogi::ProgressBonus2::showSummary(std::ostream &os) const
{
  os << name() << std::endl;
  for (int i = 0; i < 16; ++i)
  {
    for (int j = 0; j < 16; ++j)
    {
      os << value(i * 16 + j) << " ";
    }
    os << std::endl;
  }
}


int gpsshogi::ProgressBonusAttackDefense::eval(
  const osl::NumEffectState &state) const
{
  progress_t progress(state);
  return eval(state, progress);
}

int gpsshogi::ProgressBonusAttackDefense::eval(
  const NumEffectState&, const progress::ml::NewProgress& progress
  ) const
{
  return value(index(progress.progressAttack(BLACK), progress.progressDefense(WHITE))) -
    value(index(progress.progressAttack(WHITE), progress.progressDefense(BLACK)));
}

void gpsshogi::ProgressBonusAttackDefense::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  progress_t progress(state);
  return featuresNonUniq(state, progress, diffs, offset);
}

void gpsshogi::ProgressBonusAttackDefense::featuresNonUniq(
  const osl::NumEffectState &, 
  const progress_t& progress,
  index_list_t&diffs,
  int offset) const
{
  const Progress16 black_attack = progress.progressAttack(BLACK);
  const Progress16 white_attack = progress.progressAttack(WHITE);
  const Progress16 black_defense = progress.progressDefense(BLACK);
  const Progress16 white_defense = progress.progressDefense(WHITE);
  if (black_attack == white_attack && black_defense == white_defense)
  {
    return;
  }
  else
  {
    diffs.add(index(black_attack, white_defense) + offset, 1);
    diffs.add(index(white_attack, black_defense) + offset, -1);
  }
}


int gpsshogi::ProgressBonusAttackDefenseAll::eval(
  const osl::NumEffectState &state) const
{
  progress_t progress(state);
  return eval(state, progress);
}

int gpsshogi::ProgressBonusAttackDefenseAll::eval(
  const NumEffectState&, const progress::ml::NewProgress& progress
  ) const
{
  Progress16 black_attack = progress.progressAttack(BLACK);
  Progress16 white_defense = progress.progressDefense(WHITE);
  Progress16 white_attack = progress.progressAttack(WHITE);
  Progress16 black_defense = progress.progressDefense(BLACK);
  if (black_attack.value() == white_attack.value() &&
      black_defense.value() == black_defense.value())
  {
    return 0;
  }
  else if ((black_attack.value() + white_defense.value() >
	    white_attack.value() + black_defense.value()) ||
	   (black_attack.value() + white_defense.value() ==
	    white_attack.value() + black_defense.value() &&
	    black_attack.value() > white_attack.value()))
  {
    return value(index(black_attack, white_defense,
		       white_attack, black_defense));
  }
  else
  {
    return -value(index(white_attack, black_defense,
			black_attack, white_defense));
  }
}

void gpsshogi::ProgressBonusAttackDefenseAll::featuresNonUniq(
  const osl::NumEffectState &state, 
  index_list_t &diffs,
  int offset) const
{
  progress_t progress(state);
  return featuresNonUniq(state, progress, diffs, offset);
}

void gpsshogi::ProgressBonusAttackDefenseAll::featuresNonUniq(
  const osl::NumEffectState &, 
  const progress_t& progress,
  index_list_t&diffs,
  int offset) const
{
  const Progress16 black_attack = progress.progressAttack(BLACK);
  const Progress16 white_attack = progress.progressAttack(WHITE);
  const Progress16 black_defense = progress.progressDefense(BLACK);
  const Progress16 white_defense = progress.progressDefense(WHITE);
  if (black_attack.value() == white_attack.value() &&
      black_defense.value() == black_defense.value())
  {
    return;
  }
  else if ((black_attack.value() + white_defense.value() >
	    white_attack.value() + black_defense.value()) ||
	   (black_attack.value() + white_defense.value() ==
	    white_attack.value() + black_defense.value() &&
	    black_attack.value() > white_attack.value()))
  {
    const int i = index(black_attack, white_defense,
			white_attack, black_defense);
    diffs.add(i + offset, 1);
  }
  else
  {
    const int i = index(white_attack, black_defense,
			black_attack, white_defense);
    diffs.add(i + offset, -1);
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
