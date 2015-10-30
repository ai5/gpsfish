/* piecePairRawTable.cc
 */
#include "osl/eval/ppair/piecePairRawEval.h"
#include "osl/eval/ppair/piecePairEval.tcc"
#include "osl/oslConfig.h"
#include <boost/filesystem.hpp>
#ifdef __MINGW32__
#  include <boost/filesystem/fstream.hpp>
#endif
#include <algorithm>
#include <fstream>
#include <vector>
#include <iostream>

osl::eval::ppair::PiecePairRawTable osl::eval::ppair::PiecePairRawTable::Table;

namespace osl
{
  namespace eval
  {
    namespace ppair
    {
      static_assert(PiecePairIndex::maxPtypeOIndex == 32, "");
      // roundup が 2^n かつ 2 以上であることの確認
      static_assert((PiecePairEvalBase::ROUND_UP 
		     & (PiecePairEvalBase::ROUND_UP-1))
		    == 0, "");
      static_assert(PiecePairEvalBase::ROUND_UP >= 2, "");

      template class PiecePairEvalTableBase<PiecePairRawTable>;
      template class PiecePairEval<PiecePairRawEval,PiecePairRawTable>;
    } // namespace ppair
  } // namespace eval
} // namespace osl

osl::eval::ppair::
PiecePairRawTable::PiecePairRawTable()
{
}

osl::eval::ppair::
PiecePairRawTable::~PiecePairRawTable()
{
}

void osl::eval::ppair::
PiecePairRawTable::writeInBinaryFile(const char *filename) const
{
  std::ofstream os(filename);
  std::vector<signed char> buffer(82*PTYPEO_SIZE*82*PTYPEO_SIZE);
  for(int i=0;i<82;i++){
    Square pos0=SquareCompressor::melt(i);
    for(int j=0;j<82;j++){
      Square pos1=SquareCompressor::melt(j);
      for(int k=0;k<PTYPEO_SIZE;k++)
	for(int l=0;l<PTYPEO_SIZE;l++){
	  int index0=PiecePairIndex::indexOf(pos0,static_cast<PtypeO>(k+PTYPEO_MIN));
	  int index1=PiecePairIndex::indexOf(pos1,static_cast<PtypeO>(l+PTYPEO_MIN));
	  int index=PiecePairIndex::indexOf(index0,index1);

	  buffer[(i*32+k)*82*32+(j*32+l)]=values[index];
	}
    }

  }
  os.write(reinterpret_cast<const char*>(&*buffer.begin()),buffer.size());
}

bool osl::eval::ppair::
PiecePairRawTable::
loadFromBinaryFile(const char *filename) const
{
  if (OslConfig::verbose())
    std::cerr << "PiecePairRawTable loading...  " << filename << "\n";

#ifdef __MINGW32__
  namespace bf = boost::filesystem;
  bf::path filename_path(filename);
  bf::ifstream is(filename_path, std::ios_base::in | std::ios_base::binary);

  is.exceptions(std::ios_base::badbit | std::ios_base::failbit);
  // Workaround a bug of MINGW.
  std::vector<signed char> temp_buffer(82*PTYPEO_SIZE);
  is.rdbuf()->pubsetbuf(reinterpret_cast<char*>(&*temp_buffer.begin()), temp_buffer.size());
#else
  std::ifstream is(filename);
#endif
  std::vector<signed char> buffer(82*PTYPEO_SIZE*82*PTYPEO_SIZE);
  is.read(reinterpret_cast<char*>(&*buffer.begin()), buffer.size());
  for(int i=0;i<82;i++){
    Square pos0=SquareCompressor::melt(i);
    for(int j=0;j<82;j++){
      Square pos1=SquareCompressor::melt(j);
      for(int k=0;k<PTYPEO_SIZE;k++)
       for(int l=0;l<PTYPEO_SIZE;l++){
         int index0=PiecePairIndex::indexOf(pos0,static_cast<PtypeO>(k+PTYPEO_MIN));
         int index1=PiecePairIndex::indexOf(pos1,static_cast<PtypeO>(l+PTYPEO_MIN));
         int index=PiecePairIndex::indexOf(index0,index1);
         if(!pos0.isPieceStand() && !pos1.isPieceStand())
           values[index]=buffer[(i*32+k)*82*32+(j*32+l)];
         else
           values[index]=0;
       }
    }
  }

  if (! is)
    return false;

  if (OslConfig::verbose())
    std::cerr << "done.\n";
  return true;
}

bool osl::eval::ppair::
PiecePairRawTable::
setUp(const char *filename) const
{
  static std::string filename_memory;
  if (! filename_memory.empty())
  {
    if (filename_memory != filename)
    {
      std::cerr << "PiecePairRawTable: don't load " << filename 
		<< ", since " << filename_memory
		<< " already loaded \n";
      return false;
    }
    return true;
  }
  filename_memory = filename;
  return loadFromBinaryFile(filename);
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
