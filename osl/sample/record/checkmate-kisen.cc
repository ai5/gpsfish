#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/state/historyState.h"
#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"
#include "osl/checkmate/dualDfpn.h"
#include "osl/eval/see.h"
#include "osl/misc/filePath.h"
#include "osl/oslConfig.h"

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/max.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <queue>
#include <future>
#include <iostream>
#include <fstream>

static std::vector<osl::Move>
convert_from_first(osl::NumEffectState state,
		   const std::vector<osl::Move> &in,
		   size_t checkmate_limit)
{
  osl::DualDfpn dfpn;
  std::vector<osl::Move> out;
  for (osl::Move move: in) 
  {
    const int see = osl::See::see
      (state, move, state.pin(state.turn()), state.pin(alt(state.turn())));
    out.push_back(move);
    state.makeMove(move);
    if (state.inCheck() && see < 0
	&& dfpn.isLosingState(checkmate_limit, state, 
			      osl::HashKey(state), osl::PathEncoding(state.turn())))
      break;
  }
  return out;
}

static std::vector<osl::Move>
trim_last(osl::NumEffectState initial,
	  const std::vector<osl::Move> &in,
	  size_t checkmate_limit)
{
  std::vector<osl::Move> out;
  if (in.empty())
    return out;
  osl::DualDfpn dfpn;
  osl::HistoryState history(initial);
  for (osl::Move move: in) 
    history.makeMove(move);
  const osl::Player last_played = in.back().player();
  int length = in.size();
  for (; length > 0; length -= 2)
  {
    osl::NumEffectState current = history.state();
    assert(current.turn() == alt(last_played));
    if (! current.inCheck())
      break;
    if (! dfpn.isLosingState(checkmate_limit, current, 
			     osl::HashKey(current), osl::PathEncoding(current.turn())))
      break;
    history.unmakeMove();
    history.unmakeMove();
    if (length + 50 < in.size())
      break;
  }
  out = in;
  out.resize(length);
  return out;
}

static void convert(const std::vector<std::string> &input_filename,
		    const std::string &output_kisen_filename,
		    size_t checkmate_limit, bool output_ipx, bool trim)
{
  namespace acc = boost::accumulators;
  acc::accumulator_set<double, acc::features<acc::tag::max, acc::tag::mean> > accumulator;
  std::ofstream ofs(output_kisen_filename.c_str());
  osl::KisenWriter ks(ofs);

  std::unique_ptr<osl::record::KisenIpxWriter> ipx_writer;
  std::unique_ptr<std::ofstream> ipx_ofs;
  if (output_ipx)
  {
    const boost::filesystem::path ipx_path =
      boost::filesystem::change_extension(boost::filesystem::path(output_kisen_filename), ".ipx");
    const std::string ipx = osl::misc::file_string(ipx_path);
    ipx_ofs.reset(new std::ofstream(ipx.c_str()));
    ipx_writer.reset(new osl::record::KisenIpxWriter(*ipx_ofs));
  }

  for (size_t i = 0; i < input_filename.size(); ++i)
  {    
    osl::KisenFile kisen(input_filename[i]);
    osl::KisenIpxFile ipx(kisen.ipxFileName());
    std::queue<std::future<std::vector<osl::Move>>> queue;
    const osl::NumEffectState initial = kisen.initialState();
    size_t started = 0, concurrency = osl::OslConfig::concurrency();
    while (started < std::min(concurrency, kisen.size())) {
      auto moves = kisen.moves(started++);
      queue.emplace(std::async(std::launch::async, [=](){
	  if (trim)
	    return trim_last(initial, moves, checkmate_limit);
	  else
	    return convert_from_first(initial, moves, checkmate_limit);
	  }));
    }
    for (size_t j=0; j<kisen.size(); ++j) 
    {
      auto new_moves = queue.front().get();
      queue.pop();
      if (started<kisen.size()) {
	auto moves = kisen.moves(started++);
	queue.emplace(std::async(std::launch::async, [=](){
	    if (trim)
	      return trim_last(initial, moves, checkmate_limit);
	    else
	      return convert_from_first(initial, moves, checkmate_limit);
	    }));
      }
      osl::record::Record new_record;
      new_record.player[osl::BLACK] = ipx.player(j, osl::BLACK);
      new_record.player[osl::WHITE] = ipx.player(j, osl::WHITE);
      new_record.start_date = ipx.startDate(j);
      osl::NumEffectState state = kisen.initialState();
      new_record.record = { state, new_moves };
      for (size_t k=0; k<new_moves.size(); ++k) 
	state.makeMove(new_moves[k]);
      new_record.result = (state.turn() == osl::BLACK
			   ? osl::Record::WhiteWin : osl::Record::BlackWin);
      // std::cerr << "debug " << j << ' ' << kisen.moves(j).size() << ' ' << new_moves.size() << "\n";
      accumulator(kisen.moves(j).size() - new_moves.size());
      if (new_moves.size() >= 256)
	std::cerr << "long record " << j << ' ' << new_moves.size() << "\n";
      ks.save(new_record.record);
      if (output_ipx)
      {
	ipx_writer->save(new_record,
			 ipx.rating(j, osl::BLACK),ipx.rating(j, osl::WHITE), 
			 ipx.title(j, osl::BLACK), ipx.title(j, osl::WHITE));
      }
      if ((j % 1000) == 999)
	std::cerr << input_filename[i] << " " << j
		  << " max " << acc::max(accumulator)
		  << " mean " << acc::mean(accumulator) << "\n";
    }
    std::cerr << input_filename[i]
	      << " max " << acc::max(accumulator)
	      << " mean " << acc::mean(accumulator) << "\n";
  }
}

int main(int argc, char **argv)
{
  bool output_ipx, trim;
  std::string kisen_filename;
  size_t checkmate_limit;
  int concurrency;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("trim-from-last",
     boost::program_options::value<bool>(&trim)->default_value(true),
     "trim last checkmate sequence")
    ("output-ipx",
     boost::program_options::value<bool>(&output_ipx)->default_value(true),
     "Whether output IPX file in addition to KIF file")
    ("output-kisen-filename,o",
     boost::program_options::value<std::string>(&kisen_filename)->
     default_value("test.kif"),
     "Output filename of Kisen file")
    ("checkmate-limit,l",
     boost::program_options::value<size_t>(&checkmate_limit)->default_value(1000),
     "Whether output IPX file in addition to KIF file")
    ("num-cpus,N",
     boost::program_options::value<int>(&concurrency)->default_value(std::thread::hardware_concurrency()),
     "concurrency")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
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
      std::cerr << "Usage: " << argv[0] << " [options] kisen-files\n";
      std::cerr << "       " << argv[0] << " [options]\n";
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] kisen-files\n";
    std::cerr << "       " << argv[0] << " [options]\n";
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  std::vector<std::string> files;
  if (vm.count("input-file"))
    files = vm["input-file"].as<std::vector<std::string> >();

  osl::OslConfig::setUp();
  osl::OslConfig::setNumCPUs(concurrency);

  convert(files, kisen_filename, checkmate_limit, output_ipx, trim);
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
