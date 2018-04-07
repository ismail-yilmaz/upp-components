#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

void PrintOut(AsyncWork<String>& worker)
{
	try {
		LOG(worker.Get());
	}
	catch(const Ssh::Error& e) {
		LOG(e);
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();
	
	const char *file1 = "/readme.txt";
	const char *file2 = "/pub/example/readme.txt";

	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto worker1 = SFtp::AsyncGet(session, file1);
		auto worker2 = SFtp::AsyncGet(session, file2);
		while(!worker1.IsFinished() || !worker2.IsFinished())
			; // do something here...
		PrintOut(worker1);
		Cout() << "----\n";
		PrintOut(worker2);
	}
	else
		LOG(session.GetErrorDesc());
}
