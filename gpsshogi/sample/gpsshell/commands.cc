#include "commands.h"
#include "board.h"
#include "book.h"
#include "httpclient.h"
#include "ignorelist.h"
#include "gpsshogi/revision.h"
#ifdef USE_TOKYO_CABINET
#  include "gpsshogi/recorddb/facade.h"
#endif
#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
#  include "gpsshogi/redis/searchResult.h"
#endif
#endif
#include "osl/eval/pieceEval.h"
#include "osl/eval/progressEval.h"
#include "osl/eval/openMidEndingEval.h"
#include "osl/numEffectState.h"
#include "osl/book/compactBoard.h"
#include "osl/record/usiRecord.h"
#include "osl/record/csaRecord.h"
#include "osl/record/kisen.h"
#include "osl/record/ki2.h"
#include "osl/record/kakinoki.h"
#include "osl/record/myshogi.h"
#include "osl/book/openingBook.h"
#include "osl/book/bookInMemory.h"
#include "osl/game_playing/alphaBetaPlayer.h"
#include "osl/game_playing/gameState.h"
#include "osl/search/alphaBeta3.h"
#include "osl/search/alphaBeta4.h"
#include "osl/search/usiProxy.h"
#include "osl/search/searchRecorder.h"
#include "osl/misc/filePath.h"
#include "osl/random.h"
#include "osl/misc/sjis2euc.h"
#include "osl/search/moveGenerator.h"
#include "osl/search/searchState2.h"
#include "osl/search/simpleHashTable.h"
#include "osl/search/quiescenceSearch2.h"
#include "osl/search/quiescenceGenerator.h"
#include "osl/search/quiescenceRecord.h"
#include "osl/rating/featureSet.h"
#include "osl/rating/ratingEnv.h"
#include "osl/move_probability/featureSet.h"
#include "osl/misc/eucToLang.h"
#include "osl/c/facade.h"
#include "osl/oslConfig.h"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp> // with I/O
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <memory>
#include <deque>
#include <iostream>

/*========== Global variables ==========*/

extern boost::filesystem::path cache_dir;
extern std::unique_ptr<gpsshell::Board> board;
extern std::unique_ptr<gpsshell::Book> the_book;
extern std::string redis_server_host;
extern int redis_server_port;
extern std::string redis_password;

static gpsshell::IgnoreList the_ignorelist;
/* The stack of the boards the have ever been seen*/
static std::deque<gpsshell::Board> boards;

namespace { // anonymous
  const std::string adjust_move_string(const std::string& src, const osl::NumEffectState& state)
  {
    if (src[0] == '%')
      return src;
    std::string result = src;
    for (size_t i=0; i<result.size(); ++i)
      result[i] = toupper(result[i]);
    if (result[0] != '+' && result[0] != '-')
      result.insert(0, osl::csa::show(state.turn()));
    if (result.size() == 5) {
      const int from = (result[1]-'0')*10 + (result[2]-'0');
      if (from >= 11 && from <= 99 && (from % 10)) {
        osl::Ptype ptype = state.pieceOnBoard(osl::Square(from/10, from%10)).ptype();
        result += osl::Ptype_Table.getCsaName(ptype);
      }
    }
    return result;
  }
} // anonymous namespace

void setCommandsMain(gpsshell::MySession* s)
{
  s->addCommand("exit",                  0 );
  s->addCommand("quit",                  0 );
  s->addCommand("help",                  &gpsshell::MySession::help );
  s->addCommand("version",               &gpsshell::MySession::version );
  s->addCommand("open %file",            &gpsshell::MySession::open );
  s->addCommand("openurl",               &gpsshell::MySession::openurl );
  s->addCommand("next",                  &gpsshell::MySession::next );
  s->addCommand("n",                     &gpsshell::MySession::next );
  s->addCommand("prev",                  &gpsshell::MySession::prev );
  s->addCommand("p",                     &gpsshell::MySession::prev );
  s->addCommand("first",                 &gpsshell::MySession::first );
  s->addCommand("last",                  &gpsshell::MySession::last );
  s->addCommand("history",               &gpsshell::MySession::history );
  s->addCommand("move",                  &gpsshell::MySession::move );
  s->addCommand("rollback",              &gpsshell::MySession::rollBack );
  s->addCommand("eval",                  &gpsshell::MySession::eval );
  s->addCommand("recorddb",              &gpsshell::MySession::recorddb );
  s->addCommand("search",                &gpsshell::MySession::search );
  s->addCommand("qsearch",               &gpsshell::MySession::qsearch );
  s->addCommand("search4",               &gpsshell::MySession::search4 );
  s->addCommand("search_usi",            &gpsshell::MySession::search_usi );
  s->addCommand("checkmate_attack",      &gpsshell::MySession::checkmate_attack );
  s->addCommand("checkmate_escape",      &gpsshell::MySession::checkmate_escape );
  s->addCommand("threatmate",            &gpsshell::MySession::threatmate );
  s->addCommand("generatemoves",         &gpsshell::MySession::generateMoves );
  s->addCommand("generatelegalmoves",    &gpsshell::MySession::generateLegalMoves );
  s->addCommand("generate_not_losing_moves",    &gpsshell::MySession::generateNotLosingMoves );
  s->addCommand("rating",                &gpsshell::MySession::rating );
  s->addCommand("rating2",               &gpsshell::MySession::rating2 );
  s->addCommand("annotate",              &gpsshell::MySession::annotate );
  s->addCommand("quiescemoves",          &gpsshell::MySession::quiescemoves );
  s->addCommand("csashow",               &gpsshell::MySession::csaShow );
  s->addCommand("usishow",               &gpsshell::MySession::usiShow );
  s->addCommand("usihistory",            &gpsshell::MySession::usiHistory );
  s->addCommand("myshogi",               &gpsshell::MySession::myshogi );
  s->addCommand("showstates",            &gpsshell::MySession::showStates );
  s->addCommand("position",              &gpsshell::MySession::usiposition );
  s->addCommand("set_black_color",       &gpsshell::MySession::setBlackColor );
  s->addCommand("set_white_color",       &gpsshell::MySession::setWhiteColor );
  s->addCommand("set_last_color",        &gpsshell::MySession::setLastMoveColor );
  s->addCommand("set_verbose",           &gpsshell::MySession::setVerbose );
  // --- commands rerated with opening books
  s->addCommand("set_openingbook %file", &gpsshell::MySession::setOpeningBook );
  s->addCommand("oshow",                 &gpsshell::MySession::openingShow );
  s->addCommand("onext",                 &gpsshell::MySession::openingNext );
  s->addCommand("onext_random_level",    &gpsshell::MySession::openingNextRandomLevel );
  s->addCommand("onext_random_weight",   &gpsshell::MySession::openingNextRandomWeight );
  s->addCommand("oinclude",              &gpsshell::MySession::openingInclude );
  s->addCommand("oshow_shortest_lines",  &gpsshell::MySession::openingShowShortestLines);
  s->addCommand("book_in_memory",	 &gpsshell::MySession::showBookInMemory);
  // --- commands rerated with ignore lists
  s->addCommand("set_ignorelists %file", &gpsshell::MySession::setIgnoreList );
  s->addCommand("ignorelist_show",       &gpsshell::MySession::ignoreListShow );
  s->addCommand("ignorelist_showall",    &gpsshell::MySession::ignoreListShowAll );
  s->addCommand("ignorelist_next",       &gpsshell::MySession::ignoreListNext );
  s->addCommand("ignorelist_prev",       &gpsshell::MySession::ignoreListPrev );
  s->addCommand("ignorelist_first",      &gpsshell::MySession::ignoreListFirst );
  s->addCommand("ignorelist_last",       &gpsshell::MySession::ignoreListLast );
#ifdef OSL_SMP
  s->addCommand("set_num_cpus",          &gpsshell::MySession::setNumCPUs );
#endif
}

gpsshell::
MySession::MySession()
      : verbose(1)
{
#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
  redis_context = NULL;
  if (!redis_server_host.empty() &&
      redis_server_port>0) {
    gpsshogi::redis::connectRedisServer(&redis_context, redis_server_host, redis_server_port);
    if (!redis_context) {
      std::cerr << "Failed to connect to the Redis server" << std::endl;
    } else if (!redis_password.empty()) {
      if (!gpsshogi::redis::authenticate(redis_context, redis_password)) {
        std::cerr << "Failed to authenticate to the Redis server" << std::endl;
        redisFree(redis_context);
        redis_context = NULL;
      }
    }
  }
#endif
#endif
}

gpsshell::
MySession::~MySession()
{
#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
  redisFree(redis_context);
#endif
#endif
}


void gpsshell::
MySession::addCommand(const std::string& arg1, MemberFunc arg2)
{
  advanced_completers.push_back(MyElement(arg1, arg2));
}

class LookupFunctor
{
  const std::vector<std::string>& params;

public:
  // Creates a functor and memorises a refence to tokens
  LookupFunctor(const std::vector<std::string>& _params) :
    params(_params)
  {}

  // Compares the first token only
  bool operator()(const gpsshell::MySession::MyElement&  element) const
  {
    if (params.empty())
      return false;

    // accept parital match
    return strncmp(params.begin()->c_str(),
                   static_cast<std::string>(element).c_str(),
                   params.begin()->size()) == 0;
  }
};

bool gpsshell::MySession::executeCommand(const std::vector<std::string>& tokens)
{
  MyContainer::iterator found = 
    find_if(advanced_completers.begin(), 
            advanced_completers.end(), 
            LookupFunctor(tokens));
  if (found == advanced_completers.end()) {
    std::cout << 
      "Unknown command. Type 'help' for the list of acceptable commands" 
      << std::endl;
    return false;
  }

  if (found->second != 0) {
    MemberFunc f = found->second; // execute the action
    f(this, tokens);
    return true;
  }
  return false;
}

void gpsshell::
MySession::pushBoard()
{
  if (!board->isInitialState())
  {
    boards.push_back(*board);
  }
}

void gpsshell::
MySession::popBoard()
{
   if (boards.empty())
     return;
   *board = boards.back();
   boards.pop_back();
}

void gpsshell::
MySession::help(const std::vector<std::string>& /*params*/)
{
  std::cout << "List of the command variations:" << std::endl;
  for (MyContainer::const_iterator k=advanced_completers.begin();
        k != advanced_completers.end(); ++k)
  {
    std::cout << static_cast<const std::string&>(*k) << std::endl;
  }
  std::cout << "Press Ctrl+D for gracefull exit" << std::endl;
}

void gpsshell::
MySession::rollBack(const std::vector<std::string>& /*params*/)
{
  popBoard();
}

bool gpsshell::
MySession::expandTildaInPath(const std::string& in,
                             std::string& out)
{
  if (in.empty())
    return false;

  if (in[0] == '~') {
    if (!getenv("HOME")) {
      return false;
    }
    out = std::string(getenv("HOME")) + in.substr(1);
  } else {
    out = in;
  }
  return true;
}

