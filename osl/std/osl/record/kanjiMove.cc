#include "osl/record/kanjiMove.h"
#include "osl/record/kanjiCode.h"
#include "osl/record/kanjiPrint.h"
#include "osl/misc/eucToLang.h"
#include <algorithm>
#include <iterator>
#include <iostream>

namespace 
{
int moveFromX(const osl::Move& move)
{
  const osl::Square& p = move.from();
  return p.x();
}

int moveFromY(const osl::Move& move)
{
  const osl::Square& p = move.from();
  return p.y();
}

struct SortMoveFromX : 
  public std::binary_function<osl::Move, osl::Move, bool>
{
  bool operator()(const osl::Move& a, const osl::Move& b) const
  {
    const osl::Square& a_p = a.from();
    const osl::Square& b_p = b.from();
    return a_p.x() < b_p.x();
  }
};

struct SortMoveFromXDesc :
  public std::binary_function<osl::Move, osl::Move, bool>
{
  bool operator()(const osl::Move& a, const osl::Move& b) const
  {
    const osl::Square& a_p = a.from();
    const osl::Square& b_p = b.from();
    return a_p.x() > b_p.x();
  }
};

struct SortMoveFromY :
  public std::binary_function<osl::Move, osl::Move, bool>
{
  bool operator()(const osl::Move& a, const osl::Move& b) const
  {
    const osl::Square& a_p = a.from();
    const osl::Square& b_p = b.from();
    return a_p.y() < b_p.y();
  }
};

struct SortMoveFromYDesc :
  public std::binary_function<osl::Move, osl::Move, bool>
{
  bool operator()(const osl::Move& a, const osl::Move& b) const
  {
    const osl::Square& a_p = a.from();
    const osl::Square& b_p = b.from();
    return a_p.y() > b_p.y();
  }
};

struct RemoveMoveFromXOver :
  public std::unary_function<osl::Move, bool>
{
  const int min_x;
  RemoveMoveFromXOver(const int min_x)
    : min_x(min_x)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.x() > min_x;
  }
};

struct RemoveMoveFromXGTE :
  public std::unary_function<osl::Move, bool>
{
  const int min_x;
  RemoveMoveFromXGTE(const int min_x)
    : min_x(min_x)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.x() >= min_x;
  }
};

struct RemoveMoveFromYOver :
  public std::unary_function<osl::Move, bool>
{
  const int min_y;
  RemoveMoveFromYOver(const int min_y)
    : min_y(min_y)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.y() > min_y;
  }
};

struct RemoveMoveFromYGTE :
  public std::unary_function<osl::Move, bool>
{
  const int min_y;
  RemoveMoveFromYGTE(const int min_y)
    : min_y(min_y)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.y() >= min_y;
  }
};

struct RemoveMoveFromXUnder :
  public std::unary_function<osl::Move, bool>
{
  const int max_x;
  RemoveMoveFromXUnder(const int max_x)
    : max_x(max_x)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.x() < max_x;
  }
};

struct RemoveMoveFromXLTE :
  public std::unary_function<osl::Move, bool>
{
  const int max_x;
  RemoveMoveFromXLTE(const int max_x)
    : max_x(max_x)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.x() <= max_x;
  }
};

struct RemoveMoveFromYUnder :
  public std::unary_function<osl::Move, bool>
{
  const int max_y;
  RemoveMoveFromYUnder(const int max_y)
    : max_y(max_y)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.y() < max_y;
  }
};

struct RemoveMoveFromYLTE :
  public std::unary_function<osl::Move, bool>
{
  const int max_y;
  RemoveMoveFromYLTE(const int max_y)
    : max_y(max_y)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.y() <= max_y;
  }
};

struct RemoveMoveFromXEqual :
  public std::unary_function<osl::Move, bool>
{
  const int x;
  RemoveMoveFromXEqual(const int x)
    : x(x)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.x() == x;
  }
};

struct RemoveMoveFromYEqual :
  public std::unary_function<osl::Move, bool>
{
  const int y;
  RemoveMoveFromYEqual(const int y)
    : y(y)
  {}

  bool operator()(const osl::Move& m) const
  {
    const osl::Square& p = m.from();
    return p.y() == y;
  }
};
} // anonymous namespace

