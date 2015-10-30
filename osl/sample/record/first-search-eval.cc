//#include "osl/misc/math.h"
#include "osl/record/csaRecord.h"
#include "osl/record/checkDuplicate.h"
#include "osl/record/record.h"
#include "osl/record/searchInfo.h"
#include "osl/misc/math.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

typedef std::vector<int> value_t;
typedef std::pair<value_t, value_t> pair_t;
typedef std::map<std::string, pair_t> map_t;

void logValue(const std::string& player,
              const std::string& turn,
              const int value,
              const std::string& csa_file)
{
  const std::string log_file = player + "_" + turn + ".log";
  std::ofstream out(log_file.c_str(), std::ios::app);
  out << value << "\t" << csa_file << "\n";
}

/**
 * Return true when reading a csa_file for the fist time;
 * false otherwise. 
 */
void readFile(const std::string& csa_file, map_t& map)
{
  const osl::CsaFile csa(csa_file);
  const auto& record = csa.load();

  // black
  for (size_t i=0; i < record.moves().size(); i += 2) {
    if (record.move_info[i].isValid()) {
      const std::string player = record.player[osl::BLACK];
      const int eval = record.move_info[i].value;
      pair_t& pair    = map[player];
      value_t& values = pair.first;
      values.push_back(eval);

      logValue(player, "black", eval, csa_file);
      break;
    }
  }
  // white
  for (size_t i=1; i < record.moves().size(); i += 2) {
    if (record.move_info[i].isValid()) {
      const std::string player = record.player[osl::WHITE];
      const int eval = record.move_info[i].value;
      pair_t& pair    = map[player];
      value_t& values = pair.second;
      values.push_back(eval);

      logValue(player, "white", eval, csa_file);
      break;
    }
  }
}

void showResult(const map_t& map)
{
  for (const map_t::value_type& vt: map) {
    std::cout << "===== " << vt.first << " =====\n";
    {
      int sum, mean, var, std_dev, skew, kurt; 
      osl::misc::computeStats(vt.second.first.begin(), vt.second.first.end(),
                              sum, mean, var, std_dev, skew,  kurt);
      std::cout << "[BLACK] mean: " << mean << ", std_dev: " << std_dev << std::endl;
    }
    {
      int sum, mean, var, std_dev, skew, kurt; 
      osl::misc::computeStats(vt.second.second.begin(), vt.second.second.end(),
                              sum, mean, var, std_dev, skew,  kurt);
      std::cout << "[WHITE] mean: " << mean << ", std_dev: " << std_dev << std::endl;
    }
  }
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
      std::cerr << "Calculate evaluation values for the initial search moves after finishing opening moves.\n";
      std::cerr << "Usage: " << argv[0] << " [options] csa-file [...]\n";
      std::cerr << "       " << argv[0] << " [options]\n";
      std::cout << command_line_options << std::endl;
      return 0;
    }
  } catch (std::exception &e) {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Calculate evaluation values for the initial search moves after finishing opening moves.\n";
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

  map_t map;
  for (const std::string& file: files) {
    readFile(file, map);
  }

  showResult(map);

  return 0;
}

/* vim: set ts=2 sw=2 ft=cpp : */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