void gpsshell::
MySession::open(const std::vector<std::string>& params)
{
  if (params.size() < 2) {
    std::cout << "The open command supposes a csa or kif file name" << std::endl;
    return;
  }
  if (params[1].find("http://") == 0) {
    openurl(params);
    return;
  }
  if (params[1] == "-") {
    openstdin();
    return;
  }
  std::string path_str;
  if (!expandTildaInPath(params[1], path_str)) {
    std::cout << "Invalid file path: " << params[1] << std::endl;
    return;
  }

  const boost::filesystem::path path(path_str);
  if (!boost::filesystem::exists(path)) {
    std::cout << "File not found: " << path << std::endl;
    return;
  }
  const std::string ext = boost::filesystem::extension(path);

  if (boost::algorithm::iequals(".csa", ext))
  {
    const osl::CsaFile file(osl::misc::file_string(path));
    const osl::Record record = file.load();
    std::vector<osl::Move> moves;
    std::vector<int> time;
    std::vector<std::string> comments;
    std::vector<osl::SearchInfo> search_info;
    record.load(moves, time, comments, search_info);
    board->setMoves(file.initialState(), moves);
    board->setTime(time);
    board->setComments(comments);
    board->setSearchInfo(search_info);
    if (params.size() == 3)
      board->next(boost::lexical_cast<size_t>(params[2]));
  }
  else if (boost::algorithm::iequals(".usi", ext))
  {
    const osl::UsiFile file(osl::misc::file_string(path));
    const auto moves = file.load().moves();
    board->setMoves(file.initialState(), moves);
  }
  else if (boost::algorithm::iequals(".ki2", ext))
  {
    const osl::Ki2File file(osl::misc::file_string(path));
    const auto moves = file.load().moves();
    board->setMoves(file.initialState(), moves);
  }
  else if (boost::algorithm::iequals(".kif", ext) 
	   && osl::KakinokiFile::isKakinokiFile(osl::misc::file_string(path)))
  {
    const osl::KakinokiFile file(osl::misc::file_string(path));
    const osl::Record record = file.load();
    std::vector<osl::Move> moves;
    std::vector<int> time;
    std::vector<std::string> comments;
    std::vector<osl::SearchInfo> search_info;
    record.load(moves, time, comments, search_info);
    board->setMoves(file.initialState(), moves);
    board->setComments(comments);
  }
  else if (boost::algorithm::iequals(".kif", ext))
  {
    size_t index = 0;
    if (params.size() == 3)
      index = boost::lexical_cast<size_t>(params[2]);
    osl::KisenFile file(osl::misc::file_string(path));
    osl::KisenIpxFile ipx(file.ipxFileName());
    if (index >= file.size())
    {
      std::cout << "Error: Out of range " << index << "/" << file.size()-1 <<std::endl;
      return;
    }
      std::cout << "Opened " << index << "/" << file.size()-1 << "\n"
                << "  StartDate: " << ipx.startDate(index) << "\n";
    const osl::Player both[] = {osl::BLACK, osl::WHITE};
    for (const osl::Player p: both) {
      std::cout << "  " << p << ": "
                << osl::misc::sjis2euc(ipx.player(index, p))
                << "@" << ipx.rating(index, p)
                << " [" << osl::misc::sjis2euc(ipx.title(index, p))  << "]\n";
    }

    const std::vector<osl::Move> moves = file.moves(index);
    board->setMoves(file.initialState(), moves);
  }
  board->showState();
}

void gpsshell::
MySession::usiposition(const std::vector<std::string>& params)
{
  std::string all = params[0];
  for (size_t i=1; i<params.size(); ++i)
    all += " " + params[i];
  osl::NumEffectState initial_state;
  std::vector<osl::Move> moves;
  try {
    osl::usi::parse(all.substr(8), initial_state, moves);
  }
  catch (std::exception& e) {
    std::cerr << "usi parse error " << e.what() << "\n";
    return;
  }
  board->setMoves(initial_state, moves);
  board->showState();
}

void gpsshell::
MySession::openstdin()
{
  std::string lines, line;
  while (std::getline(std::cin, line)) {
    if (line == "")
      break;
    lines += line + "\n";
  }
  std::istringstream is(lines);
  const osl::CsaFile file(is);
  const osl::Record record = file.load();
  std::vector<osl::Move> moves;
  std::vector<int> time;
  std::vector<std::string> comments;
  std::vector<osl::SearchInfo> search_info;
  record.load(moves, time, comments, search_info);
  board->setMoves(file.initialState(), moves);
  board->setTime(time);
  board->setComments(comments);
  board->setSearchInfo(search_info);
  board->showState();
}

void gpsshell::
MySession::openurl(const std::vector<std::string>& params)
{
  if (params.size() < 2) {
    std::cout << "the openurl command supposes a url" << std::endl;
    return;
  }

  const std::string& url = params[1];
  const boost::filesystem::path temp_file 
    = cache_dir / boost::algorithm::replace_all_copy(url, "/", "_");
  if (gpsshell::getFileOverHttp(url, osl::misc::file_string(temp_file)))
    return;
  std::vector<std::string> tokens_to_pass;
  tokens_to_pass.push_back("open");
  tokens_to_pass.push_back(osl::misc::file_string(temp_file));
  if (params.size() == 3)
    tokens_to_pass.push_back(params[2]); // index
  open(tokens_to_pass);
}

/**
 * usage: next [step]
 */
void gpsshell::
MySession::next(const std::vector<std::string>& params)
{
  int distance = 1;
  if (params.size() > 1) {
    std::istringstream is(params[1]);
    is >> distance;
  }
  for (int i=0; i<distance; ++i)
    board->next();
  board->showState();
}

void gpsshell::
MySession::prev(const std::vector<std::string>& params)
{
  int distance = 1;
  if (params.size() > 1) {
    std::istringstream is(params[1]);
    is >> distance;
  }
  for (int i=0; i<distance; ++i)
    board->prev();
  board->showState();
}

void gpsshell::
MySession::first(const std::vector<std::string>& /*params*/)
{
  board->first();
  board->showState();
}

void gpsshell::
MySession::last(const std::vector<std::string>& /*params*/)
{
  board->last();
  board->showState();
}

void gpsshell::
MySession::history(const std::vector<std::string>& /*params*/)
{
  board->showHistory();
}

void gpsshell::
MySession::move(const std::vector<std::string>& params)
{
  if (params.size() < 2) {
    std::cout << "The move command supposes a move string" << std::endl;
    return;
  }
  for (size_t i=1; i<params.size(); ++i) {
    const std::string move_string = adjust_move_string(params[i], board->getState());
    osl::Move move;
    try {
      move = osl::csa::strToMove(move_string, board->getState());
    }
    catch (std::runtime_error& e) {
      std::cerr << "parse error " << e.what() << "\n";
      return;
    }
    if (! move.isPass()
	&& (!move.isValid() || ! board->getState().isValidMove(move)))
    {
      std::cout << "Invalid move: " << move << std::endl;
      return;
    }
    if (!board->isEndOfMoves())
    {
      pushBoard();
      board->shrink();
    }
    board->move(move);
    board->showState();
  }
}

void gpsshell::
MySession::eval(const std::vector<std::string>& params)
{
  std::string name = "test";
  if (params.size() >= 2)
    name = params[1];
  board->showEval(name);
}

