/* convert.cc
 */
#include "osl/bits/binaryIO.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <cstdio>
#include <unistd.h>

bool real_value = false, binary_to_text = false;
template <class T>
void to_binary()
{
  std::vector<T> data;
  T value;
  while (std::cin >> value) {
    data.push_back(value);
    assert(value == data.back());
  }
  osl::misc::BinaryWriter::write(std::cout, data);
}

template <class T>
void write_line(T value) 
{
  std::cout << value << std::endl;
}
void write_line(double value) 
{
  printf("%.8f\n", value);
}

template <class T>
void to_text()
{
  std::vector<T> data;
  osl::misc::BinaryReader<T> reader(std::cin);
  while (reader.read(data)) {
    for (T value: data) {
      write_line(value);
    }
    if (data.size() < reader.blockSize())
      break;
  }
}

int main(int argc, char **argv)
{
  extern int optind;
  bool error_flag = false;
  char c;
  while ((c = getopt(argc, argv, "rth")) != EOF)
  {
    switch(c)
    {
    case 'r':
      real_value = true;
      break;
    case 't':
      binary_to_text = true;
      break;
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;
  if (error_flag) {
    std::cerr << "unknown option\n";
    return 1;
  }

  if (binary_to_text) {
    if (real_value)
      to_text<double>();
    else
      to_text<int>();
  }
  else {
    if (real_value)
      to_binary<double>();
    else
      to_binary<int>();
  }
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
