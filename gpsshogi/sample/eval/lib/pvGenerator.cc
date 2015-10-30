/* pvGenerator.cc
 */
#include "pvGenerator.h"
#include "pvFile.h"
#include "quiesce.h"
#include "analyzer.h"

#include "osl/record/kisen.h"
#include "osl/record/csaRecord.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/move_classifier/shouldPromoteCut.h"
#include "osl/progress/effect5x3.h"
#include "osl/progress.h"
#include "osl/move_probability/featureSet.h"

#include <unordered_set>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iostream>

int gpsshogi::KisenAnalyzer::normal_depth = 2;
int gpsshogi::KisenAnalyzer::quiesce_depth = 4;
int gpsshogi::KisenAnalyzer::use_percent = 100;
bool gpsshogi::KisenAnalyzer::compare_pass = false;
bool gpsshogi::KisenAnalyzer::bonanza_compatible_search = false;
int gpsshogi::KisenAnalyzer::window_asymmetry = 1;
int gpsshogi::PVGenerator::limit_sibling = 0;
std::vector<int> gpsshogi::KisenAnalyzer::priority_record;

gpsshogi::KisenAnalyzer::
RecordConfig::RecordConfig(size_t tid, size_t f, size_t l, const std::string& kisen, bool a)
  : first(f), last(l), thread_id(tid), kisen_filename(kisen), allow_skip_in_cross_validation(a)
{
}

gpsshogi::
KisenAnalyzer::KisenAnalyzer(const RecordConfig& r, const OtherConfig& c, Result *out)
  : record(r), config(c), result(out)
{
  bonanza_compatible_search = (quiesce_depth < 0);
}

gpsshogi::
KisenAnalyzer::~KisenAnalyzer()
{
}

bool gpsshogi::
KisenAnalyzer::isCrossValidation() const
{
  return false;
}

void gpsshogi::
KisenAnalyzer::init() 
{
}

