#include <Core/Core.h>
#include <NetProxy/NetProxy.h>

using namespace Upp;

// This example demonstraes the basic usage of NetProxy class with socks5 tunnels.
// Target server: test.rebex.net -> A well-known FTP/SFTP test server.

// Note that this example is simply a non-blocking variant of the SocksProxyExample.
// The tcp socket will be temporarily put into non-blocking mode by the NetProxy instance.
// The socket will be returned to its original state after the connetion attempt.

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	NetProxy::Trace();
	
	const char *proxy_server = "0.0.0.0"; // Server address should go here.
	const int   proxy_port   = 1080;

	TcpSocket sock;

	NetProxy socksproxy(sock, proxy_server, proxy_port);
	socksproxy.Timeout(10000).NonBlocking().Socks5().Connect("test.rebex.net", 21);
	while(socksproxy.Do()) {
		SocketWaitEvent we;
		socksproxy.AddTo(we);
		we.Wait(100);
		// Do other stuff...
	}
	if(socksproxy.IsError())
		RLOG(socksproxy.GetErrorDesc());
	else {
		RLOG("-------------");	// Here, the socket is in its original state (blocking).
		RLOG(sock.GetLine());	// Get the first line of FTP server HELO...
		RLOG("-------------");
	}
}