#include "Ftp.h"

#ifdef PLATFORM_WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

namespace Upp {

static bool sTrace = false;

#define LLOG(x)       do { if(sTrace) RLOG("Ftp Client #" << uid << ": " << x); } while(false)

Ftp& Ftp::Timeout(int ms)
{
	control_socket.Timeout(ms);
	data_socket.Timeout(ms);
	return *this;
}

Ftp& Ftp::WaitStep(int ms)
{
	control_socket.WaitStep(ms);
	data_socket.WaitStep(ms);
	return *this;
}

bool Ftp::Connect(const String& url)
{
	UrlInfo u(url);

	connection.addr = u.host;
	connection.port = IsNull(u.port) ? 21 : StrInt(u.port);

	if(!IsNull(u.scheme))
		SSL(u.scheme == "ftps");

	auto q = u["timeout"];
	if(!IsNull(q))
		Timeout(StrInt(q));

	q = u["chunksize"];
	if(!IsNull(q))
		ChunkSize(StrInt(q));

	q = u["mode"];
	if(!IsNull(q))
		Active(q == "active");

	q = u["utf8"];
	if(!IsNull(q))
		Utf8(q == "on");

	q = u["pos"];
	if(!IsNull(q))
		SetPos(StrInt64(q));

	return Run([=, u = pick(u)]{
		StartConnect(control_socket);
		StartLogin(u.username, u.password);
	});
}

bool Ftp::Connect(const String& host, int port)
{
	connection.addr = host;
	connection.port = port;

	return Run([=]{
		StartConnect(control_socket);
	});
}

bool Ftp::Connect(const String& host, int port, const String& user, const String& pwd)
{
	connection.addr = host;
	connection.port = port;

	return Run([=]{
		StartConnect(control_socket);
		StartLogin(user, pwd);
	});
}

bool Ftp::Login(const String& user, const String& pwd)
{
	uname = user;

	return Run([=]{
		StartLogin(user, pwd);
	});
}

void Ftp::Disconnect()
{
	Run([=]{
		StartCommand(OpCode::QUIT);
	});
}

bool Ftp::ListDir(const String& path, DirList& list)
{
	auto s = path;

	if(!IsNull(path))
		s.Insert(0, " ");

	return RecvData(
		OpCode::LIST,
		EncodePath(s),
		[=, &list](void *data, int len){
			ParseDirList(uname, String((const char*) data, len), list);
		},
		true,
		true
	);
}

String Ftp::Get(const String& path, bool ascii)
{
	RecvData(
		OpCode::RETR,
		EncodePath(path),
		[=](void *data, int len){
			auto& s = (String&) result;
			s.Cat((const char*) data, len);
		},
		ascii
	);
	return pick(result);
}

bool Ftp::Get(const String& path, Stream& s, bool ascii)
{
	return RecvData(
		OpCode::RETR,
		EncodePath(path),
		[&s](void *data, int len){
			s.Put(data, len);
		},
		ascii
	);
}

bool Ftp::Put(Stream& s, const String& path, bool ascii)
{
	return SendData(OpCode::STOR, EncodePath(path), s, ascii);
}

bool Ftp::Put(const String& s, const String& path, bool ascii)
{
	wstream.Create((const void*) s.Begin(), s.GetLength());
	return SendData(OpCode::STOR, EncodePath(path), wstream, ascii);
}

bool Ftp::Append(Stream &s, const String& path, bool ascii)
{
	return SendData(OpCode::APPE, EncodePath(path), s, ascii);
}

bool Ftp::Append(const String& s, const String& path, bool ascii)
{
	wstream.Create((const void*) s.Begin(), s.GetLength());
	return SendData(OpCode::APPE, EncodePath(path), wstream, ascii);
}

AsyncWork<String> Ftp::AsyncGet(const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, progress = pick(progress)]{
		StringStream data;
		Ftp::StartAsync(OpCode::RETR, url, data, progress);
		return pick(data.GetResult());
	});
}

AsyncWork<void> Ftp::AsyncGet(const String& url, Stream& out, Gate<int64, int64, int64> progress)
{
	return Async([=, &out, progress = pick(progress)]{
		Ftp::StartAsync(OpCode::RETR, url, out, pick(progress));
	});
}

AsyncWork<void> Ftp::AsyncPut(String& in, const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, &in]{
		StringStream data(in);
		Ftp::StartAsync(OpCode::STOR, url, data, pick(progress));
	});
}

AsyncWork<void> Ftp::AsyncPut(Stream& in, const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, &in]{
		Ftp::StartAsync(OpCode::STOR, url, in, pick(progress));
	});
}

AsyncWork<void> Ftp::AsyncAppend(String& in, const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, &in]{
		StringStream data(in);
		Ftp::StartAsync(OpCode::APPE, url, data, pick(progress));
	});
}

