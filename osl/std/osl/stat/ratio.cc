/* ratio.cc
 */
#include "osl/stat/ratio.h"
#include <iostream>

osl::stat::Ratio::~Ratio()
{
  if (name && ave.numElements()
      && (show_on_destructor
#ifdef SHOW_RATIO
	  || 1
#endif
	))
  {
    show();
  }
}

void osl::stat::Ratio::show() const
{
  std::cerr << name << " " << 100.0*ratio() 
	    << " " << static_cast<int>(ave.numElements() * ratio())
	    << " / " << ave.numElements() << "\n";
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