void gpsshogi::KisenAnalyzer::operator()()
{
  result->werrors.clear();
  result->toprated.clear();
  result->toprated_strict.clear();
  result->order_lb.clear();
  result->order_ub.clear();
  result->record_processed = result->skip_by_rating = 0;
  result->node_count = result->siblings = 0;
  result->all_errors.clear();

  init();
  KisenFile kisen_file(record.kisen_filename.c_str());
  std::unique_ptr<KisenIpxFile> ipx;
  if (config.min_rating)
    ipx.reset(new KisenIpxFile(KisenFile::ipxFileName(record.kisen_filename)));

  assert(record.last >= record.first);
  std::vector<size_t> orders(record.last - record.first);
  for (size_t i=0; i<orders.size(); ++i)
    orders[i] = i + record.first;
  if (use_percent < 100) {
    std::random_shuffle(orders.begin(), orders.end());
    const size_t limit = orders.size()*use_percent/100;
    if (! priority_record.empty()) {
      size_t p = 0, q = limit;
      while (p < limit && q < orders.size()) {
	if (std::binary_search(priority_record.begin(), priority_record.end(), 
			       orders[p])) {
	  ++p;
	  continue;
	}
	if (! std::binary_search(priority_record.begin(), priority_record.end(),
				 orders[q])) {
	  ++q;
	  continue;
	}
	std::swap(orders[p], orders[q]);
	++p, ++q;
      }
    }
    orders.resize(limit);
  }
  std::unordered_set<HashKey, std::hash<HashKey>> visited;
  for (size_t i: orders) {
    if (record.allow_skip_in_cross_validation && isCrossValidation()) {
      if (i % 4)
	continue;
    }    
    if (i % 100 == 0) {
      if (i % ((record.thread_id == 0) ? 500 : 1000) == 0) {
	std::cerr << i << " ";
      }
      else
	std::cerr << ".";
    }
    if (config.min_rating
	&& (ipx->rating(i, BLACK) < config.min_rating 
	    || ipx->rating(i, WHITE) < config.min_rating)) {
      ++(result->skip_by_rating);
      continue;
    }
#if 0
    std::cerr << "go quiesce " << i << "\n";
#endif
    ++result->record_processed;

    NumEffectState state(kisen_file.initialState());
    const std::vector<Move> moves=kisen_file.moves(i);
    Quiesce quiesce(config.my_eval, normal_depth, quiesce_depth);
    osl::progress::ml::NewProgress progress(state);
    stat::Average record_errors;
    for (size_t j=0; j<moves.size(); j++) {
      const Player turn = state.turn();
      // 自分の手番で相手の王が利きがある => 直前の手が非合法手
      if (state.inCheck(alt(turn))) {
	std::cerr << "e"; // state;
	break;
      }
      if (config.max_progress < 16) {
	if (progress.progress16().value() > config.max_progress)
	  break;
      }
      assert(j < moves.size());
      HashKey key(state);
      key.add(moves[j]);
      const bool ignore = (j < 40) && ! visited.insert(key).second;
      if (! ignore
	  && (! (isCrossValidation() && record.allow_skip_in_cross_validation)
	      || (j+config.position_randomness) % 4 == 0)) {
	for (size_t k=j; k+1<moves.size(); ++k)
	  quiesce.addBigram(moves[k], moves[k+1]);
	forEachPosition(i, j, moves.size(), quiesce,
			state, progress.progress16(), moves[j]);
	if (isCrossValidation())
	  record_errors.add(result->last_error);
      }
      result->node_count += quiesce.nodeCount();
      quiesce.clear();
      state.makeMove(moves[j]);
      progress.update(state, moves[j]);
    }

    if (isCrossValidation())
      result->all_errors.push_back(std::make_tuple
				   (i, record_errors.average()));    
  }

  if (!isCrossValidation())
  {
    for (size_t i = 0; i < record.csa_filenames.size(); ++i)
    {
      CsaFile csa(record.csa_filenames[i]);
      NumEffectState state = csa.initialState();
      const std::vector<Move> moves = csa.moves();
      if (!moves.empty())
      {
        Quiesce quiesce(config.my_eval, normal_depth, quiesce_depth);
        osl::progress::ml::NewProgress progress(state);
	forEachPosition(-i - 1, 0, 1, quiesce,
			state, progress.progress16(), moves[0]);
      }
    }
  }
      
  std::cerr << "#" << std::flush;
}

#define VERBOSE_ASSIGN

void gpsshogi::
KisenAnalyzer::splitFile(const std::string& file, size_t first, size_t last, int num_assignment, double average_records, 
			 RecordConfig *out, int& written, bool verbose)
{
  for (int j=0; j<num_assignment; ++j) {
    size_t last_j = (j+1 == num_assignment) ? last : first+(int)floor(average_records);
#ifdef VERBOSE_ASSIGN
    if (verbose)
      std::cerr << "   " << written << " " << file << " " << first << " " << last_j << std::endl;
#endif
    out[written] = RecordConfig(written, first, last_j, file);
    ++written;
    first = last_j;
  }
}

void gpsshogi::
KisenAnalyzer::splitFileWithMoves(const std::string& file, size_t first, size_t last, int num_assignment, double /*average_records*/,
                                  const std::vector<std::string>& csa_files,
				  RecordConfig *out, int& written, bool verbose)
{
  if (num_assignment == 0 || first == last)
    return;
  KisenFile kisen_file(file);
  std::vector<int> moves(last-first+1);
  for (size_t i=first; i<last; ++i)
    moves[i-first+1] = 
      kisen_file.moves(i).size()
      + std::max((int)kisen_file.moves(i).size()-60, 0)
      + std::max((int)kisen_file.moves(i).size()-120, 0)
      + std::max((int)kisen_file.moves(i).size()-180, 0)
      + moves[i-first];
  const int average_moves = moves.back()/num_assignment;
  size_t cur = first;
  const int csa_number = csa_files.size() / num_assignment;
  for (int j=0; j<num_assignment; ++j) {
    size_t last_j;
    if (j+1 == num_assignment) 
    {
      last_j = last;
    }
    else 
    {
      last_j = cur+1;
      while (moves[last_j-first] - moves[cur-first] < average_moves
	     && (last_j == cur+1
		 || (average_moves - (moves[last_j-first] - moves[cur-first])
		     > (moves[last_j+1-first] - moves[cur-first]) - average_moves)))
	++last_j;
    }
#ifdef VERBOSE_ASSIGN
    if (verbose)
      std::cerr << "   " << written << " " << file << " " << cur << " " << last_j
		<< " (" << moves[last_j-first] - moves[cur-first] << ")" << std::endl;
#endif
    out[written] = RecordConfig(written, cur, last_j, file);
    const int csa_last_index = (j + 1 == num_assignment) ? csa_files.size() : (csa_number * (j + 1));
    for (int k = csa_number * j; k < csa_last_index; ++k)
    {
      out[written].csa_filenames.push_back(csa_files[k]);
    }
    ++written;
    cur = last_j;
  }
}

