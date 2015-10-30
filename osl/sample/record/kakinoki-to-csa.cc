/* kakinoki-to-csa.cc
 */
#include "osl/record/kakinoki.h"
#include "osl/record/ki2.h"
#include "osl/csa.h"
#include <boost/algorithm/string/predicate.hpp>
#include <iostream>
#include <sstream>

using namespace osl;
void convert(const char *filename)
{
  std::vector<Move> moves;
  std::vector<int> time;
  Record record;
  NumEffectState state;
  if (boost::algorithm::iends_with(filename, ".ki2")) {
    Ki2File file(filename);
    record = file.load();
    state = file.initialState();
  }
  else {
    KakinokiFile file(filename);
    record = file.load();
    state = file.initialState();
  }
  std::cout << "N+" << record.player[BLACK] << std::endl;
  std::cout << "N-" << record.player[WHITE] << std::endl;
  record.load(moves, time);
  std::cout << state;
  for (size_t i=0; i<moves.size(); ++i) 
  {
    std::cout << csa::show(moves[i]) << std::endl;
    if (i < record.comments.size()) 
    {
      std::istringstream is(record.comments[i]);
      std::string line;
      while (std::getline(is, line)) 
	std::cout << "'* " << line << std::endl;
    }
  }
}

int main(int argc, char **argv)
{
  for (int i=1; i<argc; ++i)
    convert(argv[i]);
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
