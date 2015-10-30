/* usi.h
 */
#ifndef OSL_USI_RECORD_H
#define OSL_USI_RECORD_H

#include "osl/record/record.h"
#include "osl/usi.h"
#include <string>

namespace osl
{
  namespace usi
  {
    /** 
     * URIやFile systemとして使えるように、文字をescape. 
     * これはGPSShogiによる拡張であり、standardではない.
     * @str str自体が修正される
     */
    void escape(std::string& str);
    /** 
     * escapeされた文字を元に戻す.
     * これはGPSShogiによる拡張であり、standardではない.
     * @str str自体が修正される
     */
    void unescape(std::string& str);

    class UsiFile : public RecordFile
    {
    public:
      UsiFile(const std::string& filename);
      ~UsiFile();

    };
  }
  using usi::UsiFile;
} // osl

#endif /* OSL_USI_RECORD_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
