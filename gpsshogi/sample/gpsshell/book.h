#ifndef _GPS_GPSSHELL_BOOK_H
#define _GPS_GPSSHELL_BOOK_H

#include "osl/numEffectState.h"
#include "osl/book/openingBook.h"
#include <memory>
#include <string>
#include <cmath>

  namespace gpsshell
  {
    class WMoveStats;

    typedef std::vector<osl::book::WMove> WMoveContainer;
    typedef std::vector<WMoveStats> WMoveStatsContainer;
    
    class Book
    {
      std::shared_ptr<osl::book::WeightedBook> the_book;
    public:
      Book(const std::string& filename)
      {
        the_book.reset(new osl::book::WeightedBook(filename.c_str()));
      }
      /**
       * Find a state index for a state. The moves from the initial position
       * to the state could reduce the time to find out.
       *
       * @param state a target state
       * @param moves moves from the initial position to the state.
       * @return state index; if it is not found, return -1.
       */
      int getStateIndex(const osl::SimpleState& state, 
                        const std::vector<osl::Move>& moves) const;
      /**
       * Show a state in the book.
       *
       * @param state a state to show
       * @param moves moves from the initial state to the state
       * @param max_count a max count of showing wmoves from the state to
       *        next states. 
       */
      void showState(const osl::SimpleState& state, 
                     const std::vector<osl::Move>& moves,
                     int max_count) const;
      /**
       * Return wmoves in the book from a state. The wmoves are sorted by a
       * weight (desc).
       *
       * @param state a target state
       * @param moves moves from the initial position to the state
       */
      WMoveContainer getMoves(const osl::SimpleState& state, 
                              const std::vector<osl::Move>& moves) const;
      /**
       * Return an array of WMoveStats in the book from a state. The items
       * are sorted by weight and wins (desc).
       *
       * @param state a target state
       * @param moves moves from the initial position to the state
       */
      WMoveStatsContainer getWMoveStats(const osl::SimpleState& state, 
                                        const std::vector<osl::Move>& moves) const;
      /**
       * Return a WeightedBook object.
       */
      const std::shared_ptr<osl::book::WeightedBook> getWeightedBook() const
      {
        return the_book;
      }
    };


    class WMoveStats
    {
      osl::book::WMove wmove;
      int black_wins;
      int white_wins;    

    public:
      WMoveStats(const osl::book::WMove& wmove,
                 const int black_wins,
                 const int white_wins)
        : wmove(wmove),
          black_wins(black_wins),
          white_wins(white_wins)
      {} 

      const osl::book::WMove& getWmove() const { return wmove; } 
      int blackWins() const { return black_wins; }
      int whiteWins() const { return white_wins; }
      int trials() const { return black_wins + white_wins; }
      double nwins() const
      {
        return (wmove.move.player() == osl::BLACK ? black_wins : white_wins);
      }
      double nlosses() const
      {
        return (wmove.move.player() == osl::BLACK ? white_wins : black_wins);
      }
      double winRate() const
      {
        return 1.0 * nwins() / trials();
      }
      double deviation() const
      {
        const double win_rate = winRate();
        return sqrt( win_rate * (1.0-win_rate) / trials());
      }
    };

    struct WMoveStatsSortByWeight : public std::binary_function<WMoveStats, WMoveStats, bool>
    {
      bool operator()(const WMoveStats& l, const WMoveStats& r) const 
      {
        return l.getWmove().weight > r.getWmove().weight;
      }
    };

    struct WMoveStatsSortByWins : public std::binary_function<WMoveStats, WMoveStats, bool>
    {
      bool operator()(const WMoveStats& l, const WMoveStats& r) const 
      {
        return l.nwins() > r.nwins();
      }
    };

    enum VisitFlag 
    {
      UNVISITED=0, 
      ENTERED, 
      VISITED
    };

    struct VisitState
    {
      int state_index;
      std::vector<osl::Move> moves;
      
      explicit VisitState(const int state_index) 
        : state_index(state_index)
      {}
    }; 

    // ==============================================
    // Utility functions
    // ==============================================

    int getMaxWeight(const WMoveContainer& moves);
    int getSumOfWeights(const WMoveContainer& moves);
    void deleteLessWeightedMoves(WMoveContainer& wmoves, const int criteria);
 
  } // namespace gpsshell

#endif /* _GPS_GPSSHELL_BOOK_H */

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
