#include "osl/record/kanjiPrint.h"
#include "osl/record/kanjiCode.h"
#include "osl/numEffectState.h"
#include "osl/misc/eucToLang.h"
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

bool caseInsentiveCharCompare(char a, char b)
{
  return toupper(a) == toupper(b);
}

bool caseInsentiveCompare(const std::string& s1, const std::string& s2)
{
  return s1.size() == s2.size() &&
         equal(s1.begin(), s1.end(), s2.begin(), caseInsentiveCharCompare);
}

const osl::record::Color osl::record::Color::NONE        ("", "NONE", false);
const osl::record::Color osl::record::Color::Black       ("0;30", "BLACK");
const osl::record::Color osl::record::Color::Red         ("0;31", "RED");
const osl::record::Color osl::record::Color::Green       ("0;32", "GREEN");
const osl::record::Color osl::record::Color::Brown       ("0;33", "BROWN");
const osl::record::Color osl::record::Color::Blue        ("0;34", "BLUE");
const osl::record::Color osl::record::Color::Purple      ("0;35", "PURPLE");
const osl::record::Color osl::record::Color::Cyan        ("0;36", "CYAN");
const osl::record::Color osl::record::Color::LightGray   ("0;37", "LIGHTGRAY");
const osl::record::Color osl::record::Color::DarkGray    ("1;30", "DARKGRAY");
const osl::record::Color osl::record::Color::LightRed    ("1;31", "LIGHTRED");
const osl::record::Color osl::record::Color::LightGreen  ("1;32", "LIGHTGREEN");
const osl::record::Color osl::record::Color::Yellow      ("1;33", "YELLOW");
const osl::record::Color osl::record::Color::LightBlue   ("1;34", "LIGHTBLUE");
const osl::record::Color osl::record::Color::LightPurple ("1;35", "LIGHTPURPLE");
const osl::record::Color osl::record::Color::LightCyan   ("1;36", "LIGHTCYAN");
const osl::record::Color osl::record::Color::White       ("1;37", "WHITE");

const osl::record::Color osl::record::
Color::colorFor(const std::string& str)
{
  static const CArray<const osl::record::Color,17> colors = {{
    osl::record::Color::NONE,
    osl::record::Color::Black, osl::record::Color::Red, 
    osl::record::Color::Green, osl::record::Color::Brown, 
    osl::record::Color::Blue, osl::record::Color::Purple, 
    osl::record::Color::Cyan, osl::record::Color::LightGray, 
    osl::record::Color::DarkGray, osl::record::Color::LightRed,
    osl::record::Color::LightGreen, osl::record::Color::Yellow, 
    osl::record::Color::LightBlue, osl::record::Color::LightPurple, 
    osl::record::Color::LightCyan, osl::record::Color::White}};

  for (const auto& c: colors) {
    if (caseInsentiveCompare(str, c.getName()))
      return c;
  }
  return osl::record::Color::NONE;
}

osl::record::Color::Color(const std::string& value, const std::string& name, const bool valid)
  : value(value), name(name), valid(valid) 
{
}
osl::record::Color::~Color()
{
}

std::string osl::record::
kanjiNumber(const int n)
{
  assert((1 <= n) && (n <= 18));
  switch(n) {
    case 1: return K_K1;
    case 2: return K_K2;
    case 3: return K_K3;
    case 4: return K_K4;
    case 5: return K_K5;
    case 6: return K_K6;
    case 7: return K_K7;
    case 8: return K_K8;
    case 9: return K_K9;
    case 10: return K_K10;
    case 11: return K_K11;
    case 12: return K_K12;
    case 13: return K_K13;
    case 14: return K_K14;
    case 15: return K_K15;
    case 16: return K_K16;
    case 17: return K_K17;
    case 18: return K_K18;
  }
  assert(false);
  return "";
}

osl::record::Characters::~Characters()
{
}

const osl::CArray<std::string,32> osl::record::Characters::stand = 
  {{// WHITE
    K_NAKAGURO, "+E",
    K_PPAWN, K_PLANCE, K_PKNIGHT, K_PSILVER, K_PBISHOP, K_PROOK,  K_KING, 
    K_GOLD,  K_PAWN,   K_LANCE,   K_KNIGHT,  K_SILVER,  K_BISHOP, K_ROOK, 
    // BLACK
    K_NAKAGURO, "+E",
    K_PPAWN, K_PLANCE, K_PKNIGHT, K_PSILVER, K_PBISHOP, K_PROOK,  K_KING, 
    K_GOLD,  K_PAWN,   K_LANCE,   K_KNIGHT,  K_SILVER,  K_BISHOP, K_ROOK
  }};

const osl::CArray<std::string,10> osl::record::StandardCharacters::dan = 
  {{"", K_K1, K_K2, K_K3, K_K4, K_K5, K_K6, K_K7, K_K8, K_K9}};
const osl::CArray<std::string,10> osl::record::StandardCharacters::suji = 
  {{"", K_R1, K_R2, K_R3, K_R4, K_R5, K_R6, K_R7, K_R8, K_R9}};
const osl::CArray<std::string,32> osl::record::StandardCharacters::pieces = 
  {{// WHITE
    K_NAKAGURO, "+E",
    K_PPAWN, K_PLANCE, K_PKNIGHT, K_PSILVER, K_PBISHOP, K_PROOK,  K_KING, 
    K_GOLD,  K_PAWN,   K_LANCE,   K_KNIGHT,  K_SILVER,  K_BISHOP, K_ROOK, 
    // BLACK
    K_NAKAGURO, "+E",
    K_PPAWN, K_PLANCE, K_PKNIGHT, K_PSILVER, K_PBISHOP, K_PROOK,  K_KING, 
    K_GOLD,  K_PAWN,   K_LANCE,   K_KNIGHT,  K_SILVER,  K_BISHOP, K_ROOK
  }};

