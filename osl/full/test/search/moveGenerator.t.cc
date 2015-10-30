/* moveGenerator.t.cc
 */
#include "osl/search/moveGenerator.h"
#include "osl/search/searchState2.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/search/analyzer/categoryMoveVector.h"
#include "osl/move_classifier/pawnDropCheckmate.h"
#include "osl/move_classifier/moveAdaptor.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/effect_util/pin.h"
#include "osl/eval/progressEval.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"

#include <boost/test/unit_test.hpp>

#include <set>
#include <iterator>
#include <fstream>
#include <iostream>
#include <string>

using namespace osl;
using namespace osl::search;

typedef SearchState2::checkmate_t checkmate_t;

BOOST_AUTO_TEST_CASE(MoveGeneratorTestCopy)
{
  eval::ProgressEval::setUp();
  MoveGenerator::initOnce();
  
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=100;
  if (OslConfig::inUnitTestShort()) 
    count=10;
  checkmate_t checkmate;
  std::string file_name;
  while((ifs >> file_name) && ++i<count)
  {
    if(file_name == "") 
      break;
    if (OslConfig::verbose())
      std::cerr << file_name << " ";
    auto record=CsaFileMinimal(OslConfig::testCsaFile(file_name)).load();
    NumEffectState initial_state(record.initialState());
    SearchState2 sstate(initial_state, checkmate);
    std::vector<osl::Move> moves=record.moves;
    MoveGenerator gen;
    for (unsigned int i=0; i<std::min((size_t)63,moves.size()); i++) {
      const int limit = 600;
      // generate all
      SimpleHashRecord record;
      record.setInCheck(sstate.state().inCheck());
      eval::ProgressEval eval(sstate.state());

      gen.init(limit, &record, eval, sstate.state(), true, Move());
      MoveGenerator gen2(gen);
      MoveLogProbVector search_moves;
      gen.generateAll(sstate.state().turn(), sstate, search_moves);

      // generate all by copy
      SearchState2 sstate_copy(sstate);
      MoveLogProbVector search_moves2;
      gen2.generateAll(sstate_copy.state().turn(), sstate_copy, search_moves2);

      BOOST_CHECK_EQUAL(search_moves, search_moves2);

      sstate.makeMove(moves[i]);
    }
  }  
}

BOOST_AUTO_TEST_CASE(MoveGeneratorTestAllMoves)
{
  eval::ProgressEval::setUp();
  MoveGenerator::initOnce();
  
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  int i=0;
  int count=100;
  if (OslConfig::inUnitTestShort()) 
    count=10;
  checkmate_t checkmate;
  std::string file_name;
  while((ifs >> file_name) && ++i<count)
  {
    if(file_name == "") 
      break;
    if (OslConfig::verbose())
      std::cerr << file_name << " ";
    auto record=CsaFileMinimal(OslConfig::testCsaFile(file_name)).load();
    NumEffectState state(record.initialState());
    std::vector<osl::Move> moves=record.moves;
    MoveGenerator gen;
    for(unsigned int i=0;i<moves.size();i++)
    {
      MoveVector all_moves;
      state.generateLegal(all_moves);

      SimpleHashRecord record;
      record.setInCheck(state.inCheck());
      eval::ProgressEval eval(state);
      SearchState2 sstate(state, checkmate);
      gen.init(2000, &record, eval, state, true, Move());
      MoveLogProbVector search_moves;
      gen.generateAll(state.turn(), sstate, search_moves);
      if (search_moves.size() > all_moves.size()) {
	typedef std::set<Move> set_t;
	set_t s, t;
	for (size_t j=0; j<search_moves.size(); ++j) {
	  if (! s.insert(search_moves[j].move()).second) {
	    std::cerr << "\n" << state;
	    std::cerr << "dup " << search_moves[j].move() << "\n";
	    BOOST_CHECK(search_moves.size() <= all_moves.size());
	  }
	}
	for (size_t j=0; j<all_moves.size(); ++j) {
	  assert(t.insert(all_moves[j]).second);
	}
	std::vector<Move> diff;
	std::set_difference(s.begin(), s.end(), t.begin(), t.end(), 
			    std::back_inserter(diff));
	for (size_t j=0; j<diff.size(); ++j) {
	  if (move_classifier::PlayerMoveAdaptor<move_classifier::PawnDropCheckmate>
	      ::isMember(state, diff[j]))
	    continue;
	  std::cerr << "\n" << state;
	  std::cerr << "not legal " << *search_moves.find(diff[j]) << "\n";
	  analyzer::CategoryMoveVector a;
	  MoveGenerator g2;
	  eval::ProgressEval eval(state);
	  gen.init(400, &record, eval, state, true, Move());
	  gen.generateAll(state.turn(), sstate, a);
	  for (analyzer::CategoryMoveVector::const_iterator p=a.begin(); p!=a.end(); ++p) {
	    std::cerr << p->category << "\n";
	    std::cerr << p->moves << "\n";
	  }
	  
	  BOOST_CHECK(search_moves.size() <= all_moves.size());
	}
      }
      if (all_moves.size() != search_moves.size()) {
	for (size_t j=0; j<all_moves.size(); ++j) {
	  const Move m = all_moves[j];
	  if (ShouldPromoteCut::canIgnoreAndNotDrop(m))
	    continue;
	  if (! search_moves.find(m)) {
	    std::cerr << m << "\n" << state;
	    std::cerr << search_moves;
	  }
	  BOOST_CHECK(search_moves.find(m));
	}
      }

      state.makeMove(moves[i]);
    }
  }  
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