void gpsshogi::
KisenAnalyzer::distributeJob(size_t split, RecordConfig *out, size_t kisen_start, size_t num_records,
			     const std::vector<std::string>& files, size_t /*min_rating*/,
                             const std::vector<std::string>& csa_files)
{
  size_t total = 0;
  std::vector<size_t> totals(files.size()+1), sizes(files.size());
  for (size_t i=0; i<files.size(); ++i) {
    totals[i] = total;
    try {
      KisenFile kisen_file(files[i].c_str());
      sizes[i] = kisen_file.size(); // todo min_rating
#ifdef VERBOSE_ASSIGN
      if (files.size() > 1)
	std::cerr << "  size " << files[i] << " " << sizes[i] << std::endl;
#endif
    }
    catch (osl::CsaIOError&) {
      std::cerr << "open failed " << files[i] << "\n";
      sizes[i] = 0;
    }
    total += sizes[i];
  }
  totals[files.size()] = total;
  if (total <= kisen_start) {
    std::cerr << "warning KisenAnalyzer::distributeJob kisen_start too large "
	      << kisen_start << " >= " << total << std::endl;
    return;
  }
  // find the range of files to be processed
  if (num_records == 0 || kisen_start+num_records > total)
    num_records = total - kisen_start;

  size_t start_file = 0;
  while (start_file < files.size() && totals[start_file+1] <= kisen_start)
    ++start_file;
  size_t end_file = start_file+1;
  while (end_file <= files.size() && totals[end_file] < kisen_start+num_records)
    ++end_file;

  // first, try to assgin at least one cpu for each file
  int cpu_left = split;
  const int offset = kisen_start - totals[start_file];
  std::vector<int> assignment(files.size()); 
  std::vector<double> amount(files.size());
  for (size_t i=0; i<std::min(split, end_file-start_file); ++i) {
    const int file = start_file+i;
    assignment[file] = 1;
    amount[file] = sizes[file];
    if (i == 0) 
    {
      assert(amount[start_file] > offset);
      amount[start_file] -= offset;
    }
    if (start_file+i+1 == end_file) 
    {
      amount[end_file-1] -= (totals[end_file]-num_records-offset-totals[start_file]);
      assert(amount[end_file-1] > 0);
    }
    --cpu_left;
  }
  // then, assign other cpus to the largest files in a greedy way.
  while (cpu_left--) {
    size_t file = std::max_element(amount.begin(), amount.end()) - amount.begin();
    double total = amount[file]*assignment[file];
    assignment[file]++;
    amount[file] = total / assignment[file];
  }
  
  // write out
  int written = 0;
  for (size_t i=0; i<std::min(split, end_file-start_file); ++i) {
    const size_t file_id = start_file+i;
    if (assignment[file_id] == 0)
      continue;
    size_t first = 0, last = sizes[file_id];
    if (i == 0)
      first = kisen_start - totals[start_file];
    if (file_id+1 == end_file) 
      last -= (totals[end_file]-num_records-offset-totals[start_file]);
#if 1
    splitFileWithMoves(files[file_id], first, last, assignment[file_id], amount[file_id],
                       csa_files, out, written,
		       files.size() > 1);
#else
    splitFile(files[file_id], first, last, assignment[file_id], amount[file_id], out, written,
	      files.size() > 1);
#endif
  }
}

