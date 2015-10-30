/* httpclient.h
 */
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H
#include <string>

namespace gpsshell
{
  int getFileOverHttp(const std::string& url, const std::string& tempfile_name);
}

#endif /* HTTPCLIENT_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:

