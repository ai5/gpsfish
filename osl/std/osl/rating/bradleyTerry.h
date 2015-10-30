/* bradleyTerry.h
 */
#ifndef _BRADLEYTERRY_H
#define _BRADLEYTERRY_H

#include "osl/rating/featureSet.h"
#include <valarray>

namespace osl
{
  namespace rating
  {
    class BradleyTerry
    {
      typedef std::valarray<double> valarray_t;
      FeatureSet& features;
      
      std::string kisen_filename, output_directory;
      int kisen_start;
      size_t num_cpus, num_records;
      int verbose;
      int fix_group;
      size_t min_rating;
    public:
      BradleyTerry(FeatureSet& features, const std::string& kisen_file, int kisen_start=0);
      ~BradleyTerry();
      
      void setNumCpus(int new_num_cpus) { num_cpus = new_num_cpus; };
      void setNumRecords(size_t new_num_records) { num_records = new_num_records; };
      void setOutputDirectory(const std::string& new_output) { output_directory = new_output; };
      void setFixGroup(int new_fix_group) { fix_group = new_fix_group; };
      void setVerbose(int new_verbose) { verbose = new_verbose; };
      void setMinRating(int new_min) { min_rating = new_min; };
      
      void iterate();
    private:
      void update(size_t g);
      bool addSquare(size_t g, const NumEffectState& state, 
		       const RatingEnv& env, Move selected,
		       valarray_t& wins, std::valarray<long double>& denominator) const;  

      class Thread;
      friend class Thread;
      size_t accumulate(size_t g, size_t first, size_t last, valarray_t& wins, std::valarray<long double>& denominator) const;
    };
  }
}

#endif /* _BRADLEYTERRY_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
