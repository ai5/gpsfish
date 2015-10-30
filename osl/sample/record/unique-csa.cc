#include "osl/record/csaRecord.h"
#include "osl/record/checkDuplicate.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <string>
#include <vector>

/**
 * Return true when reading a csa_file for the fist time;
 * false otherwise. 
 */
bool readFile(const std::string& csa_file,
              osl::record::CheckDuplicate& duplicates)
{
  const osl::CsaFileMinimal csa(csa_file);
  const auto& record = csa.load();
  const auto moves = record.moves;
 
  return !duplicates.regist(moves);
}


int main(int argc, char **argv)
{
  namespace bp = boost::program_options;

  bp::options_description command_line_options;
  command_line_options.add_options()
    ("input-file", bp::value<std::vector<std::string> >(),
     "input files in the CSA format")
    ("help", "Show help message");
  bp::variables_map vm;
  bp::positional_options_description p;
  p.add("input-file", -1);

  try {
    bp::store(
      bp::command_line_parser(argc, argv).options(command_line_options).positional(p).run(), vm);
    bp::notify(vm);
    if (vm.count("help")) {
      std::cerr << "Filter duplicated records from specified CSA files.\n";
      std::cerr << "Usage: " << argv[0] << " [options] csa-file [...]\n";
      std::cerr << "       " << argv[0] << " [options]\n";
      std::cout << command_line_options << std::endl;
      return 0;
    }
  } catch (std::exception &e) {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Filter duplicated records from specified CSA files.\n";
    std::cerr << "Usage: " << argv[0] << " [options] csa-file [...]\n";
    std::cerr << "       " << argv[0] << " [options]\n";
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  if (vm.count("input-file")) {
    const std::vector<std::string> temp = vm["input-file"].as<std::vector<std::string> >();
    files.insert(files.end(), temp.begin(), temp.end());
  } else {
    std::string line;
    while(std::getline(std::cin , line)) {
      boost::algorithm::trim(line);
      files.push_back(line);
    }
  }

  osl::record::CheckDuplicate check_duplicate;

  for (const std::string& file: files) {
    if (readFile(file, check_duplicate))
      std::cout << file << std::endl;
  }

  check_duplicate.print(std::cerr);

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
