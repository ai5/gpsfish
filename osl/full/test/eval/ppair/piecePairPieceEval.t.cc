// piecePairPieceEval.t.cc
#include "osl/eval/ppair/piecePairPieceEval.h"
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/pieceEval.h"
#include "osl/container/pieceValues.h"
#include "../consistencyTest.h"
#include <boost/test/unit_test.hpp>
#include <iostream>

using namespace osl;

using namespace osl;
using namespace osl::eval;
using namespace osl::eval::ppair;

BOOST_AUTO_TEST_CASE(PiecePairPieceEvalTestLoad)
{
  BOOST_CHECK(PiecePairPieceEval::setUp());
}


BOOST_AUTO_TEST_CASE(PiecePairPieceEvalTestConsistentUpdate)
{
  consistencyTestUpdate<PiecePairPieceEval>();
}

BOOST_AUTO_TEST_CASE(PiecePairPieceEvalTestConsistentExpect)
{
  consistencyTestExpect<PiecePairPieceEval>();
}

BOOST_AUTO_TEST_CASE(PiecePairPieceEvalTestPieces)
{
  for (int y=1; y<=9; ++y)
  {
    for (int x=1; x<=9; ++x)
    {
      const Square position(x,y);
      for (int p=PTYPEO_MIN; p<PTYPEO_MAX; ++p)
      {
	const PtypeO ptypeo = static_cast<PtypeO>(p);
	const unsigned int index = PiecePairIndex::indexOf(position, ptypeo);
	const int value = PiecePairPieceTable::Table.valueOf(index, index);
	BOOST_CHECK((abs(value) <= 150+PtypeEvalTraits<GOLD>::val*4/5));
      }
    }
  }

}

BOOST_AUTO_TEST_CASE(PiecePairPieceEvalTestValues)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 10 : 100) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const auto moves=record.moves;

    NumEffectState state(record.initialState());
    PieceValues values;

    PiecePairPieceEval sum_eval(state);
    PiecePairPieceEval::setValues(state, values);
    
    // 1/2 の切り捨てがあるので合計が一致するとは限らない
    BOOST_CHECK((sum_eval.rawValue() - values.sum()) < 40);
    
    for (unsigned int i=0; i<moves.size(); i++)
    {
      const Move m = moves[i];
      state.makeMove(m);
      sum_eval.update(state, m);
      PiecePairPieceEval::setValues(state, values);

      BOOST_CHECK((sum_eval.rawValue() - values.sum()) < 40);
    }
  }
}

BOOST_AUTO_TEST_CASE(PiecePairPieceEvalTestSum)
{
  std::ifstream ifs(OslConfig::testCsaFile("FILES"));
  BOOST_CHECK(ifs);
  std::string file_name;

  const int tolerance = (40*39/2)*128/100+PtypeEvalTraits<GOLD>::val*4/5*4; // cumulative errors
  int large_errors = 0;
  for (int i=0;i<(OslConfig::inUnitTestShort() ? 20 : 400) && (ifs >> file_name) ; i++)
  {
    if (file_name == "") 
      break;
    file_name = OslConfig::testCsaFile(file_name);

    const RecordMinimal record=CsaFileMinimal(file_name).load();
    const auto& moves=record.moves;

    NumEffectState state(record.initialState());
    PieceEval piece_eval(state);
    PiecePairRawEval raw_eval(state);
    PiecePairPieceEval sum_eval(state);

    BOOST_CHECK(abs(piece_eval.value() + raw_eval.value() 
		       - sum_eval.value()) < 5);
    for (unsigned int i=0; i<moves.size(); i++)
    {
      const Move m = moves[i];
      state.makeMove(m);
      piece_eval.update(state, m);
      raw_eval.update(state, m);
      sum_eval.update(state, m);

      const int expected = piece_eval.value() + (raw_eval.rawValue()*128/100)
	+ PiecePairPieceEval::standBonus(state);
      const int actual = sum_eval.rawValue();
      if (abs(expected - actual) >= tolerance)
      {
	++large_errors;
	std::cerr << expected - actual << "\n";
	if (OslConfig::verbose())
	{
	  std::cerr << state << "\n";
	  std::cerr << piece_eval.value() 
		    << " + " << raw_eval.rawValue()*128/100
		    << " (" << raw_eval.rawValue() << ")"
		    << " != " << sum_eval.rawValue();
	  PieceValues values;
	  PiecePairPieceEval::setValues(state, values);
	  values.showValues(std::cerr, state);
	  PiecePairRawEval::setValues(state, values);
	  values.showValues(std::cerr, state);
	}
      }
      BOOST_CHECK(abs(expected - actual) < tolerance);
    }
  }
  BOOST_CHECK(large_errors < 10);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
