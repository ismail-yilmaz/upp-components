#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	SshSession session;
	if(session.Connect("demo:password@test.rebex.net: 22")) {
		auto shell = session.CreateShell();
		shell.NonBlocking();
		shell.Console("vt100");
		while(shell.Do()) {
			SocketWaitEvent we;
			shell.AddTo(we);
			we.Wait(10);
		}
		if(shell.IsError())
			LOG(shell.GetErrorDesc());
	}
	else
		LOG(session.GetErrorDesc());
}