AsyncWork<void> Ftp::AsyncAppend(Stream& in, const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, &in]{
		Ftp::StartAsync(OpCode::APPE, url, in, pick(progress));
	});
}

AsyncWork<void> Ftp::AsyncGetToFile(const String& url, const String& dest, Gate<int64, int64, int64> progress)
{
	return Async([=, progress = pick(progress)]{
		FileOut fo(dest);
		if(!fo)
			throw Ftp::Error(Format("Unable to open file '%s' for writing.", dest));
		Ftp::StartAsync(OpCode::RETR, url, fo, progress);
	});
}

AsyncWork<void> Ftp::AsyncPutFromFile(const String& src, const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, progress = pick(progress)]{
		FileIn fi(src);
		if(!fi)
			throw Ftp::Error(Format("Unable to open file '%s' for reading.", src));
		Ftp::StartAsync(OpCode::STOR, url, fi, progress);
	});

}

AsyncWork<void> Ftp::AsyncAppendFromFile(const String& src, const String& url, Gate<int64, int64, int64> progress)
{
	return Async([=, progress = pick(progress)]{
		FileIn fi(src);
		if(!fi)
			throw Ftp::Error(Format("Unable to open file '%s' to append.", src));
		Ftp::StartAsync(OpCode::APPE, url, fi, progress);
	});
}

void Ftp::StartConnect(TcpSocket& s, bool dataconn)
{
	queue.AddTail() = [=, sock = &s]
	{
		switch(opcode) {
			case OpCode::DNS: {
				if(ipinfo.InProgress())
					break;
				if(!ipinfo.GetResult())
					throw Error("DNS lookup failed.");
				opcode = OpCode::CONN;
			}
			case OpCode::CONN: {
				if(!sock->Connect(ipinfo))
					break;
				LLOG("Successfully connected to " << connection.addr << ":" << connection.port);
				ipinfo.Clear();
				connection.Clear();
				if(!dataconn) {
					opcode = OpCode::HELO;
					break;
				}
				else
				if(!ssl) {
					opcode = OpCode::NONE;
					return true;
				}
				opcode = OpCode::SSL;
				sock->StartSSL();
			}
			case OpCode::SSL: {
				if(!IsBlocking())
					if((events = sock->SSLHandshake()))
						break;
				GetSSLInfo(*sock);
				opcode = OpCode::NONE;
				return true;
			}
			case OpCode::HELO: {
				if(!PutGet(Null))
					break;
				opcode = OpCode::NONE;
				return connected = true;
			}
			default: {
				if(!dataconn)
					connected = false;
				aborted = false;
				sock->Clear();
				done    = 0;
				events  = 0;
				opcode  = OpCode::DNS;
				ipinfo.Start(connection.addr, connection.port);
				LLOG("Connecting to " << connection.addr << ":" << connection.port);
				break;
			}
		};
		Check(*sock);
		return false;
	};
}

void Ftp::StartLogin(const String& u, const String& p)
{
	static constexpr const char *anon = "anonymous";

	String username = IsNull(u) ? anon : ~u;
	String password = IsNull(p) ? anon : ~p;

	// SSL
	StartSSL(0);

	// Login
	queue.AddTail() = [=]
	{
		switch(reply.prevcode) {
			case 200:
			case 220:
				PutGet("USER " + username, Reply::SUCCESS|Reply::PENDING);
				break;
			case 331:
				PutGet("PASS " + password, Reply::SUCCESS|Reply::PENDING);
				break;
			case 332:
				PutGet("ACCT noaccount");
				break;
			case 202:
			case 211:
			case 230:
				return true;
			default:
				throw Error("Unhandled reply encountered in login sequence.");
		}
		return false;
	};

	if(utf)
		StartCommand(OpCode::OPTS, "UTF8 ON");
}

void Ftp::StartSSL(int size)
{
	if(!ssl)
		return;

	queue.AddTail() = [=] {
		switch(opcode) {
			case OpCode::TLS: {
				if(!PutGet("AUTH TLS", Reply::SUCCESS|Reply::HALT))
					break;
				opcode = reply.IsSuccess()
					? OpCode::SSL
					: OpCode::SSL3;
				if(!reply.IsError())
					control_socket.StartSSL();
				else
				if(reply.code != 504 && reply.code != 534)
					throw Error(reply);
				break;
			}
			case OpCode::SSL3: {
				if(!PutGet("AUTH SSL"))
					break;
				opcode = OpCode::SSL;
				control_socket.StartSSL();
			}
			case OpCode::SSL: {
				if((events = control_socket.SSLHandshake()))
					break;
				GetSSLInfo(control_socket);
				opcode = OpCode::PBSZ;
			}
			case OpCode::PBSZ: {
				if(!PutGet("PBSZ " + AsString(size)))
					break;
				opcode = OpCode::PROT;
			}
			case OpCode::PROT: {
				if(!PutGet("PROT P"))
					break;
				LLOG("FTPS negotiation is successful. Client is in secure mode.");
				reply << 220;
				return true;
			}
			default:
				LLOG("Starting FTPS negotiation...");
				opcode = OpCode::TLS;
				break;
		};
		return false;
	};
}