void gpsshell::
MySession::recorddb(const std::vector<std::string>& params)
{
#ifdef USE_TOKYO_CABINET
  osl::NumEffectState state = board->getState();
  int win, loss, gps_win, gps_loss, bonanza_win, bonanza_loss;
  gpsshogi::recorddb::query(state, win, loss, gps_win, gps_loss, bonanza_win, bonanza_loss);
  if (win + loss)
    std::cout << osl::misc::eucToLang("¥×¥í ")
	      << win << osl::misc::eucToLang("¾¡")
	      << loss << osl::misc::eucToLang("ÇÔ ")
	      << 100.0*win/(win+loss) << "%" << std::endl;
  if (gps_win + gps_loss)
    std::cout << "GPS v.s. Bonanza "
	      << gps_win << osl::misc::eucToLang("¾¡")
	      << gps_loss << osl::misc::eucToLang("ÇÔ ")
	      << 100.0*gps_win/(gps_win+gps_loss) << "%" << std::endl;
  if (bonanza_win + bonanza_loss)
    std::cout << "Bonanza v.s. GPS "
	      << bonanza_win << osl::misc::eucToLang("¾¡")
	      << bonanza_loss << osl::misc::eucToLang("ÇÔ ")
	      << 100.0*bonanza_win/(bonanza_win+bonanza_loss) << "%" << std::endl;
#endif
}

void gpsshell::
MySession::search(const std::vector<std::string>& params)
{
  static const size_t time_default = 30;
  static size_t time = time_default;
  if (params.size() >= 2) 
  {
    try 
    {
      time = boost::lexical_cast<size_t>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) 
    {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: search <time_in_sec>" << std::endl
                << "time is now " << time << " "
                << "(default: " << time_default << ")"
                << std::endl;
      return;
    }
  }

  std::ostringstream os;
  os << board->getState();
  char move[8] =  {'\0'};
  ::search(os.str().c_str(), time, verbose, move);
  std::cout << move << std::endl;
}

void gpsshell::
MySession::search4(const std::vector<std::string>& params)
{
  static const size_t time_default = 30;
  static size_t time = time_default;
  if (params.size() >= 2) 
  {
    try 
    {
      time = boost::lexical_cast<size_t>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) 
    {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: search <time_in_sec>" << std::endl
                << "time is now " << time << " "
                << "(default: " << time_default << ")"
                << std::endl;
      return;
    }
  }

  osl::search::SearchState2::checkmate_t solver;
  osl::search::CountRecorder recorder;
  osl::SimpleHashTable table;
  table.setVerbose(true);    
  osl::NumEffectState state(board->getState());
  osl::search4::AlphaBeta4 search(state,solver,&table,recorder);
  osl::search::TimeAssigned assign(osl::milliseconds(time*1000),
				   osl::milliseconds(time*1000));
  osl::Move best_move = search.computeBestMoveIteratively(2000,100,0,0,assign);
  std::cout << best_move << std::endl;
}

void gpsshell::
MySession::search_usi(const std::vector<std::string>& params)
{
  static const size_t time_default = 30;
  static size_t time = time_default;
  if (params.size() >= 2) 
  {
    try 
    {
      time = boost::lexical_cast<size_t>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) 
    {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: search_usi <time_in_sec>" << std::endl
                << "time is now " << time << " "
                << "(default: " << time_default << ")"
                << std::endl;
      return;
    }
  }

  osl::search::SearchState2::checkmate_t solver;
  osl::search::CountRecorder recorder;
  osl::SimpleHashTable table;
  table.setVerbose(true);    
  osl::NumEffectState state(board->getState());
  osl::search::UsiProxy search(state,solver,&table,recorder);
  osl::search::TimeAssigned assign(osl::milliseconds(time*1000),
				   osl::milliseconds(time*1000));
  osl::Move best_move = search.computeBestMoveIteratively(2000,100,0,0,assign);
  std::cout << best_move << std::endl;
}

void show(const osl::search::QuiescenceRecord& qrecord)
{
  std::cerr << "best move " << osl::csa::show(qrecord.bestMove()) << "\n";
  std::cerr << "upper bound " << qrecord.upperBound() 
	    << " (" << qrecord.upperDepth() << ")\n";
  std::cerr << "lower bound " << qrecord.lowerBound() 
	    << " (" << qrecord.lowerDepth() << ")\n";
  std::cerr << "static value " << qrecord.staticValue() 
	    << " " << qrecord.staticThreat(0).value
	    << " " << osl::csa::show(qrecord.staticThreat(0).move)
	    << " " << qrecord.staticThreat(1).value
	    << " " << osl::csa::show(qrecord.staticThreat(1).move)
	    << "\n";
}

/**
 * Show an evaluation value after quiecense search.
 */
void gpsshell::
MySession::qsearch(const std::vector<std::string>& params)
{
  using namespace osl;
  using namespace osl::search;
  using namespace osl::misc;
  typedef QuiescenceSearch2<eval::ml::OpenMidEndingEval> qsearch_t;

  static const int depth_default = -2;
  static int depth = depth_default;

  if (params.size() >= 2) {
    try {
      depth = boost::lexical_cast<int>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: qsearch [depth]\n"
                << "  depth: depth [default=" << depth_default << "]\n"
                << "\n"
                << "Quiescence-search the current state\n"
                << std::endl;
      return;
    }
  }

  const NumEffectState state(board->getState());
  SimpleHashTable table(1000000, depth, verbose);
  SearchState2Core::checkmate_t  checkmate_searcher;
  SearchState2Core core(state, checkmate_searcher);
  qsearch_t qs(core, table);
  eval::ml::OpenMidEndingEval ev(state);

  Move last_move = board->getCurrentMove();
  if (! last_move.isNormal())
    last_move = Move::PASS(alt(state.turn()));
  const int val = qs.search(state.turn(), ev, last_move, 4);
  const int normalized_val = static_cast<int>(val*200.0/eval::ml::OpenMidEndingEval::captureValue(newPtypeO(WHITE,PAWN)));
  std::cout << normalized_val << std::endl;
  HashKey key = HashKey(state);
  const SimpleHashRecord *record = table.find(key);
  while (record) {
    show(record->qrecord);
    if (! record->qrecord.bestMove().isNormal()) 
      break;
    key = key.newMakeMove(record->qrecord.bestMove());
    record = table.find(key);
  }
}

void gpsshell::
MySession::checkmate_attack(const std::vector<std::string>& params)
{
  static const size_t limit_default = 20000000;
  int limit = limit_default;
  if (params.size() >= 2) 
  {
    try 
    {
      limit = boost::lexical_cast<size_t>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) 
    {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: checkmate_attack <limit>" << std::endl
                << "limit is now " << limit << " "
                << "(default: " << limit_default << ")"
                << std::endl;
      return;
    }
  }
  
  std::ostringstream os;
  os << board->getState();
  char move[8] = { '\0' };
  const int ret = ::checkmate_attack(os.str().c_str(), limit, move);
  if (ret)
  {
    std::cout << "Checkmate " << move << std::endl;
    std::cout << "node count " << limit << std::endl;
  }
  else
    std::cout << "No checkmate found" << std::endl;
}

void gpsshell::
MySession::checkmate_escape(const std::vector<std::string>& params)
{
  static const size_t limit_default = 20000000;
  static size_t limit = limit_default;
  if (params.size() >= 2) 
  {
    try 
    {
      limit = boost::lexical_cast<size_t>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) 
    {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: checkmate_escape <limit>" << std::endl
                << "limit is now " << limit << " "
                << "(default: " << limit_default << ")"
                << std::endl;
      return;
    }
  }
  
  std::ostringstream os;
  os << board->getState();
  const int ret = ::checkmate_escape(os.str().c_str(), limit);
  if (ret)
    std::cout << "Losing (Checkmated)" << std::endl;
  else
    std::cout << "No checkmate found" << std::endl;
}

void gpsshell::
MySession::threatmate(const std::vector<std::string>& params)
{
  static const size_t limit_default = 20000000;
  int limit = limit_default;
  if (params.size() >= 2) 
  {
    try 
    {
      limit = boost::lexical_cast<size_t>(params[1]);
    } 
    catch (boost::bad_lexical_cast& blc) 
    {
      std::cerr << blc.what() << std::endl;
      std::cerr << "USAGE: checkmate_attack <limit>" << std::endl
                << "limit is now " << limit << " "
                << "(default: " << limit_default << ")"
                << std::endl;
      return;
    }
  }
  
  std::ostringstream os;
  osl::NumEffectState state = board->getState();
  state.changeTurn();
  os << state;
  char move[8] = { '\0' };
  const int ret = ::checkmate_attack(os.str().c_str(), limit, move);
  if (ret)
  {
    std::cout << "Threatmate " << move << std::endl;
    std::cout << "node count " << limit << std::endl;
  }
  else
    std::cout << "No checkmate found" << std::endl;
}

void gpsshell::
MySession::generateMoves(const std::vector<std::string>& params)
{
  static int limit = 1000;
  if (params.size() > 1) {
    std::istringstream is(params[1]);
    is >> limit;
  }
  static osl::search::SearchState2::checkmate_t checkmate;
  const osl::NumEffectState state = board->getState();
  osl::search::SearchState2 sstate(state, checkmate);
  osl::search::MoveGenerator generator;
  generator.initOnce();
  osl::search::SimpleHashRecord record;
  generator.init(limit, &record, osl::eval::ml::OpenMidEndingEval(state),
		 state, true, osl::Move());
  osl::MoveLogProbVector moves;
  generator.generateAll(state.turn(), sstate, moves);

  osl::RatingEnv e;
  e.make(state);
  osl::RatedMoveVector rated_moves;
  osl::rating::StandardFeatureSet::instance().generateRating(state, e, limit, rated_moves);

  std::cout << "limit " << limit << "\n";
  for (size_t i=0; i<moves.size(); ++i) {
    std::cout << osl::csa::show(moves[i].move()) 
	      << "  " << moves[i].logProb();
    const osl::RatedMove *r = rated_moves.find(moves[i].move());
    if (r) {
      std::cout << "  " << r->rating() << "  " << r->optimisticRating();
    }
    std::cout << "\n";
  }
}

void gpsshell::
MySession::generateLegalMoves(const std::vector<std::string>& params)
{
  const osl::NumEffectState state = board->getState();
  osl::MoveVector moves;
  state.generateLegal(moves);
  for (osl::Move m: moves)
    std::cout << osl::csa::show(m) << "\n";
}

void gpsshell::
MySession::generateNotLosingMoves(const std::vector<std::string>& params)
{
  osl::game_playing::GameState state(board->getInitialState());
  std::vector<osl::Move> moves(board->getMovesToCurrent());
  for (osl::Move move: moves)
    state.pushMove(move);
  osl::MoveVector normal, loss;
  state.generateNotLosingMoves(normal, loss);
  std::cout << "normal moves\n";
  for (osl::Move m: normal)
    std::cout << osl::csa::show(m) << "\n";
  if (! loss.empty()) {
    std::cout << "losing moves\n";
    for (osl::Move m: loss)
      std::cout << osl::csa::show(m) << "\n";
  }
}

void gpsshell::
MySession::rating(const std::vector<std::string>& params)
{
  static int min_rate = -200;
  if (params.size() > 1) {
    std::istringstream is(params[1]);
    is >> min_rate;
  }
  static const osl::rating::StandardFeatureSet& fs=osl::rating::StandardFeatureSet::instance();
  const osl::NumEffectState state = board->getState();
  osl::RatingEnv e;
  e.make(state);
  osl::RatedMoveVector moves;
  fs.generateRating(state, e, 1400, moves);
  for (size_t i=0; i<moves.size(); ++i) {
    if (moves[i].rating() < min_rate)
      break;
    std::cout << osl::csa::show(moves[i].move()) 
	      << "  " << moves[i].rating() 
	      << "  " << moves[i].optimisticRating() 
	      << "\n";
  }
}