/* ------------------------------------------------------------------------- */

gpsshogi::
Validator::Validator(const RecordConfig& r, const OtherConfig& c, Result *out)
  : KisenAnalyzer(r, c, out)
{
}

gpsshogi::
Validator::~Validator()
{
}

bool gpsshogi::
Validator::isCrossValidation() const
{
  return true;
}

void gpsshogi::
Validator::forEachPosition(int /*record_id*/, int /*position_id*/, size_t,
			   Quiesce& quiesce, const NumEffectState& state, 
			   osl::Progress16, Move best_move)
{
  const int turn_sign = (state.turn() == BLACK) ? 1 : -1;
  double cur_errors = 0.0;
  result->last_error = 0.0;
  PVVector pv;
  pv.push_back(best_move);
  int best_value;  
  {
    NumEffectState new_state = state;
    new_state.makeMove(best_move);
    if (! quiesce.quiesce(new_state, best_value, pv))
      return;
  }
  if (abs(best_value) == quiesce.infty(BLACK))
    return;

  MoveVector moves;
  state.generateLegal(moves);
  if (moves.size() < 2 || ! moves.isMember(best_move))
    return;
  if (! bonanza_compatible_search && compare_pass && ! state.inCheck())
    moves.push_back(Move::PASS(state.turn()));

  size_t move_id = 0;
  std::vector<int> values; values.reserve(moves.size());
  values.push_back(best_value*turn_sign);
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p, ++move_id) {
    if (*p == best_move)
      continue;
    pv.clear();
    int value;
    {
      NumEffectState new_state = state;
      new_state.makeMove(*p);
      pv.push_back(*p);
      const int width = (int)(config.window_by_pawn*config.my_eval->pawnValue());
      if (! quiesce.quiesce(new_state, value, pv, best_value - width, best_value + width))
	continue;
    }
    if (abs(value) == quiesce.infty(BLACK))
      continue;
    values.push_back(value*turn_sign);
    cur_errors += SigmoidUtil::alphax((value - best_value)*turn_sign, config.sigmoid_alpha);
    result->siblings += 1;
  }
  std::sort(values.begin(), values.end());
  result->toprated.add(values.back()== best_value*turn_sign); // top?
  result->toprated_strict.add(values.back() == best_value*turn_sign
			      && values.size() > 1 && values[values.size()-2] != best_value*turn_sign);
  result->order_lb.add(values.end() - std::lower_bound(values.begin(), values.end(), best_value));
  result->order_ub.add(values.end() - std::upper_bound(values.begin(), values.end(), best_value));
  result->werrors.add(cur_errors);
  result->last_error = cur_errors;
}

/* ------------------------------------------------------------------------- */

gpsshogi::
PVGenerator::PVGenerator(const std::string& pv_base, 
			 const RecordConfig& r, const OtherConfig& c, Result *out)
  : KisenAnalyzer(r, c, out),
    pv_filename(pv_file(pv_base, r.thread_id))
{
  std::ostringstream ss;
  ss << pv_base << r.thread_id << "-stat.txt";
  stat_filename = ss.str();
}

gpsshogi::
PVGenerator::~PVGenerator()
{
}

const std::string gpsshogi::
PVGenerator::pv_file(const std::string& pv_base, size_t thread_id) 
{
  std::ostringstream ss;
  ss << pv_base << thread_id << ".gz";
  return ss.str();
}

void gpsshogi::
PVGenerator::init() 
{
  pw.reset(new PVFileWriter(pv_filename.c_str()));
}