const osl::CArray<std::string,10> osl::record::RussianCharacters::dan = osl::record::StandardCharacters::dan;
const osl::CArray<std::string,10> osl::record::RussianCharacters::suji = osl::record::StandardCharacters::suji;
const osl::CArray<std::string,32> osl::record::RussianCharacters::pieces = 
  {{// WHITE
    K_NAKAGURO, "+E",
    K_PPAWN_R, K_PLANCE_R, K_PKNIGHT_R, K_PSILVER_R, K_PBISHOP_R, K_PROOK_R,  K_KING_R,
    K_GOLD_R,  K_PAWN_R,   K_LANCE_R,   K_KNIGHT_R,  K_SILVER_R,  K_BISHOP_R, K_ROOK_R, 
    // BLACK
    K_NAKAGURO, "+E",
    K_PPAWN, K_PLANCE, K_PKNIGHT, K_PSILVER, K_PBISHOP, K_PROOK,  K_KING, 
    K_GOLD,  K_PAWN,   K_LANCE,   K_KNIGHT,  K_SILVER,  K_BISHOP, K_ROOK
  }};

const osl::CArray<std::string,10> osl::record::KIFCharacters::dan = 
  osl::record::StandardCharacters::dan;
const osl::CArray<std::string,10> osl::record::KIFCharacters::suji = 
  {{"", " " K_R1, " " K_R2, " " K_R3, " " K_R4, " " K_R5, " " K_R6, " " K_R7, " " K_R8, " " K_R9}};
const osl::CArray<std::string,32> osl::record::KIFCharacters::pieces = 
  {{// WHITE
    " " K_NAKAGURO, "+E",
    "v" K_PPAWN, "v" K_PLANCE, "v" K_PKNIGHT, "v" K_PSILVER, "v" K_PBISHOP, "v" K_PROOK,  "v" K_KING, 
    "v" K_GOLD,  "v" K_PAWN,   "v" K_LANCE,   "v" K_KNIGHT,  "v" K_SILVER,  "v" K_BISHOP, "v" K_ROOK,
    // BLACK
    " " K_NAKAGURO, "+E",
    " " K_PPAWN, " " K_PLANCE, " " K_PKNIGHT, " " K_PSILVER, " " K_PBISHOP, " " K_PROOK,  " " K_KING, 
    " " K_GOLD,  " " K_PAWN,   " " K_LANCE,   " " K_KNIGHT,  " " K_SILVER,  " " K_BISHOP, " " K_ROOK
  }};


std::ostream& osl::record::
operator<<(std::ostream& os, const Color& c)
{
  return os << c.value;
}

void osl::record::ChangeShellColor::
escColSet() const
{
   if (!color.isValid()) return;
   os << "\033[" << color << "m"; //文字の属性をセットする
}

void osl::record::ChangeShellColor::
escColReSet() const
{
   if (!color.isValid()) return;
   os << "\033[0m";
}

void osl::record::KanjiPrint::
print(const SimpleState& state, 
      const Move *last_move) const
{
  os << misc::eucToLang(K_WHITE_STAND) << " ";
  for (Ptype ptype: PieceStand::order) {
    const int count = state.countPiecesOnStand(WHITE, ptype);
    if (count)
      os << misc::eucToLang(pieces->stand_kanji(newPtypeO(BLACK, ptype)))
	 << count << " ";
  }
  os << std::endl;  

  os << " ";
  for(int x=9;x>0;x--)
  {
    os << misc::eucToLang(pieces->getSuji(x));
  }
  os << std::endl;  

  os << "+";
  for(int x=9*pieces->getSuji(1).size();x>0;x--)
  {
    os << "-";
  }
  os << "+" << std::endl;

  for(int y=1;y<=9;y++)
  {
    os << '|';  
    for(int x=9;x>0;x--)
    {
      const PtypeO ptypeo = state.pieceOnBoard(Square(x,y)).ptypeO();
      const std::string piece = misc::eucToLang(pieces->kanji(ptypeo));
      if (last_move && 
          !last_move->isInvalid() && 
          last_move->to() == Square(x,y))
      {
        ChangeShellColor csc(os, last_move_color);
        os << piece;
      } // csc destroyed
      else if (isPiece(ptypeo)) {
        Player owner = getOwner(ptypeo);
        osl::record::Color color; 
        if (owner == BLACK)
          color = black_color;
        else
          color = white_color;
        ChangeShellColor csc(os, color);
        os << piece;
      } // csc destroyed
      else
      { // empty space
        os << piece;
      }
    }
    os << '|';  
    os << misc::eucToLang(pieces->getDan(y));

    os << std::endl;
  }

  os << "+";
  for(int x=9*pieces->getSuji(1).size();x>0;x--)
  {
    os << "-";
  }
  os << "+" << std::endl;

  // 持ち駒の表示
  os << misc::eucToLang(K_BLACK_STAND) << " ";
  for (Ptype ptype: PieceStand::order) {
    const int count = state.countPiecesOnStand(BLACK, ptype);
    if (count)
      os << misc::eucToLang(pieces->stand_kanji(newPtypeO(BLACK, ptype)))
	 << count << " ";
  }
  os << std::endl;  
  os << state.turn() << std::endl;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
