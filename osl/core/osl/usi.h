/* usi.h
 */
#ifndef OSL_USI_H
#define OSL_USI_H

#include "osl/numEffectState.h"
#include <vector>
#include <string>
#include <stdexcept>

namespace osl
{
  namespace usi
  {
    const Move strToMove(const std::string&, const NumEffectState&);
    PtypeO charToPtypeO(char);

    const std::string show(Move);
    const std::string show(PtypeO);
    const std::string show(Piece);
    const std::string show(const NumEffectState&);

    class ParseError : public std::invalid_argument
    {
    public:
      ParseError(const std::string& msg = "")
	: invalid_argument(msg)
        { }
    };

    /** 
     * 盤面を取得する. 
     * board文字列が不正なときは、ParseErrorがthrowされる. 
     * @param board USIの文字列
     * @param state boardの解析結果が出力される
     */
    void parseBoard(const std::string& board, NumEffectState&);
    /**  [sfen <sfenstring> | startpos ] moves <move1> ... <movei> */
    void parse(const std::string& line, NumEffectState&);
    void parse(const std::string& line, NumEffectState& initial, std::vector<Move>& moves);

    NumEffectState makeState(const std::string& line);
  }

  /**
   * gnushogi で使われるフォーマット.
   * 何種類かある．
   */
  namespace psn
  {
    class ParseError : public std::invalid_argument
    {
    public:
      ParseError(const std::string& msg = "")
	: invalid_argument(msg)
        { }
    };
    const Move strToMove(const std::string&, const SimpleState&);
    const Square strToPos(const std::string&);
    Ptype charToPtype(char);

    const std::string show(Move);
    const std::string show(Square);
    char show(Ptype);

    /** decorate capture by 'x', promote by '+', and unpromote by '=' */
    const std::string showXP(Move);
  }
} // osl

#endif /* OSL_USI_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
