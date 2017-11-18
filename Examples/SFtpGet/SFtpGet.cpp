#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	const char *host = "test.rebex.net"; // A well known public (S)FTP test server.
	const char *user = "demo";
	const char *pass = "password";
	const char *file = "/pub/example/readme.txt";
	
	Ssh::Trace();
	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
		auto sftp = session.CreateSFtp();
		auto out  = sftp.Get(file);
		if(!sftp.IsError()) Cout() << out;
		else Cerr() << sftp.GetErrorDesc() << '\n';
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
