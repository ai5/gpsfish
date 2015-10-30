#include "gpsshogi/redis/searchResult.h"
#include "gpsshogi/redis/redis.h"
#include "osl/record/csa.h"
#include "osl/record/kanjiPrint.h"
#include "osl/record/record.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <glog/logging.h>
#include <iostream>
#include <sstream>
#include <cassert>
#include <ctime>

namespace { // anonymous

  void readMoves(const std::string& binary, gpsshogi::redis::moves_t& moves)
  {
    std::stringstream ss(binary);
    const int size = binary.size() / 4 /* 4 bytes per move */;

    for (int i=0; i<size; ++i) {
      const int i = osl::record::readInt(ss);
      moves.push_back(osl::Move::makeDirect(i));
    }
  }

  int parseSearchResultReply(const gpsshogi::redis::redisReplyPtr reply,
                             gpsshogi::redis::SearchResult& sr)
  {
    if (gpsshogi::redis::checkRedisReply(reply))
      exit(1);
    assert(reply->type == REDIS_REPLY_ARRAY);

    if (reply->elements == 0) {
      LOG(ERROR) << "No existing board found.";
      return 1;
    }

    for(size_t i=0; i<reply->elements; /*empty*/) {
      const redisReply *r = reply->element[i++];
      assert(r->type == REDIS_REPLY_STRING);
      const std::string field(r->str, r->len);
      if ("depth" == field) {
        const redisReply *r = reply->element[i++];
        assert(r->type == REDIS_REPLY_STRING);
        const std::string str(r->str, r->len);
        sr.depth = boost::lexical_cast<int>(str);
      } else if ("score" == field) {
        const redisReply *r = reply->element[i++];
        assert(r->type == REDIS_REPLY_STRING);
        const std::string str(r->str, r->len);
        sr.score = boost::lexical_cast<int>(str);
      } else if ("consumed" == field) {
        const redisReply *r = reply->element[i++];
        assert(r->type == REDIS_REPLY_STRING);
        const std::string str(r->str, r->len);
        sr.consumed_seconds = boost::lexical_cast<int>(str);
      } else if ("pv" == field) {
        const redisReply *r = reply->element[i++];
        assert(r->type == REDIS_REPLY_STRING);
        sr.pv.assign(r->str, r->len);
      } else if ("timestamp" == field) {
        const redisReply *r = reply->element[i++];
        assert(r->type == REDIS_REPLY_STRING);
        const std::string str(r->str, r->len);
        sr.timestamp = boost::lexical_cast<int>(str);
      } else if ("moves" == field) {
        const redisReply *r = reply->element[i++];
        assert(r->type == REDIS_REPLY_STRING);
        const std::string str(r->str, r->len);
        readMoves(str, sr.moves);
      } else {
        LOG(WARNING) << "unknown field found: " << field;
      }
    }

    return 0;
  }

} // namespace anonymous


const std::string gpsshogi::redis::
SearchResult::timeString() const
{
  char buf[128] = {0};
  ctime_r(&timestamp, buf);
  return std::string(buf);
}

const std::string gpsshogi::redis::
SearchResult::toString() const
{
  std::ostringstream out;
  out << ":depth "      << depth <<
         " :score "     << score <<
         " :consumed "  << consumed_seconds <<
         " :pv "        << pv <<
         " :timestamp " << timestamp <<
         " :moves "     << (moves.empty() ? "" : osl::record::csa::show(&*moves.begin(), &*moves.end()))
         << std::endl;
  return out.str();
}

const std::string gpsshogi::redis::
compactBoardToString(const osl::record::CompactBoard& cb)
{
  std::ostringstream ss;
  ss << cb;
  return ss.str();
}


int gpsshogi::redis::
querySearchResult(redisContext *c, SearchResult& sr)
{
  const std::string key = compactBoardToString(sr.board);
  redisReplyPtr reply((redisReply*)redisCommand(c, "HGETALL %b", key.c_str(), key.size()),
                      freeRedisReply);
  return parseSearchResultReply(reply, sr);
}

int gpsshogi::redis::
querySearchResult(redisContext *c, std::vector<SearchResult>& results)
{
  BOOST_FOREACH(const SearchResult& sr, results) {
    const std::string key = compactBoardToString(sr.board);
    redisAppendCommand(c, "HGETALL %b", key.c_str(), key.size());
  }

  BOOST_FOREACH(SearchResult& sr, results) {
    void *r;
    redisGetReply(c, &r);
    redisReplyPtr reply((redisReply*)r, freeRedisReply);
    parseSearchResultReply(reply, sr);
  }
  
  return 0;
}


const std::string gpsshogi::redis::
stateToString(const osl::SimpleState& state, const osl::Move last_move)
{
  std::ostringstream out;

  osl::record::KanjiPrint printer(out);
  printer.setBlackColor(osl::record::Color::Black);
  printer.setWhiteColor(osl::record::Color::Blue);
  printer.setLastMoveColor(osl::record::Color::Red);
  printer.print(state, &last_move);

  return out.str();
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
