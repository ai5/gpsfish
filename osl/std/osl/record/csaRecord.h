/* csaRecord.h
 */
#ifndef OSL_CSARECORD_H
#define OSL_CSARECORD_H
#include "osl/record/record.h"
namespace osl
{
  namespace record
  {
    class CsaFile : public RecordFile
    {
    public:
      CsaFile(std::istream& is);
      CsaFile(const std::string& filename);
      ~CsaFile();

      static SearchInfo makeInfo(const SimpleState& initial,
				 const std::string& line,
				 Move last_move);
      static void parseLine(SimpleState&, Record&, std::string element,
			    bool parse_move_comment=true);
    private:
      void read(std::istream&);
    };
  }
  using record::CsaFile;
}

#endif /* OSL_CSARECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
