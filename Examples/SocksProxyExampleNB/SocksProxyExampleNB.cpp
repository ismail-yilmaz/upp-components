#include <Core/Core.h>
#include <NetProxy/NetProxy.h>

using namespace Upp;

// This example demonstraes the basic usage of NetProxy class with socks5 tunnels.
// Default proxy server: Turk Telekom, a well-known ISP in Turkey. No auth required...
// Target server: test.rebex.net -> A well-known FTP/SFTP test server.

// Note that this example is simply a non-blocking variant of the SocksProxyExample.
// The tcp socket will be temporarily put into non-blocking mode by the NetProxy instance.
// The socket will be returned to its original state after the connetion attempt.

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	NetProxy::Trace();
	
	const char *proxy_server = "88.249.26.113";
	const int   proxy_port   = 1080;

	TcpSocket sock;

	NetProxy socksproxy(sock, proxy_server, proxy_port);
	if(socksproxy.Timeout(10000).NonBlocking().Socks5().Connect("test.rebex.net", 21)) {
		while(socksproxy.Do()) {
			SocketWaitEvent we;
			socksproxy.AddTo(we);
			we.Wait(100);
			// Do other stuff...
		}
		RLOG("-------------");	// Here, the socket is in its original state (blocking).
		RLOG(sock.GetLine());	// Get the first line of FTP server HELO...
		RLOG("-------------");
		return;
	}
	RLOG(socksproxy.GetErrorDesc());
}