osl::record::
KanjiMove::KanjiMove()
  : verbose(false)
{
  for (size_t x=1; x<=9; ++x)
  {
    for (size_t y=1; y<=9; ++y)
    {
      const std::string str = StandardCharacters::suji[x] + 
                              StandardCharacters::dan[y];
      str2position[str] = Square(x,y);
    }
  }
  str2piece[K_PAWN]      = PAWN;
  str2piece[K_PPAWN]     = PPAWN;
  str2piece[K_LANCE]     = LANCE;
  str2piece[K_PLANCE_D]  = PLANCE;
  str2piece[K_KNIGHT]    = KNIGHT;
  str2piece[K_PKNIGHT_D] = PKNIGHT;
  str2piece[K_SILVER]    = SILVER;
  str2piece[K_PSILVER_D] = PSILVER;
  str2piece[K_GOLD]      = GOLD;
  str2piece[K_BISHOP]    = BISHOP;
  str2piece[K_PBISHOP]   = PBISHOP;
  str2piece[K_ROOK]      = ROOK;
  str2piece[K_PROOK]     = PROOK;
  str2piece[K_PROOK2]    = PROOK;
  str2piece[K_KING]      = KING;
  str2piece[K_KING2]     = KING;

  // pieces in kakinoki-style board specification
  str2piece[K_PLANCE]  = PLANCE;
  str2piece[K_PKNIGHT] = PKNIGHT;
  str2piece[K_PSILVER] = PSILVER;
}

osl::record::
KanjiMove::~KanjiMove()
{
}

osl::Square osl::record::
KanjiMove::toSquare(const std::string& s) const
{
  str2position_t::const_iterator p=str2position.find(s);
  if (p == str2position.end())
    return Square();
  return p->second;
}

osl::Ptype osl::record::
KanjiMove::toPtype(const std::string& s) const
{
  str2piece_t::const_iterator p=str2piece.find(s);
  if (p == str2piece.end())
    return Ptype();
  return p->second;
}

void osl::record::
KanjiMove::selectCandidates(found_moves_t& found, 
                            std::string& str,
                            const osl::Square& to_pos,
                            const osl::Player& player) const
{
  assert(!str.empty());
  assert(found.size() >= 2);

  if ( (str.substr(0,2) == K_MIGI && player == BLACK) ||
       (str.substr(0,2) == K_HIDARI && player == WHITE) )
  {
    found.sort([](Move l, Move r){ return moveFromX(l) < moveFromX(r); });
    const osl::Move min = found.front();
    found.remove_if( RemoveMoveFromXOver(min.from().x()) ); // list really removes
  } 
  else if ( (str.substr(0,2) == K_HIDARI && player == BLACK) || 
            (str.substr(0,2) == K_MIGI   && player == WHITE) )
  {
    found.sort([](Move l, Move r){ return moveFromX(l) < moveFromX(r); });
    const Move max = found.back();
    found.remove_if( RemoveMoveFromXUnder(max.from().x()) ); // list really removes
  }
  else if ( (str.substr(0,2) == K_SHITA && player == BLACK) || 
            (str.substr(0,2) == K_UE    && player == WHITE) )
  {
    found.sort([](Move l, Move r){ return moveFromY(l) < moveFromY(r); });
    const Move min = found.front();
    found.remove_if( RemoveMoveFromYOver(min.from().y()) ); // list really removes
  }
  else if ( (str.substr(0,2) == K_UE    && player == BLACK) || 
            (str.substr(0,2) == K_SHITA && player == WHITE) )
  {
    found.sort([](Move l, Move r){ return moveFromY(l) > moveFromY(r); });
    const Move max = found.front();
    found.remove_if( RemoveMoveFromYUnder(max.from().y()) ); // list really removes
  }
  else if (str.substr(0,2) == K_YORU)
  {
    found.remove_if( std::not1(RemoveMoveFromYEqual(to_pos.y())) ); // list really removes
  }
  else if (str.substr(0,2) == K_SUGU && player == WHITE)
  {
    found.remove_if( std::not1(RemoveMoveFromXEqual(to_pos.x())) ); // or
    found.remove_if( std::not1(RemoveMoveFromYEqual(to_pos.y()-1)) ); // list really removes
  }
  else if (str.substr(0,2) == K_SUGU && player == BLACK)

  {
    found.remove_if( std::not1(RemoveMoveFromXEqual(to_pos.x())) ); // or
    found.remove_if( std::not1(RemoveMoveFromYEqual(to_pos.y()+1)) ); // list really removes
  }
  else if (str.substr(0,2) == K_HIKU && player == BLACK)
  {
    found.remove_if( RemoveMoveFromYGTE(to_pos.y()) ); // list really removes
  }
  else if (str.substr(0,2) == K_HIKU && player == WHITE)
  {
    found.remove_if( RemoveMoveFromYLTE(to_pos.y()) ); // list really removes
  }   
  else if (str.substr(0,2) == K_YUKU && player == BLACK)
  {
    found.remove_if( RemoveMoveFromYLTE(to_pos.y()) ); // list really removes
  }
  else if (str.substr(0,2) == K_YUKU && player == WHITE)
  {
    found.remove_if( RemoveMoveFromYGTE(to_pos.y()) ); // list really removes
  }

  str.erase(0,2);
  assert(!found.empty());

  if (found.size() > 1)
  {
    assert(!str.empty());
    selectCandidates(found, str, to_pos, player);
  }

  assert(found.size() == 1);
  if (!str.empty())
    std::cerr << "WARNING: A single candidate is selected, but the input string still has some characters: " << misc::eucToLang(str) << std::endl;
}

