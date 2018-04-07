#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	const char *file = "/pub/example/readme.txt";

	SshSession session;
	if(session.Timeout(Null).Connect("demo:password@test.rebex.net:22")) {
		auto sftp = session.CreateSFtp();
		auto out  = sftp.Get(file);
		LOG((!sftp.IsError() ? out : sftp.GetErrorDesc()));
	}
	else
		LOG(session.GetErrorDesc());
}
