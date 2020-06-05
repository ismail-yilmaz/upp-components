#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ftp::Trace();

	Ftp ftpclient;
	if(ftpclient.Timeout(30000).Connect("demo:password@test.rebex.net:21")) {
		auto reply_code = ftpclient.SendCommand("HELP");
		if(!ftpclient.IsError()) {
			RLOG(ftpclient.GetReply());
			RDUMP(reply_code);
			return;
		}
	}
	RLOG(ftpclient.GetErrorDesc());
}