void Ftp::StartCommand(const OpCode& code, const Value& req)
{
	opcode = OpCode::NONE;

	queue.AddTail() = [=] {

		auto b = false;

		switch(opcode) {
			case OpCode::PWD:
				b = PutGet("PWD");
				if(b) result = Ftp::DecodePath(*reply);
				break;
			case OpCode::CWD:
				b = PutGet("CWD " << req);
				break;
			case OpCode::CDUP:
				b = PutGet("CDUP");
				break;
			case OpCode::MKD:
				b = PutGet("MKD " << req);
				break;
			case OpCode::RMD:
				b = PutGet("RMD " << req);
				break;
			case OpCode::NOOP:
				b = PutGet("NOOP");
				break;
			case OpCode::RNFR: {
				b = PutGet("RNFR " << req[0], Reply::PENDING);
				if(!b)
					break;
				opcode = OpCode::RNTO;
			}
			case OpCode::RNTO:
				b = PutGet("RNTO " << req[1]);
				break;
			case OpCode::DELE:
				b = PutGet("DELE " << req);
				break;
			case OpCode::LIST:
				b = PutGet("LIST" << req, Reply::WAIT);
				break;
			case OpCode::FEAT:
				b = PutGet("FEAT");
				if(b) ParseFeatureList();
				break;
			case OpCode::OPTS:
				b = PutGet("OPTS " << req);
				break;
			case OpCode::SIZE:
				b = PutGet("SIZE " << req, Reply::ANY);
				if(b) ScanFileSize();
				break;
			case OpCode::TYPE:
				b = PutGet("TYPE " << req);
				break;
			case OpCode::PASV:
				b = PutGet("PASV");
				if(b) SetPassiveMode(OpCode::PASV);
				break;
			case OpCode::EPSV: {
				b = PutGet("EPSV", Reply::ANY);
				if(b) {
					b = reply.IsSuccess();
					if(b) SetPassiveMode(OpCode::EPSV);
					else  opcode = OpCode::PASV;
				}
				break;
			}
			case OpCode::PORT:
				b = PutGet("PORT " << connection.addr);
				break;
			case OpCode::EPRT: {
				b = PutGet("EPRT " << connection.addr, Reply::ANY);
				if(b) {
					b = reply.IsSuccess();
					if(!b) {
						SetActiveMode(OpCode::PORT);
						opcode = OpCode::PORT;
					}
				}
				break;
			}
			case OpCode::RETR:
				b = PutGet("RETR " << req, Reply::WAIT);
				break;
			case OpCode::STOR:
				b = PutGet("STOR " << req, Reply::WAIT);
				break;
			case OpCode::APPE:
				b = PutGet("APPE " << req, Reply::WAIT);
				break;
			case OpCode::REST:
				b = PutGet("REST " << req, Reply::PENDING);
				break;
			case OpCode::QUIT: {
				if(!connected)
					return true;
				if((b = PutGet("QUIT")))
					CloseSockets();
				break;
			}
			case OpCode::RAW:
				b = PutGet(req, Reply::WAIT|Reply::SUCCESS|Reply::PENDING);
				break;
			case OpCode::NONE:
				opcode = code;
				break;
			default:
				throw Error("Unknown opcode.");
		}
		return b;
	};
}

void Ftp::StartAccept(const OpCode& code, const Value& req)
{
	queue.AddTail() = [=]
	{
		SetActiveMode(OpCode::EPRT);
		return true;
	};

	StartCommand(OpCode::EPRT);
	StartRestart(code, position);

	queue.AddTail() = [=]
	{
		if(!listener.Listen(connection.port, 1, connection.family == AF_INET6))
			throw Error(listener.GetErrorDesc());
		LLOG("[Data] Started listening on port " << connection.port);
		return true;
	};

	StartCommand(code, req);

	queue.AddTail() = [=]
	{
		auto b = data_socket.Accept(listener);
		Check(data_socket);
		if(b)
			LLOG("[Data] Incoming connection accepted.");
		return b;
	};
}

void Ftp::StartRestart(const OpCode& code, int64 pos)
{
	if(restart) {
		switch(code) {
		case OpCode::RETR:
		case OpCode::STOR:
			StartCommand(OpCode::REST, pos);
			restart = false;
		}
	}
}

