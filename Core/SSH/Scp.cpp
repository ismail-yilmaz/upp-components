#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

bool Scp::Open(int opcode, const String& path, int64 size, long mode)
{
	return Cmd(CHOPEN, [=] {
		if(path.IsEmpty())
			SetError(-1, "Path is not set.");
		switch(opcode) {
			case FGET:
				*channel = libssh2_scp_recv2(ssh->session, path, &filestat);
				break;
			case FPUT:
				*channel = libssh2_scp_send64(ssh->session, path, mode, size, 0, 0);
				break;
			default:
				NEVER();
		}
		if(!*channel && !WouldBlock())
			SetError(-1);
		if(*channel) {
			LLOG("Scp channel obtained.");
			Unlock();
		}
		return *channel != NULL;
	});
}

bool Scp::Get(Stream& out, const String& path, Gate<int64, int64> progress)
{
	Clear();
	WhenProgress = pick(progress);
	return ComplexCmd(FGET, [=, &out]() mutable {
		Open(FGET, path, 0, 0);
		Cmd(FGET, [=, &out]{ return SshChannel::ReadStream(out, filestat.st_size); });
		SshChannel::SendRecvEof();
		SshChannel::Close();
		SshChannel::CloseWait();
	});
}

String Scp::Get(const String& path, Gate<int64, int64> progress)
{
	Clear();
	WhenProgress = pick(progress);
	ComplexCmd(FGET, [=]() mutable {
		Open(FGET, path, 0, 0);
		Cmd(FGET, [=]{ return SshChannel::ReadString((String&) result, filestat.st_size); });
		SshChannel::SendRecvEof();
		SshChannel::Close();
		SshChannel::CloseWait();
	});
	return !IsBlocking() ? Null : GetResult();
}

bool Scp::Put(Stream& in, const String& path, long mode, Gate<int64, int64> progress)
{
	Clear();
	WhenProgress = pick(progress);
	return ComplexCmd(FGET, [=, &in]() mutable {
		Open(FPUT, path, in.GetSize(), mode);
		SshChannel::Put(in, in.GetSize());
		SshChannel::SendRecvEof();
		SshChannel::Close();
		SshChannel::CloseWait();
	});
}

bool Scp::Put(const String& in, const String& path, long mode, Gate<int64, int64> progress)
{
	Clear();
	WhenProgress = pick(progress);
	return ComplexCmd(FGET, [=, &in]() mutable {
		Open(FPUT, path, in.GetLength(), mode);
		SshChannel::Put(in);
		SshChannel::SendRecvEof();
		SshChannel::Close();
		SshChannel::CloseWait();
	});
}
}