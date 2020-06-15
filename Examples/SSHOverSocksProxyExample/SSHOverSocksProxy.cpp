#include <Core/Core.h>
#include <Core/SSH/SSH.h>
#include <NetProxy/NetProxy.h>

using namespace Upp;

// This example demonstrates a basic SSH2 connection over a Socks5 proxy.

// WARNING: The sole purpose of this example is to demonstrate the  usage of
//          NetProxy package with the SSH package. The example uses an anon.
//          proxy server (with  no authentication) to access  a public  SFTP
//          server.
//
//          NEVER  USE A PUBLIC PROXY SERVER FOR YOUR SSH CONNECTIONS. IN FACT,
//          IT IS HIGHLY RECOMMENDED TO COMPLETELY AVOID USING PROXY SERVERS FOR
//          SENSITIVE/ENCRYPTED DATA TRANSFERS.

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	Ssh::Trace();
	NetProxy::Trace();
	
	const char *proxy_server = "88.249.26.113";  // Anonymous socks proxy server of a well-known ISP in Turkey (Turk Telekom).
	const int   proxy_port   = 1080;
	
	const char *ssh_server   = "test.rebex.net"; // A well-known and popular SSH test server.
	const int   ssh_port     = 22;

	SshSession session;
	session.WhenProxy = [&]() -> bool
	{
		NetProxy socksproxy(session.GetSocket(), proxy_server, proxy_port);
		return socksproxy.Timeout(10000).Socks5().Connect(ssh_server, ssh_port);
	};
	
	if(session.Timeout(30000).Connect(ssh_server, ssh_port, "demo", "password")) {
		SFtp sftp(session);
		String s = sftp.LoadFile("./readme.txt");
		if(!sftp.IsError()) {
			RLOG("---------------------------");
			RLOG(s);
			RLOG("----------------------------");
		}
		return;
	}
	RLOG(session.GetErrorDesc());
}
