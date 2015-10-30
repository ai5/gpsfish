/* featureSet.cc
 */
#include "osl/rating/featureSet.h"
#include "osl/config.h"
#include "osl/rating/group.h"
#include "osl/rating/group/captureGroup.h"
#include "osl/rating/group/escape.h"
#include "osl/rating/group/squareGroup.h"
#include "osl/rating/group/patternGroup.h"
#include "osl/rating/group/bigramGroup.h"
#include "osl/rating/group/king8Group.h"
#include "osl/rating/group/checkmateGroup.h"
#include "osl/rating/group/pinGroup.h"
#include "osl/move_generator/allMoves.h"
#include "osl/move_generator/escape_.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/move_classifier/safeMove.h"
#include "osl/stat/variance.h"
#include "osl/oslConfig.h"
#include <boost/format.hpp>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cmath>

// statistics for each feature
// #define RATING_STAT
// show statistics loaded for each feature
// #define VERBOSE_RATING
// statistics between rating and limit
// #define RATING_STAT2

const int MinRating = -4000;

struct osl::rating::FeatureSet::Statistics
{
  /** group 以降の性質 */
  int average_after, sigma_after;
  /** group 単独の性質 */
  double average, variance, probability;
  Statistics() : average_after(0), sigma_after(0), 
		 average(0), variance(0), probability(0)
  {
  }  
};


osl::rating::
FeatureSet::FeatureSet()
  : capture_group(-1), checkmate_if_capture_group(-1), sendoff_group(-1)
{
}

osl::rating::
FeatureSet::~FeatureSet()
{
#ifdef RATING_STAT
  showStatistics(std::cerr);
#endif
}

const osl::rating::range_t osl::rating::
FeatureSet::makeRange(size_t group) const
{
  int first = 0;
  for (size_t i=0; i<groups.size(); ++i) {
    if (i == group)
      return std::make_pair(first, first+groups[i].size());
    first += groups[i].size();
  }
  assert(0);
  abort();
}

void osl::rating::
FeatureSet::addFinished()
{
  assert(normal_groups.size() == groups.size());
  weights.resize(features.size(), 1.0);
  weightslog10.resize(features.size(), 1);
  assert(weights.size() == features.size());
  ranges.resize(groups.size());
  for (size_t i=0; i<groups.size(); ++i)
    ranges[i] = makeRange(i);
  variance_match.resize(groups.size());
  variance_all.resize(groups.size());
  frequency.resize(groups.size());
  statistics.resize(groups.size());
}

void osl::rating::
FeatureSet::add(Feature *f)
{
  add(new Group(f));
}

void osl::rating::
FeatureSet::addCommon(Group *g)
{
  features.reserve(features.size()+g->size());
  for (size_t i=0; i<g->size(); ++i) {
    features.push_back(&(*g)[i]);
  }
  groups.push_back(g);
  effective_in_check.push_back(g->effectiveInCheck());
}

void osl::rating::
FeatureSet::add(Group *g)
{
  normal_groups.push_back(true);
  addCommon(g);
}

void osl::rating::
FeatureSet::add(CaptureGroup *g)
{
  capture_group = normal_groups.size();
  normal_groups.push_back(false);
  addCommon(g);
}

void osl::rating::
FeatureSet::add(SendOffGroup *g)
{
  sendoff_group = normal_groups.size();
  normal_groups.push_back(false);
  addCommon(g);
}

void osl::rating::
FeatureSet::add(CheckmateIfCaptureGroup *g)
{
  checkmate_if_capture_group = normal_groups.size();
  normal_groups.push_back(false);  
  addCommon(g);
}

