#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ftp::Trace();
	
	StringStream file;
	auto consumer = [&file](int64 id, const void* buf, int len)
	{
		file.Put(buf, len);
	};
	try {
		Ftp::AsyncConsumerGet("ftp://demo:password@test.rebex.net:21/readme.txt?type=ascii", pick(consumer)).Get();
		RLOG(file.GetResult());
	}
	catch(const Ftp::Error& e) {
		RLOG(e);
	}
}