void Ftp::StartTransfer(const OpCode& code, const Value& req, bool ascii)
{
	done   = 0;
	total  = 0;
	result = Null;

	StartCommand(OpCode::TYPE, ascii ? "A" : "I");

	switch(code) {
	case OpCode::RETR:
	case OpCode::STOR:
	case OpCode::APPE:
		StartCommand(OpCode::SIZE, req);
		break;
	}

	switch(mode) {
	case ACTIVE:
		StartAccept(code, req);
		break;
	case PASSIVE:
		StartCommand(OpCode::EPSV);
		StartRestart(code, position);
		StartConnect(data_socket, true);
		StartCommand(code, req);
		break;
	}
}

void Ftp::StartAsync(const OpCode& code, const String& url, Stream& io, Gate<int64, int64, int64> progress)
{
	Ftp worker;

	worker.WhenWait = [=, &worker]{
		if(CoWork::IsCanceled()) {
			worker.Abort();
		}
	};

	worker.WhenProgress = [=, &progress, id = worker.GetId()](int64 d, int64 t) {
		return progress(id, d, t);
	};

	UrlInfo info(url);

	auto b = worker.Timeout(60000).Connect(url);
	if(b){
		switch(code) {
		case OpCode::RETR:
			b = worker.Get(info.path, io);
			break;
		case OpCode::STOR:
			b = worker.Put(io, info.path);
			break;
		case OpCode::APPE:
			b = worker.Append(io, info.path);
			break;
		default:
			NEVER();
		}
	}
	if(!b)
		throw Error(worker.GetErrorDesc());
}

void Ftp::StartAbort()
{
	aborted = false;
	data_socket.Abort();
	{
		auto cmdput = [=]{
			auto n = control_socket.Put(packet);
			if(n)
				packet.Remove(0, n);
			else
				events = WAIT_WRITE;
			return packet.IsEmpty();
		};
		#ifdef PLATFORM_POSIX
		packet = "\377\364\377";
		queue.AddTail() = [=]{
			return cmdput();
		};
		queue.AddTail() = [=]{
			auto n = send(control_socket.GetSOCKET(), "\362", 1, MSG_OOB);
			auto b = n == 1 || (n == -1 && errno != EAGAIN);
			if(b) packet = "ABOR\r\n";
			return b;
		};
		#else
		packet = "ABOR\r\n";
		#endif
		queue.AddTail() = [=]{
			auto b = cmdput();
			if(b) {
				packet.Clear();
				data_socket.Close();
				LLOG("[Send] ABOR");
			}
			return b;
		};
		queue.AddTail() = [=]{
			auto b = PutGet(Null, Reply::ANY);
			if(b){
				if(reply.IsFailure() || reply.IsError()) {
					queue.AddTail() = [=]{
						auto b = PutGet(Null);
						if(b)
							throw Error("Transfer aborted.");
						return b;
					};
				}
			}
			return b;
		};
	}
}

void Ftp::Check(TcpSocket& s)
{
	if(IsBlocking() && s.IsTimeout())
		throw Error("Connection timed out.");
	if(s.IsAbort())
		throw Error("Connection aborted.");
	if(s.IsError())
		throw Error(s.GetErrorDesc());
	WhenWait();
}

void Ftp::CloseSockets()
{
	connected = false;
	aborted   = false;

	if(data_socket.IsOpen())
		data_socket.Close();
	if(control_socket.IsOpen())
		control_socket.Close();

	LLOG("Disconnected from the FTP server.");
}

int Ftp::GetReplyCode(const String& s)
{
	if(s.GetCount() < 3 || !IsDigit(s[0]) || !IsDigit(s[1]) || !IsDigit(s[2]))
		return 0;
	return StrInt(s.Mid(0, 3));
}

bool Ftp::PutGet(const String& s, byte r)
{
	sock = &control_socket;
	bool b = false;

	while(!b) {
		switch(events) {
			case WAIT_WRITE: {
				auto n = control_socket.Put(packet);
				if(n == 0) {
					b = true;
					continue;
				}
				done += n;
				if(done == packet.GetLength()) {
					done   = 0;
					packet = Null;
					events = WAIT_READ;
				}
				break;
			}
			case WAIT_READ: {
				int c = control_socket.Get();
				if(c < 0) {
					b = true;
					continue;
				}
				packet.Cat(c);
				if(reply.code == 0 && packet.GetCount() == 3) {
					auto n = GetReplyCode(packet);
					if(!n)
						throw Error("Invalid reply.");
					reply << n;
				}
				else
				if(packet.GetCount() >= 4) {
					if(!reply.IsMultiline() && packet[3] == '-')
						reply.multiline = true;
					else
					if(reply.IsMultiline() && packet[3] == ' ')
						reply.multiline = false;
				}
				if(packet.EndsWith("\r\n")) {
					LLOG("[Recv] " << TrimRight(packet));
					reply << packet;
					packet.Clear();
					if(!reply.IsMultiline()) {
						auto b = false;
						if(r & Reply::ANY)            b = true;
						if(!b && r & Reply::WAIT)	  b = reply.IsWait();
						if(!b && r & Reply::SUCCESS)  b = reply.IsSuccess();
						if(!b && r & Reply::PENDING)  b = reply.IsPending();
						if(!b && r & Reply::FAILURE)  b = reply.IsFailure();
						if(!b && r & Reply::HALT)     b = reply.IsError();
						WhenReply();
						if(!b)
							throw Error(reply);
						events = 0;
						opcode = OpCode::NONE;
						return true;
					}
				}
				break;
			}
			default: {
				reply  = Null;
				packet = Null;
				done   = 0;
				if(!s.IsEmpty()) {
					if(!s.EndsWith("\r\n"))
						packet = s + "\r\n";
					LLOG("[Send] " << (!s.StartsWith("PASS")
											? TrimRight(s)
											: "PASS ****"));
					events = WAIT_WRITE;
				}
				else
					events = WAIT_READ;
				break;
			}
		}
	}
	Check(control_socket);
	return false;
}