bool osl::rating::
FeatureSet::tryLoad(const std::string& input_directory) 
{
  bool result = true;
  for (size_t i=0; i<groups.size(); ++i) {
    const bool success = groups[i].load(input_directory, ranges[i], weights);
    if (! success && result)
      std::cerr << "warning: rating load failed " << groups[i].group_name << " " << i 
		<< " in " << input_directory << "\n";
    result &= success;
  }
  for (size_t i=0; i<features.size(); ++i)
    weightslog10[i] = static_cast<int>(400*log10(weights[i]));
#ifndef RATING_STAT
  std::string filename = input_directory + "/statistics.txt";
  std::ifstream is(filename.c_str());
  typedef std::map<std::string,Statistics> map_t;
  map_t table;
  std::string name;
  double a, s, p, dummy;
  while (is >> name >> a >> s >> dummy >> dummy >> p) {
    Statistics& stat = table[name];
    stat.average = a;
    stat.variance = s*s;
    stat.probability = p;
  }
  for (size_t i=0; i<groups.size(); ++i) {
    double a = 0.0, v = 0.0;
    for (size_t j=i+1; j<groups.size(); ++j) {
      map_t::iterator q = table.find(groups[j].group_name);
      if (q == table.end()) {
	result = false;
	break;
      }
      a += q->second.probability * q->second.average;
      v += q->second.probability * q->second.variance;
    }
    statistics[i] = table[groups[i].group_name];
    statistics[i].average_after = static_cast<int>(a);
    statistics[i].sigma_after = static_cast<int>(sqrt(v)*3);
#  ifdef VERBOSE_RATING
    std::cerr << groups[i].group_name
	      << " " << statistics[i].average_after
	      << " " << statistics[i].sigma_after << "\n";
#  endif
  }
#endif
  return result;
}

void osl::rating::
FeatureSet::setWeight(size_t feature_id, const double& value)
{
  weights[feature_id] = value; 
  weightslog10[feature_id] = static_cast<int>(400*log10(value));
}

void osl::rating::
FeatureSet::generateRating(const NumEffectState& state, const RatingEnv& env,
			   int limit, RatedMoveVector& out, bool in_pv_or_all) const
{
#if (defined RATING_STAT) || (defined RATING_STAT2)
  in_pv_or_all = true;
#endif
  MoveVector moves;
  const bool in_check = state.inCheck();
  // generate legal moves except for pawn drop checkmate
  if (in_check)
    GenerateEscapeKing::generate(state, moves);
  else
    GenerateAllMoves::generate(state.turn(), state, moves);

  for (size_t i=0; i<moves.size(); ++i) {
    if (moves[i].ptype() == KING) {
      if (state.hasEffectAt(alt(state.turn()), moves[i].to()))
	continue;
    } else {
      if (! in_check && env.my_pin.any() && ! moves[i].isDrop()
	  && move_classifier::PlayerMoveAdaptor<move_classifier::KingOpenMove>::isMember(state, moves[i]))
	continue;
    }

    if (in_pv_or_all)
      out.push_back(makeRate(state, in_check, env, moves[i]));
    else {
      RatedMove r = makeRateWithCut(state, in_check, env, limit, moves[i]);
      if (r.rating() > MinRating)
	out.push_back(r);
    }
  }
  out.sort();
}

