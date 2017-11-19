#include "SSH.h"

namespace Upp {

namespace SSH {
bool sTrace = false;
bool sTraceVerbose = false;

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
		default:
			return "";
	}
	return pick(Format("SSH: %s, oid: %d: ", s, id));
}
}

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

void Ssh::Check()
{
	if(IsTimeout())
		SetError(-1000, "Operation timed out.");
	if(ssh->status == CANCELLED)
		SetError(-1001, "Operation cancelled.");
	ssh->event_proxy();
}

// TODO: Merge Cmd & ComplexCmd.
bool Ssh::Cmd(int code, Function<bool()>&& fn)
{
	if(ssh->status != CLEANUP)
		ssh->status = WORKING;
	if(!IsComplexCmd())
		ssh->queue.Clear();
	ssh->queue.AddTail() = MakeTuple<int, Gate<>>(code, pick(fn));
	if(!ssh->async && !IsComplexCmd())
		while(_Do());
	return !IsError();
}

bool Ssh::ComplexCmd(int code, Function<void()>&& fn)
{
	bool b = IsComplexCmd(); // Is this complex command a part of another complex command?
	ssh->ccmd = code;
	if(ssh->status != CLEANUP)
		ssh->status = WORKING;
	if(!b)
		ssh->queue.Clear();
	fn();
	if(!b && !ssh->async)
		while(_Do());
	return !IsError();
}

bool Ssh::_Do()
{
try {
		if(ssh->start_time == 0)
			ssh->start_time = msecs();
INTERLOCKED {
		if(!ssh->init) {
			ssh->init = Init();
		}
		else
		if(!ssh->queue.IsEmpty()) {
			auto cmd = ssh->queue.Head().Get<Gate<>>();
			if(cmd())
				ssh->queue.DropHead();
		}
}
		if(ssh->queue.IsEmpty()) {
			switch(ssh->status) {
				case CLEANUP:
					ssh->status = FAILED;
					break;
				case WORKING:
					ssh->status = FINISHED;
					break;
				case FAILED:
					break;
			}
			ssh->ccmd = -1;
			ssh->start_time =  0;
		}
		else Check();
	}
	catch(Error& e) {
		Cleanup(e);
	}
	return ssh->status == WORKING || ssh->status == CLEANUP;
}

bool Ssh::Do()
{
	ASSERT(ssh->async);
	return _Do();
}

dword Ssh::GetWaitEvents() const
{
	dword event = 0;
	if(ssh->socket  && ssh->session) {
		auto e = libssh2_session_block_directions(ssh->session);
		if(e & LIBSSH2_SESSION_BLOCK_INBOUND)
			event |= WAIT_READ;
		if(e & LIBSSH2_SESSION_BLOCK_OUTBOUND)
			event |= WAIT_WRITE;
	}
	return event;
}

bool Ssh::Cleanup(Error& e)
{
	ssh->queue.Clear();
	ssh->start_time = 0;
	auto b = ssh->status == CLEANUP;
	ssh->status = FAILED;
	if(b) return false;
	ssh->error  = MakeTuple<int, String>(e.code, e);
	LLOG("Failed." << " Code = " << e.code << ", " << e);
	return !b && e.code != -1000; // Make sure we don't loop on timeout errors.
}

void Ssh::SetError(int rc, const String& reason)
{
	if(IsNull(reason) && ssh && ssh->session) {
		Buffer<char*> libmsg(256);
		int rc = libssh2_session_last_error(ssh->session, libmsg, NULL, 0);
		throw Error(rc, *libmsg);
	}
	else
		throw Error(rc, reason);
}

int64 Ssh::GetNewId()
{
	static int64 objectid;
	if(objectid == INT64_MAX)
		objectid = 1;
	else
		++objectid;
	return objectid;
}

Ssh::Ssh()
{
	ssh.Create();
	ssh->session		= NULL;
	ssh->socket			= NULL;
	ssh->async			= false;
	ssh->init			= false;
	ssh->timeout		= 0;
	ssh->start_time		= 0;
	ssh->chunk_size		= 65536;
	ssh->packet_length  = 0;
	ssh->status			= FINISHED;
	ssh->ccmd			= -1;
	ssh->oid			= GetNewId();
	ssh->otype          = 0;
}

Ssh::~Ssh()
{
}

INITIALIZER(SSH) {
	LOG("Initializing libssh2...");
	libssh2_init(0);
}
EXITBLOCK {
	LOG("Deinitializing libssh2...");
	libssh2_exit();
}
}