bool Ftp::Run(const Event<>& cmd)
{
	status = WORKING;
	reply.Clear();
	queue.Clear();
	data_error = false;
	cmd();
	if(control_socket.GetTimeout() != 0)
		while(Do0());
	else
		return true;
	return !IsError();
}

bool Ftp::Do0()
{
	try {
		if(!queue.IsEmpty()) {
			auto& cmd = queue.Head();
			if(cmd())
				queue.DropHead();
		}
		if(queue.IsEmpty())
			status = FINISHED;
	}
	catch(const Error& e) {
		queue.Clear();
		events = 0;
		opcode = OpCode::NONE;
		sock   = &control_socket;
		if(aborted)
		{
			StartAbort();
			return true;
		}
		status = FAILED;
		error  = MakeTuple<int, String>(e.code, e);
		LLOG("Failed. " << e);
	}
	return status == WORKING;
}

bool Ftp::WriteData(Stream& s)
{
	sock = &data_socket;

	while(!data_socket.IsEof()) {
		auto ss = s.Get(chunk_size);
		auto n  = data_socket.Put(ss);
		s.SeekCur(-(ss.GetLength() - n));
		if(n > 0) {
			done += n;
			if(WhenProgress(done, total)) {
				aborted = true;
				break;
			}
		}
		if(s.IsEof() || n == 0) {
			if(n == 0) {
				events = WAIT_WRITE;
				break;
			}
			if(s.IsError())
				LLOG("[Data] Stream error: " << s.GetErrorText());
			data_socket.Close();
			break;
		}
	}
	return IsDataEof();
}

bool Ftp::ReadAscii(const Event<void*, int>& fn, bool log)
{
	sock = &data_socket;

	while(!data_socket.IsEof()) {
		auto c = data_socket.Get();
		if(c < 0) {
			events = WAIT_READ;
			break;
		}
		done++;
		if(c == '\r')
			continue;
		if(c == '\n') {
			if(log)
				LLOG("[Data] " << packet);
#ifdef PLATFORM_WIN32
			packet.Cat('\r');
#endif
			packet.Cat('\n');
			WhenContent
				? WhenContent((void*)packet.Begin(), packet.GetLength())
				: fn((void*)packet.Begin(), packet.GetLength());
			packet = Null;
			continue;
		}
		packet.Cat(c);
		if(WhenProgress(done, total)) {
			aborted = true;
			break;
		}
	}
	return IsDataEof();
}

bool Ftp::ReadBinary(const Event<void*, int>& fn)
{
	sock = &data_socket;
	Buffer<char> buffer(chunk_size);

	while(!data_socket.IsEof()) {
		auto n = data_socket.Get(buffer, chunk_size);
		if(!n) {
			events = WAIT_READ;
			break;
		}
		done += n;
		WhenContent
			? WhenContent(buffer, n)
			: fn(buffer, n);
		if(WhenProgress(done, total)) {
			aborted = true;
			break;
		}
	}
	return IsDataEof();
}

bool Ftp::SendData(const OpCode& code, const Value& req, Stream& s, bool ascii)
{
	return Run([=, ss = std::ref<Stream>(s)]{
		StartTransfer(code, req, ascii);
		queue.AddTail() = [=] { return WriteData(ss.get()); };
		queue.AddTail() = [=] { return PutGet(Null); };
	});
}

bool Ftp::RecvData(const OpCode& code, const Value& req, const Event<void*, int>&& fn, bool ascii, bool log)
{
	return Run([=,&fn]{
		StartTransfer(code, req, ascii);
		ascii
			? queue.AddTail() = [=, fn = pick(fn)] { return ReadAscii(fn, log); }
			: queue.AddTail() = [=, fn = pick(fn)] { return ReadBinary(fn); };
		queue.AddTail() = [=] { return PutGet(Null); };
	});
}

