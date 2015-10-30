/* kakinoki.h
 */
#ifndef OSL_KAKINOKI_H
#define OSL_KAKINOKI_H

#include "osl/record/record.h"
#include <memory>
#include <string>
#include <iosfwd>
#include <stdexcept>

namespace osl
{
  namespace kakinoki
  {
    Move strToMove(const std::string&, const SimpleState&, 
		   Move last_move=Move());
    std::pair<Player,Ptype> strToPiece(const std::string&);

    class KakinokiFile : public RecordFile
    {
    public:
      KakinokiFile(const std::string& filename);
      ~KakinokiFile();

      static bool isKakinokiFile(const std::string& filename);
      static void parseLine(SimpleState& state, Record& record, 
			    std::string s, CArray<bool,9>& board_parsed);
    };

    struct KakinokiIOError : public std::runtime_error
    {
      KakinokiIOError(const std::string& w) : std::runtime_error(w) {
      }
    };
  } // namespace kakinoki
  using kakinoki::KakinokiFile;
  using kakinoki::KakinokiIOError;
}

#endif /* OSL_KAKINOKI_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
