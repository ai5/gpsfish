#include "httpclient.h"
#ifndef MINIMAL_GPSSHELL
#  include "Poco/URIStreamOpener.h"
#  include "Poco/StreamCopier.h"
#  include "Poco/URI.h"
#  include "Poco/Exception.h"
#  include "Poco/Net/HTTPStreamFactory.h"
#endif
#include <memory>
#include <iostream>
#include <fstream>
#include <string>

int gpsshell::getFileOverHttp(const std::string& url, const std::string& tempfile_name)
{
#ifndef MINIMAL_GPSSHELL
  try
  {
    std::ofstream localFile(tempfile_name.c_str(), std::ios_base::out | std::ios_base::binary);
    if (!localFile)
    {
    	std::cerr << "Failed to open local file for writing: " << tempfile_name << std::endl;
    	return 1;
    }
    Poco::URI uri(url);
    std::auto_ptr<std::istream> ptrHttpStream(Poco::URIStreamOpener::defaultOpener().open(uri));
    Poco::StreamCopier::copyStream(*ptrHttpStream.get(), localFile);
  }
  catch (Poco::Exception& exc)
  {
    std::cerr << exc.displayText() << std::endl;
    return 1;    
  }
#endif
  return 0;
}

