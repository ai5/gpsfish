/* consistencyTest.h
 */
#ifndef TEST_EVAL_CONSISTENCYTEST_H
#define TEST_EVAL_CONSISTENCYTEST_H

#include "osl/numEffectState.h"
#include "osl/csa.h"
#include "osl/effect_util/effectUtil.h"
#include "osl/effect_util/neighboring8Direct.h"
#include "osl/progress/effect5x3.h"
#include "osl/bits/centering5x3.h"
#include "osl/centering3x3.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <fstream>
#include <string>

namespace osl
{
  inline bool quiet_position(int progress_b, int progress_w, const NumEffectState& state, Move m) 
  {
    if (state.inCheck() )
      return false;
#if 0
    if (progress_b + progress_w < 13 && m.capturePtype() == PTYPE_EMPTY
	&& std::max(progress_b, progress_w) < 8)
      return true;
#endif
    if (m.capturePtype() != PTYPE_EMPTY
	&& isMajor(m.capturePtype()))
      return false;
    if (std::max(progress_b, progress_w) > 10 && m.capturePtype() != PTYPE_EMPTY)
      return false;
    const Square black_king = Centering3x3::adjustCenter(state.kingSquare<BLACK>());
    const Square white_king = Centering3x3::adjustCenter(state.kingSquare<WHITE>());
    const Square black_king_c = Centering5x3::adjustCenter(black_king);
    const Square white_king_c = Centering5x3::adjustCenter(white_king);
    const Square to = m.to();
    if (abs(black_king_c.x() - to.x()) < 4
	&& abs(black_king_c.y() - to.y()) < 3)
      return false;
    if (abs(white_king_c.x() - to.x()) < 4
	&& abs(white_king_c.y() - to.y()) < 3)
      return false;
    if (Neighboring8Direct::hasEffect(state, m.ptypeO(), to, black_king)
	|| Neighboring8Direct::hasEffect(state, m.ptypeO(), to, white_king))
	return false;
    if (Neighboring8Direct::hasEffect(state, m.capturePtypeOSafe(), to, black_king)
	|| Neighboring8Direct::hasEffect(state, m.capturePtypeOSafe(), to, white_king))
	return false;
    if (Neighboring8Direct::hasEffect(state, m.ptypeO(), to, black_king_c)
	|| Neighboring8Direct::hasEffect(state, m.ptypeO(), to, white_king_c))
	return false;
    // todo: sokofu
    return true;
  }
}

template <class Eval>
void consistencyTestExpect(int threshold=0)
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<10 && (ifs >> file_name) ; i++)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const std::vector<osl::Move> moves=record.moves;

    NumEffectState state(record.initialState());
    Eval eval(state);
    
    for (unsigned int i=0; i<moves.size(); i++) {
      const Move m = moves[i];
      const int value = eval.expect(state, m);
      const int progress_b = progress::Effect5x3(state).progress16(BLACK).value();
      const int progress_w = progress::Effect5x3(state).progress16(WHITE).value();
      const bool in_check = state.inCheck();
      state.makeMove(m);
      eval.update(state, m);	// update with new state
      if (threshold == 0 
	  || (! in_check && quiet_position(progress_b, progress_w, state, m))){
	if ((m.player() == BLACK && eval.value() > value + threshold)
	    || (m.player() == WHITE && eval.value() < value - threshold))
	  std::cerr << "\n" << state << std::endl << progress_b << " " << progress_w
		    << " " << m << " " << eval.value() << " " << value << std::endl;
	BOOST_CHECK((m.player() == BLACK && eval.value() <= value + threshold)
		       || (m.player() == WHITE && eval.value() >= value - threshold));
      }
    }
  }
}

template <class Eval>
void consistencyTestUpdate()
{
  using namespace osl;

  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 100 : 900) && (ifs >> file_name) ; i++)
  {
    if ((i % 100) == 0)
      std::cerr << '.';
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const std::vector<osl::Move> moves=record.moves;

    NumEffectState state(record.initialState());
    Eval eval(state);
    
    for (unsigned int i=0; i<moves.size(); i++)
    {
      const Move m = moves[i];
      state.makeMove(m);
      eval.update(state, m);	// update with new state
      const Eval new_eval(state);
      if (new_eval.value() != eval.value()) {
	std::cerr << state << std::endl << m << std::endl;
      }
      BOOST_CHECK_EQUAL(new_eval.value(), eval.value());
    }
  }
}


#endif /* TEST_EVAL_CONSISTENCYTEST_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
