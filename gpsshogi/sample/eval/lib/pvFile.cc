/* pvFile.cc
 */
#include "pvFile.h"
#include "pvVector.h"

gpsshogi::
PVFileWriter::PVFileWriter(const char *filename)
  : fp(gzopen(filename, "wb9")), written(0)
{
}

gpsshogi::
PVFileWriter::~PVFileWriter()
{
  gzclose(fp);
}

void gpsshogi::
PVFileWriter::newPosition(int record, int position)
{
  if (written) {
    Move invalid;
    gzwrite(fp, &invalid, sizeof(invalid));
  }
  ++written;
  gzwrite(fp, &record, sizeof(record));
  gzwrite(fp, &position, sizeof(position));
}

void gpsshogi::
PVFileWriter::addPv(const PVVector& pv)
{
  assert(! pv.empty());
  gzwrite(fp, &pv[0], sizeof(Move)*pv.size());
  Move invalid;
  gzwrite(fp, &invalid, sizeof(invalid));
}

/* ------------------------------------------------------------------------- */
gpsshogi::
PVFileReader::PVFileReader(const char *filename)
  : fp(gzopen(filename, "rb"))
{
}

gpsshogi::
PVFileReader::~PVFileReader()
{
  gzclose(fp);
}

bool gpsshogi::
PVFileReader::newPosition(int& record, int& position)
{
  int r0 = gzread(fp, &record, sizeof(record));
  int r1 = gzread(fp, &position, sizeof(position));
  return r0 == sizeof(record) && r1 == sizeof(position);
}

bool gpsshogi::
PVFileReader::readPv(PVVector& pv)
{
  pv.clear();
  Move m;
  gzread(fp, &m, sizeof(m));
  if (m.isInvalid())
    return false;
  do {
    pv.push_back(m);
    gzread(fp, &m, sizeof(m));
  } while (! m.isInvalid());
  return true;
}

void gpsshogi::
PVFileReader::count(const char *filename, int& num_position, int& num_pv)
{
  num_position = num_pv = 0;

  PVFileReader pr(filename);
  int record, position;
  PVVector pv;
  while (pr.newPosition(record, position)) {
    ++num_position;
    while (pr.readPv(pv))
      ++num_pv;
  }
}

int gpsshogi::
PVFileReader::countPosition(const char *filename)
{
  int position, pv;
  count(filename, position, pv);
  return position;
}


/* ------------------------------------------------------------------------- */
// ;;; Local Variables:
// ;;; mode:c++
// ;;; c-basic-offset:2
// ;;; End:
