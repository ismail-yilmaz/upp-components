#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	Ssh::Trace();
//	Ssh::TraceVerbose(LIBSSH2_TRACE_ERROR);

	const char *cmd  = "ls -l";
	
	SshSession session;
	if(session.Timeout(30000).Connect("maldoror:succubus@localhost:22")) {
		Array<SshExec> execs;
		Array<StringStream> s;
		
		for(int i = 0; i < 8; i++) {
			auto& e = execs.AddPick(session.CreateExec());
			e.NonBlocking();
			e.Execute(cmd, s.Add(), NilStream());
		}
	//	exec.Execute(cmd, Cout(), Cerr());
	    int j = 0;
		while(!execs.IsEmpty())
			for(int i = 0; i < execs.GetCount(); i++) {
				auto& ex = execs[i];
				if(ex.Do()) {
					SocketWaitEvent we;
					ex.AddTo(we);
					we.Wait(10);
					continue;
				}
				if(ex.IsError())
					LOG(ex.GetErrorDesc());
				else {
					Cout() << "#" << j++ << "\n";
					LOG(s[i].GetResult());
				}
				s.Remove(i);
				execs.Remove(i);
				break;
			}
	}
	else
		LOG(session.GetErrorDesc());
}