bool Ftp::IsDataEof()
{
	position = done;

	CheckAbort();

	auto t = IsBlocking() && data_socket.IsTimeout();
	auto b = data_socket.IsEof() || t;
	data_error = data_socket.IsError() || data_socket.IsAbort() || t;

	if(b) {
		if(data_socket.IsOpen())
			data_socket.Close();
		if(mode == ACTIVE)
			listener.Close();

		sock   = &control_socket;
		events = 0;

		LLOG("[Data] " << done << " bytes transferred.");

		if(!data_error)
			return b;
		if(data_socket.IsTimeout())
			LLOG("[Data] Failed. Operation timed out.");
		else
		if(data_socket.IsError())
			LLOG("[Data] Failed. " << data_socket.GetErrorDesc());
		else
		if(data_socket.IsAbort())
			LLOG("[Data] Aborted.");
		b = data_error;
	}
	return b;
}

void Ftp::SetPassiveMode(const OpCode& code)
{
	String addr;
	connection.Clear();

	switch(code) {
		case OpCode::PASV: {
			if(GetConnectionInfo(ConnectionInfo::Type::PEER).family != AF_INET)
				throw Error("PASV command requires IPV4");
			String addr, h1, h2, h3, h4, p1, p2;
			if((!(addr = GetDelimitedString(*reply, '(', ')')).IsEmpty()  ||
				!(addr = GetDelimitedString(*reply, '=', ' ')).IsEmpty()) &&
				SplitTo(addr, ',', h1, h2, h3, h4, p1, p2)) {
				connection.addr = h1 + "." + h2 + "." + h3 + "." + h4;
				connection.port = StrInt(p1) * 256 + StrInt(p2);
				break;
			}
			throw Error("Couldn't parse PASV reply.");
		}
		case OpCode::EPSV: {
			addr = GetConnectionInfo(ConnectionInfo::Type::PEER).addr;
			int begin = reply.text.FindAfter("(|||");
			int end = reply.text.ReverseFind("|)");
			if(begin == -1 || end == -1 || addr.IsEmpty())
				throw Error("Couldn't parse EPSV reply.");
			connection.addr = pick(addr);
			connection.port = StrInt(reply.text.Mid(begin, end - begin));
			break;
		}
		default:
			NEVER();
	};
}

void Ftp::SetActiveMode(const OpCode& code)
{
	String addr;
	ConnectionInfo ci;

	switch(code) {
		case OpCode::PORT: {
			ci = GetConnectionInfo(ConnectionInfo::Type::LOCAL);
			if(ci.family != AF_INET)
				throw Error("PORT command requires IPV4");
			connection.addr = pick(ci.addr);
			connection.addr += "," + AsString((ci.port & 0xff00) >> 8) + "," + AsString(ci.port & 0xff);
			break;
		}
		case OpCode::EPRT: {
			ci = GetConnectionInfo(ConnectionInfo::Type::LOCAL);
			connection.addr << (ci.family == AF_INET6 ? "|2|" : "|1|") << ci.addr << "|" << ci.port << "|";
			break;
		}
		default:
			NEVER();
	};
	connection.port = ci.port;
	connection.family = ci.family;
}

void Ftp::ScanFileSize()
{
	if(reply.IsSuccess()) {
		total = ScanInt64(TrimBoth(reply.text.Mid(3)));
		result = queue.GetCount() == 1 ? total : Null;
	}
	else
	if(queue.GetCount() == 1) // Standalone SIZE command should fail.
		throw Error(reply);
}

void Ftp::ParseFeatureList()
{
	if(!reply.IsSuccess()) {
		result = Null;
		return;
	}

	ValueMap features;
	StringStream ss(*reply);
	while(!ss.IsEof()) {
		// For more details, see RFC 2389, pp. 4-5.
		String s = TrimBoth(ToLower(ss.GetLine()));
		if(!s.StartsWith("211")) {
			Vector<String> v = Split(s, ' ');
		features.GetAdd(v[0]);
		for(int i = 1;  i < v.GetCount(); i++)
			features(v[0]) << v[i];
		}
	}
	result = pick(features);
}

String Ftp::GetReplyAsXml()
{
	StringStream ss(reply.text);
	String output;

	while(!ss.IsEof()) {
		String s = ss.GetLine();
		int rc = GetReplyCode(s);
		XmlTag xml;
		xml.Tag("reply")("code", rc)("type", rc > 0 ? "protocol" : "internal");
		if(rc > 0 && s[3] == '-') {
			String ll;
			for(bool eof = false;;) {
				ll << XmlTag("line").Text(TrimBoth(rc > 0 ? s.Mid(4) : s));
				if((rc > 0 && s[3] == ' ') || eof)
					break;
				s = ss.GetLine();
				rc = GetReplyCode(s);
				eof = ss.IsEof();
			}
			output << xml(ll);
		}
		else
			output << xml(XmlTag("line").Text(TrimBoth(rc > 0 ? s.Mid(4) : s)));
	}
	return pick(output);
}

