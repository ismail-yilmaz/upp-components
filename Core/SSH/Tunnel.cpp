#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

void SshTunnel::Validate()
{
	ASSERT(*channel);
	bool b = false;
	switch(mode) {
		case CONNECT:
			b = *channel || listener;
			break;
		case LISTEN:
			b = *channel || listener;
			break;
		case ACCEPT:
			b = *channel || !listener;
			break;
		default:
			NEVER();
	}
	if(b)
		SetError(-1, "Proxy channel is already allocated.");
}

bool SshTunnel::Connect(const String& host, int port)
{
	return Cmd(CONNECT, [=]() mutable {
		Validate();
		*channel = libssh2_channel_direct_tcpip(ssh->session, host, port);
		if(!*channel && !WouldBlock())
			SetError(-1);
		if(*channel) {
			LLOG("Direct tcp-ip connection to " << host << ":" << port << " is established.");
			mode = CONNECT;
		}
		return *channel != NULL;
	});
}

bool SshTunnel::Connect(const String& url)
{
	UrlInfo u(url);
	if(!u.host.IsEmpty() && u.port.IsEmpty())
		return Connect(u.host, StrInt(u.port));
	else
		return Cmd(CONNECT, [=]{
			SetError(-1, "Malformed proxy connection URL.");
			return false; // Just to prevent compiler warnings.
		});
}

bool SshTunnel::Listen(const String& host, int port, int* bound_port, int listen_count)
{
	return Cmd(LISTEN, [=]() mutable {
		Validate();
		listener =libssh2_channel_forward_listen_ex(
			ssh->session,
			host.IsEmpty() ? NULL : ~host,
			port,
			bound_port ? bound_port : NULL,
			listen_count
		);
		if(!listener && !WouldBlock())
			SetError(-1);
		if(listener) {
			mode = LISTEN;
			LLOG("Started listening on port #" << port);
		}
		return listener != NULL;
	});
}

bool SshTunnel::Accept(SshTunnel& listener)
{
	return Cmd(ACCEPT, [=, &listener]() mutable {
		if(IsNull(listener))
			SetError(-1, "Invalid listener.");
		Validate();
		*channel = libssh2_channel_forward_accept(listener.listener);
		if(!*channel && !WouldBlock())
			SetError(-1);
		if(*channel) {
			mode = ACCEPT;
			LLOG("Connection accepted.");
		}
		return *channel != NULL;
	});
}
}