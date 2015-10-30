#include "board.h"
#include "osl/numEffectState.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/progress/effect5x3.h"
#include "osl/csa.h"
#include "osl/usi.h"
#include "osl/threatmate/richPredictor.h"
#include "osl/misc/eucToLang.h"
#include <iostream>

gpsshell::
Board::Board()
  : initial((osl::SimpleState(osl::HIRATE))),
    current_move(0),
    black_color(osl::record::Color::NONE),
    white_color(osl::record::Color::NONE),
    last_color(osl::record::Color::NONE)
{
  static const char *env_term = getenv("TERM");
  if (env_term && std::string(env_term) != "dumb") {
    // setBlackColor(osl::record::Color::Green);
    setWhiteColor(osl::record::Color::Blue);
    setLastMoveColor(osl::record::Color::Red);
  }
}

gpsshell::
Board::~Board()
{
}

bool gpsshell::
Board::next(size_t n)
{
  while (n--)
  {
    if (current_move >= moves.size())
    {
      std::cout << "false" << std::endl;
      return false;
    }
    else
      current_move++;
  }
  return true;
}

bool gpsshell::
Board::prev(size_t n)
{
  while (n--)
  {
    if (current_move <= 0)
    {
      return false;
    }
    else
      current_move--;
  }
  return true;
}

void gpsshell::
Board::first()
{
  while(prev())
  {}
}

void gpsshell::
Board::last()
{
  while(next())
  {}
}

void gpsshell::
Board::move(const osl::Move move)
{
  assert(current_move == moves.size());
  moves.push_back(move);
  next();
  return;
}

void gpsshell::
Board::shrink()
{
  moves.resize(current_move);
  time.resize(current_move);
  search_info.resize(current_move);
}

osl::NumEffectState gpsshell::
Board::getState() const
{
  osl::NumEffectState state = initial;
  for (size_t i=0; i < current_move; i++)
  {
     osl::Move move = moves[i];
     state.makeMove(move);
  }
  return state;
}

void gpsshell::
Board::showHistory() const
{
  for (size_t i=0; i<current_move; ++i) {
    std::cout << " " << osl::csa::show(moves[i]);
  }
  std::cout << std::endl;
}

void gpsshell::
Board::showUsiHistory() const
{
  std::cout << "position "
	    << osl::usi::show(initial)
	    << " moves";
  for (size_t i=0; i<current_move; ++i) {
    std::cout << " " << osl::usi::show(moves[i]);
  }
  std::cout << std::endl;
}

void gpsshell::
Board::showState() const
{
  const osl::NumEffectState state = getState();
  const osl::Move move = getCurrentMove();
  const std::shared_ptr<osl::record::KIFCharacters> characters(new osl::record::KIFCharacters());
  osl::record::KanjiPrint printer(std::cout, characters);
  printer.setBlackColor(black_color);
  printer.setWhiteColor(white_color);
  printer.setLastMoveColor(last_color);
  printer.print(state, &move);

  if (move.isNormal())
    std::cout << osl::csa::show(move) << std::endl;
  if (current_move > 0) {
    const size_t id = current_move-1;
    if (id < time.size())
      std::cout << "T " << time[id] << std::endl;
    if (id < search_info.size()) {
      std::cout << "eval " << search_info[id].value;
      for (size_t i=0; i<search_info[id].moves.size(); ++i) {
	std::cout << ' ' << osl::csa::show(search_info[id].moves[i]);
      }
      std::cout << std::endl;
    }
    if (id < comments.size())
      std::cout << osl::misc::eucToLang(comments[id]) << std::endl;
  }
}

void gpsshell::
Board::showEval(const std::string& name) const
{
  osl::NumEffectState state = getState();
  if (name == "test") 
  {
    osl::progress::ml::NewProgress progress(state);
    osl::eval::ml::OpenMidEndingEval eval(state);
    std::cout << osl::misc::eucToLang("É¾²ÁÃÍ: ")
	      << eval.value() << " (centipawn "
	      << round(100.0*eval.value()/(eval.captureValue(newPtypeO(osl::WHITE,osl::PAWN))/2))
	      << ")" << " open "
	      << eval.openingValue()*progress.maxProgress()/3 << " mid1 "
	      << eval.midgameValue()*progress.maxProgress()/3 << " mid2 "
	      << eval.midgame2Value()*progress.maxProgress()/3 << " endg "
	      << eval.endgameValue()*progress.maxProgress()/3 << " inde "
	      << eval.progressIndependentValue()*progress.maxProgress()/3
	      << std::endl;
    std::cout << osl::misc::eucToLang("¿Ê¹ÔÅÙ: ")
	      << progress.progress() << " / "
	      << progress.maxProgress() << " "
	      << progress.progress16().value() << " / 16, "
	      << progress.progress16(osl::BLACK).value() << " "
	      << progress.progress16(osl::WHITE).value() << " "
	      << progress.progressAttack(osl::BLACK).value() << " "
	      << progress.progressAttack(osl::WHITE).value() << " "
	      << progress.progressDefense(osl::BLACK).value() << " "
	      << progress.progressDefense(osl::WHITE).value() << " "
	      << std::endl;
    osl::progress::ml::NewProgressData rp = progress.rawData();
    for (int i=0; i<2; ++i)
      std::cout << rp.progresses[i] << " " << rp.attack5x5_progresses[i] << " "
		<< rp.stand_progresses[i] << " " << rp.effect_progresses[i] << " "
		<< rp.defenses[i] << " " << rp.king_relative_attack[i]
		<< " " << rp.king_relative_defense[i] << "\n";
  }
  else 
  {
    if (name != "progress")
      std::cerr << name << " is not supported. use progress instead\n";    
    osl::eval::ProgressEval eval(state);
    std::cout << osl::misc::eucToLang("É¾²ÁÃÍ: ")
	      << eval.value() << " ("
	      << eval.openingValue()*16 << " "
	      << eval.endgameValue()*16 << " "
	      << eval.attackDefenseBonus() << " "
	      << eval.minorPieceValue() << ")"
	      << std::endl;
    osl::progress::Effect5x3 progress(state);
    std::cout << osl::misc::eucToLang("¿Ê¹ÔÅÙ: ")
	      << progress.progress16().value() << " "
	      << progress.progress16(osl::BLACK).value() << " "
	      << progress.progress16(osl::WHITE).value() << " "
	      << std::endl;
  }
  if (current_move > 0 && current_move-1 < moves.size()) {
    osl::threatmate::RichPredictor predictor;
    std::cout << osl::misc::eucToLang("µÍ¤á¤í³ÎÎ¨: ")
	      << predictor.predict(state, moves[current_move-1])
	      << std::endl;
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
