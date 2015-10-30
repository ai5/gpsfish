#include "threat_search.h"
#include <osl/record/csaRecord.h>
#include <mysql_connection.h>

std::unique_ptr<sql::Connection> con;

int main(int argc, char **argv)
{
  rc::ThreatSearch threatSearch(70);
  threatSearch.setup();

  for (int i=1; i<argc; ++i) {
    const char *file = argv[i];
    
    std::cout << "Reading...  " << file << "\n";

    osl::record::CsaFile csa(file);
    const std::vector<osl::Move> moves = csa.moves();

    rc::ThreatSearchResult result = threatSearch.search(moves);
    for (const int i : result.moves) {
      std::cout << "### FOUND ===: " << i << "\n"; 
    }
  }

  return 0;
}
