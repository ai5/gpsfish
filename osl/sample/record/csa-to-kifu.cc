/* csa-to-kifu.cc
 */
#include "osl/record/kanjiPrint.h"
#include "osl/record/kanjiCode.h"
#include "osl/record/ki2.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/iconvConvert.h"
#include <boost/program_options.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>

namespace po = boost::program_options;
using namespace osl;

std::string header;
std::vector<std::string> files;

void run(const std::string& filename);

int main(int argc, char **argv)
{
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("header,h", boost::program_options::value<std::string>(&header)->default_value(""),
     "header for kifu-file")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in csa format (.csa)")
    ("help,h", "Show help message");
  boost::program_options::variables_map vm;
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  try
  {
    boost::program_options::store(
      boost::program_options::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    boost::program_options::notify(vm);
    files = vm["input-file"].as< std::vector<std::string> >();
    if (vm.count("help"))
    {
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  try
  {
    for (size_t i=0; i<files.size(); ++i)
      run(files[i]);
  }
  catch (...)
  {
    return 1;
  }
  return 0;
}

void run(const std::string& filename)
{
  if (header != "") {
    std::ifstream is(header.c_str()); // sjis, \r\n?
    std::string line;
    while (std::getline(is, line))
      std::cout << line << "\n";
  }
  else {
    std::cout << "# " 
	      << IconvConvert::convert("EUC-JP", "SHIFT_JIS", K_KIFU)
	      << "\r\n";
  }
  try {
    CsaFile file(filename);
    auto record = file.load();
    std::vector<Move> moves;
    std::vector<int> time;
    std::vector<record::SearchInfo> search_info;
    std::vector<std::string> raw_comments;
    record.load(moves, time, raw_comments, search_info);
    
    NumEffectState state(file.initialState());
    CArray<int,2> total = {{0,0}};
    for (size_t i=0; i<moves.size(); ++i) {
      if (! moves[i].isNormal() || ! state.isValidMove(moves[i]))
	break;
      const Square to = moves[i].to();
      std::ostringstream ss;
      ss << std::setw(4) << std::setfill(' ') << i+1 << ' '
	 << record::StandardCharacters::suji[to.x()]
	 << record::StandardCharacters::dan[to.y()];
      ss << ki2::show(moves[i].oldPtype());
      if (moves[i].isPromotion())
	ss << K_NARU;
      const Square from = moves[i].from();
      if (from.isPieceStand())
	ss << K_UTSU;
      else
	ss << "(" << from.x() << from.y() << ")";
      ss << "   ";
      if (time.size() > i) {
	total[i%2] += time[i];
	ss << "(" << std::setw(2) << std::setfill(' ') << time[i]/60
	   << ':' << std::setw(2) << std::setfill('0') << time[i]%60
	   << "/" << std::setw(2) << total[i%2]/60/60
	   << ':' << std::setw(2) << total[i%2]/60%60
	   << ':' << std::setw(2) << total[i%2]%60
	   << ")";
      }
      ss << "\r\n";

      state.makeMove(moves[i]);	

      if (search_info.size() > i && ! search_info[i].moves.empty()) {
	ss << IconvConvert::convert("UTF-8", "EUC-JP", "*読筋 ")
	   << search_info[i].value << " ";
	NumEffectState copy(state);
	for (size_t j=0; j<search_info[i].moves.size(); ++j) {
	  const Move move = search_info[i].moves[j];
	  if (move.isInvalid() 
	      || (! move.isPass() && ! copy.isValidMove(move)))
	    break;
	  ss << ki2::show(move, copy, j ? search_info[i].moves[j-1] : moves[i]);
	  copy.makeMove(move);
	}
	ss << "\r\n";
      }
      std::cout << IconvConvert::convert("EUC-JP", "SHIFT_JIS", ss.str()) << std::flush;
    }
  }
  catch (CsaIOError&) {
    std::cerr << "parse error\n";
    throw;
  }
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
