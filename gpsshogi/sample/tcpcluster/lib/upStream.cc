/* upStream.cc
 */
#include "upStream.h"
#include "osl/oslConfig.h"

gpsshogi::
UpStream::UpStream()
{
  osl::OslConfig::setUp();
}

gpsshogi::
UpStream::~UpStream()
{
}

void gpsshogi::
UpStream::bind(Coordinator &c, boost::asio::io_service &i)
{
  coordinator = &c;
  io = &i;
}


// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
