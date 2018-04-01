#include "Ftp.h"

#ifdef PLATFORM_WIN32
#pragma comment(lib, "ws2_32.lib")
#endif

namespace Upp {

static Mutex ftp_mutex;
static Atomic ftp_workers;
static Vector<int> ftp_aborted;

static bool trace_ftp;
#define LLOG(x)	do{ if(trace_ftp) RLOG(x); }while(false)

static inline bool IsMultithreaded()
{
#ifdef flagMT
	return true;
#else
	return false;
#endif
}

static String Slice(const String& s, int delim1, int delim2)
{
	int b = -1, e = -1;
	if((b = s.Find(delim1, 0)) == -1 || (e = s.Find(delim2, ++b)) == -1)
		return String::GetVoid();
	return s.Mid(b, e - b);
}

void Ftp::Trace(bool b)
{
	trace_ftp = b;
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

String Ftp::EncodePath(const String& path)
{
	String r;
	for(int i = 0; i < path.GetLength(); i++)
		r.Cat((path[i] == '\n') ? '\0' : path[i]);
	return r;
}

int Ftp::GetRandomPort(int min, int max)
{
	int range = max - min + 1;
	return (int) Random() % range + min;
}

bool Ftp::GetSockAddr(int type, int& family, String& ip, int& port)
{
	struct sockaddr_storage ss;
	struct sockaddr_in *in = (sockaddr_in*) &ss;
	struct sockaddr_in6 *in6 = (sockaddr_in6*) &ss;
	socklen_t ss_size = sizeof(sockaddr_storage);
	memset(&ss, 0, ss_size);
	if(type == LOCAL) {
		if(getsockname(control_socket.GetSOCKET(), (sockaddr*) &ss, &ss_size) != 0)
			return false;
	}
	else
	if(type == PEER) {
		if(getpeername(control_socket.GetSOCKET(), (sockaddr*) &ss, &ss_size) != 0)
			return false;
	}
	family = ss.ss_family;
#ifdef PLATFORM_WIN32
	ip = inet_ntoa(in->sin_addr);
#else
	Buffer<char> dummy(64, 0), ip_buffer(16, 0);
	if(family == AF_INET6)
		memcpy(ip_buffer, &in6->sin6_addr, sizeof(in6_addr));
	else
		memcpy(ip_buffer, &in->sin_addr, sizeof(in_addr));
	ip = inet_ntop(family, ip_buffer, dummy, 64);
#endif
	if(ip.IsEmpty())
		return false;
	port = GetRandomPort(49152, 65535);
	return true;
}

bool Ftp::SetError(const String& e)
{
	reply = e;
	reply_code = -1;
	if(control_socket.IsError()) {
		String socket_error = control_socket.GetErrorDesc();
		if(!socket_error.IsEmpty())
			reply << " [" << socket_error << "]";
	}
	LLOG("-- " << reply);
	return progress = false;
}

int Ftp::GetReplyCode(const String& s)
{
	if(s.IsVoid() || s.GetLength() < 3 || !IsDigit(s[0]) || !IsDigit(s[1]) || !IsDigit(s[2]))
		return -1;
	return StrInt(s.Mid(0, 3));
}

String Ftp::GetReplyAsXml()
{
	StringStream ss(reply);
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

bool Ftp::PutGet(const String& s, bool nolog)
{
	try {
		if(progress && !aborted)
			throw Exc("A request is already in progress."); ;
		progress = true;
		// 02-024-2017:
		// We need to check the control connection.
		if(!control_socket.IsOpen())
			throw Exc("Socket is not open.");
		// Put request.
		if(!s.IsEmpty()) {
			if(!nolog)
				LLOG(">> " << s);
			if(!control_socket.PutAll(s + "\r\n")) {
				throw Exc("Write failed.");
			}
		}
		// Get response
		reply = control_socket.GetLine();
		reply_code = GetReplyCode(reply);
		if(reply_code == -1) {
			throw Exc("Read failed.");
		}
		LLOG("<< " << reply);
		reply.Cat("\r\n");
		if(reply[3] && reply[3] == '-') {
			for(;;) {
				String line = control_socket.GetLine();
				if(line.IsVoid()) {
					throw Exc("Read failed.");
				}
				int end_code = GetReplyCode(line);
				LLOG("<< " << line);
				reply.Cat(line);
				reply.Cat("\r\n");
				if(reply_code == end_code && line[3] && line[3] == ' ')
					break;
			}
		}
		progress = false;
	}
	catch(Exc e) {
		return SetError("Control Socket: " << e);
	}
	return true;
}

bool Ftp::SetDataType(int type)
{
	PutGet(type == BINARY ? "TYPE I" : "TYPE A");
	return ReplyIsSuccess();
}

bool Ftp::ResumeTransfer(int cmd, Stream& s)
{
	bool rc = position > 0 && position <= s.GetSize();
	if(rc) {
		s.Seek(position);
		if(cmd == GET || cmd == PUT) {
			PutGet("REST " + FormatInt64(position));
			if(!ReplyIsPending()) {
				position = 0;
				rc = false;
			}
		}
	}
	return rc;
}

bool Ftp::SetTransferMode(String& addr, int& port, int& family)
{
	bool result = false;
	switch(transfer_mode) {
		case ACTIVE: {
			if(!GetSockAddr(LOCAL, family, addr, port))
				break;
			// Try extendend command first.
			String xcmd;
			xcmd << (family == AF_INET6 ? "|2|" : "|1|") << addr << "|" << port << "|";
			PutGet("EPRT " + xcmd);
			if(ReplyIsSuccess()) {
				result = true;
				break;
			}
			else
			if(family == AF_INET6)
				break;
			addr.Replace(".", ",");
			addr += "," + AsString((port & 0xff00) >> 8) + "," + AsString(port & 0xff);
			PutGet("PORT " + addr);
			result = ReplyIsSuccess();
			break;
		}
		case PASSIVE: {
			if(!GetSockAddr(PEER, family, addr, port))
				break;
			// Try extendend command first.
			PutGet("EPSV");
			if(ReplyIsSuccess()) {
				addr = control_socket.GetPeerAddr();
				int begin = reply.FindAfter("(|||");
				int end = reply.ReverseFind("|)");
				if(begin == -1 || end == -1 || addr.IsEmpty())
					break;
				port = StrInt(reply.Mid(begin, end - begin));
				result = true;
				break;
			}
			else
			if(family == AF_INET6)
				break;
			PutGet("PASV");
			if(!ReplyIsSuccess())
				break;
			String h1, h2, h3, h4, p1, p2;
			if((!(addr = Slice(reply, '(', ')')).IsEmpty()  ||
				!(addr = Slice(reply, '=', ' ')).IsEmpty()) &&
				SplitTo(addr, ',', h1, h2, h3, h4, p1, p2)) {
				addr = h1 + "." + h2 + "." + h3 + "." + h4;
				port = StrInt(p1) * 256 + StrInt(p2);
				result = true;
			}
			break;
		}
		default:
			NEVER();
	}
	return result;
}

bool Ftp::TransferData(int cmd, const String& request, Stream& file, int64 size, int type)
{
	if(ftps && transfer_mode == ACTIVE)
		return SetError("FTPS mode is not supported with active ftp connection.");
	String addr;
	int port, family;
	bool log = request.StartsWith("LIST");
	bool exc = request.StartsWith("APPE");
	if(!SetTransferMode(addr, port, family))
		return false;
	TcpSocket data_socket, listener;
	data_socket.Timeout(control_socket.GetTimeout());
	data_socket.WaitStep(control_socket.GetWaitStep());
	data_socket.WhenWait = [=] { WhenWait(); };
	target = &data_socket;
//	if(exc)
		ResumeTransfer(cmd, file);
	switch(transfer_mode) {
		case ACTIVE: {
			if(!listener.Listen(port, 5, family == AF_INET6))
				break;
			PutGet(request);
			if(!ReplyIsWait())
				break;
			for(;;)
				if(data_socket.Accept(listener))
					break;
			cmd == GET
				? GetData(data_socket, file, size, type, log)
				: PutData(data_socket, file, type);
			break;
		}
		case PASSIVE: {
			if(!data_socket.Connect(addr, port))
				break;
			if(ftps) {
				if(!data_socket.StartSSL() || !data_socket.IsSSL()) {
					LLOG("-- Negotiation error. Couldn't put data socket into FTPS mode.");
					break;
				}
			}
			PutGet(request);
			if(!ReplyIsWait())
				break;
			cmd == GET
				? GetData(data_socket, file, size, type, log)
				: PutData(data_socket, file, type);
			break;
		}
		default:
			NEVER();
	}
	target = NULL;
	if(!aborted) {
		if(data_socket.IsOpen())
			data_socket.Close();
		// 02-04-2017:
		// Check the reply here.
		// We don't want to wait for a messege we'll never recieve.
		if(ReplyIsWait())
			PutGet(Null);
		return ReplyIsSuccess();
	}
	else
		return aborted = false;
}

bool Ftp::GetData(TcpSocket& socket, Stream& out, int64 sz, int type, bool log)
{
	progress = true;
	while(!socket.IsEof()) {
		String chunk = type == ASCII ? socket.GetLine() : socket.Get(chunk_size);
		if(chunk.IsVoid())
			continue;
		if(type == ASCII) {
			if(log)
				LLOG("|| " << chunk);
#ifdef PLATFORM_WIN32
			chunk.Cat('\r');
#endif
			chunk.Cat('\n');
		}
		out.Put(chunk);
		// 09-04-2017:
		// Binary data transfers should not call WhenList().
		if(type == ASCII ? WhenList(chunk) : WhenData(sz, out.GetSize())) {
			Abort();
			break;
		}
	}
	progress = false;
	auto b = !control_socket.IsError() && !aborted;
	if(!b) position = out.GetSize();
	return b;
}

bool Ftp::PutData(TcpSocket& socket, Stream& in, int type)
{
	// Here, it is servers' responsibility to process ascii EOL.
	progress = true;
	int64 done  = in.GetPos();
	int64 total = in.GetSize();
	while(!socket.IsEof() && !in.IsEof()) {
		String buf = in.Get(chunk_size);
		done += socket.Put(buf, buf.GetLength());
		if(WhenData(total, done)) {
			Abort();
			break;
		}
	}
	progress = false;
	auto b = !control_socket.IsError() && !aborted;
	if(!b) position = done;
	return b;
}

bool Ftp::InitFtps(int size)
{
	LLOG("** Starting FTPS negotiation...");
	PutGet("AUTH TLS");
	if(ReplyIsError()) {
		if(reply_code == 504 || reply_code == 534) {
			// Try SSL if TLS is rejected.
			LLOG("!! TLS request rejected. Trying SSL3 instead...");
			PutGet("AUTH SSL");
		}
	}
	if(ReplyIsSuccess()) {
		if(control_socket.StartSSL() &&	control_socket.IsSSL()) {
			PutGet("PBSZ " + AsString(size));
			if(ReplyIsSuccess()) {
				PutGet("PROT P");
				if(ReplyIsSuccess()) {
					// Start login sequence.
					reply_code = 220;
					LLOG("++ FTPS negotiation is successful. Client is in secure mode.");
					return true;
				}
			}
		}
	}
	return SetError("FTPS negotiation failed.");
}

bool Ftp::Connect(const String& host, int port)
{
	bool online = false;
	try {
		if(host.IsEmpty())
			throw Exc("Hostname is not specified.");
		ftp_host = host;
		ftp_port = port;
		if(user_id.IsEmpty() || user_password.IsEmpty()) {
			static const char *default_user = "anonymous";
			static const char *default_pass = "anonymous@";
			user_id = default_user;
			user_password = default_pass;
		}
		control_socket.Clear();
		LLOG("** Connecting to " << host << ":" << port);
		if((WhenProxy && !WhenProxy()) ||
		  (!control_socket.Connect(ftp_host, Nvl(ftp_port, 21))))
			throw Exc("Couldn't connect to " << ftp_host << ":" << ftp_port);
		// Get server greeting.
		PutGet(Null);
		if(!ReplyIsSuccess())
			return false;
		// TLS/SSL connection.
		if(ftps && !InitFtps(0))
			return false;
		// Do Login.
		String replies = reply;
		while(!online) {
			switch(reply_code) {
				case 220:
					PutGet("USER " + user_id);
					break;
				case 331:
					LLOG(">> PASS ********");
					PutGet("PASS " + user_password, true);
					break;
				case 332:
					PutGet("ACCT noaccount");
					break;
				case 202:
				case 230:
					online = true;
					continue;
				default:
					throw Exc(reply);
			}
			replies << reply;
		}
		reply = replies;
	}
	catch(Exc e) {
		SetError(e);
	}
	return online;
}

void Ftp::Disconnect()
{
	if(control_socket.IsOpen()) {
		PutGet("QUIT");
		control_socket.Clear();
		LLOG("++ Disconnected from: " << ftp_host << ":" << ftp_port);
	}
}

String Ftp::GetDir()
{
	PutGet("PWD");
	return ReplyIsSuccess() ? DecodePath(reply) : String::GetVoid();
}

bool Ftp::SetDir(const String& path)
{
	PutGet("CWD " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::DirUp()
{
	PutGet("CDUP");
	return ReplyIsSuccess();
}

bool Ftp::ListDir(const String& path, DirList& list, Gate1<String> progress)
{
	if(!SetDataType(ASCII))
		return false;
	WhenList = progress;
	StringStream ls;
	String request = "LIST" + (!path.IsEmpty() ? (" " + path) : "");
	if(!TransferData(GET, request, ls, 0, ASCII))
		return false;
	if(ParseFtpDirEntry(ls.GetResult(), list))
		for(auto& e : list)	e.User(user_id);
	return true;
}

bool Ftp::MakeDir(const String& path)
{
	PutGet("MKD " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::RemoveDir(const String& path)
{
	PutGet("RMD " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::Get(const String& path, Stream& out, Gate2<int64, int64> progress, bool ascii)
{
	if(path.IsEmpty())
		return SetError("Nothing to download. Filename is not specified.");
	WhenData = progress;
	auto type = ascii ? Ftp::ASCII : Ftp::BINARY;
	if(!SetDataType(type))
		return false;
	auto size = GetSize(path);
	if(size < 0)
		return false;
	String request = "RETR " + EncodePath(path);
	return TransferData(GET, request, out, size, type);
}

bool Ftp::Put(Stream& in, const String& path, Gate2<int64, int64> progress, bool ascii)
{
	if(path.IsEmpty())
		return SetError("Nothing to upload. Filename is not specified.");
	WhenData = progress;
	auto type = ascii ? Ftp::ASCII : Ftp::BINARY;
	if(!SetDataType(type))
		return false;
	String request = "STOR " + EncodePath(path);
	return TransferData(PUT, request, in, 0, type);
}

bool Ftp::Append(Stream& in, const String& path, Gate2<int64, int64> progress, bool ascii)
{
	if(path.IsEmpty())
		return SetError("Nothing to append to. Filename is not specified.");
	WhenData = progress;
	auto type = ascii ? Ftp::ASCII : Ftp::BINARY;
	if(!SetDataType(type))
		return false;
	String request = "APPE " + EncodePath(path);
	return TransferData(APPEND, request, in, 0, type);
}

bool Ftp::Info(const String& path, DirEntry& info)
{
	DirList list;
	if(!ListDir(path, list) || list.GetCount() != 1)
		return false;
	info = pick(list[0]);
	info.User(user_id);
	return true;
}

bool Ftp::Rename(const String& oldname, const String& newname)
{
	if(oldname.IsEmpty() || newname.IsEmpty())
		return SetError("Couldn't change file name. Old name or new name is not specified.");
	PutGet("RNFR " + EncodePath(oldname));
	if(!ReplyIsPending())
		return false;
	PutGet("RNTO " + EncodePath(newname));
	return ReplyIsSuccess();
}

bool Ftp::Delete(const String& path)
{
	if(path.IsEmpty())
		return SetError("Nothing to delete. File name is not specified.");
	PutGet("DELE " + EncodePath(path));
	return ReplyIsSuccess();
}

bool Ftp::Noop()
{
	PutGet("NOOP");
	return ReplyIsSuccess();
}

void Ftp::Abort()
{
	if(!InProgress() || !target || !target->IsOpen()) {
		return;
	}
	// RFC 959: p. 34-35:
	// Send TELNET IAC-IP ('\377' - '\364') & IAC-DM ('\377' - '\362') succesively.
	// DM, being a data-marker, should be sent as a single byte, "urgent" data.
	// -------------------------------------------------------------
	// Note that this sequence is not working on every server,
	// due to the problematic server-side (non-)implementation of ABOR command
	// and the out-of-band data tranfer concept. Thus, we simply close the data
	// connection, and accept error replies (>= 500) as success too.
	target->Abort();
#ifdef PLATFORM_POSIX
	control_socket.Put("\377\364\377");
	send(control_socket.GetSOCKET(), "\362", 1, MSG_OOB);
#endif
	control_socket.Put("ABOR\r\n");
	LLOG(">> ABOR");
	target->Close();
	aborted = true;
	PutGet(Null);
	if(ReplyIsFailure() || ReplyIsError()) {
		if(reply_code != -1)
			PutGet(Null);
	}
}

ValueMap Ftp::GetFeatures()
{
	ValueMap features;
	PutGet("FEAT");
	if(ReplyIsSuccess()) {
		StringStream ss(reply);
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
	}
	return pick(features);
}

int64 Ftp::GetSize(const String& path)
{
	PutGet("SIZE " + EncodePath(path));
	return ReplyIsSuccess()
		? ScanInt64(TrimBoth(reply.Mid(3)))
		: -1;
}

int Ftp::GetWorkerCount()
{
	return ftp_workers;
}

void Ftp::AbortWorker(int id)
{
	Mutex::Lock __(ftp_mutex);
	ftp_aborted.Add(id);
}

bool Ftp::IsWorkerAborted(int id)
{
	Mutex::Lock __(ftp_mutex);
	for(auto tid : ftp_aborted)
		if(tid == id) {
			tid = 0;
			return true;
		}
	return false;
}

Ftp::Ftp()
{
	control_socket.WhenWait = [=] { WhenWait(); };
	control_socket.Timeout(60000);
	control_socket.WaitStep(10);
	Passive();
	position = 0;
	reply_code = 0;
	chunk_size = 65536;
	progress = false;
	aborted = false;
	ftps = false;
	target = NULL;
}

Ftp::~Ftp()
{
	Disconnect();
}

void Ftp::DirEntry::Clone(const DirEntry& src)
{
	entry = clone(src.entry);
	owner = clone(src.owner);
	group = clone(src.group);
	other = clone(src.other);
}

bool Ftp::DirEntry::Parse(const String& dir_entry)
{
	bool rc = false;
	Vector<String> list_item = Split(NormalizeSpaces(dir_entry), ' ');
	if(list_item.GetCount() >= 9) {
		// UNIX:
		// -rw-r--r-- 1 owner group 12738 Dec  1 2013  FTP.cpp
		// drw-r--r-- 2 owner group  4096 Apr 26 12:08 src.tpp
		String attr = list_item[0];
		// Set style
		entry(STYLE) = RawToValue(Style::UNIX);
		// Set type
		if(attr[0] == '-')
			entry(TYPE) = "file";
		else
		if(attr[0] == 'd')
			entry(TYPE) = "dir";
		else
		if(attr[0] == 'l')
			entry(TYPE) = "link";
		else
			return false;
		// Set permissions.
		owner.set(0, attr[1] == 'r');
		owner.set(1, attr[2] == 'w');
		owner.set(2, attr[3] == 'x');
		group.set(0, attr[4] == 'r');
		group.set(1, attr[5] == 'w');
		group.set(2, attr[6] == 'x');
		other.set(0, attr[7] == 'r');
		other.set(1, attr[8] == 'w');
		other.set(2, attr[9] == 'x');
		// Set owner, group, size, name
		entry(OWNER) = list_item[2];
		entry(GROUP) = list_item[3];
		entry(SIZE)  = ScanInt64(list_item[4]);
		// Check if the last modification time is <= 6 months.
		// In that case, ls will give us the hours, not the year.
		// U++ date/time scanning routines will automatically add the current year.
		// Format can be either "(m)onth (d)ay (y)ear", or "(m)onth (d)day [hour:minutes]"
		String datetime = list_item[5] + " " + list_item[6] + " " + list_item[7];
		SetDateScan(list_item[7].Find(':') != -1 ? "md" : "mdy");
		Time t;
		if(StrToTime(t, datetime) != NULL && t.IsValid())
			entry(TIME) = t;
		// Resolve link, if exists.
		// 15-04-2017: Check if the file/folder path contains any SP
		String s;
		int start = list_item.GetCount() == 10 && IsSymLink() ? 10 : 8;
		s = list_item[start];
		for(auto i = ++start; i < list_item.GetCount(); i++)
			if(i != list_item.GetCount()) s << " " << list_item[i];
		entry(FILE)  = s;
		entry(ENTRY) = dir_entry;
		rc = true;
	}
	else
	if(list_item.GetCount() == 4) {
		// DOS:
		// 12-1-13		13:48AM 	12738	FTP.cpp
		// 04-26-14		12:08PM		<DIR>	src.tpp

		// Style
		entry(STYLE) = RawToValue(Style::DOS);
		// Last modification date and time.
		String datetime = list_item[0] + " " + list_item[1];
		SetDateScan("mdy");
		Time t;
		if(StrToTime(t, datetime) != NULL && t.IsValid())
			entry(TIME) = t;
		else
			return false;
		// Entry type or file size.
		if(ToUpper(list_item[2]) == "<DIR>") {
			entry(TYPE) = "dir";
		}
		else {
			entry(SIZE) = ScanInt64(list_item[2]);
			if(GetFileExt(ToLower(list_item[3])) == ".lnk")
				entry(TYPE) = "link";
			else
				entry(TYPE) = "file";
		}
		// FIXME.
		owner.set(0, true);
		owner.set(1, true);
		owner.set(2, true);
		group.set(0, true);
		group.set(1, true);
		group.set(2, true);
		other.set(0, true);
		other.set(1, true);
		other.set(2, true);
		String s;
		// 15-04-2017: Check if the file/folder path contains any SP
		s = list_item[3];
		for(int i = 4; i < list_item.GetCount(); i++)
			if(i != list_item.GetCount()) s << " " << list_item[i];
		entry(FILE)  = s;
		entry(ENTRY) = dir_entry;
		rc = true;
	}
	return rc;
}

bool ParseFtpDirEntry(const String& in, Ftp::DirList& out)
{
	out.Clear();
	StringStream l(in);
	while(!l.IsEof()) {
		Ftp::DirEntry e;
		if(e.Parse(l.GetLine()))
			out.Add(pick(e));
	}
	return !out.IsEmpty();
}

Ftp::Result FtpGet(const Ftp::Request& request, Gate<int64, int64> progress, Event<> whenwait)
{
	return FtpIO(pick(request), progress, whenwait);
}

Ftp::Result FtpPut(const Ftp::Request& request, Gate<int64, int64> progress, Event<> whenwait)
{
	return FtpIO(pick(request), progress, whenwait, true);
}

Ftp::Result FtpIO(const Ftp::Request& request, Gate<int64, int64> progress, Event<> whenwait, bool put)
{
 	Ftp worker;
 	Ftp::Result result;

 	try {
 		bool rc = false;
		const String& local_file  = request["local_file"];
		const String& remote_file = request["remote_file"];
		if(local_file.IsEmpty() || remote_file.IsEmpty())
			throw Exc("Local or remote file[path] is not specified.");

		bool  file_exists = FileExists(local_file);
		if(put && !file_exists)
			throw Exc("Local file \"" << local_file << "\" does not exist.");

		bool  file_delete   = request["del"];
		int64 file_pos      = request["rest"];
		bool is_restarting  = file_pos > 0 && file_exists;

		FileStream fs(local_file, put
			? FileStream::READ
			: (is_restarting
				? FileStream::APPEND
				: FileStream::CREATE) | FileStream::NOWRITESHARE
			);
		if(!fs.IsOpen())
			throw  Exc("Unable to open local file \"" + local_file + "\"");
		worker.WhenWait = whenwait;
 		worker.Active(request["active"])
		      .SSL(request["ssl"])
 			  .User(request["user"], request["password"])
 			  .Restart(file_pos);
		rc = worker.Connect(request["host"], request["port"]);
		if(rc) {
			Gate<int64, int64> pe = [&](int64 total, int64 done) {
				result.total = total;
				result.done  = done;
				return progress(total, done);
			};
			rc = put
			? is_restarting
				? worker.Append(fs, remote_file, pe, request["ascii"])
				: worker.Put(fs, remote_file, pe, request["ascii"])
			: worker.Get(remote_file, fs, pe, request["ascii"]);
		}
		fs.Close();
		if(!rc) {
			if(!put && file_delete)
				// Leave no trace!
				FileDelete(local_file);
			throw Exc(worker.GetReply());
		}
		result.status = Ftp::Result::Status::SUCCESS;
	}
 	catch(Exc& e) {
 		result.rc  = worker.GetCode();
 		result.msg = e;
 		result.status = Ftp::Result::Status::FAILURE;
 		LLOG("-- File transfer failed. Reason: "  <<  e);
 	}
 	return pick(result);
}

Ftp::Result FtpAsyncGet(Ftp::Request& request, Event<Ftp::Result> progress)
{
	return FtpAsyncIO(pick(request), progress);
}

Ftp::Result FtpAsyncPut(Ftp::Request& request, Event<Ftp::Result> progress)
{
	return FtpAsyncIO(pick(request), progress, true);
}

Ftp::Result FtpAsyncIO(Ftp::Request request, Event<Ftp::Result> progress, bool put)
{
	static Atomic ftp_worker_id;
	ONCELOCK {  ftp_worker_id = 0; ftp_workers = 0; }
	Ftp::Result r;
	try {
		// Check if U++ MT is enabled.
		if(!IsMultithreaded())
			throw Exc("Multithreading is not enabled.");

		// Limit available slots to max. 256.
		if(Ftp::GetWorkerCount() > 255)
			throw Exc("Worker slots are full. (Max. 256 workers are allowed.)");

		// Set thread id.
		ftp_worker_id = ftp_worker_id < INT_MAX
				? ++ftp_worker_id
				: 1;
		int id = ftp_worker_id;

		// Check local and remote file paths.
		String local_file  = request["local_file"];
		String remote_file = request["remote_file"];
		if(local_file.IsEmpty() || remote_file.IsEmpty())
			throw Exc("Local or remote path is not specified.");

		// Check if the local file exists.
		bool  file_exists = FileExists(local_file);
		if(put && !file_exists)
			throw Exc("Local file \"" << local_file << "\" does not exist.");

		bool  file_delete  = request["del"];
		int64 file_pos     = request["rest"];
		bool is_restarting = file_pos > 0 && file_exists;

		// Get priority info.
		int thread_priority = request["priority"];

		// Actual worker thread code:
		Event<> worker_thread = [=, request = pick(request), progress = pick(progress)] {
			AtomicInc(ftp_workers);
			Ftp worker;
			Ftp::Result result;
			result.id     = id;
			result.file   = pick(remote_file);
			result.data   = pick(request["data"]);
			result.status = Ftp::Result::Status::PROGRESS;
			LLOG("++ Worker thread (id: " << id << ") succesfully started.");
			try {
				bool rc = false;
				FileStream fs(local_file, put
					? FileStream::READ
					: (is_restarting
						? FileStream::APPEND
						: FileStream::CREATE) | FileStream::NOWRITESHARE
					);
				if(!fs.IsOpen())
					throw  Exc("Unable to open local file \"" + local_file + "\"");
				// 08-04-2017:
				// We'll use a 5 MB (max) buffer to reduce disk I/O.
				const dword buffer_size = 1024 *1024 * 5;
				fs.SetBufferSize(buffer_size);
				LLOG("++ Worker thread (id: " << id << "): Local file \"" << local_file << " successfully opened.");
				worker.Active(request["active"])
				    .SSL(request["ssl"])
					.WaitStep(1)
					.Timeout(request["timeout"])
					.Restart(file_pos)
					.User(request["user"], request["password"]);
				rc = worker.Connect(request["host"], request["port"]);
				if(rc) {
					worker.WhenWait << [&] {
						// Let's re-use Ftp::WhenWait.
						if(Ftp::IsWorkerAborted(id) || Thread::IsShutdownThreads()) {
							worker.InProgress()
								? worker.Abort()
								: worker.Disconnect();
						}
						else
							Sleep(10);
					};
					Gate<int64, int64> pe = [&](int64 total, int64 done) {
						// Translate Gate to Event.
						result.total = total;
						result.done  = done;
						result.status = Ftp::Result::Status::PROGRESS;
						progress(result);
						return false;
					};
					rc = put
						? is_restarting // Ftp append command is more reliable.
							? worker.Append(fs, remote_file, pe, request["ascii"])
							: worker.Put(fs, remote_file, pe, request["ascii"])
						: worker.Get(remote_file, fs, pe, request["ascii"]);
				}
				fs.Close();
				if(!rc) {
					if(!put && file_delete)
						// Leave no trace!
						FileDelete(local_file);
					throw Exc(worker.GetReply());
				}
				result.status = Ftp::Result::Status::SUCCESS;
				progress(result);
			}
			catch(Exc& e) {
				// Worker halted.
				result.rc  = worker.GetCode();
				result.msg = e;
				result.pos = result.done;
				result.status = Ftp::Result::Status::FAILURE;
				progress(result);
				LLOG("-- Worker thread (id: " << id << ") failed. Reason: "  <<  e);
			}
			AtomicDec(ftp_workers);
			if(!ftp_workers) {
				Mutex::Lock __(ftp_mutex);
				ftp_aborted.Clear();
			}
		};
		Thread t;
		if(!t.Run(worker_thread))
			throw Exc("Couldn't run thread.");

		// Set thread priority, if possible.
		if(thread_priority >= 0 && !t.Priority(thread_priority))
			LOG("!! Worker thread (id: " << id << "): Couldn't change thread prioriry to " << thread_priority);

		// A worker thread is successfully spawned.
		r.id   = id;
		r.file = remote_file;
		r.status = !is_restarting
					? Ftp::Result::Status::STARTED
					: Ftp::Result::Status::RESTARTED;
	}
	catch(Exc& e) {
		// Couldn't create or run the worker thread.
		r.rc  = -1;
		r.msg =  e;
		r.status = Ftp::Result::Status::FAILURE;
		LLOG("-- Failed to create FTP worker. Reason: " << e);
	}
	return pick(r);
}
}