#include "osl/game_playing/csaClient.h"
#include "osl/game_playing/recordTracer.h"
#include "osl/game_playing/bookPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/game_playing/csaLogger.h"

#include <boost/test/unit_test.hpp>
#include <sstream>

using namespace osl;
using namespace osl::game_playing;

BOOST_AUTO_TEST_CASE(CsaClientTestUndo) 
{
  std::vector<Move> moves;
  const Move m76fu(Square(7,7),Square(7,6),PAWN,PTYPE_EMPTY,false,BLACK);
  const Move m34fu(Square(3,3),Square(3,4),PAWN,PTYPE_EMPTY,false,WHITE);
  moves.push_back(m76fu);
  moves.push_back(m34fu);
    
  BookPlayer white(new RecordTracer(moves), new ResignPlayer());
  std::ostringstream log;
  CsaLogger *logger = new CsaLogger(log);
  std::istringstream is("+7776FU\n%TORYO\n");
  std::ostringstream os;

  CsaClient client(0, &white, logger, is, os);
  client.setSilent(true);
  client.run();
  BOOST_CHECK_EQUAL(std::string("-3334FU\n"), os.str());
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
