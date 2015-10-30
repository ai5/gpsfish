/* show-ipx.cc
 */
#include "osl/record/kisen.h"
#include "osl/misc/eucToLang.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <iostream>
using namespace osl;

int main(int argc, char **argv)
{
  for (int i=1; i<argc; ++i) {
    KisenIpxFile ipx(argv[i]);
    for (size_t j=0; j<ipx.size(); ++j) {
      if (ipx.startDate(j).is_special())
	std::cout << "N/A\t";
      else
	std::cout << ipx.startDate(j) << "\t";
      std::cout << misc::eucToLang(ipx.player(j, BLACK)) << "\t" 
		<< misc::eucToLang(ipx.title(j, BLACK)) << "\t"
		<< ipx.rating(j, BLACK) << "\t";
      std::cout << misc::eucToLang(ipx.player(j, WHITE)) << "\t" 
		<< misc::eucToLang(ipx.title(j, WHITE)) << "\t"
		<< ipx.rating(j, WHITE) << "\n";
    }
  }

  return 0;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
