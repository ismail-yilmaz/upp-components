#include <Core/Core.h>
#include <NetProxy/NetProxy.h>

using namespace Upp;

// This example demonstraes the basic usage of NetProxy class with socks5 tunnels.
// Target server: test.rebex.net -> A well-known FTP/SFTP test server.

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	NetProxy::Trace();
	
	const char *proxy_server = "0.0.0.0"; // Socks server address should go here.
	const int   proxy_port   = 1080;

	TcpSocket sock;

	NetProxy socksproxy(sock, proxy_server, proxy_port);
	if(socksproxy.Timeout(10000).Socks5().Connect("test.rebex.net", 21)) {
		RLOG("-------------");
		RLOG(sock.GetLine());	// Get the first line of FTP server HELO...
		RLOG("-------------");
		return;
	}
	RLOG(socksproxy.GetErrorDesc());
}
