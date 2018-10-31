#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	const char *cmd = "ls -l /pub/example";
	
	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		try {
			auto result = SshExec::AsyncRun(session, cmd).Get();
			LOG("Return code: " << result.a);
			LOG("Result:\n"     << result.b);
		}
		catch(const Ssh::Error& e) {
			LOG("Worker failed: " << e);
		}
	}
	else
		LOG(session.GetErrorDesc());
}
