#ifndef OSL_CONFIG_H
#define OSL_CONFIG_H

#include <cstdint>

#define OSL_WORDSIZE 64

#ifndef MINIMAL
#  define ALLOW_KING_ABSENCE
#endif

// for helgrind or drd
// #define OSL_USE_RACE_DETECTOR

#ifdef OSL_USE_RACE_DETECTOR
#  ifndef OSL_NO_SSE
#    define OSL_NO_SSE 1
#  endif
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4099 )
#pragma warning( disable : 4146 )
#pragma warning( disable : 4244 )
#pragma warning( disable : 4267 )
#pragma warning( disable : 4661 )
#pragma warning( disable : 4800 )
#pragma warning( disable : 4805 )
#pragma warning( disable : 4906 )
#pragma warning( disable : 4996 )
#define OSL_NO_SSE 1
#endif

#endif /* _OSL_CONFIG_H */