void gpsshell::
MySession::rating2(const std::vector<std::string>& params)
{
  const osl::NumEffectState state = board->getState();
  static int min_rate = -200;
  osl::Move single_move;
  if (params.size() > 1) {
    std::string option_string = params[1];
    try {
      single_move = osl::csa::strToMove(option_string, state);
    }
    catch (...) {
      std::istringstream is(option_string);
      is >> min_rate;
    }
  }
  static const osl::move_probability::StandardFeatureSet&
    fs = osl::move_probability::StandardFeatureSet::instance();
  osl::MoveStack history;
  if (board->getCurrentMove().isNormal())
    history.push(board->getCurrentMove());
  osl::progress::ml::NewProgress progress(state);
  osl::Move threatmate = osl::move_probability::StateInfo::findShortThreatmate(state, board->getCurrentMove());
  osl::move_probability::StateInfo info(state, progress.progress16(),
					history, threatmate);
  if (info.threatmate_move.isNormal())
    std::cout << "note: threatmate "
	      << osl::csa::show(info.threatmate_move) 
	      << "\n";
  if (single_move.isNormal()) {
    double p = fs.matchExp(info, single_move);
    std::cout << p << ' ' << log(p) << '\n';
  }
  osl::MoveLogProbVector moves;
  fs.generateLogProb2(info, moves);
  for (size_t i=0; i<moves.size(); ++i) {
    std::cout << std::setw(3) << i 
	      << std::setw(8) << osl::csa::show(moves[i].move()) 
	      << " " << std::setw(4) << moves[i].logProb();
    std::cout << ((i % 4 == 3) ? "\n" : "   ");
  }
  if (((moves.size()+3) % 4) != 3)
    std::cout << "\n";
}

void gpsshell::
MySession::quiescemoves(const std::vector<std::string>& params)
{
  using namespace osl;
  using osl::search::QuiescenceGenerator;
  const NumEffectState state = board->getState();
  MoveVector moves;
  if (state.turn() == BLACK)
    QuiescenceGenerator<BLACK>::promote(state, state.pin(alt(state.turn())),
					moves);
  else
    QuiescenceGenerator<WHITE>::promote(state, state.pin(alt(state.turn())),
					moves);
  std::cout << "promote";
  for (Move move: moves)
    std::cout << " " << osl::csa::show(move);
  std::cout << "\n";
}

void gpsshell::
MySession::annotate(const std::vector<std::string>& params)
{
  static osl::rating::StandardFeatureSet fs;

  if (params.size() != 2) {
    std::cout << "The annotate command supposes a move string" << std::endl;
    return;
  }
  const std::string move_string = adjust_move_string(params[1], board->getState());
  osl::Move move;
  try {
    move = osl::csa::strToMove(move_string, board->getState());
  }
  catch (std::runtime_error& e) {
    std::cerr << "parse error " << e.what() << "\n";
    return;
  }
  const osl::NumEffectState state = board->getState();
  if (!move.isValid() || !state.isValidMove(move)) {
    std::cout << "Invalid move: " << move << std::endl;
    return;
  }

  osl::RatingEnv env;
  env.make(state);
  osl::RatedMoveVector moves;
  fs.generateRating(state, env, 1400, moves);
  if (const osl::RatedMove *found = moves.find(move)) {
    std::cout << found->rating() << " ";
  }
  std::cout << "[ " << fs.annotate(state, env, move) << " ]\n";
  const int see = osl::PieceEval::computeDiffAfterMoveForRP(state, move);
  std::cout << "see " << see << "\n";
}

void gpsshell::
MySession::showStates(const std::vector<std::string>& /*params*/)
{
  while(board->next())
    board->showState();
}

void gpsshell::
MySession::csaShow(const std::vector<std::string>& params)
{
  if (params.size() == 1) {
    std::cout << board->getState();
    return;
  }
  std::ofstream os(params[1].c_str());
  os << board->getState();
}

void gpsshell::
MySession::usiShow(const std::vector<std::string>& params)
{
  if (params.size() == 1) {
    std::cout << "position " 
	      << osl::usi::show(board->getState())
	      << std::endl;
    return;
  }
  std::ofstream os(params[1].c_str());
  os << "position " 
     << osl::usi::show(board->getState())
     << std::endl;
}
void gpsshell::
MySession::usiHistory(const std::vector<std::string>& /*params*/)
{
  board->showUsiHistory();
}

void gpsshell::
MySession::myshogi(const std::vector<std::string>& params)
{
  if (params.size() == 1) {
    std::cout << osl::record::myshogi::show(board->getState());
    return;
  }
  std::ofstream os(params[1].c_str());
  os << osl::record::myshogi::show(board->getState());
}

void gpsshell::
MySession::setBlackColor(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    return;

  const std::string& color_name = params[1];
  osl::record::Color color = osl::record::Color::colorFor(color_name);
  board->setBlackColor(color);
  board->showState();
}

void gpsshell::
MySession::setWhiteColor(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    return;

  const std::string& color_name = params[1];
  osl::record::Color color = osl::record::Color::colorFor(color_name);
  board->setWhiteColor(color);
  board->showState();
}

void gpsshell::
MySession::setLastMoveColor(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    return;

  const std::string& color_name = params[1];
  osl::record::Color color = osl::record::Color::colorFor(color_name);
  board->setLastMoveColor(color);
  board->showState();
}

void gpsshell::
MySession::setVerbose(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    return;
  try 
  {
    verbose = boost::lexical_cast<int>(params[1]);
  } 
  catch (boost::bad_lexical_cast& blc) 
  {
    std::cerr << blc.what() << std::endl;
    std::cerr << "USAGE: set_verbose verboseness" << std::endl;
  }
}

/**
 * Set an opening book.
 * @param params a path of a opening book file.
 */
void gpsshell::
MySession::setOpeningBook(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    return;

  std::string path_str;
  if (!expandTildaInPath(params[1], path_str)) {
    std::cout << "Invalid file path: " << params[1] << std::endl;
    return;
  }

  the_book.reset(new ::gpsshell::Book(path_str));
}

