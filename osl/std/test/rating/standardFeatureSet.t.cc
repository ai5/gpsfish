#include "osl/rating/featureSet.h"
#include "osl/rating/group.h"
#include "osl/rating/feature.h"
#include "osl/rating/ratingEnv.h"
#include "osl/stat/histogram.h"
#include "osl/stat/average.h"
#include "osl/oslConfig.h"
#include "osl/csa.h"

#include <boost/test/unit_test.hpp>
#include <string>
#include <fstream>
#include <iostream>

using namespace osl;
using namespace osl::rating;

static void testFileMatch(std::string const& filename)
{
  static StandardFeatureSet feature_set;
  static int kifu_count = 0;
  if ((++kifu_count % 8) == 0)
    std::cerr << '.';
  const RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  auto& moves=record.moves;
  MoveStack history;

  for (size_t i=0; i<moves.size(); ++i) {
    if (i > 150)
      break;
    const Move move = moves[i];
    RatingEnv env;
    env.history = history;
    env.make(state);
    
    {
      MoveVector moves;
      state.generateLegal(moves);
      for (size_t k=0; k<moves.size(); ++k) {
	for (size_t j=0; j<feature_set.groupSize(); ++j) {
	  int match = feature_set.group(j).findMatch(state, moves[k], env);
	  if (match < 0)
	    continue;
	  match += feature_set.range(j).first;
	  if (! feature_set.feature(match).match(state, moves[k], env)) {
	    std::cerr << feature_set.group(j).group_name << " " 
		      << feature_set.feature(match).name() << " " << match - feature_set.range(j).first 
		      << "\n" << state << moves[k];
	  }
	  BOOST_CHECK(feature_set.feature(match).match(state, moves[k], env));
	}
      }
    }

    state.makeMove(move);
    history.push(move);
  }
}

BOOST_AUTO_TEST_CASE(StandardFeatureSetTestMatch)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for(int i=0;i<100 && (ifs >> filename) ; i++){
    testFileMatch(OslConfig::testCsaFile(filename));
  }
}

static stat::Average all_average;
static void testFile(std::string const& filename)
{
  static StandardFeatureSet feature_set;
  static int kifu_count = 0;
  if ((++kifu_count % 8) == 0)
    std::cerr << '.';
  const RecordMinimal record=CsaFileMinimal(filename).load();
  NumEffectState state(record.initial_state);
  auto& moves=record.moves;
  MoveStack history;
  const size_t limit = 1000;

  stat::Histogram stat(200,10);
  stat::Average order_average;
  for (size_t i=0; i<moves.size(); ++i) {
    if (i > 150)
      break;
    const Move move = moves[i];
    RatingEnv env;
    env.history = history;
    env.make(state);
    // 合法手生成のテスト
    MoveLogProbVector all_moves;
    feature_set.generateLogProb(state, env, limit, all_moves);
    for (MoveLogProbVector::const_iterator p=all_moves.begin(); p!=all_moves.end(); ++p) {
      if (p->isPass())
	continue;
      BOOST_CHECK(p->move().isValid());
      BOOST_CHECK(state.isValidMove(p->move()));
    }

    // 確率のテスト
    const MoveLogProb *found = all_moves.find(move);
    if (! found) {
      std::cerr << state << move << "\n";
    }
    BOOST_CHECK(found);
    stat.add(found->logProb());
    order_average.add(found - &*all_moves.begin());

    state.makeMove(move);
    history.push(move);
  }
  if (OslConfig::verbose()) {
    std::cout << filename 
	      << " average order " << order_average.average() << "\n";
    if (order_average.average() >= 10)
      stat.show(std::cout);
  }
  BOOST_CHECK(order_average.average() < 10);
  all_average.merge(order_average);
}

BOOST_AUTO_TEST_CASE(StandardFeatureSetTestCover)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string filename;
  for(int i=0;i<100 && (ifs >> filename) ; i++){
    testFile(OslConfig::testCsaFile(filename));
  }
  BOOST_CHECK(all_average.average() < 6);
}

BOOST_AUTO_TEST_CASE(StandardFeatureSetTestPass)
{
  static StandardFeatureSet feature_set;
  NumEffectState state((SimpleState(HIRATE)));
  
  RatingEnv env;
  env.history.push(Move::PASS(WHITE));
  env.make(state);

  MoveLogProbVector all_moves;
  feature_set.generateLogProb(state, env, 1200, all_moves);
}

BOOST_AUTO_TEST_CASE(StandardFeatureSetTestKingEscape)
{
  static StandardFeatureSet feature_set;
  NumEffectState state(CsaString(
			 "P1-KY-KE-OU-KI *  *  * +HI * \n"
			 "P2 *  * -GI * -KI-GI *  *  * \n"
			 "P3-FU-FU-FU-FU *  *  *  *  * \n"
			 "P4 *  *  *  * -FU+GI+HI *  * \n"
			 "P5 * +FU * +FU *  *  * -KE+FU\n"
			 "P6 *  *  *  * +FU *  * -FU * \n"
			 "P7+FU * +KE * +UM * -UM-GI+OU\n"
			 "P8 *  * +FU+KI *  * -KI *  * \n"
			 "P9 *  *  *  *  *  * +KE * +KY\n"
			 "P+00KY00KY00FU00FU00FU00FU\n"
			 "P-00FU00FU\n"
			 "+\n").initialState());
  RatingEnv env;
  env.make(state);
  MoveLogProbVector all_moves;
  feature_set.generateLogProb(state, env, 1200, all_moves);
  const Move m25hi(Square(2,1), Square(2,5), ROOK, KNIGHT, false, BLACK);
  // 成ると詰み 第33期朝日アマ名人戦三番勝負第3局より
  BOOST_CHECK(all_moves.find(m25hi));
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
