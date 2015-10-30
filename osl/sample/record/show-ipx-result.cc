#include "osl/record/kisen.h"
#include <boost/lexical_cast.hpp>
#include <vector>
#include <iostream>

using namespace osl;

int main(int argc, char **argv)
{
  std::vector<size_t> results(9, 0);

  for (int i=1; i<argc; ++i) {
    KisenIpxFile ipx(argv[i]);
    for (size_t j=0; j<ipx.size(); ++j) {
      const int result = ipx.result(j);
      results[result] = results.at(result) + 1; 
    }
  }

  std::cout << "BY_PARITY       " << boost::lexical_cast<std::string>(results.at(0)) << std::endl;
  std::cout << "BLACK_WIN       " << boost::lexical_cast<std::string>(results.at(1)) << std::endl;
  std::cout << "WHITE_WIN       " << boost::lexical_cast<std::string>(results.at(2)) << std::endl;
  std::cout << "SENNNICHITE     " << boost::lexical_cast<std::string>(results.at(3)) << std::endl;
  std::cout << "JISHOGI         " << boost::lexical_cast<std::string>(results.at(4)) << std::endl;
  std::cout << "BLACK_WIN_256   " << boost::lexical_cast<std::string>(results.at(5)) << std::endl;
  std::cout << "WIHTE_WIN_256   " << boost::lexical_cast<std::string>(results.at(6)) << std::endl;
  std::cout << "SENNNICHITE_256 " << boost::lexical_cast<std::string>(results.at(7)) << std::endl;
  std::cout << "JISHOGI_256     " << boost::lexical_cast<std::string>(results.at(8)) << std::endl;

  return 0;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
