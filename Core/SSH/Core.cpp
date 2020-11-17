#include "SSH.h"

namespace Upp {

namespace SSH {
bool sTrace = false;
int  sTraceVerbose = 0;

// SSH diagnostic utilities.

String GetName(int type, int64 id)
{
	String s;
	switch(type) {
		case Ssh::SESSION:
			s = "Session";
			break;
		case Ssh::SFTP:
			s = "SFtp";
			break;
		case Ssh::CHANNEL:
			s = "Channel";
			break;
		case Ssh::SCP:
			s = "Scp";
			break;
		case Ssh::EXEC:
			s = "Exec";
			break;
		case Ssh::SHELL:
			s = "Shell";
			break;
		case Ssh::TUNNEL:
			s = "Tunnel";
			break;
		default:
			return "";
	}
	return pick(Format("SSH: %s, oid: %d: ", s, id));
}
}

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

// Ssh: SSH objects core class.

static StaticMutex sLoopLock;

void Ssh::Check()
{
	auto sock = ssh->socket;

	if(IsTimeout())
		SetError(-1, "Operation timed out.");

	if(ssh->status == ABORTED)
		SetError(-1, "Operation aborted.");

	if(sock && ssh->socket->IsError())
		SetError(-1, "[Socket error]: " << ssh->socket->GetErrorDesc());
}

bool Ssh::Do(Gate<>& fn)
{
	Mutex::Lock __(sLoopLock);

	Check();
	if(!ssh->init)
		ssh->init = Init();
	return !ssh->init || !fn();
}

bool Ssh::Run(Gate<>&& fn)
{
	try {
		ssh->status = WORKING;
		ssh->start_time = msecs();

		while(Do(fn))
			Wait();

		ssh->status = IDLE;
	}
	catch(const Error& e) {
		ReportError(e.code, e);
	}
	catch(...) {
		ReportError(-1, "Unhandled exception.");
	}
	return !IsError();
}

void Ssh::Wait()
{
	RefreshUI();
	if(!ssh->socket || !ssh->session)
		return;
	dword q = 0, r = libssh2_session_block_directions(ssh->session);
	if(r & LIBSSH2_SESSION_BLOCK_INBOUND)
		q |= WAIT_READ;
	if(r & LIBSSH2_SESSION_BLOCK_OUTBOUND)
		q |= WAIT_WRITE;
	SocketWaitEvent we;
	we.Add(*ssh->socket, q);
	we.Wait(ssh->waitstep);
}

void Ssh::SetError(int rc, const String& reason)
{
	if(IsNull(reason) && ssh && ssh->session) {
		Buffer<char*> libmsg(256, 0);
		rc = libssh2_session_last_error(ssh->session, libmsg, nullptr, 0);
		throw Error(rc, *libmsg);
	}
	else
		throw Error(rc, reason);
}

void Ssh::ReportError(int rc, const String& reason)
{
	ssh->status  = FAILED;
	ssh->error.a = rc;
	ssh->error.b = reason;
	if(ssh->socket) {
		ssh->socket->ClearAbort();
		ssh->socket->ClearError();
	}
	LLOG("Failed. Code = " << rc << ", " << reason);
}

int64 Ssh::GetNewId()
{
	static int64 objectid;
	return objectid == INT64_MAX ? objectid = 1	: ++objectid;
}

Ssh::Ssh()
{
    ssh.Create();
    ssh->session        = nullptr;
    ssh->socket         = nullptr;
    ssh->init           = false;
    ssh->timeout        = Null;
    ssh->start_time     = 0;
    ssh->waitstep       = 10;
    ssh->chunk_size     = CHUNKSIZE;
    ssh->status         = IDLE;
    ssh->oid            = GetNewId();
    ssh->otype          = CORE;
}

Ssh::~Ssh()
{
}

INITIALIZER(SSH) {
	libssh2_init(0);
}
EXITBLOCK {
	libssh2_exit();
}
}