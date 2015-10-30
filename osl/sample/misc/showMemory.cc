#include "osl/oslConfig.h"

#include <iostream>

int main(int /*argc*/, char ** /*argv*/)
{
  osl::OslConfig oslConfig;

  std::cout << "Memory Use Limit: " << oslConfig.memoryUseLimit() << "\n";
  std::cout << "Resident Memory Use: " << oslConfig.residentMemoryUse() << "\n";

  const int MAX = 10000000; // 10M
  std::cout << "new int[" << MAX << "]\n";
  int *large_space = new int[MAX];
  std::cout << "Resident Memory Use: " << oslConfig.residentMemoryUse() << "\n";
  
  std::cout << "Delete it\n";
  delete[] large_space;
  std::cout << "Resident Memory Use: " << oslConfig.residentMemoryUse() << "\n";

  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

