#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
//	Ssh::Trace();

	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto shell = session.CreateShell();
		if(!shell.Console("vt100"))
			LOG(shell.GetErrorDesc());
	}
	else
		LOG(session.GetErrorDesc());
	
}
