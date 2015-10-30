/* featureSet.h
 */
#ifndef OSL_FEATURESET_H
#define OSL_FEATURESET_H

#include "osl/rating/range.h"
#include "osl/rating/ratedMoveVector.h"
#include "osl/container/moveLogProbVector.h"
#include "osl/numEffectState.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <string>
#include <iosfwd>

namespace osl
{
  namespace stat
  {
    class Average;
    class Variance;
  }
  namespace rating
  {
    class Group;
    class Feature;
    class RatingEnv;
    class CheckmateIfCaptureGroup;
    class CaptureGroup;
    class SendOffGroup;
    class FeatureSet 
    {
      // noncopyable
      FeatureSet(const FeatureSet&) = delete;
      FeatureSet& operator=(const FeatureSet&) = delete;
      // range は Groupのメンバーにした方が綺麗な気も
      boost::ptr_vector<Group> groups;
      std::vector<char> effective_in_check;
      std::vector<Feature*> features;	// acquaintance
      std::vector<range_t> ranges;
      std::vector<double> weights;
      std::vector<int> weightslog10;
      /** makeRateで特別扱いのgroup */
      int capture_group, checkmate_if_capture_group, sendoff_group;
      std::vector<char> normal_groups;
      /** 統計測定用 */
      mutable std::vector<CArray<stat::Average,8> > frequency;
      mutable std::vector<CArray<stat::Variance,8> > variance_match;
      mutable std::vector<stat::Variance> variance_all;
      struct Statistics;
      std::vector<Statistics> statistics;
    public:
      FeatureSet();
      virtual ~FeatureSet();
      bool tryLoad(const std::string& input_directory);
      void setWeight(size_t feature_id, const double& value);

      const Group& group(size_t group_id) const { return groups[group_id]; }
      bool effectiveInCheck(size_t group_id) const { return effective_in_check[group_id]; }
      const Feature& feature(size_t feature_id) const { return *features[feature_id]; }
      const range_t& range(size_t group) const { return ranges[group]; }
      const double& weight(size_t feature_id) const { return weights[feature_id]; }
      size_t groupSize() const { return groups.size(); }
      size_t featureSize() const { return features.size(); }
      const RatedMove makeRate(const NumEffectState& state, bool in_check,
			       const RatingEnv& env, Move move) const;
      const RatedMove makeRateWithCut(const NumEffectState& state, bool in_check, 
				      const RatingEnv& env, int limit, Move move) const;
      const std::string annotate(const NumEffectState& state, 
				 const RatingEnv& env, Move move) const;
      void generateRating(const NumEffectState& state, const RatingEnv& env,
			  int limit, RatedMoveVector& out, bool in_pv_or_all=true) const;
      void generateLogProb(const NumEffectState& state, const RatingEnv& env,
			   int limit, MoveLogProbVector& out, bool in_pv_or_all=true) const;
      int logProbTakeBack(const NumEffectState& state, const RatingEnv& env, Move) const;
      int logProbSeePlus(const NumEffectState& state, const RatingEnv& env, Move) const;
      int logProbKingEscape(const NumEffectState& state, const RatingEnv& env, Move) const;

      void showGroup(std::ostream&, size_t group_id) const;
      void save(const std::string& output_directory, size_t group_id) const;

      void showStatistics(std::ostream&) const;

      static void normalize(const RatedMoveVector&, MoveLogProbVector& out);
      static std::string defaultDirectory();
    protected:
      void add(Feature *f);
      void add(Group *g);
      void add(CaptureGroup *g);
      void add(SendOffGroup *g);
      void add(CheckmateIfCaptureGroup *g);
      void addCommon(Group *g);
      void addFinished();
    private:
      const range_t makeRange(size_t group) const;
      int rating(const NumEffectState& state, 
		 const RatingEnv& env, Move move, size_t group_id) const;
    };
    
    class StandardFeatureSet : public FeatureSet
    {
    public:
      explicit StandardFeatureSet(bool allow_load_failure=false);
      static const StandardFeatureSet& instance();
      static bool healthCheck();
    };

    /** 駒得のみ*/
    class CaptureSet : public FeatureSet
    {
    public:
      explicit CaptureSet(bool allow_load_failure=false);
    };
    /** 駒得のみ*/
    class TacticalSet : public FeatureSet
    {
    public:
      explicit TacticalSet(bool allow_load_failure=false);
    };
  }
}

#endif /* OSL_FEATURESET_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
