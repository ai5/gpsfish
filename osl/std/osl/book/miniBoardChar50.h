/* miniBoardChar50.h
 */
#ifndef OSL_MINIBOARDCHAR50_H
#define OSL_MINIBOARDCHAR50_H

#include "osl/simpleState.h"
#include <string>
namespace osl
{
  namespace book
  {  
    class MiniBoardChar50;
    bool operator<(const MiniBoardChar50&, const MiniBoardChar50&);
    bool operator==(const MiniBoardChar50&, const MiniBoardChar50&);
    /** 50 byte の盤面. 手番なし. (常に先手番と解釈).
     * MiniBoardと比較するとbyte access重視の設計.
     */
    class MiniBoardChar50
    {
    public:
      MiniBoardChar50();
      explicit MiniBoardChar50(const SimpleState&);
      explicit MiniBoardChar50(const std::string &src);
      const std::string toString() const;
      const SimpleState toSimpleState(Player turn=BLACK) const;
    private:
      /**
       * - 40 byte: Square (各1byte)
       * -  5 byte: owner   (各1bit)
       * -  5 byte: promote (各1bit)
       */
      CArray<uint8_t,50> data;
      friend bool operator<(const MiniBoardChar50&, const MiniBoardChar50&);
      friend bool operator==(const MiniBoardChar50&, const MiniBoardChar50&);
    };
  }
}


#endif /* OSL_MINIBOARDCHAR50_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