// width 4, [0,80]
static const osl::CArray2d<int, 8, 20> order_to_depth = {{
   186, 213, 243, 247, 249,  255, 252, 258, 263, 269,  267, 279, 295, 295, 295,  295, 295, 295, 295, 295, 
   191, 245, 283, 300, 313,  315, 319, 323, 326, 339,  321, 347, 334, 346, 328,  368, 328, 328, 328, 328, 
   183, 250, 304, 328, 346,  352, 373, 366, 365, 379,  396, 379, 392, 416, 420,  374, 423, 378, 395, 399, 
   184, 253, 312, 346, 358,  378, 389, 407, 409, 403,  404, 421, 432, 395, 421,  444, 444, 461, 411, 408, 
   190, 256, 319, 350, 373,  397, 397, 403, 420, 431,  415, 450, 424, 416, 436,  447, 456, 439, 429, 428, 
   197, 262, 324, 357, 374,  390, 407, 423, 415, 425,  436, 444, 458, 455, 439,  474, 451, 466, 464, 457, 
   202, 268, 332, 360, 381,  386, 416, 416, 418, 433,  447, 446, 452, 462, 479,  468, 467, 486, 483, 459, 
   205, 270, 330, 361, 383,  394, 410, 418, 427, 438,  438, 452, 446, 445, 447,  463, 475, 472, 483, 485, 
}};
static const osl::CArray2d<int, 8, 20> order_to_width = {{
   262, 445, 584, 685, 788,  890, 982,1067,1120,1148, 1137,1156,1182,1231,1259, 1343,1352,1359,1359,1359, 
   265, 456, 577, 665, 745,  809, 874, 938, 997,1061, 1088,1154,1179,1231,1259, 1343,1352,1359,1359,1359, 
   260, 467, 596, 680, 751,  807, 872, 908, 951,1003, 1054,1072,1117,1168,1198, 1188,1267,1259,1311,1344, 
   261, 467, 599, 688, 747,  810, 861, 914, 948, 975, 1008,1055,1092,1084,1142, 1189,1214,1254,1231,1258, 
   264, 463, 595, 679, 746,  808, 844, 885, 933, 973,  987,1049,1048,1068,1115, 1151,1184,1191,1209,1233, 
   268, 459, 588, 673, 732,  788, 840, 887, 910, 950,  989,1022,1059,1078,1088, 1144,1144,1180,1201,1216, 
   271, 459, 587, 664, 727,  771, 835, 866, 899, 942,  984,1006,1037,1069,1105, 1114,1134,1173,1188,1186, 
   272, 458, 581, 661, 725,  773, 824, 863, 902, 940,  966,1005,1023,1047,1074, 1113,1145,1163,1193,1214, 
}};

const int sc_width = 100, sc_length = 18, sc_start = -400;
static const osl::CArray2d<int, 8, sc_length> score_to_depth = {{
   263, 271,  274, 270, 278, 253, 235,  201, 171, 151, 111,  95,   83,  76,  78,  65,  71,   61,
   330, 334,  328, 316, 312, 304, 284,  256, 218, 188, 159, 136,  113, 103,  92,  87,  82,   71,
   377, 374,  376, 368, 356, 337, 311,  278, 246, 203, 175, 146,  131, 118, 107,  96,  81,   65,
   415, 424,  406, 396, 376, 345, 315,  276, 243, 211, 179, 155,  138, 121, 110,  91,  80,   62,
   423, 422,  433, 423, 405, 381, 341,  313, 276, 243, 210, 182,  158, 142, 123, 104,  85,   73,
   442, 451,  448, 437, 417, 395, 364,  333, 297, 267, 234, 202,  178, 158, 133, 107,  91,   76,
   446, 447,  455, 439, 427, 402, 373,  339, 307, 274, 242, 212,  188, 162, 133, 111,  92,   75,
   467, 468,  469, 453, 433, 412, 389,  365, 334, 301, 268, 236,  205, 177, 153, 131, 116,  101,
}};
static const osl::CArray2d<int, 8, sc_length> score_to_width = {{
   978, 880,  786, 676, 586, 475, 383,  302, 239, 208, 167, 153,  134, 127, 126, 100, 100,   82,
  1020, 935,  836, 730, 634, 549, 472,  412, 351, 312, 269, 232,  190, 167, 143, 127, 112,   95,
  1095, 998,  910, 810, 715, 623, 543,  471, 407, 338, 291, 246,  216, 189, 160, 140, 115,   90,
  1106,1031,  929, 829, 730, 635, 551,  469, 402, 341, 290, 249,  217, 186, 159, 127, 108,   85,
  1185,1092, 1011, 913, 811, 717, 617,  538, 459, 391, 331, 285,  242, 210, 176, 143, 114,   96,
  1224,1150, 1058, 957, 853, 755, 658,  573, 493, 424, 363, 308,  262, 223, 181, 142, 116,   96,
  1224,1134, 1057, 953, 857, 759, 666,  579, 501, 432, 373, 315,  267, 220, 178, 141, 115,   93,
  1296,1201, 1115,1009, 904, 807, 717,  638, 563, 492, 425, 363,  305, 254, 210, 172, 145,  123,
}};

