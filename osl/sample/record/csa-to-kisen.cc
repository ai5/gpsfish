#include "osl/numEffectState.h"
#include "osl/hashKey.h"
#include "osl/record/ki2.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/record/kakinoki.h"
#include "osl/record/checkDuplicate.h"
#include "osl/record/kanjiCode.h"
#include "osl/misc/filePath.h"
#include "osl/oslConfig.h"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/format.hpp>
#include <boost/progress.hpp>
#include <boost/regex.hpp>
// #include <regex>
#include <deque>
#include <exception>
#include <iostream>
#include <fstream>

std::vector<std::string> good_tournaments;
bool accept_tournament(const std::string& name) 
{
  for (const std::string& tournament: good_tournaments) {
    if (tournament.find(name) == 0)
      return true;
    else if (tournament.find(name) != tournament.npos)
      std::cerr << tournament << " != " << name << "\n";
  }
  return good_tournaments.empty();
}

std::string heuristic_find_title(osl::Record& record, osl::Player player)
{
  static const osl::CArray<const char*,25> titles = {{
      K_K1 K_DAN, K_K2 K_DAN, K_K3 K_DAN, 
      K_K4 K_DAN, K_K5 K_DAN, K_K6 K_DAN, K_K7 K_DAN, K_K8 K_DAN, K_K9 K_DAN, 
      K_MEIJIN, K_PROOK2 K_KING2, K_KING2 K_KURAI, K_KING2 K_SHOU, K_KING2 K_ZA,
      K_KI K_KING2, K_KI K_SEI, 
      K_K2 K_KANMURI, K_K3 K_KANMURI, K_K4 K_KANMURI, K_K5 K_KANMURI,
      K_K6 K_KANMURI, K_K7 K_KANMURI, K_K8 K_KANMURI, K_K9 K_KANMURI, 
      K_JORYUU,
    }};
  std::string name = record.player[player];
  std::string title_found = "";
  for (const char *title: titles) {
    if (boost::algorithm::iends_with(name, title)) {
      title_found = title + title_found;
      name.resize(name.size() - strlen(title));
    }
  }
  record.player[player] = name;
  return title_found;
}

void run(osl::record::Record& record,
	 osl::KisenWriter& ks,
	 std::unique_ptr<osl::KisenIpxWriter>& ipx_writer,
	 osl::record::CheckDuplicate& check_duplicates,
	 int default_rating, int min_year, int max_year)
{
  boost::gregorian::date date = record.start_date;
  if (min_year > 0 && (date.is_special() || date.year() < min_year))
    return;
  if (max_year > 0 && (date.is_special() || date.year() > max_year))
    return;
  // 重複チェック 
  const std::vector<osl::Move>& moves = record.moves();
  if (check_duplicates.regist(moves))
    return;

  std::string black_title = heuristic_find_title(record, osl::BLACK);
  std::string white_title = heuristic_find_title(record, osl::WHITE);
  ks.save(record.record);
  if (ipx_writer)
  {
    ipx_writer->save(record, default_rating, default_rating,
		     black_title, white_title);
  }
}
static void convert(const std::string &kisen_filename,
		    const std::vector<std::string> &files,
		    bool output_ipx,
                    osl::record::CheckDuplicate& check_duplicates,
		    int default_rating, int min_year, int max_year)
{
  std::ofstream ofs(kisen_filename.c_str());
  osl::KisenWriter ks(ofs);

  std::unique_ptr<osl::record::KisenIpxWriter> ipx_writer;
  std::unique_ptr<std::ofstream> ipx_ofs;
  if (output_ipx)
  {
    const boost::filesystem::path ipx_path =
      boost::filesystem::change_extension(boost::filesystem::path(kisen_filename), ".ipx");
    const std::string ipx = osl::misc::file_string(ipx_path);
    ipx_ofs.reset(new std::ofstream(ipx.c_str()));
    ipx_writer.reset(new osl::record::KisenIpxWriter(*ipx_ofs));
  }

  boost::progress_display progress(files.size());
  boost::regex date_time_regex("/(20[0-9][0-9]/[0-9][0-9]/[0-9][0-9])/");
  for (size_t i = 0; i < files.size(); ++i, ++progress)
  {
    const std::string& filename = files[i];
    try
    {
      osl::record::Record record;
      if (boost::algorithm::iends_with(filename, ".kif")) 
      {
	if (osl::KakinokiFile::isKakinokiFile(filename))
	{
	  const osl::KakinokiFile kif(filename);
	  record = kif.load();
	}
	else
	{
	  osl::KisenFile kisen(filename);
	  osl::KisenIpxFile ipx(kisen.ipxFileName());
	  for (size_t j=0; j<kisen.size(); ++j) {
	    osl::record::Record record;
	    record.record = {kisen.initialState(), kisen.moves(j)};
	    record.player[osl::BLACK] = ipx.player(j, osl::BLACK)
			     + ipx.title(j, osl::BLACK);
	    record.player[osl::WHITE] = ipx.player(j, osl::WHITE)
			     + ipx.title(j, osl::WHITE);
	    record.start_date = ipx.startDate(j);
	    run(record, ks, ipx_writer, check_duplicates,
		default_rating, min_year, max_year);
	  }
	  if (kisen.size() > 0)
	    continue;		// it was actually kisen file, going to next file
	}
	// fall through
      }
      else if (boost::algorithm::iends_with(filename, ".csa"))
      {
        const osl::CsaFile csa(filename);
        record = csa.load();
      }
      else if (boost::algorithm::iends_with(filename, ".ki2"))
      {
        const osl::Ki2File ki2(filename);
        record = ki2.load();
	// std::cerr << osl::misc::eucToLang(record.tounamentName()) << "\n";
	if (! accept_tournament(record.tournament_name))
	  continue;
      }
      else
      {
        std::cerr << "Unknown file type: " << filename << "\n";
        continue;
      }
      if (record.start_date.is_special()) {
	boost::smatch match;
	if (boost::regex_search(filename, match, date_time_regex)) {
	  std::string s(match[1].first, match[1].second);
	  record.start_date = boost::gregorian::from_string(s);
	}
      }
      run(record, ks, ipx_writer, check_duplicates, default_rating, min_year, max_year);
    }
    catch(std::exception& e)
    {
      std::cerr << "ERROR: reading " <<  files[i] << "; " << 
        e.what() << std::endl;
      continue;
    }
  }
}