void gpsshell::
MySession::openingShow(const std::vector<std::string>& params)
{
  if (!the_book)
    return;
  static int opeing_show_count = 3; // default value
  if (params.size() == 2)
    opeing_show_count = boost::lexical_cast<int>(params[1]);
  the_book->showState(board->getState(), board->getMovesToCurrent(), opeing_show_count);
#ifndef MINIMAL_GPSSHELL
#ifdef ENABLE_REDIS
  if (redis_context) {
    const osl::record::CompactBoard cb(board->getState());
    gpsshogi::redis::SearchResult sr(cb);
    if (gpsshogi::redis::querySearchResult(redis_context, sr)) {

    } else {
      std::cout << sr.toString() << std::endl;
    }
  }
#endif
#endif
}

void gpsshell::
MySession::openingNext(const std::vector<std::string>& params)
{
  if (!the_book)
    return;
  size_t nth = 0; // default value
  if (params.size() == 2)
    nth = boost::lexical_cast<size_t>(params[1]);

  const ::gpsshell::WMoveContainer wmoves = 
    the_book->getMoves(board->getState(), board->getMovesToCurrent());
  if (wmoves.empty())
  {
    std::cout << "There is no next move." << std::endl;
    return;
  }
  if (wmoves.at(0).weight == 0 && params.size() == 1)
  {
    // do not visit a zero-weighted move automatically.
    std::cout << "There is no weighted move. Specify an index (starting with zero) you want to visit.\n";
    return;
  }
  const osl::book::WMove wmove = wmoves.at(std::min(nth, wmoves.size()-1));
  const std::string move_str = osl::csa::show(wmove.move);
  std::vector<std::string> command;
  command.push_back(params[0]);
  command.push_back(move_str);
  move(command);
}

/**
 * usage: onext_random_level
 */
void gpsshell::
MySession::openingNextRandomLevel(const std::vector<std::string>& params)
{
  if (!the_book)
    return;

  ::gpsshell::WMoveContainer wmoves = 
    the_book->getMoves(board->getState(), board->getMovesToCurrent());

  ::gpsshell::deleteLessWeightedMoves(wmoves, 1); // remove zero-weighted moves
  if (wmoves.empty()) {
    std::cout << "There is no next move." << std::endl;
    return;
  }

  const size_t index = osl::misc::time_seeded_random() % wmoves.size();
  std::cout << boost::format("selected %2d of %2d move(s) [1,2,...]\n")
               % (index+1) % wmoves.size();
  const osl::book::WMove wmove = wmoves.at(index);
  const std::string move_str = osl::csa::show(wmove.move);
  std::vector<std::string> command;
  command.push_back(params[0]);
  command.push_back(move_str);
  move(command);
}

/**
 * usage: onext_random_weight [coef]
 */
void gpsshell::
MySession::openingNextRandomWeight(const std::vector<std::string>& params)
{
  if (!the_book)
    return;
  static int coef = 3;
  if (params.size() == 2)
    coef = boost::lexical_cast<int>(params[1]);
  if (coef <= 0) {
    std::cout << "coef should be > 0\n";
    coef = 3;
    return;
  }

  ::gpsshell::WMoveContainer wmoves = 
    the_book->getMoves(board->getState(), board->getMovesToCurrent());

  const int max_weight = ::gpsshell::getMaxWeight(wmoves);
  int criteria = max_weight / coef;
  if (criteria < 1)
    criteria = 1;
  ::gpsshell::deleteLessWeightedMoves(wmoves, criteria);
  if (wmoves.empty()) {
    std::cout << "There is no next move." << std::endl;
    return;
  }

  const int sum = ::gpsshell::getSumOfWeights(wmoves);
  assert(sum>0);
  const int target_weight = osl::misc::time_seeded_random() % sum;
  ::gpsshell::WMoveContainer::iterator it = wmoves.begin();
  for (int total = 0; it != wmoves.end(); ++it) {
    total += it->weight;
    if (target_weight < total)
      break;
  }

  std::cout << boost::format("selected %2d of %2d move(s) [1,2,...]\n")
               % (std::distance(wmoves.begin(), it) + 1) % wmoves.size();
  const osl::book::WMove wmove = *it;
  const std::string move_str = osl::csa::show(wmove.move);
  std::vector<std::string> command;
  command.push_back(params[0]);
  command.push_back(move_str);
  move(command);
}

/**
 * oinclude <file_name> <n-th move>
 */
void gpsshell::
MySession::openingInclude(const std::vector<std::string>& params)
{
  const static std::string USAGE = "USAGE: oinclude <file_name> <n-th move>\n";
  if (!the_book)
    return;

  if (params.size() != 3)
  {
    std::cerr << USAGE;
    return;
  }

  const boost::filesystem::path path(params[1]);
  const std::string ext = boost::filesystem::extension(path);
  if (!boost::algorithm::iequals(".csa", ext))
  {
    std::cerr << boost::format("Incorrect file name: %s\n") % params[1];
    std::cerr << USAGE;
    return;
  }

  size_t nth_move = 0;
  try
  {
    nth_move = boost::lexical_cast<size_t>(params[2]);
  }
  catch (boost::bad_lexical_cast& blc) 
  {
    std::cerr << blc.what() << std::endl;
    std::cerr << "USAGE: oinclude <file_name> <n-th move>" << std::endl;
    return;
  }

  try
  {
    const osl::CsaFile file(osl::misc::file_string(path));
    const osl::Record record = file.load();
    const std::vector<osl::Move> moves = record.moves();

    if (nth_move > moves.size())
    {
      std::cerr << boost::format("Invalid argument. %d is too large for the file %s [%d moves]\n")
                   % nth_move % osl::misc::file_string(path) % moves.size();
      std::cerr << USAGE;
      return;
    }

    osl::NumEffectState state;
    for (const osl::Move& move: moves)
    {
      state.makeMove(move);
    }

    osl::Player self = nth_move % 2 == 1 ? osl::BLACK : osl::WHITE;
    const std::shared_ptr<osl::book::WeightedBook> book = 
      the_book->getWeightedBook();
    const int rc =  book->stateIndex(state, false, self);
    if (rc == -1)
    {
      std::cout << "The state is not found in the book\n";
    }
    else
    {
      std::cout << "The state is included in the book\n";
    }
  }
  catch (osl::csa::CsaIOError& e)
  {
    std::cerr << boost::format("Failed to open the csa file: %s\n") % osl::misc::file_string(path);
    std::cerr << USAGE;
    return;
  }
}

