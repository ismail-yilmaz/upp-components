#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ftp::Trace();

	try {
		RLOG(Ftp::AsyncGet("ftp://demo:password@test.rebex.net:21/readme.txt?type=ascii").Get());
	}
	catch(const Ftp::Error& e) {
		RLOG(e);
	}
}