#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	const char *cmd  = "ls -l /pub/example/";
	
	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto exec = session.CreateExec();
		auto rc = exec(cmd, Cout(), Cerr());
		LOG((!exec.IsError() ? String("Return code: " << AsString(rc)) : exec.GetErrorDesc()));
	}
	else
		LOG(session.GetErrorDesc());
}