/**
 * Show the N shortest lines in a book.
 * command: oshow_shortest_lines [top_n]
 */
void gpsshell::
MySession::openingShowShortestLines(const std::vector<std::string>& params)
{
  const static std::string USAGE = "USAGE: oshow_shortest_lines [top_n]\n";
  if (!the_book)
    return;

  if (params.size() >= 3) {
    std::cerr << USAGE;
    return;
  }

  static int TOPN = 3; // default value
  if (params.size() == 2)
    TOPN = boost::lexical_cast<int>(params[1]);

  std::shared_ptr<osl::book::WeightedBook> book = the_book->getWeightedBook();

  std::vector<gpsshell::VisitFlag> history(book->totalState(), gpsshell::UNVISITED);
  std::deque<gpsshell::VisitState> fronts;
  fronts.push_back(gpsshell::VisitState(book->startState()));
  
  int temp_top = TOPN; 
  while (!fronts.empty()) {
    gpsshell::VisitState vs = fronts.front(); 
    fronts.pop_front();
    history[vs.state_index] = gpsshell::VISITED;

    const auto wmoves = book->moves(vs.state_index, false);
    if (wmoves.empty()) {
      // found a leaf node!
      std::cout << "\n";
      for (const osl::Move& m: vs.moves) {
        std::cout << osl::csa::show(m);
      }
      std::cout << "\n";
      the_book->showState(book->board(vs.state_index), vs.moves, 3);
      if (--temp_top < 1) 
        break; // exit the whole loop
      else
        continue;
    }

    for (const osl::book::WMove& wmove: wmoves) {
      const int next_index = wmove.stateIndex();
      if (history[next_index] != gpsshell::UNVISITED)
        continue;

      gpsshell::VisitState next(next_index);
      next.moves = vs.moves; // copy
      next.moves.push_back(wmove.move);
      fronts.push_back(next);
      history[next_index] = ENTERED;
    } // for wmoves
  } // while
}

void gpsshell::
MySession::setIgnoreList(const std::vector<std::string>& params)
{
  if (params.size() != 2) {
    std::cerr << "set_ignorelists filename\n";
    return;
  }
  const std::string& path = params[1];
  the_ignorelist.openFile(path);
}

void gpsshell::
MySession::ignoreListShow(const std::vector<std::string>& params)
{
  if (the_ignorelist.isEmpty()) {
    std::cerr << "set \"set_ignorelists filename\" first\n";
    return;
  }
  const gpsshell::IgnoreList::index_t index = the_ignorelist.getCurrentIndex();
  std::vector<std::string> args;
  args.push_back("open");
  args.push_back(osl::misc::file_string(std::get<0>(index)));
  args.push_back(std::to_string(std::get<1>(index)));
  const std::string& comment = std::get<2>(index);
  if (!comment.empty())
  std::cout << boost::format("%s\n") % comment;

  std::cout << boost::format("%s %s\n")
               % args[1] % args[2];
  open(args);
}

void gpsshell::
MySession::ignoreListNext(const std::vector<std::string>& params)
{
  if (!the_ignorelist.hasNext())
  {
    std::cout << "There is no next game.\n";
    return;
  }

  the_ignorelist.next();
  std::vector<std::string> args;
  args.push_back("ignorelist_show");
  ignoreListShow(args);
}

void gpsshell::
MySession::ignoreListPrev(const std::vector<std::string>& params)
{
  if (!the_ignorelist.hasPrev())
  {
    std::cout << "There is no previous game.\n";
    return;
  }

  the_ignorelist.prev();
  std::vector<std::string> args;
  args.push_back("ignorelist_show");
  ignoreListShow(args);
}

void gpsshell::
MySession::ignoreListFirst(const std::vector<std::string>& params)
{
  if (the_ignorelist.isEmpty()) {
    std::cerr << "set \"set_ignorelists filename\" first\n";
    return;
  }
  the_ignorelist.first();
  std::vector<std::string> args;
  args.push_back("ignorelist_show");
  ignoreListShow(args);
}

void gpsshell::
MySession::ignoreListLast(const std::vector<std::string>& params)
{
  if (the_ignorelist.isEmpty()) {
    std::cerr << "set \"set_ignorelists filename\" first\n";
    return;
  }
  the_ignorelist.last();
  std::vector<std::string> args;
  args.push_back("ignorelist_show");
  ignoreListShow(args);
}

void gpsshell::
MySession::ignoreListShowAll(const std::vector<std::string>& params)
{
  if (the_ignorelist.isEmpty()) {
    std::cerr << "set \"set_ignorelists filename\" first\n";
    return;
  }
  the_ignorelist.first();
  std::vector<std::string> args;
  args.push_back("ignorelist_show");
  ignoreListShow(args);
  while (the_ignorelist.hasNext())
  {
    args.clear();
    args.push_back("ignorelist_next");
    ignoreListNext(args);

    std::cout << "\n";
  }
}

void gpsshell::
MySession::showBookInMemory(const std::vector<std::string>& params)
{
  const osl::BookInMemory& book = osl::BookInMemory::instance();
  osl::HashKey key(board->getState());
  osl::MoveVector moves;
  book.find(key, moves);
  for (osl::Move move: moves)
    std::cout << osl::csa::show(move) << std::endl;
  std::cout << "BookInMemory has " << moves.size() << std::endl;  
}

#ifdef OSL_SMP
void gpsshell::
MySession::setNumCPUs(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    return;
  try 
  {
    const int num_cpus = boost::lexical_cast<int>(params[1]);
    osl::OslConfig::setNumCPUs(num_cpus);
  } 
  catch (boost::bad_lexical_cast& blc) 
  {
    std::cerr << blc.what() << std::endl;
    std::cerr << "USAGE: set_num_cpus #cpus" << std::endl;
  }
}
#endif

void gpsshell::
MySession::version(const std::vector<std::string>& params)
{
  std::string name = "gpsshogi ";
#ifdef OSL_SMP
  name += "(smp) ";
#endif
  std::cout << name << gpsshogi::gpsshogi_revision << std::endl;
}

// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
