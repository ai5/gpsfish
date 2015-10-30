/* hash-dump.cc
 */
#include "osl/record/csaRecord.h"
#include "osl/hash/hashKey.h"
#include <iostream>

using namespace osl;
int main(int argc, char **argv)
{
    std::string filename(argv[1]);
    NumEffectState state=CsaFile(filename).getInitialState();
    HashKey key(state);
    key.dumpContents(std::cerr);
    std::cerr << std::endl;
}

