#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ftp::Trace();

	Ftp ftpclient;
	if(ftpclient.Timeout(60000).Connect("ftps://demo:password@test.rebex.net:21")) {
		auto f = ftpclient.Get("readme.txt", true);
		if(!ftpclient.IsError()) {
			RLOG(f);
			return;
		}
	}
	RLOG(ftpclient.GetErrorDesc());
}
