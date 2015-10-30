#ifndef OSL_CSA_H
#define OSL_CSA_H

#include "osl/numEffectState.h"
#include <vector>
#include <string>
#include <iosfwd>
#include <stdexcept>
namespace osl
{
  /**
   * CSA形式.
   * CSA形式の定義 http://www.computer-shogi.org/wcsc12/record.html
   */
  namespace csa
  {
    struct CsaIOError : public std::runtime_error
    {
      CsaIOError(const std::string& w) : std::runtime_error(w) {
      }
    };

    const Move strToMove(const std::string& s,const SimpleState& st);
    Player charToPlayer(char c);
    const Square strToPos(const std::string& s);
    Ptype strToPtype(const std::string& s);

    const std::string show(Move);
    const std::string fancyShow(Move);
    const std::string show(Square);
    const std::string show(Ptype);
    const std::string show(Piece);
    const std::string show(Player);
    const std::string show(const Move *first, const Move *last);

    const std::string show(Move, std::string& buf);
    const std::string show(Square, std::string& buf, size_t offset=0);
    const std::string show(Ptype, std::string& buf, size_t offset=0);
    const std::string show(Player, std::string& buf, size_t offset=0);
  } // namespace csa

  struct RecordMinimal
  {
    NumEffectState initial_state;
    std::vector<Move> moves;
    const NumEffectState& initialState() const { return initial_state; }
  };

  namespace csa
  {
    class CsaFileMinimal
    {
      RecordMinimal record;
    public:
      CsaFileMinimal(const std::string& filename);
      CsaFileMinimal(std::istream& is);
      virtual ~CsaFileMinimal();
      RecordMinimal load() const { return record; }
      std::vector<Move> moves() const { return load().moves; }
      const NumEffectState& initialState() const { return load().initialState(); }
    protected:
      CsaFileMinimal() {}
      void load(std::istream&);
    public:
      static bool parseLine(SimpleState&, RecordMinimal&, std::string element,
			    CArray<bool,9>&);
    };

    class CsaString : public CsaFileMinimal
    {
    public:
      CsaString(const std::string&);
      NumEffectState initialState() const { return load().initial_state; }
    };
  }
  using csa::CsaIOError;
  using csa::CsaFileMinimal;
  using csa::CsaString;
} // namespace osl
#endif /* _CSA_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
