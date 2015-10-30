#include "threat_search.h"
#include "logging.h"
#include <osl/csa.h>
#include <osl/checkmate/dfpnParallel.h>
#include <osl/hashKey.h>
#include <osl/numEffectState.h>
#include <osl/oslConfig.h>
#include <osl/pathEncoding.h>
#include <boost/format.hpp>
#include <boost/chrono.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <iomanip>
#include <iostream>

void rc::
showMoves(const std::vector<osl::Move>& moves)
{
  for (size_t i=0; i<moves.size(); ++i) {
    std::cerr << std::setw(4) << std::setfill(' ') << i+1
      << ' ' << osl::csa::show(moves[i]) << " ";
    if (i % 6 == 5)
      std::cerr << "\n";
  }
  if (moves.size() % 6 != 0)
    std::cerr << "\n";
}

rc::
ThreatSearch::ThreatSearch(int ignore_initial_moves_)
  : ignore_initial_moves(ignore_initial_moves_)
{ }

void rc::
ThreatSearch::setup()
{
  osl::OslConfig::setUp();
}

size_t rc::
ThreatSearch::isCheckmate(const osl::NumEffectState& state,
                          std::vector<osl::Move>& pv) const
{
  const static std::string method = "isCheckmate";

  osl::checkmate::DfpnParallel dfpn(1);

  boost::packaged_task<size_t> task([&](){
    osl::checkmate::DfpnTable table(state.turn());
    table.setGrowthLimit(TABLE_GROWTH_LIMIT);
    dfpn.setTable(&table);
    const osl::PathEncoding path(state.turn());
    osl::Move checkmate_move;
    osl::checkmate::ProofDisproof result =
      dfpn.hasCheckmateMove(state, osl::HashKey(state), path, LIMIT, checkmate_move, osl::Move(), &pv);

    if (result.isCheckmateSuccess()) {
      return dfpn.nodeCount();
    } else {
      return 0UL;
    }
  });

  // Boot 1.55 boost::future
  boost::unique_future<size_t> future = task.get_future();
  boost::thread t(boost::move(task));
  // Boost 1.55 t.join_for
  if (t.timed_join(boost::posix_time::seconds(CHECKMATE_TIMELIMIT_SECONDS))) {
    const size_t nodes = future.get();
    logInfo(method, boost::format("checkmate nodes: %d") % nodes);
    return nodes;
  } else {
    logInfo(method, "checkmate timed out");
    dfpn.stopNow();
    t.join();
    return 0;
  }
}

size_t rc::
ThreatSearch::isThreatmate(const osl::NumEffectState& _state,
                           std::vector<osl::Move>& pv) const
{
  osl::NumEffectState state(_state);
  state.changeTurn();
  return isCheckmate(state, pv);
}

rc::ThreatSearchResult rc::
ThreatSearch::search(const std::vector<osl::Move>& moves)
{
  ThreatSearchResult ret;

  osl::NumEffectState state;
  bool prev_result = false;
  for (int i=0; i<moves.size(); ++i) {
    const osl::Move move = moves[i];
    if (!(move.isNormal() && state.isValidMove(move))) {
      break;
    }

    state.makeMove(move);
    if (i < ignore_initial_moves) {
      continue;
    }

    if (state.inCheck()) {
      prev_result = false;
      continue;
    }

    std::vector<osl::Move> dummy1;
    ret.checkmate_limit = isCheckmate(state, dummy1); 
    if (ret.checkmate_limit > 0) {
      ret.checkmate_nth = i+1;
      break;
    }

    std::vector<osl::Move> dummy2;
    const int node_count = isThreatmate(state, dummy2);
    // Easy threatmantes are not counted in.
    const bool current_result = node_count > 100;

    if (current_result && prev_result &&
        move.to() != moves[i-1].to()) {
      // Found!!!
      ret.moves.push_back(i+1);
    }

    prev_result = current_result;
  } // for move

  return ret;
}

