#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

void PrintOut(AsyncWork<String>& worker)
{
	try {
		Cout() << worker.Get();
	}
	catch(Ssh::Error& e) {
		Cerr() << e << '\n';
	}
}

CONSOLE_APP_MAIN
{
	const char *host  = "test.rebex.net";  // A well known public (S)FTP test server.
	const char *user  = "demo";
	const char *pass  = "password";
	const char *file1 = "/readme.txt";
	const char *file2 = "/pub/example/readme.txt";

	Ssh::Trace();
	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
		auto worker1 = SFtp::AsyncGet(session, file1);
		auto worker2 = SFtp::AsyncGet(session, file2);
		while(!worker1.IsFinished() || !worker2.IsFinished())
			; // do something here...
		PrintOut(worker1);
		Cout() << "----\n";
		PrintOut(worker2);
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
