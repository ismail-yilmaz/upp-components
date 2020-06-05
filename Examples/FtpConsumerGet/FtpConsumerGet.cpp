#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ftp::Trace();
	
	String data;
	
	Ftp ftpclient;
	if(ftpclient.Timeout(60000).Connect("ftps://demo:password@test.rebex.net:21")) {
		ftpclient.WhenContent = [&data](const void* buf, int len)
		{
			data.Cat(static_cast<const char*>(buf), len);
		};
		ftpclient.Get("readme.txt", true);
		if(!ftpclient.IsError()) {
			RLOG(data);
			return;
		}
	}
	RLOG(ftpclient.GetErrorDesc());
}
