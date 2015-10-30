/* ki2.h
 */

#ifndef OSL_RECORD_KI2_H
#define OSL_RECORD_KI2_H

#include "osl/record/record.h"
#include "osl/record/kanjiMove.h"
#include <string>
#include <iosfwd>

namespace osl
{
  namespace ki2
  {
    /**
     * 「.ki2」という拡張子を持つ2ch形式ファイル.  
     * ファイルはShift_JIS (Windows-31J)であることが期待され、
     * 内部ではEUC-JPに文字変換される。
     */
    class Ki2File : public RecordFile
    {
    private:
      bool verbose;
    public:
      Ki2File(const std::string& filename, bool verbose=false);

      enum ParseResult {
        OK = 0, Komaochi, Illegal,
      };
      static ParseResult parseLine(NumEffectState&, Record&, KanjiMove&, std::string element);
    };

    const std::string show(Square);
    const std::string show(Square cur, Square prev);
    const std::string show(Ptype);
    const std::string showPromote(bool);
    const std::string show(Move move, const NumEffectState& state, Move prev=Move());
    const std::string show(const Move *first, const Move *last, const NumEffectState& state, Move prev=Move());
    const std::string show(const Move *first, const Move *last, const char *threatmate_first, const char *threatmate_last, const NumEffectState& state, Move prev=Move());
  } // namespace ki2
  using ki2::Ki2File;
} // namespace osl

#endif /* OSL_RECORD_KI2_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
