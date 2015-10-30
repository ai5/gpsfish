#include "ignorelist.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

void gpsshell::
IgnoreList::openFile(const std::string& file)
{
  using namespace boost::filesystem;

  ifstream in(file);
  if (!in)
  {
    std::cout << boost::format("File not found: %s\n") % file;
    return;
  }

  // reset
  index_path = path(file);
  current = 0;
  lines.clear();

  std::string line;
  std::string previous_comment;
  while (std::getline(in, line))
  {
    boost::algorithm::trim(line);
    if (line.empty())
     continue; 

    if (line[0] == '#')
    {
      previous_comment.assign(line);
      continue;
    }

    std::istringstream ss(line);
    std::string filename;
    ss >> filename;
    const path p(this->index_path.branch_path() / filename);
      // parent_path for boost 1.36.0 or later
    size_t i;
    ss >> i;
    lines.push_back(std::make_tuple(p, i, previous_comment));
    previous_comment.clear();
  }
  std::cout << boost::format("Loaded %d games\n")
               % lines.size();
}

void gpsshell::
IgnoreList::next()
{
  if (!hasNext())
  {
    std::cout << "There is no next game\n";
    return;
  }

  ++current;
}

void gpsshell::
IgnoreList::prev()
{
  if (!hasPrev())
  {
    std::cout << "There is no previous game\n";
    return;
  }

  --current;
}

void gpsshell::
IgnoreList::first()
{
  while (hasPrev())
    prev();
}


void gpsshell::
IgnoreList::last()
{
  while (hasNext())
    next();
}

