/* sjis2euc.h
 */
              
#ifndef OSL_SJIS2EUC_H
#define OSL_SJIS2EUC_H

#include <string>

namespace osl
{
  namespace misc
  {
    /**
     * Convert character encoding from Shift_JIS to EUC-JP.
     * This converter is simple enough to be applied to Shogi records.
     * It may not completely implement the conversion algorithm.
     */
    std::string sjis2euc(const std::string& str);
    void sjis2euc(unsigned char& c1, unsigned char& c2);
  }
}

#endif /* OSL_SJIS2EUC_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
