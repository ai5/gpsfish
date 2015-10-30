#ifndef GAME_PLAYING_SEARCHPLAYER_H
#define GAME_PLAYING_SEARCHPLAYER_H

#include "osl/game_playing/computerPlayer.h"
#include "osl/search/searchTimer.h"
#include "osl/misc/milliSeconds.h"

namespace osl
{
  namespace misc
  {
    class RealTime;
  }
  namespace search
  {
    class CountRecorder;
    class SimpleHashTable;
    struct TimeAssigned;
    class SearchMonitor;
  }
  namespace checkmate
  {
    class DualDfpn;
  }
  namespace game_playing
  {
    struct Config;
    bool operator==(const Config& l, const Config& r);
    struct PVHistory;
    /**
     * MtdfPlayer と AlphaBetaPlayer の共通部分
     */
    class SearchPlayer
      : public ComputerPlayer,
	public ComputerPlayerSelectBestMoveInTime
    {
    public:
      struct NtesukiThread;
      struct Config
      {
	int limit;
	size_t node_limit;
	size_t table_size;
	int table_record_limit;
	int initial_limit;
	int deepening_step;
	size_t total_checkmate_limit;
	int verbose;
	/** SearchBase::next_iteration_coefficient に設定するもの */
	double next_iteration_coefficient;
	/** 千日手に対するボーナス/ペナルティの歩の相対値. -2なら1歩損しても避ける */
	double draw_coef;
	bool save_pv;
	uint64_t node_count_hard_limit;
	/** 最善手以外も探索する幅 */
	int multi_pv_width;
	std::vector<std::shared_ptr<search::SearchMonitor> > monitors;

	Config();
	friend bool operator==(const Config& l, const Config& r);
      };
    protected:
      Config config;
      std::shared_ptr<search::SimpleHashTable> table_ptr;
      std::shared_ptr<checkmate::DualDfpn> checkmate_ptr;
      std::unique_ptr<search::CountRecorder> recorder_ptr;
      volatile bool searching;
      std::unique_ptr<search::SearchTimer> searcher;
      /** 探索に入る前に止める */
      volatile bool plan_stop;
      const MoveVector *root_ignore_moves; // acquaintance
      bool prediction_for_speculative_search;
      std::unique_ptr<PVHistory> pv_history;
      int almost_resign_count;
    public:
      SearchPlayer();
      SearchPlayer(const SearchPlayer&);
      ~SearchPlayer();

      void setDepthLimit(int limit, int initial_limit, int deepening_step);
      void setNodeLimit(size_t node_limit);
      void setNodeCountHardLimit(size_t node_limit);
      void setTableLimit(size_t size, int record_limit);
      void setVerbose(int verbose=1);
      void setDrawCoef(double new_value) { config.draw_coef = new_value; }
      void setNextIterationCoefficient(double new_value);
      double nextIterationCoefficient() const 
      {
	return config.next_iteration_coefficient;
      }
      void enableSavePV(bool enable=true) { config.save_pv = enable; }
      void enableMultiPV(int width) { config.multi_pv_width = width; }
      void addMonitor(const std::shared_ptr<search::SearchMonitor>&);

      /** 所有権移転 */
      void resetRecorder(search::CountRecorder *new_recorder);

      void pushMove(Move m);
      void popMove();

      /**
       * other の局面表と取り替える
       */
      void swapTable(SearchPlayer& other);

      const search::SimpleHashTable* table() const { return table_ptr.get(); }
      const search::CountRecorder& recorder() const { return *recorder_ptr; }

      bool stopSearchNow();
      bool canStopSearch();	// 呼出を省略されないよう念の為 const でなくした
      /**
       * searchWithSecondsForThisMove を呼び出す
       */
      const MoveWithComment selectBestMove(const GameState&, int limit, int elapsed,
                                           int byoyomi);
      const MoveWithComment selectBestMoveInTime(const GameState&, const search::TimeAssigned&);
      static const search::TimeAssigned assignTime(const GameState& state, int limit, int elapsed,
						   int byoyomi, int verbose);
      const search::TimeAssigned assignTime(const GameState& state, int limit, int elapsed,
					    int byoyomi) const;
      void saveSearchResult(const GameState&, const MoveWithComment&);
    protected:
      template <class Searcher>
      ComputerPlayer* cloneIt(const Searcher&) const;
      /** @return time consumed in milliseconds */
      const milliseconds setUpTable(const GameState&, int pawn_value);
      template <class Searcher>
      const MoveWithComment search(const GameState&, const search::TimeAssigned&);
      template <class Searcher>
      bool isReasonableMoveBySearch(Searcher&, Move move, int pawn_sacrifice);
      template <class Searcher>
      static int pawnValue();
      template <class Searcher>
      static int pawnValueOfTurn(Player turn);
      const search::TimeAssigned adjust(const search::TimeAssigned& org, const milliseconds& elapsed);
    public:
      virtual const MoveWithComment searchWithSecondsForThisMove(const GameState&, const search::TimeAssigned&)=0;
      void setRootIgnoreMoves(const MoveVector *rim, bool prediction) 
      {
	root_ignore_moves = rim; 
	prediction_for_speculative_search = prediction;
      }

      const Config& getConfig() const { return config; }

      static int secondsForThisMove(const GameState& state,
				    int limit, int elapsed, int byoyomi, int verboseness);
      int secondsForThisMove(const GameState& state, int limit, int elapsed, int byoyomi) const;

      void setTimeAssign(const search::TimeAssigned& new_assign);
      const time_point startTime() const;
    };

  } // namespace game_playing
} // namespace osl

#endif /* GAME_PLAYING_SEARCHPLAYER_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
