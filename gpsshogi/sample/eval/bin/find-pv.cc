#include "pvFile.h"
#include "pvVector.h"
#include "osl/numEffectState.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/record/kisen.h"
#include "osl/eval/pieceEval.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using namespace osl;
using namespace gpsshogi;

class StatePredicate
{
public:
  StatePredicate() { }
  virtual ~StatePredicate() { }
  virtual bool match(const osl::NumEffectState &state,
		     const osl::NumEffectState &original_state,
		     const PVVector&) const
  {
    return false;
  }
  virtual bool isLoaded() const { return false; }
};

class KingUpPredicate : public StatePredicate
{
private:
  bool match(const osl::NumEffectState &state,
	     osl::Player player) const
  {
    const osl::Square king = state.kingSquare(player);
    if ((player == osl::BLACK && king.y() == 8) ||
	(player == osl::WHITE && king.y() == 2))
    {
      const osl::Square up = osl::Board_Table.nextSquare(player,
							     king, osl::U);
      const osl::Piece p = state.pieceAt(up);
      return (!p.isEmpty() && p.owner() == alt(player) &&
	      state.countEffect(player, up) >= 2 &&
	      state.countEffect(alt(player), up) == 0);
    }
    return false;
  }
public:
      KingUpPredicate() : StatePredicate() { }
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector&) const
  {
    return match(state, osl::BLACK) || match(state, osl::WHITE);
  }
};

class AnyPredicate : public StatePredicate
{
public:
  AnyPredicate() : StatePredicate() { }
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector&) const
  {
    return true;
  }
};

class LengthPredicate : public StatePredicate
{
  size_t length;
public:
  LengthPredicate(size_t l) : length(l) { }
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector& moves) const
  {
    return moves.size() >= length;
  }
};

class InCheckPredicate : public StatePredicate
{
public:
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector&) const
  {
    return state.inCheck();
  }
};

class NoPromotePredicate : public StatePredicate
{
public:
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector& moves) const
  {
    for (size_t i=0; i<moves.size(); ++i)
      if (osl::ShouldPromoteCut::canIgnoreAndNotDrop(moves[i]))
	return true;
    return false;
  }
};

class PawnPtype3Predicate : public StatePredicate
{
public:
  PawnPtype3Predicate() : StatePredicate() { }
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector&) const
  {
    for (int x = 1; x <= 9; ++x)
    {
      const Piece p3 = state.pieceAt(Square(x, 3));
      const Piece p2 = state.pieceAt(Square(x, 2));
      const Piece p1 = state.pieceAt(Square(x, 1));

      const Piece p7 = state.pieceAt(Square(x, 7));
      const Piece p8 = state.pieceAt(Square(x, 8));
      const Piece p9 = state.pieceAt(Square(x, 9));

      if ((!p3.isEmpty() && p3.owner() == BLACK && p3.ptype() == PAWN &&
	   !p2.isEmpty() && p2.owner() == WHITE && p2.ptype() == PAWN &&
	   !p1.isEmpty() && p1.owner() == WHITE && p1.ptype() == LANCE) ||
	  (!p7.isEmpty() && p7.owner() == WHITE && p7.ptype() == PAWN &&
	   !p8.isEmpty() && p8.owner() == BLACK && p8.ptype() == PAWN &&
	   !p9.isEmpty() && p9.owner() == BLACK && p9.ptype() == LANCE))
      {
	return true;
      }
    }
    return false;
  }
};

class PawnPtype7Predicate : public StatePredicate
{
public:
  PawnPtype7Predicate() : StatePredicate() { }
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector&) const
  {
    for (int x = 1; x <= 9; ++x)
    {
      const Piece p7 = state.pieceAt(Square(x, 7));
      const Piece p6 = state.pieceAt(Square(x, 6));
      const Piece p5 = state.pieceAt(Square(x, 5));

      const Piece p3 = state.pieceAt(Square(x, 3));
      const Piece p4 = state.pieceAt(Square(x, 4));

      if ((!p7.isEmpty() && p7.owner() == BLACK && p7.ptype() == PAWN &&
	   !p6.isEmpty() && p6.owner() == WHITE && p6.ptype() == PAWN &&
	   !p5.isEmpty() && p5.owner() == WHITE && p5.ptype() == LANCE) ||
	  (!p3.isEmpty() && p3.owner() == WHITE && p3.ptype() == PAWN &&
	   !p4.isEmpty() && p4.owner() == BLACK && p4.ptype() == PAWN &&
	   !p5.isEmpty() && p5.owner() == BLACK && p5.ptype() == LANCE))
      {
	return true;
      }
    }
    return false;
  }
};

