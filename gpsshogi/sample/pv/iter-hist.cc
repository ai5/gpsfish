#include <tcutil.h>
#include <tchdb.h>
#include "moves.pb.h"
#include "osl/record/compactBoard.h"
#include "osl/apply_move/applyMove.h"
#include "osl/state/numEffectState.h"
#include "osl/record/csaRecord.h"
#include "osl/misc/carray3d.h"
#include "gpsshogi/dbm/tokyoCabinet.h"

#include <iostream>
#include <sstream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace osl;

int main(int argc, char **argv)
{
  int max_records;
  bool tsv;
  boost::program_options::options_description command_line_options;
  command_line_options.add_options()
    ("max-records",
     po::value<int>(&max_records)->default_value(-1),
     "Number of records to process.  -1 means all records.")
    ("tsv",
     po::value<bool>(&tsv)->default_value(false),
     "Output result in TSV format")
    ;

  po::variables_map vm;
  try
  {
    po::store(po::parse_command_line(argc, argv, command_line_options),
	      vm);
    po::notify(vm);
  }
  catch (std::exception& e)
  {
    std::cerr << "error in parsing options" << std::endl
	      << e.what() << std::endl;
    std::cerr << command_line_options << std::endl;
    return 1;
  }

  gpsshogi::dbm::TokyoCabinetWrapper tc("pvs-iter.tch", HDBOREADER);

  int count = 0;
  tc.initIterator();
  std::string key, value;
  CArray<int, 10> histogram_all;
  histogram_all.fill(0);
  CArray2d<int, 16, 10> histogram_progress;
  histogram_progress.fill(0);
  CArray2d<int, 16, 10> histogram_depth;
  histogram_depth.fill(0);
  CArray3d<int, 16, 16, 10> histogram_progress_depth;
  histogram_progress_depth.fill(0);
  while (tc.next(key, value))
  {
    PvPerIter pv;
    pv.ParseFromString(value);
    int prev = 0;
    for (int i = 0; i < pv.iter_size(); ++i)
    {
      if (std::abs(pv.iter(i).value()) > 3000)
      {
	goto done;
      }
    }
    for (int i = 1; i < pv.iter_size(); ++i)
    {
      if (pv.iter(i).depth() == pv.iter(prev).depth())
      {
	continue;
      }
      const int depth = (pv.iter(i).depth() - 400) / 200;
      const int bucket = std::min(std::abs(pv.iter(i - 1).value() - pv.iter(i).value()) / 128,
				  static_cast<int>(histogram_all.size() - 1));
      ++histogram_all[bucket];
      ++histogram_progress[pv.progress()][bucket];
      ++histogram_depth[depth][bucket];
      ++histogram_progress_depth[pv.progress()][depth][bucket];
      prev = i;
    }
    if (max_records >= 0 &&
	++count >= max_records)
    {
      break;
    }
  done:
    ;
  }
  if (tsv)
  {
    for (size_t i = 0; i < histogram_all.size(); ++i)
    {
      std::cout << i << "\t";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < histogram_all.size(); ++i)
    {
      std::cout << histogram_all[i] << "\t";
    }
    std::cout << std::endl;
    std::cout << "Progress/bucket" << "\t";
    for (size_t j = 0; j < histogram_progress.size2(); ++j)
    {
      std::cout << j << "\t";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < histogram_progress.size1(); ++i)
    {
      std::cout << i << "\t";
      for (size_t j = 0; j < histogram_progress.size2(); ++j)
      {
	std::cout << histogram_progress[i][j] << "\t";
      }
      std::cout << std::endl;
    }
    std::cout << "Depth/bucket" << "\t";
    for (size_t j = 0; j < histogram_depth.size2(); ++j)
    {
      std::cout << j << "\t";
    }
    std::cout << std::endl;
    for (size_t i = 0; i < histogram_depth.size1(); ++i)
    {
      std::cout << i << "\t";
      for (size_t j = 0; j < histogram_depth.size2(); ++j)
      {
	std::cout << histogram_depth[i][j] << "\t";
      }
      std::cout << std::endl;
    }
    for (size_t i = 0; i < histogram_progress_depth.size(); ++i)
    {
      std::cout << "Progress " << i << std::endl;
      std::cout << "Depth/bucket" << "\t";
      for (size_t k = 0; k < histogram_progress_depth[i].size2(); ++k)
      {
	std::cout << k << "\t";
      }
      std::cout << std::endl;
      for (size_t j = 0; j < histogram_progress_depth[i].size1(); ++j)
      {
	std::cout << j << "\t";
	for (size_t k = 0; k < histogram_progress_depth[i].size2(); ++k)
	{
	  std::cout << histogram_progress_depth[i][j][k] << "\t";
	}
	std::cout << std::endl;
      }
    }
  }
  else
  {
    for (size_t i = 0; i < histogram_all.size(); ++i)
    {
      std::cout << i << " " << histogram_all[i] << std::endl;
    }
    for (size_t i = 0; i < histogram_progress.size1(); ++i)
    {
      std::cout << "Progress " << i << std::endl;
      for (size_t j = 0; j < histogram_progress.size2(); ++j)
      {
	std::cout << j << " " << histogram_progress[i][j] << std::endl;
      }
    }
    for (size_t i = 0; i < histogram_depth.size1(); ++i)
    {
      std::cout << "Depth " << i << std::endl;
      for (size_t j = 0; j < histogram_depth.size2(); ++j)
      {
	std::cout << j << " " << histogram_depth[i][j] << std::endl;
      }
    }

    for (size_t i = 0; i < histogram_progress_depth.size(); ++i)
    {
      std::cout << "Progress " << i << std::endl;
      for (size_t j = 0; j < histogram_progress_depth[i].size1(); ++j)
      {
	std::cout << "Depth " << j << std::endl;
	for (size_t k = 0; k < histogram_progress_depth[i].size2(); ++k)
	{
	  std::cout << k << " " << histogram_progress_depth[i][j][k] << std::endl;
	}
      }
    }
  }
  return 0;
}
