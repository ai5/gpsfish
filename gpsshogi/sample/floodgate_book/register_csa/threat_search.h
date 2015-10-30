#ifndef _REGISTER_CSA_THREAT_SEARCH_H
#define _REGISTER_CSA_THREAT_SEARCH_H

#include <osl/basic_type.h>
#include <boost/optional.hpp>
#include <vector>

namespace osl
{
  class NumEffectState;
}

namespace rc
{
  struct ThreatSearchResult
  {
    /** 0 if there is no checkmate; a positive value to find a checkmate */
    size_t checkmate_limit;
    /** */
    int checkmate_nth;
    /** n-th moves for threat-escaping-threat, starting with 1. */
    std::vector<int> moves;

    ThreatSearchResult()
      : checkmate_limit(0), checkmate_nth(0)
    {}
  };

  class ThreatSearch
  {
    const static int LIMIT = 1200000;
    const static size_t TABLE_GROWTH_LIMIT = 8000000;
    const static int CHECKMATE_TIMELIMIT_SECONDS = 20;

    /**
     * Skip the first ignore_initial_moves moves since it is very less
     * likely that there are threats in the initial part of games.
     */
    int ignore_initial_moves;
  public:
    explicit ThreatSearch(int ignore_initial_moves_ = 70);
    void setup();
    /**
     * Search for threat-escaping-threat (Tsumero-nogre-no-tsumero in
     * Japanese) in moves of a game and returns n-th moves,
     * starting with 1.
     */
    ThreatSearchResult search(const std::vector<osl::Move>& moves);

    /**
     * Returns true if a player in turn is loosing game.
     */
    size_t isCheckmate(const osl::NumEffectState& state, std::vector<osl::Move>& pv) const;

  private:

    /**
     * Returns 0 unless the last move is a threatmate; otherwise a positive
     * integer which was required to find it.
     */
    size_t isThreatmate(const osl::NumEffectState& state, std::vector<osl::Move>& pv) const;
  };

  void showMoves(const std::vector<osl::Move>& moves);
}

#endif /* _REGISTER_CSA_THREAT_SEARCH_H */
