/* ntesukiExceptions
 */
#ifndef OSL_NTESUKI_EXCEPTIONS
#define OSL_NTESUKI_EXCEPTIONS
#include <iostream>
#include <stdexcept>

#ifndef NDEBUG
# define ntesuki_assert(assertion)\
if (!(assertion))\
 throw DfpnError("assertion failed", __FILE__, __LINE__);
#else
#define ntesuki_assert(assertion)
#endif

#ifndef NDEBUG
#define TRY_DFPN \
try\
{\

#define CATCH_DFPN \
}\
catch (DfpnError err)\
{\
 ntesuki_assert(false);\
}
#else
#define TRY_DFPN
#define CATCH_DFPN
#endif

namespace osl
{
  namespace ntesuki
  {
    /**
     * Throwed when something wrong happend with the df-pn search.
     */
    struct DfpnError : std::runtime_error
    {
      DfpnError(const char *message,
		const char *filename,
		int linenum) : std::runtime_error(message)
      {
	std::cerr << message
		  << "\n\tin " << filename
		  << " line " << linenum << "\n";
      }
    };
  }// ntesuki
}//osl

#endif /* OSL_NTESUKI_EXCEPTIONS */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
