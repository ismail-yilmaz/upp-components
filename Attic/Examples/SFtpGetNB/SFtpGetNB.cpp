#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	const char *file  = "/readme.txt";
	
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto sftp = session.CreateSFtp();
		sftp.NonBlocking();
		sftp.Get(file);
		while(sftp.Do()) {
			SocketWaitEvent we;
			sftp.AddTo(we);
			we.Wait(10);
		}
		LOG((!sftp.IsError() ? sftp.GetResult().To<String>() : sftp.GetErrorDesc()));
	}
	else
		LOG(session.GetErrorDesc());
}
