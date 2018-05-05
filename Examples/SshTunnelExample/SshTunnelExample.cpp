#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// This example requires upp/reference/SocketServer and upp/reference/SocketClient examples.
// SocketClient: Set the port number to 3215.
//
// |SocketClient (client)|<---> |SshTunnelExample (tunnel/server)| <---> |SocketClient (server)|

bool ServerSendRecv(SshSession& session, String& data)
{
	// SshTunnel <-> SocketServer
	SshTunnel tunnel(session);
	if(!tunnel.Connect("127.0.0.1", 3214)) {
		LOG("ServerSendRecv(): " << tunnel.GetErrorDesc());
		return false;
	}
	tunnel.Put(data + '\n');
	data = tunnel.GetLine();
	return !data.IsEmpty();
}

void StartTunnel(SshSession& session)
{
	SshTunnel listener(session);
	if(!listener.Listen(3215, 5)) {
		LOG("StartTunnel(): " << listener.GetErrorDesc());
		return;
	}
	LOG("SSH tunnel (server mode): Waiting for the requests to be tunneled...");
	for(;;) {
		SshTunnel tunnel(session);
		if(!tunnel.Accept(listener)) {
			LOG("StartTunnel(): " << tunnel.GetErrorDesc());
			return;
		}
		// SocketClient <-> SshTunnel
		auto data = tunnel.GetLine();
		LOG("Tunneled Request: " << data);
		if(!data.IsEmpty() && ServerSendRecv(session, data)) {
			LOG("Tunneled Response: " << data);
			tunnel.Put(data + '\n');
		}
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE | LOG_COUT);
//	Ssh::Trace();

	SshSession session;
	if(session.Timeout(30000).Connect("username:password@localhost:22")) {
		StartTunnel(session.Timeout(Null));
		return;
	}
	LOG(session.GetErrorDesc());
}
