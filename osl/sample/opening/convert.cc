#include "openingBookConverter.h"

int main(int argc, char **argv)
{
  if (argc != 3)
    {
      fprintf(stderr, "Usage: %s old-file new-file\n", argv[0]);
      return 1;
    }
  OpeningBookConverter rw(argv[1]);
  // rw.write(argv[2]);
  rw.writeInNewEditFormat(argv[2]);
  return 0;
}
