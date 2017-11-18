#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

void PrintOut(AsyncWork<Tuple<int, String, String>>& worker)
{
	try {
		auto result = worker.Get();
		Cout() << result.Get<1>();
		Cerr() << result.Get<2>();
	}
	catch(Ssh::Error& e) {
		Cerr() << e << '\n';
	}
}

CONSOLE_APP_MAIN
{
	const char *host = "test.rebex.net";  // A well known public (S)FTP test server.
	const char *user = "demo";
	const char *pass = "password";
	const char *cmd1 = "ls -l /";
	const char *cmd2 = "ls -l /pub/example";
	
	Ssh::Trace();
	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
		auto worker1 = SshExec::Async(session, cmd1);
		auto worker2 = SshExec::Async(session, cmd2);
		while(!worker1.IsFinished() || !worker2.IsFinished())
			; // do something here...
		PrintOut(worker1);
		Cout() << "----\n";
		PrintOut(worker2);
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