gpsshogi::
PVGenerator::SearchResult gpsshogi::
PVGenerator::searchSiblings(Quiesce& quiesce, int width,
			    const NumEffectState& state, 
			    osl::Progress16 progress, Move best_move,
			    int rand_seed, size_t accept_rank_threshold,
			    int accept_piece_sacrifice_threshold,
			    int& best_value, PVVector& best_pv, values_t& out,
			    size_t& rank_search, size_t& rank_probability,
			    int & outrange_better_move,
			    int & outrange_other_move)
{
  const int ext2limit = 8;
  out.clear();
  outrange_better_move = outrange_other_move = 0;
  // generate all
  MoveVector moves;
  rank_probability = rank_search = 0;
  size_t rank_search_sacrifice = 0; 
#ifdef USE_ALL_MOVES
  LegalMoves::generate(state, moves);
#else
  static const move_probability::StandardFeatureSet& feature_set
    = move_probability::StandardFeatureSet::instance();
  MoveLogProbVector all;
  MoveStack h;
  move_probability::StateInfo info(state, progress, h);
  feature_set.generateLogProb2(info, all);
  size_t i=0;
  for (; i<std::min(all.size(), (size_t)48); ++i) 
    moves.push_back(all[i].move());
  for (; i<all.size(); ++i)
    if (((rand_seed + i) % 8 == 0) || bonanza_compatible_search)
      moves.push_back(all[i].move());
  const MoveLogProb *selected = all.find(best_move);
  if (!selected)
    return BestMoveNotFound;
  if (moves.size() == 1)
    return OneReply;
  rank_probability = selected - &*all.begin();
#endif
  if (! bonanza_compatible_search && compare_pass && ! state.inCheck())
    moves.push_back(Move::PASS(state.turn()));

  // search bestmove
  const int full_depth = quiesce.fullWidthDepth(), qdepth = quiesce.quiesceDepth();
  best_pv.push_back(best_move);
  {
    if (! bonanza_compatible_search) {
      if (rank_probability < 4 && full_depth > 0 && progress.value() < ext2limit)
	quiesce.setDepth(full_depth+2, qdepth);
      else if (rank_probability < 8 && full_depth > 0)
	quiesce.setDepth(full_depth+1, qdepth);
    }
    NumEffectState new_state = state;
    new_state.makeMove(best_move);
    const bool good_pv = quiesce.quiesce(new_state, best_value, best_pv);
    if (! good_pv)
      return BestmoveSearchFailed;
  }
  if (abs(best_value) == quiesce.infty(BLACK))
    return PVCheckmate;
  const int turn_sign = (state.turn() == BLACK) ? 1 : -1;
  const int alpha = best_value - width*turn_sign/window_asymmetry;
  const int beta = best_value + width*turn_sign;
  best_value *= turn_sign;
  const int best_piece = best_pv.pieceValueOfTurn();
  // search other moves
  PVVector pv;
  size_t move_id = 0;
  out.reserve(moves.size());
  NumEffectState copy;
  for (MoveVector::const_iterator p=moves.begin(); p!=moves.end(); ++p, ++move_id) {
    if (*p == best_move)
      continue;
    if (! p->isPass()
	&& osl::ShouldPromoteCut::canIgnoreAndNotDrop(*p)
	&& state.hasEffectAt(alt(p->player()), p->to()))
      continue;
    pv.clear();
    int value;
    {
      if (! bonanza_compatible_search) {
	if (move_id < 4 && rank_probability < 4 && full_depth > 0 && progress.value() < ext2limit)
	  quiesce.setDepth(full_depth+2, qdepth);
	else if (move_id < 8 && rank_probability < 8 && full_depth > 0)
	  quiesce.setDepth(full_depth+1, qdepth);
	else if (move_id >= 24 && move_id >= rank_probability && full_depth > 0)
	  quiesce.setDepth(full_depth-1, qdepth);
	else
	  quiesce.setDepth(full_depth, qdepth);
      }
      copy.copyFrom(state);
      copy.makeMove(*p);
      pv.push_back(*p);
      if (! quiesce.quiesce(copy, value, pv, alpha, beta))
	continue;
      value *= turn_sign;
      if (! bonanza_compatible_search
	  && quiesce.fullWidthDepth() < full_depth && value > best_value) {
	pv.clear();
	pv.push_back(*p);
	quiesce.setDepth(full_depth, qdepth);
	if (! quiesce.quiesce(copy, value, pv, alpha, beta))
	  continue;
      }
    }
    if (abs(value) == quiesce.infty(BLACK))
      continue;
    if (bonanza_compatible_search && abs(value - best_value) >= width){
      if (value > best_value) 
	++outrange_better_move;
      else
	++outrange_other_move;
      continue;
    }
    if (value > best_value) {
      ++rank_search;
      if (pv.pieceValueOfTurn()
	  >= best_piece + accept_piece_sacrifice_threshold) {
	++rank_search_sacrifice;
	if (! bonanza_compatible_search && rank_search_sacrifice >= accept_rank_threshold)
	  return DifficultMove; 
      }
    }
    out.push_back(std::make_pair(value, pv));
  }
  if (out.empty() && outrange_better_move + outrange_other_move == 0)
    return OneReplyBySearch;
  return SearchOK;
}

