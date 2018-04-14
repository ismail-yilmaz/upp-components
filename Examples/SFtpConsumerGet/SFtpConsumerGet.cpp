#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	const char *file = "/pub/example/readme.txt";
	
	String data;

	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		auto sftp = session.CreateSFtp();
		sftp.WhenContent = [&data](const void *buf, int len)
		{
			data.Cat(static_cast<const char*>(buf), len);
		};
		
		sftp.Get(file);
		LOG((!sftp.IsError() ? data : sftp.GetErrorDesc()));
	}
	else
		LOG(session.GetErrorDesc());
}