const int rsc_length = 15;
static const osl::CArray2d<int, 8, rsc_length> relative_score_to_depth = {{
   193, 220, 235, 249, 256,  263, 268, 274, 279, 284,  283, 279, 292, 267, 272, 
   220, 243, 263, 273, 287,  300, 306, 308, 317, 325,  328, 339, 319, 336, 323, 
   215, 242, 267, 287, 302,  314, 329, 340, 347, 360,  367, 364, 349, 387, 374, 
   209, 243, 267, 293, 317,  332, 347, 360, 372, 383,  387, 387, 395, 398, 405, 
   216, 244, 276, 303, 322,  344, 360, 374, 378, 397,  405, 414, 408, 400, 424, 
   220, 251, 278, 307, 331,  355, 365, 381, 398, 406,  418, 423, 414, 433, 403, 
   226, 254, 284, 311, 336,  354, 378, 390, 408, 418,  420, 448, 414, 446, 408, 
   219, 250, 283, 310, 333,  356, 377, 391, 403, 417,  426, 426, 440, 445, 452, 
}};
static const osl::CArray2d<int, 8, rsc_length> relative_score_to_width = {{
   214, 285, 357, 442, 520,  596, 669, 742, 816, 881,  928, 972,1045,1079,1143, 
   237, 302, 374, 442, 519,  595, 662, 731, 799, 870,  925, 994,1031,1112,1159, 
   230, 294, 367, 442, 517,  595, 675, 746, 815, 884,  951,1012,1060,1149,1185, 
   224, 292, 361, 441, 524,  602, 682, 758, 833, 904,  964,1028,1105,1164,1223, 
   231, 295, 369, 449, 525,  611, 692, 771, 839, 922,  985,1041,1094,1150,1239, 
   235, 301, 370, 450, 532,  616, 690, 769, 851, 920,  991,1054,1100,1194,1217, 
   240, 300, 373, 448, 527,  607, 693, 768, 845, 919,  981,1066,1094,1191,1218, 
   233, 294, 364, 435, 511,  591, 674, 753, 832, 917,  993,1065,1157,1224,1300, 
}};

inline int make_prob(int score, int order, int limit, int highest, int progress8, bool in_pv_or_all) 
{
  const int order_index = std::min((int)order/4, 19);
  int result = limit+1;
  if (order_to_width[progress8][order_index] <= limit) {
    result = (order == 0) ? 100 : order_to_depth[progress8][order_index];
  }  
  score = std::max(sc_start, score);
  highest = std::max(sc_start, highest);
  const int score_index = std::min((score - sc_start)/sc_width, sc_length-1);
  if (limit > 600 && score_to_width[progress8][score_index] <= limit) {
    result = std::min(result, score_to_depth[progress8][score_index]);
  }
  if (limit > 700 && order > 0 && in_pv_or_all) {
    const int rscore_index = std::min((highest - score)/100, rsc_length-1);
    assert(rscore_index >= 0);
    if (relative_score_to_width[progress8][rscore_index] <= limit)
      result = std::min(result, relative_score_to_depth[progress8][rscore_index]);
  }
  return result;
}