void gpsshogi::
PVGenerator::forEachPosition(int record_id, int position_id, size_t position_size,
			     Quiesce& quiesce, const NumEffectState& state, 
			     osl::Progress16 progress, Move best_move)
{
  const int width = config.window_by_pawn*config.my_eval->pawnValue();
  const int full_depth = quiesce.fullWidthDepth(), qdepth = quiesce.quiesceDepth();
  int best_value, outrange_better_move, outrange_other_move;
  PVVector best_pv;
  values_t values;
  size_t rank_search, rank_probability;
  SearchResult analyses_result = searchSiblings
    (quiesce, width, state, progress, best_move,
     record_id+position_id+config.position_randomness+record.thread_id,
     (position_size - position_id < 10) ? 1 : 16,
     256,
     best_value, best_pv, values, rank_search, rank_probability,
     outrange_better_move, outrange_other_move);
  quiesce.setDepth(full_depth, qdepth);
  if (analyses_result != SearchOK) {
    if (analyses_result == DifficultMove) {
      result->toprated.add(false);
      result->toprated_strict.add(false);
    } else if (analyses_result == OneReplyBySearch) {
      result->werrors.add(0);
      result->toprated.add(true);
      result->toprated_strict.add(true);
    }
    return;
  }

  double cur_errors = 0.0;
  for (const value_and_pv_t& p: values) {
    cur_errors += SigmoidUtil::alphax(p.first - best_value,
				      config.sigmoid_alpha);
  }
  cur_errors += outrange_better_move;
  result->siblings += values.size();
  result->siblings += outrange_better_move + outrange_other_move;
  if (values.empty()) {
    result->werrors.add(cur_errors);
    result->toprated.add(outrange_better_move == 0);
    result->toprated_strict.add(outrange_better_move == 0);
    return;
  }
  pw->newPosition(record_id, position_id);
  pw->addPv(best_pv);
  
  if (limit_sibling <= 0) 
  {
    values_t::const_iterator p
      = std::max_element(values.begin(), values.end());
    result->toprated.add(outrange_better_move == 0 && p->first <= best_value);
    result->toprated_strict.add(outrange_better_move == 0 && p->first < best_value);
  }
  else
  {
    std::sort(values.begin(), values.end()); // ascending order
    result->toprated.add(outrange_better_move ==0 && values.back().first <= best_value);
    result->toprated_strict.add(outrange_better_move == 0 && values.back().first < best_value);
    size_t i=0;
    for (;i<values.size(); ++i) 
      if (values[i].first > best_value)
	break;
    values.resize(std::min(i+limit_sibling, values.size()));
  }
  for (size_t i=0; i<values.size(); ++i) {
    bool good_move = values[i].first > best_value - config.window_by_pawn*config.my_eval->pawnValue();
    if (good_move || ((record_id + position_id + i + config.position_randomness) % 8 == 0))
      pw->addPv(values[i].second);
  }
  result->werrors.add(cur_errors);
  result->last_error = cur_errors;
}

/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; coding:utf-8
// ;;; End:
