/* pvGenerator.h
 */
#ifndef GPSSHOGI_EVAL_PVGENERATOR_H
#define GPSSHOGI_EVAL_PVGENERATOR_H

#include "osl/numEffectState.h"
#include "osl/progress.h"
#include "osl/stat/average.h"
#include <tuple>
#include <memory>
#include <string>
#include <vector>
#include <iosfwd>
namespace gpsshogi
{
  class PVFileWriter;
  class PVVector;
  class Quiesce;
  class Eval;
  /** run in thread */
  class KisenAnalyzer
  {
  public:
    struct RecordConfig
    {
      size_t first, last;
      size_t thread_id;
      std::string kisen_filename;
      bool allow_skip_in_cross_validation;
      std::vector<std::string> csa_filenames;

      explicit RecordConfig(size_t thread_id=0, 
			    size_t first=0, size_t last=0, const std::string& kisen="",
			    bool allow_skip_in_cross_validation=true);
    };
    struct OtherConfig
    {
      /** search window relative to pawn value */
      double window_by_pawn, sigmoid_alpha;
      int max_progress, position_randomness;
      size_t min_rating;
      Eval *my_eval;
    };
    struct Result
    {
      unsigned long long node_count, siblings;
      int record_processed, skip_by_rating;
      osl::stat::Average werrors, order_lb, order_ub, toprated, toprated_strict;
      /** validation による棋譜並べ替えのためのみのデータ */
      std::vector<std::tuple<int,double> > all_errors;
      double last_error;
      Result() : node_count(0), skip_by_rating(0)
      {
      }
    };
  protected:
    RecordConfig record;
    OtherConfig config;
    Result *result;
    static int normal_depth, quiesce_depth;
    static int use_percent, window_asymmetry;
    static bool compare_pass;
    static bool bonanza_compatible_search;
    static std::vector<int> priority_record;
  public:
    KisenAnalyzer(const RecordConfig&, const OtherConfig& , Result *result);
    virtual ~KisenAnalyzer();
    void operator()();

    /**
     * n 並列にfiles を探索する場合に棋譜の数を均等に分ける 
     * @param start 最初のstart棋譜をスキップする。(files全て通しての数)
     * @param num_records 扱う合計の棋譜
     * @param min_rating 0 なら全て使う
     */
    static void distributeJob(size_t n, RecordConfig *out, size_t start, size_t num_records,
			      const std::vector<std::string>& files, size_t min_rating,
                              const std::vector<std::string>& csa_files);
    static void setPriorityRecord(const std::vector<int>& record) 
    {
      priority_record = record;
      std::sort(priority_record.begin(), priority_record.end());
    }
  protected:
    static void splitFile(const std::string& file, size_t first, size_t last,
			  int num_assignment, double average_records, 
			  RecordConfig *out, int& written, bool verbose);
    static void splitFileWithMoves(const std::string& file, size_t first, size_t last,
				   int num_assignment, double average_records,
                                   const std::vector<std::string>& csa_files,
				   RecordConfig *out, int& written, bool verbose);
    virtual void init();
    virtual void forEachPosition(int record_id, int position_id, size_t position_size,
				 Quiesce& quiesce, const osl::NumEffectState& state, 
				 osl::Progress16, osl::Move best_move)=0;
    virtual bool isCrossValidation() const;
  };

  /** run in thread */
  class Validator : public KisenAnalyzer
  {
  public:
    Validator(const RecordConfig&, const OtherConfig& , Result *result);
    ~Validator();  

  protected:
    void forEachPosition(int record_id, int position_id, size_t position_size,
			 Quiesce& quiesce, const osl::NumEffectState& state, 
			 osl::Progress16, osl::Move best_move);
    bool isCrossValidation() const;
  };

  /** run in thread */
  class PVGenerator : public KisenAnalyzer
  {
    std::string pv_filename, stat_filename;
    std::shared_ptr<PVFileWriter> pw;
  public:
    PVGenerator(const std::string& pv_base, 
		const RecordConfig&, const OtherConfig& , Result *result);
    ~PVGenerator();  

    static const std::string pv_file(const std::string& pv_base, size_t thread_id);
    static void setLimitSibling(int new_value) { limit_sibling = new_value; }
    static void setNormalDepth(int new_depth) { normal_depth = new_depth; }
    static void setQuiesceDepth(int new_depth) { 
      quiesce_depth = new_depth; 
      bonanza_compatible_search = (quiesce_depth < 0);
    }
    static void setUsePercent(int new_percent) { use_percent = new_percent; }
    static void setComparePass(bool new_compare) { compare_pass = new_compare; }
    static void setWindowAsymmetry(int new_value) { window_asymmetry = new_value; }

    typedef std::pair<int,PVVector> value_and_pv_t;
    typedef std::vector<value_and_pv_t> values_t;
    
    enum SearchResult { SearchOK, DifficultMove, OneReply, OneReplyBySearch, BestMoveNotFound, PVCheckmate, BestmoveSearchFailed };
    static SearchResult
    searchSiblings(Quiesce& quiesce, int width,
		   const osl::NumEffectState& state, 
		   osl::Progress16 progress, osl::Move best_move,
		   int rand_seed, size_t accept_rank_threshold,
		   int accept_piece_sacrifice_threshold,
		   int& best_value, PVVector& best_pv, values_t& out,
		   size_t& rank_search, size_t& rank_probability,
		   int &outrange_better_move, int& outrange_other_move);
  protected:
    void init();
    void forEachPosition(int record_id, int position_id, size_t position_size,
			 Quiesce& quiesce, const osl::NumEffectState& state, 
			 osl::Progress16, osl::Move best_move);
    static int limit_sibling;
  };
}

#endif /* GPSSHOGI_EVAL_PVGENERATOR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
