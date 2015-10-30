/* group.cc
 */
#include "osl/rating/group.h"
#include "osl/rating/feature/karanari.h"
#include "osl/rating/feature/pinAttack.h"
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstdio>
#include <cmath>

osl::rating::Group::Group(const std::string& name)
  : group_name(name)
{
}

osl::rating::Group::~Group()
{
}

int osl::rating::Group::findMatch(const NumEffectState& state, Move m, const RatingEnv& env) const
{
  for (size_t j=0; j<size(); ++j) {
    if ((*this)[j].match(state, m, env)) 
      return j;
  }
  return -1;
}

void osl::rating::Group::saveResult(const std::string& directory, const range_t& range, 
				    const std::vector<double>& weights) const
{
  {
    boost::filesystem::path dir(directory);
    boost::filesystem::create_directory(dir);
  }
  
  std::string filename = directory + "/" + group_name + ".txt";
  std::ofstream os(filename.c_str());
  for (int i=range.first; i<range.second; ++i)
    os << std::setprecision(8) << weights[i] << "\n";
}

bool osl::rating::Group::load(const std::string& directory, const range_t& range, 
			      std::vector<double>& weights) const
{  
  std::string filename = directory + "/" + group_name + ".txt";
  FILE *fp = fopen(filename.c_str(), "r");
  if (! fp)
    return false;
  for (int i=range.first; i<range.second; ++i) {
    if (fscanf(fp, "%lf", &weights[i]) != 1)
      return false;
  }
  fclose(fp);
  return true;
}

void osl::rating::Group::show(std::ostream& os, int name_width, const range_t& range, 
			      const std::vector<double>& weights) const
{
#ifndef MINIMAL
  for (size_t f=0; f<size(); ++f) {
    os << std::setw(name_width) 
       << (*this)[f].name() 
       << "  " << 400*log10(weights[f+range.first]) << "\n";
  }
#endif
}

void osl::rating::Group::showAll(std::ostream& os, int name_width, const range_t& range, 
				 const std::vector<double>& weights) const
{
#ifndef MINIMAL
  showMinMax(os, name_width, range, weights);
  for (size_t i=0; i<size(); ++i) {
    os << " " << (*this)[i].name() << " " << 400*log10(weights[i+range.first]);
  }
  os << "\n";
#endif
}
void osl::rating::Group::showMinMax(std::ostream& os, int name_width, const range_t& range, 
				    const std::vector<double>& weights) const 
{
#ifndef MINIMAL
  double min = 10000000000.0, max = -min;
  for (size_t i=0; i<size(); ++i) {
    min = std::min(min, 400*log10(weights[i+range.first]));
    max = std::max(max, 400*log10(weights[i+range.first]));
  }
  os << std::setw(name_width) 
     << group_name
     << "  [" << min << " -- " << max << "] ";
#endif
}

void osl::rating::Group::showTopN(std::ostream& os, int name_width, const range_t& range, 
				  const std::vector<double>& weights, int n) const
{
#ifndef MINIMAL
  if ((int)weights.size() <= n*2)
    return showAll(os, name_width, range, weights);
  showMinMax(os, name_width, range, weights);
  std::vector<double> w;
  w.reserve(size());
  for (int i=range.first; i<range.second; ++i)
    w.push_back(weights[i]);
  std::sort(w.begin(), w.end());
  for (int i=0; i<n; ++i) {
    double value = w[size()-1-i];
    int j=range.first;
    for (; j<range.second; ++j)
      if (weights[j] == value)
	break;
    os << " " << (*this)[j-range.first].name() << " " << 400*log10(value);
  }
  os << " ... ";
  for (int i=0; i<n; ++i) {
    double value = w[n-1-i];
    int j=range.first;
    for (; j<range.second; ++j)
      if (weights[j] == value)
	break;
    os << " " << (*this)[j-range.first].name() << " " << 400*log10(value);
  }  
  os << "\n";
#endif
}

osl::rating::
ChaseGroup::ChaseGroup() : Group("Chase")
{
  for (int o=0; o<4; ++o) {
    for (int t=PTYPE_PIECE_MIN; t<=PTYPE_MAX; ++t) {
      Ptype target = static_cast<Ptype>(t);
      for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
	Ptype self = static_cast<Ptype>(s);
	push_back(new Chase(self, target, false, static_cast<Chase::OpponentType>(o)));
	if (isBasic(self))
	  push_back(new Chase(self, target, true, static_cast<Chase::OpponentType>(o)));
      }      
    }
  }
}

int osl::rating::
ChaseGroup::findMatch(const NumEffectState& state, Move move, const RatingEnv& env) const
{
  Move last_move = env.history.lastMove();
  if (! last_move.isNormal())
    return -1;
  if (! state.hasEffectIf(move.ptypeO(), move.to(), last_move.to()))
    return -1;
  int base = 0;
  const int unit = (PTYPE_MAX+1 - PTYPE_PIECE_MIN)*(PTYPE_MAX+1-PTYPE_PIECE_MIN+PTYPE_MAX+1-PTYPE_BASIC_MIN);
  if (last_move.capturePtype() == PTYPE_EMPTY) {
    if (last_move.isDrop()) {
      base = unit;
    } else {
      if (state.hasEffectAt(state.turn(), last_move.from()))
	base = unit*2;
      else
	base = unit*3;
    }
  }
  Ptype self = move.ptype();
  Ptype target = last_move.ptype();
  int index = base + (target - PTYPE_PIECE_MIN)*(PTYPE_MAX+1-PTYPE_PIECE_MIN+PTYPE_MAX+1-PTYPE_BASIC_MIN);
  if (isBasic(self)) {
    index += (PTYPE_BASIC_MIN - PTYPE_PIECE_MIN);
    index += (self - PTYPE_BASIC_MIN)*2;
    index += move.isDrop();
  } else {
    index += (self - PTYPE_PIECE_MIN);
  }
  assert((*this)[index].match(state, move, env));
  return index;
}

osl::rating::KaranariGroup::KaranariGroup() : Group("Karanari")
{
  push_back(new Karanari(false, true));
  push_back(new Karanari(false, false));
  push_back(new Karanari(true, true));
  push_back(new Karanari(true, false));
}

int osl::rating::
KaranariGroup::findMatch(const NumEffectState& state, Move move, const RatingEnv&) const
{
  return Karanari::index(state, move);
}

osl::rating::
ImmediateAddSupportGroup::ImmediateAddSupportGroup() 
  : Group("ImmediateAddSupport")
{
  for (int s=PTYPE_PIECE_MIN; s<=PTYPE_MAX; ++s) {
    for (int a=PTYPE_PIECE_MIN; a<=PTYPE_MAX; ++a) {
      for (int p=0; p<8; ++p)	// progress8
	push_back(new ImmediateAddSupport(static_cast<Ptype>(s), static_cast<Ptype>(a)));
    }
  }
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
