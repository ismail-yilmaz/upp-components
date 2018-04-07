#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

void PrintOut(AsyncWork<Tuple<int, String, String>>& worker)
{
	try {
		auto result = worker.Get();
		LOG(result.Get<1>());
		LOG(result.Get<2>());
	}
	catch(const Ssh::Error& e) {
		LOG(e);
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	const char *cmd1 = "ls -l /";
	const char *cmd2 = "ls -l /pub/example";
	
	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto worker1 = SshExec::Async(session, cmd1);
		auto worker2 = SshExec::Async(session, cmd2);
		while(!worker1.IsFinished() || !worker2.IsFinished())
			; // do something here...
		PrintOut(worker1);
		LOG("----");
		PrintOut(worker2);
	}
	else
		LOG(session.GetErrorDesc());
}
