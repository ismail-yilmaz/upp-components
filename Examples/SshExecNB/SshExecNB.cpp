#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	const char *cmd  = "ls -l /pub/example";
	
	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto exec = session.CreateExec();
		exec.NonBlocking();
		exec.Execute(cmd, Cout(), Cerr());
		while(exec.Do()) {
			SocketWaitEvent we;
			exec.AddTo(we);
			we.Wait(10);
		}
		if(exec.IsError())
			LOG(exec.GetErrorDesc());
	}
	else
		LOG(session.GetErrorDesc());
}
