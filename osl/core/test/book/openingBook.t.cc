#include "osl/book/openingBook.h"
#include "osl/book/compactBoard.h"
#include "osl/csa.h"
#include "osl/oslConfig.h"
#include <boost/test/unit_test.hpp>

using osl::OslConfig;

BOOST_AUTO_TEST_CASE(OpeningBookTestOpen)
{
  osl::book::WeightedBook book(osl::OslConfig::openingBook());
}

BOOST_AUTO_TEST_CASE(OpeningBookTestGetMoves)
{
  osl::book::WeightedBook book(osl::OslConfig::openingBook());
  auto moves = book.moves(book.startState());
  auto moves_nonzero = book.moves(book.startState(), false);
  for (const auto& move: moves_nonzero)
  {
    BOOST_CHECK(move.weight > 0);
  }
  // moves usually include a (or more) zero weighted move. 
  BOOST_CHECK(moves_nonzero.size() < moves.size());
}

BOOST_AUTO_TEST_CASE(OpeningBookTestMove)
{
  std::ifstream ifs(osl::OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  const int max=OslConfig::inUnitTestShort() ? 10 : 100;
  std::string filename;
  int i=0;
  while((ifs >> filename) && (++i<max)) 
  {
    std::string fullFileName = osl::OslConfig::testCsaFile(filename);

    auto record=osl::CsaFileMinimal(fullFileName).load();
    const auto moves=record.moves;
    for (size_t i=0; i<moves.size(); ++i)
    {
      const osl::Move& m = moves[i];
      osl::book::OMove om(m);
      BOOST_CHECK_EQUAL(m.from(), om.from());
      BOOST_CHECK_EQUAL(m.to(), om.to());
      BOOST_CHECK_EQUAL(m.isPromotion(), om.isPromotion());
      BOOST_CHECK_EQUAL(m.capturePtype(), om.capturePtype());
      BOOST_CHECK_EQUAL(m.ptype(), om.ptype());
      BOOST_CHECK_EQUAL(m.player(), om.player());
      BOOST_CHECK_EQUAL(m, om.operator osl::Move());
    }
  }
}

BOOST_AUTO_TEST_CASE(OpeningBookTestCompactBoard)
{
  std::ifstream ifs(osl::OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  const int max=OslConfig::inUnitTestShort() ? 10 :100;
  std::string filename;
  int i=0;
  while((ifs >> filename) && (++i<max)) 
  {
    std::string full_filename = osl::OslConfig::testCsaFile(filename);

    auto record=osl::CsaFileMinimal(full_filename).load();
    const auto moves=record.moves;
    osl::NumEffectState state(record.initialState());
    // This doesn't test initial state itself but oh well...
    for (size_t i=0; i<moves.size(); ++i)
    {
      const osl::Move& m = moves[i];
      state.makeMove(m);
      for (int p_index = 0; p_index < osl::Piece::SIZE; p_index++)
      {
	osl::Piece piece = state.pieceOf(p_index);
	osl::book::OPiece opiece(piece);
	BOOST_CHECK_EQUAL(piece.square(), opiece.square());
	BOOST_CHECK_EQUAL(piece.ptype(), opiece.ptype());
	BOOST_CHECK_EQUAL(piece.owner(), opiece.owner());
      }
      osl::book::CompactBoard board(state);
      BOOST_CHECK_EQUAL(static_cast<const osl::SimpleState&>(state), board.state());

      std::ostringstream oss;
      osl::book::CompactBoard reconstructed_board;
      oss << board;
      std::istringstream iss(oss.str());
      iss >> reconstructed_board;
      BOOST_CHECK_EQUAL(board.state(), reconstructed_board.state());
    }
  }
}

BOOST_AUTO_TEST_CASE(OpeningBookTestCompactBoardOrder)
{
  std::ifstream ifs(osl::OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  const int max=OslConfig::inUnitTestShort() ? 10 : 100;
  std::string filename;
  int i=0;
  while((ifs >> filename) && (++i<max)) 
  {
    std::string full_filename = osl::OslConfig::testCsaFile(filename);

    auto record=osl::CsaFileMinimal(full_filename).load();
    const auto moves=record.moves;
    osl::NumEffectState state(record.initialState());
    // This doesn't test initial state itself but oh well...
    for (size_t i=0; i<moves.size(); ++i)
    {
      const osl::Move& m = moves[i];
      state.makeMove(m);
      osl::book::CompactBoard board(state);

      std::ostringstream oss;
      oss << board;
      // test uniquness of CompactBoard representationg for given state.
      const std::string s = oss.str();
      char str[s.length() + 1];
      s.copy(str, s.length());
      str[s.length()] = '\0';
      // Swap pieces
      for (int i = 0; i < 1; i++)
      {
	for (int j = 0; j < 4; j++)
	{
	  char tmp = str[i * 4 + j];
	  str[i * 4 + j] = str[(40 - 1 - i) * 4 + j];
	  str[(40 - 1 - i) * 4 + j] = tmp;
	}
       }
      std::string board_string(str, s.length());
      std::istringstream iss(board_string);
      osl::book::CompactBoard tmp_board;
      iss >> tmp_board;
      // We need to do SimpleState -> CompactBoard convesion because
      // that's where sorting happens.
      osl::book::CompactBoard reconstructed_board(tmp_board.state());
      std::ostringstream oss2;
      oss2 << reconstructed_board;
      BOOST_CHECK_EQUAL(oss.str().size(), oss2.str().size());
      BOOST_REQUIRE_EQUAL(oss.str(), oss2.str());
    }
  }
}
// ;;; local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
