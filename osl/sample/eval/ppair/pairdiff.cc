/* pairdiff.cc
 */

#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/csa.h"
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>

// 二つのテーブルの差を表示

using namespace osl;
using namespace osl::eval;

void usage(const char *prog)
{
  using namespace std;
  cerr << "Usage: " << prog << " table1 table2"
       << endl;
  exit(1);
}

void show(std::ostream& os, Square pos, PtypeO ptypeo)
{
  os << csa::show(pos) << " ";
  os << getOwner(ptypeo);
  os << csa::show(getPtype(ptypeo));
}

int main(int argc, char **argv)
{
  const char *program_name = argv[0];
  bool error_flag = false;
  const char *filename1 = 0;
  const char *filename2 = 0;
  
  // extern char *optarg;
  extern int optind;
  char c;
  while ((c = getopt(argc, argv, "vh")) != EOF)
  {
    switch(c)
    {
    default:	error_flag = true;
    }
  }
  argc -= optind;
  argv += optind;

  if (error_flag || (argc < 2))
    usage(program_name);
  filename1 = argv[0];
  filename2 = argv[1];

  std::unique_ptr<PiecePairRawTable> table1(new PiecePairRawTable());
  table1->loadFromBinaryFile(filename1);
  std::unique_ptr<PiecePairRawTable> table2(new PiecePairRawTable());
  table2->loadFromBinaryFile(filename2);

  for (unsigned int i=0; i<PiecePairRawTable::maxPairIndex; ++i)
  {
    const int val1 = table1->value(i);
    const int val2 = table2->value(i);
    if (val1 != val2)
    {
      size_t i1, i2;
      PiecePairRawTable::meltIndex(i, i1, i2);
      Square pos1, pos2;
      PtypeO ptypeo1, ptypeo2;
      PiecePairRawTable::meltIndex(i1, pos1, ptypeo1);
      PiecePairRawTable::meltIndex(i2, pos2, ptypeo2);
      show(std::cout, pos1, ptypeo1);
      std::cout << " ";
      show(std::cout, pos2, ptypeo2);
      std::cout << " : " << val1 << " != " << val2 << "\n";
    }
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