void Ftp::GetSSLInfo(TcpSocket& s)
{
	if(WhenSSLInfo && WhenSSLInfo(s.GetSSLInfo())) {
		CloseSockets();
		throw Error("SSL credentials are rejeceted. Bailing out!..");
	}
}

Ftp::ConnectionInfo Ftp::GetConnectionInfo(ConnectionInfo::Type t)
{
	ConnectionInfo ci;
	struct sockaddr_storage ss;
	const auto *in4 = reinterpret_cast<sockaddr_in*>(&ss);
	const auto *in6 = reinterpret_cast<sockaddr_in6*>(&ss);

	socklen_t ss_size = sizeof(sockaddr_storage);
	Zero(ss);

	switch(t) {
		case ConnectionInfo::Type::PEER:
			if(getpeername(control_socket.GetSOCKET(), (sockaddr*) &ss, &ss_size) != 0)
				throw Error("Couldn't get peer information.");
			break;
		case ConnectionInfo::Type::LOCAL:
			if(getsockname(control_socket.GetSOCKET(), (sockaddr*) &ss, &ss_size) != 0)
				throw Error("Couldn't get socket information.");
			break;
		default:
			NEVER();
	};
	ci.family = ss.ss_family;
#ifdef PLATFORM_WIN32
	ci.addr = inet_ntoa(in4->sin_addr);
#else
	Buffer<char> buffer(64, 0), ip0(64, 0);
	ci.family == AF_INET
		?	memcpy(ip0, &in4->sin_addr,  sizeof(in_addr))
		:	memcpy(ip0, &in6->sin6_addr, sizeof(in6_addr));
	ci.addr = inet_ntop(ci.family, ip0, buffer, 64);
#endif
	if(ci.addr.IsEmpty())
		throw Error("Couldn't get connection info.");

	constexpr dword LOPORT = 49152;
	constexpr dword HIPORT = 65535;

	ci.port = listen_port == 0
		? ((int) Random(HIPORT - LOPORT + 1) + LOPORT)
		:  listen_port;
	return ci;
}

String Ftp::EncodePath(const String& path)
{
	String r;
	for(int i = 0; i < path.GetLength(); i++)
		r.Cat((path[i] == '\n') ? '\0' : path[i]);
	return r;
}

String Ftp::DecodePath(const String& path)
{
	int b = -1, e = -1;
	String r, d;
	if((b = path.Find('\"')) == -1 || (e = path.ReverseFind('\"')) == -1 || b == e)
		return String::GetVoid();
	d = path.Mid(b + 1, (e - 1) - b);
	for(int i = 0; i < d.GetLength(); i++) {
		if(d[i] == '\"' && d[i + 1] == '\"') {
			r.Cat(d[++i]);
			continue;
		}
		r.Cat((d[i] == '\0') ? '\n' : d[i]);
	}
	return r;
}

String Ftp::GetDelimitedString(const String& s, int delim1, int delim2)
{
	int b = -1, e = -1;
	if((b = s.Find(delim1, 0)) == -1 || (e = s.Find(delim2, ++b)) == -1)
		return String::GetVoid();
	return s.Mid(b, e - b);
}

bool Ftp::ParseDirList(const String& user, const String& in, Ftp::DirList& out)
{
	StringStream l(in);
	while(!l.IsEof()) {
		Ftp::DirEntry e;
		if(e.Parse(l.GetLine())) {
			e.User(user);
			out.AddPick(pick(e));
		}
	}
	return !out.IsEmpty();
}

void Ftp::Trace(bool b)
{
	sTrace = b;
}

Ftp::Ftp()
{
	sock        = &control_socket;
	listen_port = 0;
	chunk_size  = 65536;
	status      = IDLE;
	mode        = PASSIVE;
	opcode      = OpCode::NONE;
	done        = 0;
	total		= 0;
	events      = 0;
	position	= 0;
	reply       = Null;
	utf         = false;
	ssl         = false;
	restart     = false;
	connected	= false;
	aborted		= false;
	data_error  = false;
	data_socket.WhenWait = WhenWait;
	control_socket.WhenWait = WhenWait;
	static int64 id; // Unique ID for each instance.
	uid         = (id = (id == INT64_MAX ? 1 : ++id));
}

Ftp::~Ftp()
{
	Timeout(5000).Disconnect();
}

String Ftp::DirEntry::ToXml() const
{
	static const char *hypen = "-", *r = "r", *w = "w", *x = "x";
	auto xml = XmlTag("ftp:direntry")
			("style", IsUnixStyle()
				? "unix" : "dos")
			("type", (IsFile()
				? "file" : (IsDirectory()
				? "directory" : (IsSymLink()
				? "symlink" : "other"))))
			("owner", GetOwner())
			("group", GetGroup())
			("size",  AsString(GetSize()))
			("modified", AsString(GetLastModified()))
			("permissions", Format("%c%c%c%c%c%c%c%c%c",
				(owner[0] ? *r : *hypen),
				(owner[1] ? *w : *hypen),
				(owner[2] ? *x : *hypen),
				(group[0] ? *r : *hypen),
				(group[1] ? *w : *hypen),
				(group[2] ? *x : *hypen),
				(other[0] ? *r : *hypen),
				(other[1] ? *w : *hypen),
				(other[2] ? *x : *hypen)
			));
		if(IsUnixStyle() && IsSymLink())
			xml("realname", GetRealName());
		return pick(xml.Text(GetName()));
}

bool Ftp::DirEntry::Parse(const String& dir_entry)
{
	static constexpr const char* NixPattern = "^([a-zA-Z\\-]{10,10})\\s+(\\d+)\\s+(\\S*)\\s+(\\S*)\\s+(\\d+)\\s+(\\S+\\s+\\S+\\s+\\S*)\\s+(\\S.*)";
	static constexpr const char* DosPattern = "^(\\d+-\\d+-\\d+)\\s+(\\d+:\\S*[AP|M])\\s+(<DIR>|\\d+)\\s+(\\S.*)";

	RegExp regexp(NixPattern);

	if(regexp.GlobalMatch(dir_entry)) {
		auto list_item = regexp.GetStrings();
		if(list_item.GetCount() == 7) {

			// UNIX:
			// -rw-r--r-- 1 owner group 12738 Dec  1 2013  FTP.cpp
			// drw-r--r-- 2 owner group  4096 Apr 26 12:08 src.tpp

			entry(STYLE) = RawToValue(Style::UNIX);

			const auto& attr  = list_item[0];

			// Set file system object type.
			entry(TYPE) = String(attr[0], 1);

			// Set permissions.
			owner.Set(0, attr[1] == 'r');
			owner.Set(1, attr[2] == 'w');
			owner.Set(2, attr[3] == 'x');
			group.Set(0, attr[4] == 'r');
			group.Set(1, attr[5] == 'w');
			group.Set(2, attr[6] == 'x');
			other.Set(0, attr[7] == 'r');
			other.Set(1, attr[8] == 'w');
			other.Set(2, attr[9] == 'x');

			// Set owner, group, size, name
			entry(OWNER) = list_item[2];
			entry(GROUP) = list_item[3];
			entry(SIZE)  = ScanInt64(list_item[4]);

			// Check if the last modification time is <= 6 months.
			// In that case, ls will give us the hours, not the year.
			// U++ date/time scanning routines will automatically add the current year.
			// Format can be either "(m)onth (d)ay (y)ear", or "(m)onth (d)day [hour:minutes]"
			String datetime = list_item[5];
			SetDateScan(datetime.Find(':') != -1 ? "md" : "mdy");
			Time t;
			if(StrToTime(t, datetime) != NULL && t.IsValid())
				entry(TIME) = t;

			String link, path;
			if(IsSymLink() && SplitTo(list_item[6], "->", link, path)) {
				entry(FILE) = pick(TrimBoth(link));
				entry(PATH) = pick(TrimBoth(path));
			}
			else
				entry(FILE) = pick(list_item[6]);
			entry(ENTRY)    = pick(dir_entry);
			return true;
		}
	}

	regexp.Clear();
	regexp.SetPattern(DosPattern);

	if(regexp.GlobalMatch(dir_entry)) {
		auto list_item = regexp.GetStrings();
		if(list_item.GetCount() == 4) {

			// DOS:
			// 12-1-13		13:48AM		12738	FTP.cpp
			// 04-26-14		12:08PM		<DIR>	src.tpp

			entry(STYLE) = RawToValue(Style::DOS);

			// Last modification date and time.
			String datetime = list_item[0] + " " + list_item[1];
			SetDateScan("mdy");
			Time t;
			if(StrToTime(t, datetime) != NULL && t.IsValid())
				entry(TIME) = t;

			// Entry type or file size.
			if(ToUpper(list_item[2]) == "<DIR>") {
				entry(TYPE) = "d";
			}
			else {
				entry(SIZE) = ScanInt64(list_item[2]);
				if(GetFileExt(ToLower(list_item[3])) == ".lnk")
					entry(TYPE) = "l";
				else
					entry(TYPE) = "-";
			}
			entry(FILE)  = pick(list_item[3]);
			entry(ENTRY) = pick(dir_entry);
			return true;
		}
	}
	return false;
}
}
