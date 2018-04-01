#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
//	Ftp::Trace();

	Ftp ftpclient;
	if(ftpclient.Timeout(30000).Connect("test.rebex.net", 21)) {
		
		auto features = ftpclient.GetFeatures(); // We are not logged in yet.
		if(!ftpclient.IsError()) {
			LOG("Available server wxtensions, and their parameters, if any:");
			DUMP(features);
	
			// Now let's see if the server supports explicit TLS connections with private (full)
			// data protection.
			String options = features["auth"][0];
			bool secure = options.Find("tls-p") >= 0;
			
			if(secure) {
				if(ftpclient.SSL().Login("demo", "password")) {
					LOG("Secure login successful!");
					return;
				}
			}
		}
	}
	LOG(ftpclient.GetErrorDesc());
}