class RookEffectPredicateP37 : public StatePredicate
{
public:
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector &) const
  {
    for (int i = PtypeTraits<ROOK>::indexMin;
	 i < PtypeTraits<ROOK>::indexLimit; ++i)
    {
      const Piece rook = state.pieceOf(i);
      const Square pos = state.mobilityOf(rook.owner() == BLACK ? D : U, i);
      if (pos.isOnBoard())
      {
	const Piece p = state.pieceAt(pos);
	const Square king = state.kingSquare(alt(rook.owner()));
	if (p.ptype() == PAWN && p.owner() == alt(rook.owner()) &&
	    std::abs(king.y() - pos.y()) == 7 &&
	    std::abs(king.x() - pos.x()) == 3)
	{
	  return true;
	}
      }
    }
    return false;
  }
};

class BishopEffectPredicateR55 : public StatePredicate
{
private:
  bool match(const NumEffectState &state,
	     Piece bishop,
	     Square pos) const
  {
    if (pos.isOnBoard())
    {
      const Piece p = state.pieceAt(pos);
      const Square king = state.kingSquare(alt(bishop.owner()));
      if (p.ptype() == ROOK && p.owner() == alt(bishop.owner()) &&
	  ((bishop.owner() == BLACK && king.y() - pos.y() == -5) ||
	   (bishop.owner() == WHITE && king.y() - pos.y() == 5)) &&
	  std::abs(king.x() - pos.x()) == 5)
      {
	return true;
      }
    }
    return false;
  }
public:
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector &) const
  {
    for (int i = PtypeTraits<BISHOP>::indexMin;
	 i < PtypeTraits<BISHOP>::indexLimit; ++i)
    {
      const Piece bishop = state.pieceOf(i);
      const Square pos1 =
	state.mobilityOf(bishop.owner() == BLACK ? UL : DR, i);
      const Square pos2 =
	state.mobilityOf(bishop.owner() == BLACK ? DR : UL, i);
      if (match(state, bishop, pos1) ||
	  match(state, bishop, pos2))
      {
	return true;
      }
    }
    return false;
  }
};

class NoRecapturePredicate : public StatePredicate
{
public:
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector &v) const
  {
    if (v.empty())
      return false;
    const int see =
      osl::eval::PieceEval::computeDiffAfterMoveForRP(original_state, v[0]);
    if (see >= 0)
      return false;

    for (size_t i = 1; i < v.size(); ++i)
    {
      if (v[i].to() == v[0].to())
	return false;
    }
    return true;
  }
};

class KingSquarePredicate : public StatePredicate
{
public:
  KingSquarePredicate(int x, int y) : king_position(x, y)
  {
  }
  bool match(const osl::NumEffectState &state,
	     const osl::NumEffectState &original_state,
	     const PVVector &v) const
  {
    const Player P = state.turn();
    return state.kingSquare(P).squareForBlack(P)
      == king_position;
  }
private:
  Square king_position;
};

int main(int argc, char **argv)
{
  std::string kisen_filename, predicate_name;
  std::vector<std::string> pv_filenames;
  bool show_record_move, quiet;
  int pv_length, king_position;

  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("predicate",
     boost::program_options::value<std::string>(&predicate_name)->
     default_value("king_up"),
     "Predicate to use.  Valid options are king_up, any, in_check, "
     "no_promote, pawnptype3 and pawnptype7")
    ("pv-length",
     boost::program_options::value<int>(&pv_length)->
     default_value(0),
     "If positive, use LengthPredicate + this length.")
    ("pv-file", po::value<std::vector<std::string> >(),
     "filename containing PVs")
    ("kisen-file,k",
     po::value<std::string>(&kisen_filename)->default_value(""),
     "Kisen filename corresponding to pv file")
    ("king-position",
     po::value<int>(&king_position)->default_value(0),
     "King position to match")
    ("show-record-move",
     po::value<bool>(&show_record_move)->default_value(false),
     "show record move in addition to position when predicate matches")
    ("quiet,q",
     po::value<bool>(&quiet)->default_value(false),
     "counting only.  do not show positions matched.")
    ;
  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, command_line_options), vm);
    po::notify(vm);
    if (vm.count("pv-file"))
      pv_filenames = vm["pv-file"].as<std::vector<std::string> >();
    else
    {
      std::cerr << "PV file wasn't specified" << std::endl;
      return 1;
    }
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  osl::record::KisenFile kisen(kisen_filename);


  std::unique_ptr<StatePredicate> predicate;
  if (pv_length > 0) 
  {
    predicate.reset(new LengthPredicate(pv_length));
  }
  else if (king_position > 0)
  {
    predicate.reset(new KingSquarePredicate(king_position / 10, king_position % 10));
  }
  else if (predicate_name == "king_up")
  {
    predicate.reset(new KingUpPredicate());
  }
  else if (predicate_name == "in_check")
  {
    predicate.reset(new InCheckPredicate());
  }
  else if (predicate_name == "no_promote")
  {
    predicate.reset(new NoPromotePredicate());
  }
  else if (predicate_name == "any")
  {
    predicate.reset(new AnyPredicate());
  }
  else if (predicate_name == "pawnptype3")
  {
    predicate.reset(new PawnPtype3Predicate());
  }
  else if (predicate_name == "pawnptype7")
  {
    predicate.reset(new PawnPtype7Predicate());
  }
  else if (predicate_name == "rookp37")
  {
    predicate.reset(new RookEffectPredicateP37());
  }
  else if (predicate_name == "bishopr55")
  {
    predicate.reset(new BishopEffectPredicateR55());
  }
  else if (predicate_name == "norecapture")
  {
    predicate.reset(new NoRecapturePredicate());
  }
  else
  {
    std::cerr << "Unknown predicate "  << predicate_name;
    return 1;
  }

  std::vector<osl::Move> moves;
  osl::NumEffectState state(kisen.initialState());
  size_t matched = 0, num_positions=0, num_pv=0;
  for (size_t i = 0; i < pv_filenames.size(); ++i)
  {
    gpsshogi::PVFileReader pr(pv_filenames[i].c_str());
    int record, position;
    int cur_record = -1;
    int cur_position = 0;
    while (pr.newPosition(record, position))
    {
      ++num_positions;
      if (record != cur_record)
      {
	cur_record = record;
	moves = kisen.moves(cur_record);
	cur_position = 0;
	state = NumEffectState(kisen.initialState());
      }
      assert(position > cur_position || cur_position == 0);
      while (position > cur_position)
      {
	state.makeMove(moves[cur_position]);
	++cur_position;
      } 
      PVVector pv;
      while (pr.readPv(pv))
      {
	++num_pv;
	osl::NumEffectState current_state(state);
	for (size_t j = 0; j < pv.size(); ++j)
	{
	  current_state.makeMove(pv[j]);
	}
	if (predicate->match(current_state, state, pv))
	{
	  if (! quiet)
	  {
	    std::cout << cur_record << " " << cur_position << std::endl;
	    if (show_record_move && cur_position < (int)moves.size())
	      std::cout << " next record move " << moves[cur_position] << std::endl;
	    std::cout << state;
	    for (size_t j = 0; j < pv.size(); ++j)
	    {
	      std::cout << pv[j] << " ";
	    }
	    std::cout << pv.size() << std::endl;
	    std::cout << current_state;
	  }
	  ++matched;
	}
	pv.clear();
      }
    }
  }
  std::cout << "match " << matched << "\n";
  std::cout << "positions " << num_positions << "\n";
  std::cout << "pv " << num_pv << "\n";
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
