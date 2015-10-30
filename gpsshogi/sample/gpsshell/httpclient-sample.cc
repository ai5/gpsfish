/*
 * A sample program to use the PocoNet functionarities.
 * It connects to a web site and saves the content to a file.
 */

#include "httpclient.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include <string>

using namespace std;

int main(int argc, char **argv)
{
  const string url = "http://wdoor.c.u-tokyo.ac.jp/shogi/";
  const string filename = "./test.html";

  Poco::Net::HTTPStreamFactory::registerFactory();
  gpsshell::getFileOverHttp(url, filename);

  return 0;
}
