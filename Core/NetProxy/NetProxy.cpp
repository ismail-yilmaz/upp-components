#include "NetProxy.h"

namespace Upp {

const char* NetProxy::GetMsg(int code)
{
	static Tuple<int, const char*> errors[] = {
		// NetProxy error messages.
		{ 10000,	t_("No client to serve (No socket attached).") },
		{ 10001,	t_("Proxy address or port not specified.") },
		{ 10002,	t_("Target address or port not specified.") },
		{ 10003,	t_("Couldn't resolve address.") },
		{ 10004,	t_("Couldn't connect to proxy server.") },
		{ 10005,	t_("Couldn't start SSL negotioation.") },
		{ 10006,	t_("Invalid packet received.") },
		{ 10007,	t_("Socket error occured.") },
		{ 10008,	t_("Operation was aborted.") },
		{ 10009,	t_("Connection timed out.") },
		// Http CONNECT method error messages.
		{ 10010,	t_("Http CONNECT method failed.") },
		{ 10011,	t_("BINDing is not possible in Http tunneling. Consider using Socks protocol.") },
		// SOCKS4 protocol error messages.
		{ 91,		t_("Request rejected or failed.") },
		{ 92,		t_("Request failed. Client is not running identd (or not reachable from the server).") },
		{ 93,		t_("Request failed. Cilent's identd could not confirm the user ID string in the request.") },
		{ 94,		t_("Socks4 protocol doesn't support IP version 6 address family. Considers using Socks5 protocol instead.") },
		// SOCKS5 protocol error messages.
		{ 1,		t_("General failure.") },
		{ 2,		t_("Connection not allowed by the ruleset.")},
		{ 3,		t_("Network unreachable.") },
		{ 4,		t_("Target machine unreachable.") },
		{ 5,		t_("Connection refused by the destination host.")},
		{ 6,		t_("TTL expired.") },
		{ 7,		t_("Command not supported / protocol error.") },
		{ 8,		t_("Address type not supported.") },
		{ 255,		t_("Invalid authentication method. No acceptable methods were offered.") },
		{ 256,		t_("Authentication failed.") },
	};
	const Tuple<int, const char *> *x = FindTuple(errors, __countof(errors), code);
	return x ? x->b : "-1";
}

static bool sTrace = false;
static bool sTraceVerbose = false;

#define LLOG(x)       do { if(sTrace) RLOG("NetProxy: " << x); } while(false)
#define LDUMPHEX(x)	  do { if(sTraceVerbose) RDUMPHEX(x); } while(false)

static  bool NtoP(int family, const String& in, String& bound_ip)
{
	// MingWG has some issues with InetNtop or inet_ntop functions on windows...

	if(family == AF_INET && in.GetLength() != 4 ||
		family == AF_INET6 && in.GetLength() != 16)
			return false;

	const uint8 *p = (uint8*) in.Begin();
	if(family == AF_INET) {
		bound_ip = Format("%d.%d.%d.%d", *p, *(p + 1), *(p + 2), *(p + 3));
	}
	else
	if(family == AF_INET6) {
		bound_ip = Format("%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
		                  "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
			*p, *(p + 1), *(p + 2), *(p + 3), *(p + 4), *(p + 5),
			*(p + 6), *(p + 7), *(p + 8), *(p + 9), *(p + 10),
			*(p + 11), *(p + 12), *(p + 13), *(p + 14),*(p + 15));
	}
	LDUMPHEX(bound_ip);
	return true;
}

void NetProxy::Trace(bool b)
{
	sTrace = b;
	sTraceVerbose = false;
}

void NetProxy::TraceVerbose(bool b)
{
	NetProxy::Trace(b);
	sTraceVerbose = b;
}

bool NetProxy::Init()
{
	LLOG("Starting... ");
	if(!socket)
		SetError(NO_SOCKET_ATTACHED);

	if(proxy_host.IsEmpty() || !proxy_port)
		SetError(HOST_NOT_SPECIFIED);

	if(target_host.IsEmpty() || !target_port)
		SetError(TARGET_NOT_SPECIFIED);

	timeout_backup = socket->GetTimeout();
	socket->GlobalTimeout(timeout);
	socket->Timeout(0);
	packet.Clear();
	packet_length = 0;
	bound = false;
	status = WORKING;
	start_time = msecs();
	ipinfo.Start(proxy_host, proxy_port);
	events = WAIT_READ | WAIT_WRITE;
	LLOG(Format("Connecting to proxy server: %s:%d", proxy_host, proxy_port));
	return true;
}

bool NetProxy::Exit()
{
	socket->Timeout(timeout_backup);
	socket = NULL;
	events  = 0;
	LLOG("Exiting...");
	return true;
}

bool NetProxy::Dns()
{
	auto b = !ipinfo.InProgress();
	if(b) {
		if(!ipinfo.GetResult())
			SetError(DNS_FAILED);
	}
	return b;
}

bool NetProxy::Connect()
{
	auto b = socket->Connect(ipinfo);
	if(b) {
		ipinfo.Clear();
		LLOG(Format("Successfully connected to proxy server at %s:%d",
			proxy_host, proxy_port));
	}
	return b;
}

bool NetProxy::Get()
{
	while(!IsTimeout()) {
		char c;
		if(socket->Get(&c, sizeof(char)) == 0)
			return false;
		packet.Cat(c);
		if(IsEof()) {
			return true;
		}
	}
}

bool NetProxy::Put()
{
	while(!IsTimeout()) {
		int n = packet.GetCharCount() - packet_length;
		n = socket->Put(~packet + packet_length, n);
		if(n == 0)
			break;
		packet_length += n;
		if(packet_length == packet.GetCharCount()) {
			packet.Clear();
			packet_length = 0;
			return true;
		}
	}
	return false;
}

void NetProxy::PutGet()
{
	queue.AddTail() = [=]{ return Put(); };
	queue.AddTail() = [=]{ return Get(); };
}

void NetProxy::StartSSL()
{
	queue.AddTail() = [=]{
		bool b = socket->StartSSL();
		if(b) LLOG("SSL negotiation successfully started.");
		return b;
	};
	queue.AddTail() = [=]{
		bool b = socket->SSLHandshake();
		if(!b) LLOG("SSL handshake successful.");
		return !b;
	};
}

void NetProxy::Check()
{
	if(status != WORKING)
		return;
	if(IsTimeout())
		SetError(CONNECTION_TIMED_OUT);
	if(socket->IsError())
		throw Error("Socket failure. " + socket->GetErrorDesc());
	if(socket->IsAbort())
		SetError(ABORTED);
}


void NetProxy::HttpcConnect()
{
	queue.Clear();
	{
		IsEof = [=] { return HttpcIsEof(); };
		queue.AddTail([=]{ return Init();});
		queue.AddTail([=]{ return Dns(); });
		queue.AddTail([=]{ return Connect();});
		queue.AddTail([=]{ return HttpcRequest(); });
	}
}

bool NetProxy::HttpcRequest()
{
	LLOG("Starting HTTP_CONNECT tunneling...");
	packet.Clear();
	packet_length = 0;
	int port = Nvl(target_port, ssl ? 443 : 80);
	packet << "CONNECT " << target_host << ":" <<  port << " HTTP/1.1\r\n"
           << "Host: " << target_host << ":" << port << "\r\n";
	if(!proxy_user.IsEmpty() && !proxy_password.IsEmpty())
		packet << "Proxy-Authorization: Basic " << Base64Encode(proxy_user + ":" + proxy_password) << "\r\n";
    packet << "\r\n";
	LLOG(">> HTTP_CONNECT: Sending request.");
	LDUMPHEX(packet);
	PutGet();
	return true;
}

bool NetProxy::HttpcParseReply()
{
	LLOG("<< HTTP_CONNECT: Request reply received.");
	LDUMPHEX(packet);
	int q = min(packet.Find('\r'), packet.Find('\n'));
	if(q >= 0)
		packet.Trim(q);
	if(!packet.StartsWith("HTTP") || packet.Find(" 2") < 0) {
		SetError(HTTPCONNECT_FAILED);
	}
	if(ssl) {
		StartSSL();
		return true;
	}
	LLOG("HTTP_CONNECT: Connection successful.");
	return Exit();
}

bool NetProxy::HttpcIsEof()
{
	if(packet.GetCount() > 3) {
		const char *c = packet.Last();
		if(c[-2] == '\n' && c[-1] == '\r' && c[0] == '\n') {
			return HttpcParseReply();
		}
	}
	return false;
}

bool NetProxy::SocksStart()
{
	LLOG(Format("Starting SOCKS%d connection.", proxy_type));
	packet_type = SOCKS5_HELO;
	if(!lookup) {
		ipinfo.Start(target_host, target_port);
		LLOG(Format("** SOCKS%d: Local name resolving started for %s:%d",
			proxy_type, target_host, target_port));
	}
	return true;
}

bool NetProxy::SocksCommand(int cmd)
{
	switch(cmd) {
		case BIND:
			if(!bound) {
				LLOG("SOCKS" << proxy_type << ": BIND info received.");
				bound = true;
				ParseBoundAddr();
				packet.Clear();
				packet_length = 0;
				queue.AddTail() = [=] { return Get(); };
				return true;
			}
			LLOG("SOCKS" << proxy_type << ": BIND command successful.");
			break;
		case CONNECT:
			if(ssl) {
				StartSSL();
				LLOG("SOCKS" << proxy_type << ": Starting SSL...");
				return true;
			}
			LLOG(Format("SOCKS%d: Successfully connected to %s:%d (via proxy %s:%d)",
				proxy_type, target_host, target_port, proxy_host, proxy_port));
			break;
		default:
			NEVER();
	}
	return Exit();
}

void NetProxy::Socks4Connect(int cmd)
{
	command = (byte) cmd;
	queue.Clear();
	{
		IsEof = [=] { return Socks4IsEof(); };
		queue.AddTail([=]{ return Init();});
		queue.AddTail([=]{ return Dns(); });
		queue.AddTail([=]{ return Connect();});
		queue.AddTail([=]{ return SocksStart(); });
		queue.AddTail([=]{ return !lookup ? Dns() : true; });
		queue.AddTail([=]{ return Socks4Request(); });
	}
}

bool NetProxy::Socks4Request()
{
	packet.Clear();
	packet_length = 0;
	{
		packet.Cat(0x04);
		packet.Cat(command);
		if(lookup) {
			uint16 port = htons(target_port);
			uint32 addr = htonl(0x00000001);
			packet.Cat((const char*) &port, sizeof(uint16));
			packet.Cat((const char*) &addr, sizeof(uint32));
		}
		else {
			auto *info = ipinfo.GetResult();
			if(info->ai_family == AF_INET6) {
				SetError(SOCKS4_ADDRESS_TYPE_NOT_SUPPORTED);
			}
			sockaddr_in *target = (sockaddr_in*) info->ai_addr;
			packet.Cat((const char*) &target->sin_port, sizeof(uint16));
			packet.Cat((const char*) &target->sin_addr.s_addr, sizeof(uint32));
			ipinfo.Clear();
		}
		if(!proxy_user.IsEmpty()) {
			packet.Cat(proxy_user);
		}
		packet.Cat(0x00);
		if(lookup) {
			packet.Cat(target_host);
			packet.Cat(0x00);
		}
	}
	LLOG(">> SOCKS4: Sending connection request.");
	LDUMPHEX(packet);
	PutGet();
	return true;
}

bool NetProxy::Socks4ParseReply()
{
	LLOG("<< SOCKS4: Command request reply received.");
	LDUMPHEX(packet);
	auto *reply = (Reply::Socks4*) packet.Begin();
	if(reply->version != 0)
		SetError(INVALID_PACKET);
	if(reply->status != 0x5a)
		SetError(reply->status);
	return SocksCommand(command);
}

bool NetProxy::Socks4IsEof()
{
	bool   b = packet.GetCharCount() == sizeof(Reply::Socks4);
	if(b)  b = Socks4ParseReply();
	return b;
}

void NetProxy::Socks5Connect(int cmd)
{
	command = (byte) cmd;
	queue.Clear();
	{
		IsEof = [=] { return Socks5IsEof(); };
		queue.AddTail([=]{ return Init();});
		queue.AddTail([=]{ return Dns(); });
		queue.AddTail([=]{ return Connect();});
		queue.AddTail([=]{ return SocksStart(); });
		queue.AddTail([=]{ return !lookup ? Dns() : true; });
		queue.AddTail([=]{ return Socks5Request(); });
	}
}

bool NetProxy::Socks5Request()
{
	packet.Clear();
	packet_length = 0;

	if(packet_type == SOCKS5_HELO) {
		packet.Cat(0x05);
		packet.Cat(0x02);
		packet.Cat(0x00);
		packet.Cat(0x02);
		LLOG(">> SOCKS5: Sending initial greetings.");
	}
	else
	if(packet_type == SOCKS5_AUTH) {
		packet.Cat(0x01);
		packet.Cat(proxy_user.GetLength());
		packet.Cat(proxy_user);
		packet.Cat(proxy_password.GetLength());
		packet.Cat(proxy_password);
		LLOG(">> SOCKS5: Sending authorization request.");
	}
	else
	if(packet_type == SOCKS5_REQUEST) {
		packet.Cat(0x05);
		packet.Cat(command);
		packet.Cat(0x00);
		if(lookup) {
			packet.Cat(0x03);
			packet.Cat(target_host.GetLength());
			packet.Cat(target_host);
			uint16 port = htons(target_port);
			packet.Cat((const char*) &port, sizeof(uint16));
		}
		else {
			struct addrinfo *info = ipinfo.GetResult();
			if(info->ai_family == AF_INET) {
				sockaddr_in *target = (sockaddr_in*) info->ai_addr;
				packet.Cat(0x01);
				packet.Cat((const char*) &target->sin_addr.s_addr, sizeof(uint32));
				packet.Cat((const char*) &target->sin_port, sizeof(uint16));
			}
			else
			if(info->ai_family == AF_INET6) {
				sockaddr_in6 *target = (sockaddr_in6*) info->ai_addr;
				packet.Cat(0x04);
				packet.Cat((const char*) &target->sin6_addr.s6_addr, byte(16));
				packet.Cat((const char*) &target->sin6_port, sizeof(uint16));
			}
			ipinfo.Clear();
		}
		LLOG(">> SOCKS5: Sending command request.");
	}
	LDUMPHEX(packet);
	PutGet();
	return true;
}

bool NetProxy::Socks5ParseReply()
{
	if(packet_type == SOCKS5_HELO) {
		LLOG("<< SOCKS5: Server greeting reply recieved.");
		LDUMPHEX(packet);
		auto *p = (Reply::Helo*) packet.Begin();
		if(p->version != 0x05)
			SetError(INVALID_PACKET);
		if(p->method == 0x00)
			packet_type = SOCKS5_REQUEST;
		else
		if(p->method == 0x02)
			packet_type = SOCKS5_AUTH;
		else
			SetError(SOCKS5_INVALID_AUTHENTICATION_METHOD);
		return Socks5Request();
	}
	else
	if(packet_type == SOCKS5_AUTH) {
		LLOG("<< SOCKS5: Authorization reply received.");
		LDUMPHEX(packet);
		auto *p = (Reply::Auth*) packet.Begin();
		if(p->version != 0x01)
			SetError(INVALID_PACKET);
		if(p->status != 0x00)
			SetError(p->status);
		packet_type = SOCKS5_REQUEST;
		return Socks5Request();
	}
	else
	if(packet_type == SOCKS5_REQUEST) {
		LLOG("<< SOCKS5: Command request reply received.");
		LDUMPHEX(packet);
		auto *p = (Reply::Socks5*) packet.Begin();
		if(p->version != 0x05)
			SetError(INVALID_PACKET);
		if(p->status != 0x00)
			SetError(p->status);
		return SocksCommand(command);
	}
	NEVER();
}

bool NetProxy::Socks5IsEof()
{
	auto n = packet.GetLength();
	if(packet_type == SOCKS5_HELO && n == sizeof(Reply::Helo) ||
	   packet_type == SOCKS5_AUTH && n == sizeof(Reply::Auth))
		return Socks5ParseReply();
	if(packet_type == SOCKS5_REQUEST) {
		auto* p = (Reply::Socks5*) packet.Begin();
		const int header = 4;
		if(n == 5) {
			if(p->addrtype == 0x01)
				packet_length = header + sizeof(p->ipv4);		// 4 bytes for IPv4 address.
			else
			if(p->addrtype == 0x03)
				packet_length = header + p->namelen;			// 1 byte of name length (followed by 1–255 bytes the domain name).
			else
			if(p->addrtype == 0x04)
				packet_length = header + sizeof(p->ipv6);		// 16 bytes for IPv6 address.
			packet_length += int(2);					// 2 bytes for server bound port number.
		}
		if(n == packet_length) {
			return Socks5ParseReply();
		}
	}
	return false;
}

bool NetProxy::Connect(int type, const String& host, int port)
{
	target_host = host;
	target_port = port;
	proxy_type  = type;

	switch(proxy_type) {
		case HTTP:
			HttpcConnect();
			break;
		case SOCKS4:
			Socks4Connect(CONNECT);
			break;
		case SOCKS5:
			Socks5Connect(CONNECT);
			break;
		default:
			NEVER();
	};
	return Run();
}

bool NetProxy::Bind(int type, const String& host, int port)
{
	target_host = host;
	target_port = port;
	proxy_type  = type;

	switch(proxy_type) {
		case SOCKS4:
			Socks4Connect(BIND);
			break;
		case SOCKS5:
			Socks5Connect(BIND);
			break;
		default:{
			String err = GetMsg(HTTPCONNECT_NOBIND);
			error = MakeTuple<int, String>(10011, err);
			LLOG("Failed. " << err);
			status = FAILED;
			return false;
		}
	};
	return Run();
}

bool NetProxy::Run()
{
	if(async)
		return true;
	while(Do());
	return !IsError();
}

bool NetProxy::Do()
{	try {
		Check();
		if(!queue.IsEmpty() && queue.Head()()) {
			queue.DropHead();
		}
		if(queue.IsEmpty()) {
			status = FINISHED;
			LLOG("Proxy connection is successful.");
		}
		else WhenDo();
	}
	catch(Error& e) {
		status = FAILED;
		queue.Clear();
		error = MakeTuple<int, String>(e.code, e);
		LLOG("failed. " << e);
		Exit();
	}
	return status == WORKING;
}

void NetProxy::ParseBoundAddr()
{
	int port   = 0;
	int family = 0;
	String ip;
	switch(proxy_type) {
		case SOCKS4: {
			auto *p = (Reply::Socks4*) packet.Begin();
			family = AF_INET;
			port = p->port;
			ip.Cat((char*) &p->address, sizeof(p->address));
			break;
		}
		case SOCKS5: {
			auto *p = (Reply::Socks5*) packet.Begin();
			port = (*(packet.Last() - sizeof(uint16)));
			switch(p->addrtype) {
				case 0x01: {
					family = AF_INET;
					ip.Cat((char*) &p->ipv4, sizeof(p->ipv4));
					break;
				}
				case 0x04: {
					family = AF_INET6;
					ip.Cat((char*) &p->ipv6, sizeof(p->ipv6));
					break;
				}
				case 0x03:
					return;
				default:
					NEVER();
			}
		}
	};
	String ip_buffer;
	if(!NtoP(family, ip, ip_buffer))
		throw Error(-1, Format("SOCKS%d: Malformed BIND address.", proxy_type));
	LLOG(Format("SOCKS%d: Bind successful. [%s:%d]", proxy_type, ip_buffer, ntohs(port)));
	WhenBound(ip_buffer, ntohs(port));
}

NetProxy::NetProxy()
{
	socket = NULL;
	proxy_type = 0;
	start_time = 0;
	timeout = 60000;
	timeout_backup = 0;
	status = IDLE;
	async = false;
	ssl = false;
	lookup = false;
	bound = false;
	events = 0;
	proxy_type = HTTP;
	command = CONNECT;
}

NetProxy::~NetProxy()
{
}
}
