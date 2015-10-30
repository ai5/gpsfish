/* ki2IOError.h
 */
#ifndef _KI2IOERROR_H
#define _KI2IOERROR_H

#include <stdexcept>
#include <string>

namespace osl
{
  namespace record
  {
    namespace ki2
    {
      struct Ki2IOError : public std::runtime_error
      {
	Ki2IOError(const std::string& w) : std::runtime_error(w)
	{
	}
      };
    } // namespace ki2
  } // namespace record
  using record::ki2::Ki2IOError;
} // namespace osl

#endif /* _KI2IOERROR_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