#ifdef RATING_STAT2
namespace osl
{
  namespace 
  {
    CArray2d<CArray<stat::Average,8>,14,40> data; // selected/generated for each range
    CArray2d<CArray<double,8>,14,80> selected_all;
    void add_stat(int limit, int rating, bool added, int progress8) 
    {
      limit = std::min(limit, 999);
      limit -= 300;
      limit = std::max(limit, 0);
      rating = std::max(-999,rating);
      rating = std::min(999,rating);
      data[limit/50][(rating+1000)/50][progress8].add(added);
      if (added)
	selected_all[limit/50][(rating+1000)/25][progress8] += 1.0;
    }
    struct Reporter
    {
      ~Reporter()
      {
	std::cerr << "limit " << 0*50+300 << " - " << (data.size1()-1)*50+300 << "\n";
	for (int p=0; p<8; ++p) 
	{
	  std::cerr << "progress8 " << p << "\n  ";
	  for (size_t j=0; j<data.size1(); ++j) 
	  {
	    size_t i=0;
	    for (; i<data.size2(); ++i)
	      if (data[j][i][p].getAverage() > 0.05)
		break;
	    std::cerr << (boost::format("%+4d, ") % static_cast<int>(i)*50-1000);
	  }
	  std::cerr << "\n";
	}
	std::cerr << "limit " << 0*50+300 << " - " << (selected_all.size1()-1)*50+300 << "\n";
	CArray<double, 3> prob = {{ 0.01, 0.03, 0.05 }};
	for (size_t pp=0; pp<prob.size(); ++pp) {
	  std::cerr << "prob " << prob[pp] << "\n";
	  for (int p=0; p<8; ++p) 
	  {
	    std::cerr << "progress8 " << p << "\n  ";
	    for (size_t j=0; j<selected_all.size1(); ++j) 
	    {
	      double sum = 0;
	      for (size_t i=0; i<selected_all.size2(); ++i)
		sum += selected_all[j][i][p];
	      size_t i=0
	      for (double so_far = 0; i<selected_all.size2(); ++i) {
		so_far += selected_all[j][i][p];
		if (so_far > prob[pp]*sum) 
		  break;
	      }
	      std::cerr << (boost::format("%+4d, ") % static_cast<int>(i)*25-1000);
	    }
	    std::cerr << "\n";
	  }
	}
      }
    } _reporter;
  }
}
#endif

void osl::rating::
FeatureSet::generateLogProb(const NumEffectState& state, const RatingEnv& env,
			    int limit, MoveLogProbVector& out, bool in_pv_or_all) const
{
  RatedMoveVector score;
  generateRating(state, env, limit, score, in_pv_or_all);
  if (score.empty())
    return;

  const int highest = score[0].rating();
  const int progress8 = env.progress.value()/2;
  for (size_t i=0; i<score.size(); ++i) {
    const int log_prob = make_prob(score[i].rating(), i, limit, highest, progress8, in_pv_or_all);
#ifdef RATING_STAT2
    add_stat(limit, score[i].rating(), log_prob <= limit, progress8);
#endif
    out.push_back(MoveLogProb(score[i].move(), log_prob));
  }
}

const int max_score = 999, min_score = 0;
static const osl::CArray<int, 10> score_to_depth_takeback = {{
  223, 204, 208, 190, 159, 137, 124, 110, 100, 89
}};
static const osl::CArray<int, 10> score_to_depth_seeplus = {{
  356, 337, 296, 262, 230, 200, 171, 148, 132, 120,
}};
static const osl::CArray<int, 10> score_to_depth_kingescape = {{
  203, 201, 199, 188, 181, 169, 159, 147, 136, 122,
}};

int osl::rating::
FeatureSet::logProbTakeBack(const NumEffectState& state, const RatingEnv& env, Move move) const
{
  const bool in_check = state.inCheck();
  const int score = makeRate(state, in_check, env, move).rating();
  if (score >= 1000) {
    const int score_index = std::min((score - sc_start)/sc_width, sc_length-1);
    return score_to_depth[env.progress.value()/2][score_index];
  }
  return score_to_depth_takeback[std::max(min_score, std::min(max_score, score))/100];
}
int osl::rating::
FeatureSet::logProbSeePlus(const NumEffectState& state, const RatingEnv& env,
			   Move move) const
{
  const bool in_check = state.inCheck();
  const int score = makeRate(state, in_check, env, move).rating();
  if (score >= 1000) {
    const int score_index = std::min((score - sc_start)/sc_width, sc_length-1);
    return score_to_depth[env.progress.value()/2][score_index];
  }
  return score_to_depth_seeplus[std::max(min_score, std::min(max_score, score))/100];
}
int osl::rating::
FeatureSet::logProbKingEscape(const NumEffectState& state, const RatingEnv& env,
			      Move move) const
{
  const bool in_check = state.inCheck();
  const int score = makeRate(state, in_check, env, move).rating();
  if (score >= 1000) {
    const int score_index = std::min((score - sc_start)/sc_width, sc_length-1);
    return score_to_depth[env.progress.value()/2][score_index];
  }
  const int prob = score_to_depth_kingescape[std::max(min_score, std::min(max_score, score))/100];
  assert(prob > 0);
  return prob;
}

