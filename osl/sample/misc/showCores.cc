#include <thread>
#include <iostream>

int main(int /*argc*/, char ** /*argv*/)
{
  std::cout << "Cores: " << std::thread::hardware_concurrency() << std::endl;

  return 0;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
