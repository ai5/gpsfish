/* recordSet.h
 */
#ifndef _SEARCH_RECORDSET_H
#define _SEARCH_RECORDSET_H

#include <unordered_set>

namespace osl
{
  namespace search
  {
    class SimpleHashRecord;
  }
}

namespace osl
{
  namespace search
  {
    namespace analyzer
    {
      class RecordSet : public std::unordered_set<const SimpleHashRecord*>
      {
      public:
	RecordSet();
	~RecordSet();
      };
    } // namespace analyzer
  } // namespace search
} // namespace osl

#endif /* _SEARCH_RECORDSET_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