int osl::rating::
FeatureSet::rating(const NumEffectState& state, 
		   const RatingEnv& env, Move move, size_t group_id) const
{
  int found = groups[group_id].findMatch(state, move, env);
  if (found < 0) {
#ifdef RATING_STAT
    const int progress8 = env.progress.value()/2;
    frequency[group_id][progress8].add(0);
    variance_all[group_id].add(0);
#endif
    return 0;
  }
  found += ranges[group_id].first;
#ifdef RATING_STAT
  const int progress8 = env.progress.value()/2;
  frequency[group_id][progress8].add(1);
  variance_match[group_id][progress8].add(weightslog10[found]);
  variance_all[group_id].add(weightslog10[found]);
#endif
  return weightslog10[found];
}

const osl::rating::RatedMove osl::rating::
FeatureSet::makeRate(const NumEffectState& state, bool in_check,
		     const RatingEnv& env, Move move) const
{
  int sum = 0;
  for (size_t j=0; j<groups.size(); ++j) {
    if (! normal_groups[j])
      continue;
    if (in_check && ! effectiveInCheck(j))
      continue;
    sum += rating(state, env, move, j);
  }
  int capture = 0;
  if (capture_group >= 0)
    capture = rating(state, env, move, capture_group);
  int checkmate_if_capture = 0;
  if (checkmate_if_capture_group >= 0)
    checkmate_if_capture = rating(state, env, move, checkmate_if_capture_group);
  sum += checkmate_if_capture;
  int sendoff = 0;
  if (sendoff_group >= 0)
    sendoff = rating(state, env, move, sendoff_group);
  sum += sendoff;

  if (checkmate_if_capture > 0)
    capture = std::max(0, capture);
  else if (sendoff > 0 && capture < 0)
    capture /= 2;
  const int optimistic = sum + std::max(0, capture);
  sum += capture;
  
  return RatedMove(move, sum, optimistic);
}


