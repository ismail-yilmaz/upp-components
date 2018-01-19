#include <Core/Core.h>
#include <SSH/SSH.h>
#include <NetProxy/NetProxy.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	// This example requires a running TOR daemon and NetProxy package.
	// Change below strings to your preferred values.
	
	const char* ssh_host = "dummysshhostname";
	const char* ssh_user = "dummysshusername";
	const char* ssh_pass = "dummysshpassword";
	int         ssh_port = 22;
	
	StdLogSetup(LOG_FILE|LOG_COUT);
	Ssh::Trace();
	NetProxy::Trace();
	
	SshSession session;
	session.WhenProxy = [=,&session] {
		return NetProxy(session.GetSocket(), "127.0.0.1", 9050)
		            .Timeout(30000)
		            .Socks5()
		            .Auth("none", "none")
		            .Connect(ssh_host, ssh_port);
	};
	
	if(session.Timeout(60000).Connect(ssh_host, ssh_port, ssh_user, ssh_pass)) {
		LOG("Successfully connected to " << ssh_host << " (over TOR)");
	}
	else
		LOG("Ssh connection via TOR failed. " << session.GetErrorDesc());
}
