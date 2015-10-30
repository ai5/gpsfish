/* pvFile.h
 */
#ifndef GPSSHOGI_PVFILE_H
#define GPSSHOGI_PVFILE_H

#include "pvVector.h"
#include "osl/container.h"
#include <zlib.h>

namespace gpsshogi
{
  using namespace osl;
  /**
   * file := [record-id, position-id, pv*, MOVE_INVALID]*
   * pv := MOVE+ MOVE_INVALID
   */
  class PVFileWriter
  {
    gzFile fp;
    int written;
  public:
    PVFileWriter(const char *filename);
    ~PVFileWriter();

    void newPosition(int record, int position);
    void addPv(const PVVector&);
  };
  class PVFileReader
  {
    gzFile fp;
  public:
    PVFileReader(const char *filename);
    ~PVFileReader();

    bool newPosition(int& record, int& position);
    bool readPv(PVVector&);

    static int countPosition(const char *filename);
    static void count(const char *filename, int& position, int& pv);
  };
}

#endif /* _PVFILE_H */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
