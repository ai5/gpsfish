#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/eval/ml/openMidEndingEval.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/alphaBeta2.h"
#include "osl/search/moveWithComment.h"
#include "osl/record/kisen.h"
#include "osl/record/csa.h"

#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>

#include <iostream>
#include <fstream>

int main(int argc, char **argv)
{
  size_t start, end;
  bool include_all;
  std::string output;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("start",
     boost::program_options::value<size_t>(&start)->default_value(0),
     "Start index of kisen file")
    ("end",
     boost::program_options::value<size_t>(&end)->default_value(60000),
     "End index of kisen file")
    ("all",
     boost::program_options::value<bool>(&include_all)->default_value(false),
     "Whether to include all plays.  When false, only plays with players "
     "more than or equal to rating 1500 are included")
    ("input-file", boost::program_options::value<std::vector<std::string> >(),
     "input files in kisen format")
    ("help", "Show help message");
  boost::program_options::variables_map vm;
  boost::program_options::positional_options_description p;
  p.add("input-file", -1);

  try
  {
    boost::program_options::store(
      boost::program_options::command_line_parser(
	argc, argv).options(command_line_options).positional(p).run(), vm);
    boost::program_options::notify(vm);
    if (vm.count("help"))
    {
      std::cerr << "Usage: " << argv[0] << " [options] kisen-file"
		<< std::endl;
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0]
	      << " [options] result-file kisen-file" << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  const std::vector<std::string> files =
    vm["input-file"].as< std::vector<std::string> >();

  if (files.size() != 2)
  {
    std::cerr << "Need two files" << std::endl;
    return 1;
  }

  std::ifstream fin(files[0].c_str());
  std::vector<std::string> results;
  results.reserve(end - start);
  std::string result;
  while (std::getline(fin, result))
  {
    results.push_back(result);
  }

  osl::record::KisenFile kisen(files[1]);
  std::string ipx(files[1]);
  ipx.replace(ipx.rfind("."), 4, ".ipx");
  osl::record::KisenIpxFile kisen_ipx(ipx);

  typedef osl::eval::ml::OpenMidEndingEval ome_eval_t;
  ome_eval_t::setUp();
  osl::progress::ml::NewProgress::setUp();

  const int pawn = ome_eval_t::captureValue(newPtypeO(osl::WHITE,osl::PAWN))/2;
  for (size_t i = start; i < kisen.size() && i < end; i++)
  {
    const int ratingb = kisen_ipx.getRating(i, osl::BLACK);
    const int ratingw = kisen_ipx.getRating(i, osl::WHITE);
    if (!include_all && ratingb < 1500 && ratingw < 1500)
    {
      continue;
    }

    osl::search::SimpleHashTable table(1000000, 200);
    osl::search::SearchState2::checkmate_t checkmate_searcher;
    osl::search::CountRecorder recorder;

    const osl::vector<osl::Move> moves = kisen.getMoves(i);
    osl::state::NumEffectState state(kisen.getInitialState());
    ome_eval_t eval(state);
    osl::search::SearchState2 core(state, checkmate_searcher);

    std::string result = results[i - start];
    size_t end_index = moves.size();
    if (result.find(" ") != std::string::npos)
    {
      end_index =
	std::max(0, atoi(result.substr(result.find(" ")).c_str()) - 1);
      assert(end_index <= moves.size());
      result = result.substr(0, 1);
    }
    bool valid_record = true;
    for (size_t j = 0; j < end_index; j++)
    {
      osl::ApplyMoveOfTurn::doMove(state, moves[j]);
      if (state.inCheck(alt(state.turn())))
      {
	valid_record = false;
	break;
      }
    }
    if (! valid_record)
      continue;

    state = kisen.getInitialState();
    for (size_t j = 0; j <= end_index; j++)
    {
      const osl::Square black_king = state.kingSquare(osl::BLACK);
      const osl::Square white_king = state.kingSquare(osl::WHITE);

      std::cout << i << " " << ratingb << " " << ratingw << " ";
      std::cout << result << " " << j << " " << eval.progress16().value() << " ";
      std::cout << eval.value()*100/pawn << " ";

      core.setState(state);
      osl::search::QuiescenceSearch2<ome_eval_t>
	qsearch(core, table);
      const osl::search::SimpleHashRecord *record = table.allocate(osl::HashKey(state), 2000);
      for (int d=0; d<=4; d+=2)
      {
	const int qvalue = qsearch.search(
	  state.turn(), eval,
	  j == 0 ? osl::Move::PASS(osl::WHITE) : moves[j-1], d);
	std::cout << qvalue*100/pawn << " " << osl::record::csa::show(record->qrecord.bestMove())
		  << " " << qsearch.nodeCount() << " ";
      }
      for (int l=400; l<=600; l+=200)
      {
	osl::search::AlphaBeta2<ome_eval_t> search(state, checkmate_searcher, &table, recorder);
	osl::search::MoveWithComment additional_info;
	osl::Move best_move = search.computeBestMoveIteratively
	  (l, 200, 400, 1000000, osl::search::TimeAssigned(osl::milliseconds(60*1000)),
	   &additional_info);
	std::cout << additional_info.value << " " << osl::record::csa::show(best_move)
		  << " " << search.nodeCount() << " ";
      }
      std::cout << std::endl;
      if (j != moves.size())
      {
	osl::apply_move::ApplyMoveOfTurn::doMove(state, moves[j]);
	eval.update(state, moves[j]);
      }
      if (table.size() > table.capacity()/2)
	table.clear();
    }
    std::cout << std::flush;
  }

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
