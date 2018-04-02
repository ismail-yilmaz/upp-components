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
			LOG(ftpclient.GetReply());
			DUMP(reply_code);
			return;
		}
	}
	LOG(ftpclient.GetErrorDesc());
}