int main(int argc, char **argv)
{
  bool output_ipx;
  std::string kisen_filename, tournament_filename;
  int default_rating, year, min_year, max_year;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("output-ipx",
     boost::program_options::value<bool>(&output_ipx)->default_value(true),
     "Whether output IPX file in addition to KIF file")
    ("tournament-file", boost::program_options::value<std::string>(&tournament_filename)
     ->default_value(""),
     "ignore records unless the name of their tournament is listed in the file in EUC-JP")
    ("year", boost::program_options::value<int>(&year)->default_value(0),
     "year to select (0 for all)")
    ("min-year", boost::program_options::value<int>(&min_year)->default_value(0),
     "min year to select (0 for all)")
    ("max-year", boost::program_options::value<int>(&max_year)->default_value(0),
     "max year to select (0 for all)")
    ("kisen-filename,o",
     boost::program_options::value<std::string>(&kisen_filename)->
     default_value("test.kif"),
     "Output filename of Kisen file")
    ("input-file", boost::program_options::value< std::vector<std::string> >(),
     "input files in kisen format")
    ("default-rating", boost::program_options::value<int>(&default_rating)->
     default_value(0),
     "default rating")
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
      std::cerr << "Usage: " << argv[0] << " [options] csa-files | ki2-files \n";
      std::cerr << "       " << argv[0] << " [options]\n";
      std::cout << command_line_options << std::endl;
      return 0;
    }
  }
  catch (std::exception &e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << "Usage: " << argv[0] << " [options] csa-files | ki2-files\n";
    std::cerr << "       " << argv[0] << " [options]\n";
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  if (tournament_filename != "")
  {
    std::ifstream is(tournament_filename.c_str());
    std::string name;
    while(std::getline(is, name))
    {
      boost::algorithm::trim(name);
      good_tournaments.push_back(name);
    }
    if (good_tournaments.empty())
      throw std::runtime_error("read failed "+tournament_filename);
  }
  if (year)
    min_year = max_year = year;

  std::vector<std::string> files;
  if (vm.count("input-file"))
  {
    const std::vector<std::string> temp = vm["input-file"].as<std::vector<std::string> >();
    files.insert(files.end(), temp.begin(), temp.end());
  }
  else
  {
    std::string line;
    while(std::getline(std::cin , line))
    {
      boost::algorithm::trim(line);
      files.push_back(line);
    }
  }
  osl::OslConfig::setUp();
  osl::record::CheckDuplicate check_duplicate;
  convert(kisen_filename, files, output_ipx, check_duplicate, default_rating,
	  min_year, max_year);

  std::locale::global(std::locale(""));
  check_duplicate.print(std::cout);

  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
