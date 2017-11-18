#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	const char *host = "test.rebex.net"; // A well known public (S)FTP test server.
	const char *user = "demo";
	const char *pass = "password";
	const char *file  = "/pub/example/readme.txt";
	
	Ssh::Trace();
	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
		auto sftp = session.CreateSFtp();
		sftp.NonBlocking();
		sftp.Get(file);
		while(sftp.Do()) {
			SocketWaitEvent we;
			sftp.AddTo(we);
			we.Wait(10);
		}
		if(!sftp.IsError())
			Cout() << sftp.GetResult();
		else
			Cerr() << sftp.GetErrorDesc() << '\n';
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
