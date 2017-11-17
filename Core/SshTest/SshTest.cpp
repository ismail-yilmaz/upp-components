#include <Core/Core.h>
#include <SSH/SSH.h>
using namespace Upp;

CONSOLE_APP_MAIN
{
	const char *file_in  = "/readme.txt";
	const char *file_out = "/home/maldoror/SSHTestFolder/readme_1.txt";
	//StdLogSetup(LOG_FILE | LOG_COUT);
	Ssh::Trace();
	SshSession session;
	session.Timeout(0);
	if(session.Connect("test.rebex.net", 22, "demo", "password")) {
		auto worker1 = SFtp::AsyncGet(session, file_in, file_out);
		auto worker2 = SshExec::Async(session, "ls -l /pub/example/");
		while(!worker1.IsFinished() || !worker2.IsFinished())
			;
		worker1.Get();
		auto rc = worker2.Get();
		Cout() << rc.Get<1>();
	}
	else DUMP(session.GetErrorDesc());
}
