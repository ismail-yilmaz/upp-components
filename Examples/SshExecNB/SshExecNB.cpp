#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	const char *host = "test.rebex.net"; // A well known public (S)FTP test server.
	const char *user = "demo";
	const char *pass = "password";
	const char *cmd  = "ls -l /pub/example";
	
	Ssh::Trace();
	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
		auto exec = session.CreateExec();
		exec.NonBlocking();
		exec.Execute(cmd, Cout(), Cerr());
		while(exec.Do()) {
			SocketWaitEvent we;
			exec.AddTo(we);
			we.Wait(10);
		}
		if(exec.IsError())
			Cerr() << exec.GetErrorDesc() << '\n';
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
