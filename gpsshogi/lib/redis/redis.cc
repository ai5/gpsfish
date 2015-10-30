#include "gpsshogi/redis/redis.h"
#include <glog/logging.h>
#include <iostream>
#include <cassert>

void gpsshogi::redis::
connectRedisServer(redisContext **context, const std::string& host, const int port) {
  const struct timeval timeout = { 1, 500000 }; // 1.5 seconds
  redisContext *c = redisConnectWithTimeout(host.c_str(), port, timeout);
  if (c->err) {
    std::cerr << "Connection error: " << c->errstr << std::endl;
  } else {
    *context = c;
  }
}

bool gpsshogi::redis::
authenticate(redisContext *c, const std::string& password) {
  redisReplyPtr reply((redisReply*)redisCommand(c, "AUTH %s", password.c_str()),
                      freeRedisReply);
  if (checkRedisReply(reply))
    exit(1);
  assert(reply->type == REDIS_REPLY_STATUS);
  const std::string ret(reply->str);
  DLOG(INFO) << "authenticated: " << ret;
  return "OK" == ret;
}

void gpsshogi::redis::
freeRedisReply(redisReply *reply) {
  assert(reply);
  freeReplyObject(reply);
}

int gpsshogi::redis::
checkRedisReply(const redisReplyPtr& reply) {
  assert(reply);
  if (reply->type == REDIS_REPLY_ERROR) {
    assert(reply->str);
    LOG(ERROR) << reply->str;
    return 1;
  }
  return 0;
}
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
