#include "book.h"
#include "osl/record/kanjiPrint.h"
#include "osl/csa.h"
#ifndef MINIMAL_GPSSHELL
#  include <gsl_cdf.h>
#endif
#include <boost/format.hpp>
#include <cmath>
#include <iostream>

int gpsshell::
Book::getStateIndex(const osl::SimpleState& state, 
                    const std::vector<osl::Move>& moves) const
{
  int state_index = the_book->stateIndex(moves);
  if (state_index < 0)
    state_index = the_book->stateIndex(state); // it could take a long time
  return state_index;
}

void gpsshell::
Book::showState(const osl::SimpleState& state, 
                const std::vector<osl::Move>& moves,
                int max_count) const
{
  osl::record::KanjiPrint printer(std::cerr, 
                                  std::shared_ptr<osl::record::Characters>(
                                            new osl::record::KIFCharacters())
                                  );

  const int state_index = getStateIndex(state, moves);
  if (state_index < 0)
  {
    std::cout << "The current position not found in the book" << std::endl;
    return;
  }

  const osl::Player turn   = state.turn();
  const int black_win = the_book->blackWinCount(state_index);
  const int white_win = the_book->whiteWinCount(state_index);
  const int trial     = black_win + white_win;
  const int nwins     = (turn == osl::BLACK ? black_win : white_win); 
  double win_rate     = 0.0;
  double deviation    = 0.0;
  double cd           = 0.0; // cumulative distribution
  if (trial > 0)
  {
    win_rate = 1.0 * nwins / trial;
#ifndef MINIMAL_GPSSHELL
    cd = gsl_cdf_binomial_P(nwins, 0.5, trial);
#else
    cd = 0;
#endif
    deviation = sqrt( win_rate * (1-win_rate) / trial);
  }

  std::cout << boost::format("State_index: % 7d\n")   % state_index
            << boost::format("Black win:   % 7d\n")   % black_win
            << boost::format("White win:   % 7d\n")   % white_win
            <<               "Binomial:\n" 
            << boost::format("  E(X) [win rate] : % 7.4f\n") % win_rate
            << boost::format("  D(X):             % 7.4f\n") % deviation
            << boost::format("  cumulative:       % 7.4f\n") % cd;

  std::cout << "\n";
  printer.print(state);
  std::cout << "\n";

  gpsshell::WMoveStatsContainer wstats = getWMoveStats(state, moves);
  std::cout << boost::format("found %d moves\n") % wstats.size();

  for (const gpsshell::WMoveStats& st: wstats)
  {
    if (--max_count < 0) break;

    std::cout << boost::format("[% 6d] %s  win:% 7d loss:% 7d  rate:% 7.4f - % 7.4f = % 7.4f\n") 
                                % st.getWmove().weight 
                                % osl::csa::show(st.getWmove().move)
                                % st.nwins()
                                % st.nlosses()
                                % st.winRate()
                                % st.deviation()
                                % (st.winRate() - st.deviation());
  }
}

gpsshell::WMoveContainer gpsshell::
Book::getMoves(const osl::SimpleState& state, 
               const std::vector<osl::Move>& moves) const
{
  const gpsshell::WMoveStatsContainer wstats
    = getWMoveStats(state, moves);
  
  gpsshell::WMoveContainer wmoves;

  if (wstats.empty())
    return wmoves; // empty
  
  for (const gpsshell::WMoveStats& ws: wstats)
  {
    wmoves.push_back(ws.getWmove());
  }

  return wmoves;
}

gpsshell::WMoveStatsContainer gpsshell::
Book::getWMoveStats(const osl::SimpleState& state, 
                    const std::vector<osl::Move>& moves) const
{
  const int state_index = getStateIndex(state, moves);
  gpsshell::WMoveStatsContainer wstats;

  if (state_index < 0)
    return wstats; // empty

  {
    WMoveContainer wmoves = the_book->moves(state_index);
    wstats.reserve(wmoves.size());
    for (const auto& wmove: wmoves)
    {
      const int next_state_index = wmove.stateIndex();
      const int black_win        = the_book->blackWinCount(next_state_index);
      const int white_win        = the_book->whiteWinCount(next_state_index);
      gpsshell::WMoveStats st(wmove, black_win, white_win);
      wstats.push_back(st);
    }
  }

  /*
   * Sort:
   *   weight such as wegiht > 0
   *   #wins such as weight == 0
   */
  std::sort(wstats.begin(), wstats.end(), gpsshell::WMoveStatsSortByWeight());
  gpsshell::WMoveStatsContainer::iterator it = wstats.begin();
  for (/* none */; it != wstats.end(); ++it)
  {
    if (it->getWmove().weight < 1)
      break;
  }
  std::sort(it, wstats.end(), gpsshell::WMoveStatsSortByWins());
  return wstats;
}

// ==============================================
// Utility functions
// ==============================================

int gpsshell::getMaxWeight(const gpsshell::WMoveContainer& moves)
{
  int max_weight = 0;
  for (const osl::book::WMove& wmove: moves) {
    if (max_weight < wmove.weight) {
      max_weight = wmove.weight;
    }
  }
  return max_weight;
}

int gpsshell::getSumOfWeights(const gpsshell::WMoveContainer& moves)
{
  int sum = 0;
  for (const osl::book::WMove& wmove: moves) {
    sum += wmove.weight;
  }
  return sum;
}

struct WeightLessThan
{
  int criteria;

  explicit WeightLessThan(const int criteria)
    : criteria(criteria)
  {}

  bool operator()(const osl::book::WMove& wmove) const
  {
    return (wmove.weight < criteria ? true : false);
  }
};

void gpsshell::deleteLessWeightedMoves(gpsshell::WMoveContainer& wmoves,
                                            const int criteria)
{
  wmoves.erase(std::remove_if(wmoves.begin(), wmoves.end(), WeightLessThan(criteria)), 
               wmoves.end());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