// limit [0-800)
#if 1
// 1%
static const osl::CArray2d<int,8,16> threshold = {{
    0, 0, 0, 0, 0, 0,
    100, 100,  50,   0,   0, -75,-100,-150,-200,-200,

    0, 0, 0, 0, 0, 0,
    125, 125, 125,  25,  25, -50, -50,-100,-125,-225,

    0, 0, 0, 0, 0, 0,
    100,  75, 100,  25,   0, -25, -50,-100,-125,-175,

    0, 0, 0, 0, 0, 0,
     75,  50,  75,   0, -25, -25, -75,-100,-125,-200,

    0, 0, 0, 0, 0, 0,
    125, 125, 150,  50,  50,  50, -25,   0, -50,-200,

    0, 0, 0, 0, 0, 0,
    175, 200, 200,  75,  75,  75,   0,   0,-175,-300,

    0, 0, 0, 0, 0, 0,
    175, 175, 200,  50,  75,  75,  25,   0,-100,-250,

    0, 0, 0, 0, 0, 0,
    225, 200, 225,  75, 100,  75,  50,   0,   0,-250,
}};
#endif
#if 0
static const osl::CArray2d<int,8,16> threshold = {{
  // 0, 50, 100, 150, 200, ...
    0, 0, 0, 0, 0, 0,
    100,100,100,0,0,-100,-100,-200,-200,-200,

    0, 0, 0, 0, 0, 0,
    100,100,100,0,0,-100,-100,-100,-100,-200,

    0, 0, 0, 0, 0, 0,
    100,100,100,0,0,0,-100,-100,-100,-200

    0, 0, 0, 0, 0, 0,
    100,100,100,0,0,0,-100,-100,-100,-200

    0, 0, 0, 0, 0, 0,
    200,200,200,100,100,100,0,0,0,-100

    0, 0, 0, 0, 0, 0,
    300,300,300,100,100,100,100,0,-200,-300

    0, 0, 0, 0, 0, 0,
    300,300,300,100,100,100,100,0,0,-200

    0, 0, 0, 0, 0, 0,
    300,300,300,100,200,200,100,0,0,-200
}};
#endif
const osl::rating::RatedMove osl::rating::
FeatureSet::makeRateWithCut(const NumEffectState& state, 
			    bool in_check,
			    const RatingEnv& env, 
			    int limit, Move move) const
{
  if (limit >= 800)
    return makeRate(state, in_check, env, move);

  limit /= 50;
  int sum = 0;
  int capture = 0;
  int checkmate_if_capture = 0;
  const int progress8 = env.progress.value()/2;
  for (size_t j=0; j<groups.size(); ++j) {
    if (in_check && ! effectiveInCheck(j))
      continue;
    const int r = rating(state, env, move, j);
    sum += r;
    if ((int)j == capture_group) {
      capture = r;
    }
    else if ((int)j == checkmate_if_capture_group) {
      checkmate_if_capture = r;
      if (checkmate_if_capture > 0 && capture < 0) {
	sum -= capture;
	capture = 0;
      }
    }
    // cut?
    if (j % 8 == 7) {
      int sigma = statistics[j].sigma_after;
      if (sum + statistics[j].average_after + sigma < threshold[progress8][limit]) {
	return RatedMove(move, MinRating, MinRating);
      }
    }
  }

  const int optimistic = sum + std::max(0, capture);  
  return RatedMove(move, sum, optimistic);
}

const std::string osl::rating::
FeatureSet::annotate(const NumEffectState& state, 
		     const RatingEnv& env, Move move) const
{
  const bool in_check = state.inCheck();
  std::vector<std::pair<int, std::string> > values;
  for (size_t j=0; j<groups.size(); ++j) {
    if (in_check && ! effectiveInCheck(j))
      continue;
    int found = groups[j].findMatch(state, move, env);
    if (found < 0)
      continue;
    found += ranges[j].first;
    values.push_back(std::make_pair(weightslog10[found], groups[j].group_name));
  }
  std::sort(values.begin(), values.end());
  std::reverse(values.begin(), values.end());
  std::ostringstream ss;
  for (size_t i=0; i<values.size(); ++i) {
    if (i)
      ss << "  ";
    ss << values[i].second << " " << values[i].first;
  }
  return ss.str();
}

#ifndef MINIMAL
void osl::rating::
FeatureSet::showGroup(std::ostream& os, size_t group_id) const
{
  os << std::setprecision(3);
  group(group_id).show(os, 12, range(group_id), weights);
}

void osl::rating::
FeatureSet::save(const std::string& output_directory, size_t group_id) const
{
  group(group_id).saveResult(output_directory, range(group_id), weights);
}

void osl::rating::
FeatureSet::showStatistics(std::ostream& os) const
{
  os << std::setprecision(3);
  for (size_t i=0; i<groups.size(); ++i) {
    os << groups[i].group_name << "\t";
    for (int p=0; p<8; ++p) {
      os << "  " << variance_match[i][p].average() 
	 << "  " << sqrt(variance_match[i][p].variance())
	 << "  " << frequency[i][p].average() << "  ";
    }
    os << "\t" << variance_all[i].average() 
       << "\t" << sqrt(variance_all[i].variance())
       << "\n";
  }
}
#endif

