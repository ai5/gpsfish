/* usiRecord.cc
 */
#include "osl/record/usiRecord.h"
#include "osl/usi.h"
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

void osl::usi::
escape(std::string& str)
{
  boost::algorithm::replace_all(str, "/", "_");
  boost::algorithm::replace_all(str, "+", "@");
  boost::algorithm::replace_all(str, " ", ".");
}

void osl::usi::
unescape(std::string& str)
{
  boost::algorithm::replace_all(str, "_", "/");
  boost::algorithm::replace_all(str, "@", "+");
  boost::algorithm::replace_all(str, ".", " ");
}


osl::usi::
UsiFile::UsiFile(const std::string& filename)
{
  std::ifstream is(filename.c_str());
  std::string line;
  if (! std::getline(is, line))
  {
    const std::string msg = "UsiFile::UsiFile file cannot read ";
    std::cerr << msg << filename << "\n";
    throw usi::ParseError(msg + filename);
  }
  ::osl::usi::parse(line, record.record.initial_state, record.record.moves);
  assert(record.record.initial_state.isConsistent());
}

osl::usi::
UsiFile::~UsiFile()
{
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
