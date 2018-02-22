#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	const char *url = "demo:password@test.rebex.net:22"; // A well-known public SSH test server.

	Ssh::Trace();

	SshSession session;
	if(session.Connect(url)) {
		auto shell = session.CreateShell();
		if(!shell.Console("vt100"))
			LOG(shell.GetErrorDesc());
	}
	else LOG(session.GetErrorDesc());
	
}