std::string osl::rating::FeatureSet::defaultDirectory()
{
  return OslConfig::home()+"/data/rating";
}

/* ------------------------------------------------------------------------- */

osl::rating::
StandardFeatureSet::StandardFeatureSet(bool allow_load_failure)
{
  add(new CaptureGroup());
  add(new SquareYGroup());
  add(new RelativeKingXGroup(true));
  add(new SquareXGroup());
  add(new TakeBackGroup());
  add(new RelativeKingYGroup(true));
  add(new FromEffectGroup());
  add(new PatternGroup(U));
  add(new RelativeKingXGroup(false));
  add(new PatternGroup(D));

  add(new PatternLongGroup(0));
  add(new CheckGroup());
  add(new BlockGroup()); 
  add(new PtypeAttackedGroup());

  add(new PatternGroup(U,U));
  add(new ImmediateAddSupportGroup()); // 300 cycles
  add(new PatternGroup(DR));
  add(new RelativeKingYGroup(false));
  add(new DefenseKing8Group());
  add(new PatternGroup(L));
  add(new PatternGroup(UL));

  add(new ToSupportedGroup());

  add(new PatternGroup(UR));
  add(new PatternBlockGroup(ROOK));
  add(new AttackKing8Group());
  add(new PatternGroup(R));
  add(new PatternGroup(DL));

  add(new PatternGroup(R,R));
  add(new PatternLongGroup(3));
  add(new PatternGroup(UUL));
  add(new PatternGroup(UUR));
  add(new PatternGroup(L,L));

  add(new PatternLongGroup(2));
  add(new OpenGroup());
  add(new PatternBlockGroup(LANCE));
  add(new ChaseGroup());
  add(new PatternLongGroup(1));
  add(new PatternLongGroup2(1));
  add(new PatternBlockGroup(BISHOP));
  add(new PatternGroup(UR,R));
  add(new PatternLongGroup2(0));
  add(new PatternGroup(UL,L));

  add(new ImmediateEscapeGroup());
  add(new PatternLongGroup2(3));
  add(new PatternLongGroup2(2));
  add(new KaranariGroup());

  add(new BigramAttackGroup(true, true));
  add(new BigramAttackGroup(false, true));
  add(new BigramAttackGroup(true, false));
  add(new BigramAttackGroup(false, false));

  add(new DropCapturedGroup());
  add(new ContinueCaptureGroup());
  add(new PawnAttackGroup());
  add(new ThreatmateGroup());

  add(new BadLanceGroup());
  add(new CheckmateIfCaptureGroup());
  add(new RookDefense());
  add(new SendOffGroup());

  add(new PinGroup());
  add(new KingEscapeGroup());
  add(new EscapePinGroup());

  addFinished();
  bool success = tryLoad(defaultDirectory());
  if (! allow_load_failure && ! success) {
    std::cerr << "error: unable to load rating from " << defaultDirectory();
    throw std::runtime_error("load failed " + OslConfig::home()+defaultDirectory());
  }
}


const osl::rating::StandardFeatureSet& osl::rating::
StandardFeatureSet::instance()
{
  static osl::rating::StandardFeatureSet common_instance;
  return common_instance;
}

bool osl::rating::StandardFeatureSet::healthCheck()
{
  std::cerr << "loading " << defaultDirectory() << ' ';
  try {
    instance();
    std::cerr << "success\n";
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << "\n";
    return false;
  }
  catch (...) {
    std::cerr << "unknown exception\n";
    return false;
  }
  return true;
}

osl::rating::
CaptureSet::CaptureSet(bool allow_load_failure)
{
  add(new CaptureGroup());
  add(new ShadowEffectGroup());

  addFinished();
  bool success = tryLoad(defaultDirectory());
  if (! allow_load_failure && ! success) {
    std::cerr << "error: unable to load rating from " << defaultDirectory();
    throw std::runtime_error("load failed " + defaultDirectory());
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
