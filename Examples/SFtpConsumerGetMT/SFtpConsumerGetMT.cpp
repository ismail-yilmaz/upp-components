#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	StringStream file;
	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto consumer = [=, &file](int64 id, const void* buf, int len)
		{
			file.Put(buf, len);
		};
		try {
			SFtp::AsyncConsumerGet(session, "readme.txt", pick(consumer)).Get();
			LOG(file.GetResult());
		}
		catch(const Ssh::Error& e) {
			LOG(e);
		}
	}
	else
		LOG(session.GetErrorDesc());
}
