#include "osl/checkmate/dualDfpn.h"
#include "osl/checkmate/proofDisproof.h"
#include "osl/csa.h"

#include <boost/program_options.hpp>
#include <boost/progress.hpp>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

namespace po = boost::program_options;
size_t max_nodes, min_nodes, filenumber;
bool search_proof;
void run(const std::string& filename);
int main(int argc, char **argv)
{
  po::options_description options("options");
  options.add_options()
    ("help", "produce help message")
    ("maximum-nodes,M",
     po::value<size_t>(&max_nodes)->default_value(80000),
     "search proof/disproof positions within this limit")
    ("min-nodes,m",
     po::value<size_t>(&min_nodes)->default_value(8000),
     "ignore positions proven/disproven by search with less than this limit")
    ("proof,p",
     po::value<bool>(&search_proof)->default_value(1),
     "search proof/disproof problems")
    ("file-number,n",
     po::value<size_t>(&filenumber)->default_value(1),
     "start number of filenames for generated problems")
    ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
    ("target-file", po::value<std::vector<std::string> >());
  po::options_description command_line_options;
  command_line_options.add(options).add(hidden);
  po::options_description visible_options("All options");
  visible_options.add(options);

  po::positional_options_description p;
  p.add("target-file", -1);

  po::variables_map vm;
  std::vector<std::string> filenames;

  try {
    po::store(po::command_line_parser(argc, argv).
	      options(command_line_options).positional(p).run(), vm);
    notify(vm);
    if (vm.count("help")) {
      std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
      std::cout << visible_options << std::endl;
      return 0;
    }
    filenames = vm["target-file"].as<std::vector<std::string> >();
  }
  catch (std::exception& e) {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] files" << std::endl;
    std::cerr << visible_options << std::endl;
    return 1;
  }
  boost::progress_display progress(filenames.size());
  for (const std::string& filename: filenames) {
    run(filename);
    ++progress;
  }
}

using namespace osl;
std::string write_file(const NumEffectState& state, Move move, size_t count)
{
  std::ostringstream ss;
  ss << std::setw(4) << std::setfill('0') << filenumber++ << ".csa";
  std::ofstream os(ss.str().c_str());
  os << state;
  if (search_proof)
    os << csa::show(move) << "\n";
  os << "' " << count << " nodes\n";
  return ss.str();
}
bool find_problem(DualDfpn& dfpn, NumEffectState& state)
{
  HashKey key(state);
  PathEncoding path(state.turn());
  Move win_move;
  const size_t before = dfpn.totalNodeCount();
  ProofDisproof pdp = dfpn.findProof(max_nodes, state, key, path, win_move);
  const size_t after = dfpn.totalNodeCount();
  if ((search_proof && !pdp.isCheckmateSuccess())
      || (!search_proof && !pdp.isCheckmateFail())
      || after-before < min_nodes)
    return false;
  write_file(state, win_move, after-before);
  return true;
}
void run(const std::string& filename)
{
  CsaFileMinimal file(filename);
  const auto moves=file.moves();
  NumEffectState state;
  DualDfpn dfpn;
  size_t moved = 0;
  for (Move move: moves) {
    state.makeMove(move);
    if (++moved < 50 || state.inCheck())
      continue;
    if (search_proof) {
      MoveVector legal_moves;
      state.generateLegal(legal_moves);
      std::random_shuffle(legal_moves.begin(), legal_moves.end());
      for (Move a: legal_moves) {
	NumEffectState copy(state);
	copy.makeMove(a);
	if (copy.inCheck())
	  continue;
	copy.makeMove(Move::PASS(copy.turn()));
	if (find_problem(dfpn, copy))
	  return;
      }
    }
    else {
      DualDfpn dfpn;
      if (find_problem(dfpn, state))
	return;
    }
  }
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