const osl::Move osl::record::
KanjiMove::strToMove(const std::string& orig, 
                     const osl::NumEffectState& state, 
                     const osl::Move& last_move) const
{
  std::string str(orig);
  if (str.find(K_RESIGN) != str.npos)
    return Move();
  assert(str.size() >= 4*2
	 || (str.size() >= 3*2
	     && (str.substr(2,2) == K_ONAZI
		 || (isdigit(str[2]) && isdigit(str[3])))));
  const Player player = str.substr(0,2) == K_BLACK_SIGN ? BLACK : WHITE;
  assert(player == state.turn());
  str.erase(0,2);

  Square to_pos;
  if (str.substr(0,2) == K_ONAZI)
  {
    to_pos = last_move.to();
    str.erase(0,2);
    if (str.substr(0,2) == K_SPACE)
      str.erase(0,2);
  }
  else if (isdigit(str[0]) && isdigit(str[1]))
  {
    to_pos = Square(str[0]-'0', str[1]-'0');
    str.erase(0,2);
  }
  else
  {
    to_pos = toSquare(str.substr(0,4));
    str.erase(0,4);
  }

  Ptype ptype;
  if (str.substr(0,2) == K_NARU) // PLANCE, PKIGHT, PSILVER
  {
    ptype = toPtype(str.substr(0,4));
    str.erase(0,4);
  }
  else
  {
    ptype = toPtype(str.substr(0,2));
    str.erase(0,2);
  }

  // promote or not
  bool is_promote = false;
  if (str.size() >= 4 && str.substr(0,4) == K_FUNARI)
    str.erase(0,4);
  else if (str.size() >= 4 && str.substr(str.size()-4,4) == K_FUNARI)
    str.erase(str.size()-4,4);
  else if (str.size() >= 2 && str.substr(0,2) == K_NARU)
  {
    is_promote = true;
    str.erase(0,2);
  }
  else if (str.size() >= 2 && str.substr(str.size()-2,2) == K_NARU)
  {
    is_promote = true;
    str.erase(str.size()-2,2);
  }

  MoveVector moves;
  state.generateWithFullUnpromotions(moves);
  found_moves_t found;
  for (Move move: moves)
  {
    if (move.oldPtype()  == ptype  &&
        move.to()        == to_pos &&
        move.isPromotion() == is_promote)
    {
      /** eliminate duplicate moves */
      if (std::find(found.begin(), found.end(), move) == found.end())
        found.push_back(move);
    }
  }
  if (verbose)
  {
    std::cerr << "\n" << orig << "\n" << state;
    std::cerr << "remain: " << str  << " (" << str.size() << " bytes)\n";
    std::cerr << "promote: " << is_promote << "\n";
    std::cerr << "ptype: " << ptype << "\n";
    std::cerr << "to_position: " << to_pos << "\n";
    std::cerr << "candidates: " << found.size() << std::endl;
    if (found.size() >=2) {
      for (const Move move: found) {
        std::cerr << "            " << move << std::endl;
      }
    }
  }
  if (found.empty()) {
    // there is no leagal move
    return Move::INVALID();
  }
  assert(!found.empty());

  // Single candidate
  if (found.size() == 1)
    return found.front();

  // Multiple candidates
  assert(found.size() >= 2);

  // drop
  if (str.substr(0,2) == K_UTSU)
  {
    found_moves_t::iterator it = 
      std::find_if(found.begin(), found.end(), [](Move m){ return m.isDrop(); });
    str.erase(0,2);
    assert(str.empty());
    assert(it != found.end());
    return *it;
  }
  else
  {
    found.remove_if([](Move m){ return m.isDrop(); }); // list really removes
    if (found.size() == 1)
      return found.front();
  }

  // Multiple candidates
  assert(found.size() >= 2);
  if (str.empty())
    return Move();
  assert(!str.empty());
  selectCandidates(found, str, to_pos, player);
  assert(found.size() == 1);
  return found.front();
}

const osl::record::KanjiMove& osl::record::
KanjiMove::instance() 
{
  static const KanjiMove Kanji_Move;
  return Kanji_Move;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
