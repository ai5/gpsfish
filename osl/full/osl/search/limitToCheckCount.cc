#include "osl/search/limitToCheckCount.h"

namespace osl
{
  namespace checkmate
  {
    CArray<size_t,32> LimitToCheckCountTable={{
      0,  // 0-127   
      0,	 // 128-
      0,	 // 256-
      0,	 // 384-
      70,	 // 512-    
      80,	 // 640-    
      90,  // 768-   
      100, // 896-    
      150, // 1024-   
      200, 

      250, // 1280
      300, 
      300, 
      300, 
      300, 
      300, 
      300, // 1664-   
      1600,
      2400,
      3200,

      4800, // 2560
      6400, 
      9600,
      12800,  
      19200, 
      25600, 
      38400, 
      51200, 
      76800, 
      102400, 

      409600, 
      1920000,
    }};
  } // namespace checkmate
} // namespace osl

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
