#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

bool SshChannel::Cleanup(Error& e)
{
	if(!Ssh::Cleanup(e) || !IsComplexCmd())
		return false;
	ssh->ccmd = -1;
	if(chdata->channel) {
		LLOG("Cleaning up...");
		ssh->status = CLEANUP;
		Close();
	}
	return true;
}

bool SshChannel::Init()
{
	chdata->channel = libssh2_channel_open_session(ssh->session);
	if(!chdata->channel && !WouldBlock())
		SetError(-1);
	if(chdata->channel)
		LLOG("A new channel is opened.");
	return chdata->channel != NULL;
}

void SshChannel::Exit()
{
	Cmd(CHEXIT, [=]() mutable {
		if(!chdata || !chdata->channel)
			return true;
		auto rc = libssh2_channel_free(chdata->channel);
		if(!WouldBlock(rc) && rc < 0)
			SetError(rc);
		if(rc == 0) {
			chdata->channel = NULL;
			LLOG("Channel succesfully closed.");
		}
		return rc == 0;
	});
}

bool SshChannel::Open()
{
	return Cmd(CHOPEN, [=]() mutable {
		return SshChannel::Init();
	});
}

bool SshChannel::Close()
{
	return Cmd(CHCLOSE, [=]() mutable {
		if(!chdata || !chdata->channel)
			return true;
		auto rc = libssh2_channel_close(chdata->channel);
		if(!WouldBlock(rc) && rc < 0) {
			SetError(rc);
		}
		if(rc == 0) {
			LLOG("Closing channel...");
			chdata->channel = NULL;
		}
		return rc == 0;
	});
}

bool SshChannel::CloseWait()
{
	return Cmd(CHWAIT, [=]() mutable {
		ASSERT(chdata->channel);
		auto rc = libssh2_channel_wait_closed(chdata->channel);
		if(!WouldBlock(rc) && rc < 0)
			SetError(rc);
		if(rc == 0)
			LLOG("Channel successfully closed.");
		return rc == 0;
	});
}

bool SshChannel::Request(const String& request, const String& params)
{
	return Cmd(CHREQUEST, [=]() mutable {
		ASSERT(chdata->channel);
		auto rc = libssh2_channel_process_startup(
			chdata->channel,
			request,
			request.GetLength(),
			params.GetLength() ? ~params : NULL,
			params.GetLength()
		);
		if(!WouldBlock(rc) && rc < 0)
			SetError(rc);
		if(rc == 0)
			LLOG("\"" << request << "\" request (params: " << params << ") is successful.");
		return rc == 0;
	});
}

bool SshChannel::Terminal(const String& term, int width, int height)
{
	return ComplexCmd(CHREQUEST, [=]() mutable {
		Cmd(CHREQUEST, [=]() mutable {
			ASSERT(chdata->channel);
			auto rc = libssh2_channel_request_pty(chdata->channel, ~term);
			if(!WouldBlock(rc) && rc < 0)
				SetError(rc);
			if(rc == 0)
				LLOG("Terminal (" << term << ") opened.");
			return rc == 0;
		});
		Cmd(CHREQUEST, [=]() mutable {
			ASSERT(chdata->channel);
			auto rc = libssh2_channel_request_pty_size(chdata->channel, width, height);
			if(!WouldBlock(rc) && rc < 0)
				LLOG("Warning: Couldn't set terminal size.");
			if(rc == 0)
				LLOG("Terminal size adjusted. (W:" << width << ", H:" << height << ")");
			return !WouldBlock(rc);
		});
	});
}

bool SshChannel::SetEnv(const String& variable, const String& value)
{
	return Cmd(CHSETENV, [=]() mutable {
		ASSERT(chdata->channel);
		auto rc = libssh2_channel_setenv(chdata->channel, variable, value);
		if(!WouldBlock(rc) && rc < 0)
			SetError(rc);
		if(rc == 0)
			LLOG("Environment variable " << variable << " set to " << value);
		return rc == 0;
	});
}

bool SshChannel::Read(Stream& out, int64 size, Gate<int64, int64> progress)
{
	ssh->packet_length = 0;
	return Cmd(CHREAD, [=, &out]() mutable {
		return DataRead(0, out, size, pick(progress));
	});
}

bool SshChannel::DataRead(int id, Stream& out, int64 size, Gate<int64, int64> progress)
{
	ASSERT(chdata->channel);
	ssize_t sz = size > 0 ? (ssize_t) size : (ssize_t) ssh->chunk_size;

	while(!IsTimeout()) {
		Buffer<char> buffer(sz);
		auto rc = libssh2_channel_read_ex(chdata->channel, id, ~buffer, sz);
		if(rc < 0) {
			if(!WouldBlock(rc))
				SetError(rc);
			break;
		}
		if(rc == 0)
			return true;
		out.Put64(buffer, rc);
		out.Flush();
		if(progress(size, out.GetSize()))
			SetError(-1, "Read aborted.");
	}
	return false;
}

bool SshChannel::Write(Stream& in, int64 size, Gate<int64, int64> progress)
{
	ssh->packet_length = 0;
	return Cmd(CHWRITE, [=, &in]() mutable {
		return DataWrite(0, in, size, pick(progress));
	});
}

bool SshChannel::DataWrite(int id, Stream& in, int64 size, Gate<int64, int64> progress)
{
	ASSERT(chdata->channel);
	ssize_t sz = size > 0 ? (ssize_t) size : (ssize_t) ssh->chunk_size;

	while(!IsTimeout()) {
		auto remaining  = (ssize_t) sz - ssh->packet_length;
		auto rc = libssh2_channel_write_ex(chdata->channel, id, in.Get(remaining), remaining);
		if(rc < 0) {
			if(!WouldBlock(rc))
				SetError(rc);
			break;
		}
		if(rc == 0) {
			ssh->packet_length = 0;
			return true;
		}
		ssh->packet_length += rc;
		if(progress(sz, ssh->packet_length))
			SetError(-1, "Write aborted");
	}
	return false;
}

bool SshChannel::SendEof()
{
	return Cmd(CHEOF, [=]() mutable {
		ASSERT(chdata->channel);
		auto rc = libssh2_channel_send_eof(chdata->channel);
		if(!WouldBlock(rc) && rc < 0)
			SetError(rc);
		if(rc == 0)
			LLOG("EOF message's sent to the server.");
		return rc == 0;
	});
}

bool SshChannel::RecvEof()
{
	return Cmd(CHEOF, [=]() mutable {
		ASSERT(chdata->channel);
		auto rc = libssh2_channel_wait_eof(chdata->channel);
		if(!WouldBlock(rc) && rc < 0)
			SetError(rc);
		if(rc == 0)
			LLOG("EOF message's acknowledged by the server.");
		return rc == 0;
	});
}

bool SshChannel::SendRecvEof()
{
	return ComplexCmd(CHEOF, [=]() mutable {
		SendEof();
		RecvEof();
	});
}

bool SshChannel::ReadStdErr(Stream& err)
{
	return Cmd(CHSTDERR, [=, &err]() mutable {
		return DataRead(SSH_EXTENDED_DATA_STDERR, err, ssh->chunk_size);
	});
}

bool SshChannel::WriteStdErr(Stream& err)
{
	return Cmd(CHSTDERR, [=, &err]() mutable {
		return DataWrite(SSH_EXTENDED_DATA_STDERR, err, ssh->chunk_size);
	});
}

int SshChannel::GetExitCode()
{
	if(chdata && chdata->channel) {
		chdata->code = libssh2_channel_get_exit_status(chdata->channel);
	}
	return chdata->code;
}

String SshChannel::GetExitSignal()
{
	Buffer<char*> sig(64);
	if(chdata && chdata->channel) {
		libssh2_channel_get_exit_signal(chdata->channel, sig, NULL, NULL, NULL, NULL, NULL);
		chdata->signal = *sig;
	}
	return chdata->signal;
}

SshChannel::SshChannel(SshSession& session)
: Ssh()
{
	chdata.Create();
	Zero(chdata->fstat);
	chdata->channel	= NULL;
	chdata->code	= 0;
	chdata->open	= false;
	ssh->otype		= CHANNEL;
	ssh->session	= session.GetHandle();
	ssh->timeout	= session.GetTimeout();
	ssh->event_proxy = Proxy(session.WhenDo);

}

SshChannel::~SshChannel()
{
	if(chdata && chdata->channel) { // Picked?
		ssh->async = false;
		Exit();
	}
}

int SshExec::Execute(const String& cmd, Stream& out, Stream& err)
{
	ComplexCmd(EXEC, [=, &out, &err]() mutable {
		SshChannel::Open();
		SshChannel::Exec(cmd);
		SshChannel::Read(out, ssh->chunk_size);
		SshChannel::ReadStdErr(err);
		SshChannel::Close();
		Cmd(CHRC,  [=]() mutable { SshChannel::GetExitCode(); return true; });
		Cmd(CHSIG, [=]() mutable { SshChannel::GetExitSignal(); return true; });
	});
	return chdata->code;
}


bool Scp::Get(Stream& out, const String& path, Gate<int64, int64> progress)
{
	return ComplexCmd(FGET, [=, &out]() mutable {
		ssh->packet_length = 0;
		Cmd(FGET, [=]() mutable {
			ASSERT(ssh->session);
			if(!chdata || path.IsEmpty())
				SetError(-1, "Path is not set.");
			chdata->channel = libssh2_scp_recv2(ssh->session, path, &chdata->fstat);
			if(!chdata->channel && !WouldBlock())
				SetError(-1);
			if(chdata->channel)
				LLOG("Channel obtained.");
			return chdata->channel != NULL;
		});
		Cmd(FGET, [=, &out]() mutable {
			return DataRead(0, out, chdata->fstat.st_size, pick(progress));
		});
		SendRecvEof();
		Close();
	});
}

String Scp::Get(const String& path, Gate<int64, int64> progress)
{
	ComplexCmd(FGET, [=]() mutable {
		chdata->data.Create();
		Get(chdata->data, path, pick(progress));
	});
	return ssh->async ? Null : GetResult();
}

bool Scp::Put(Stream& in, const String& path, long mode, Gate<int64, int64> progress)
{
	return ComplexCmd(FGET, [=, &in]() mutable {
		ssh->packet_length = 0;
		Cmd(FPUT, [=, &in]() mutable {
			ASSERT(ssh->session);
			if(!chdata || path.IsEmpty())
				SetError(-1, "Path is not set.");
			chdata->channel = libssh2_scp_send64(ssh->session, path, mode, in.GetSize(), 0, 0);
			if(!chdata->channel && !WouldBlock())
				SetError(-1);
			if(chdata->channel)
				LLOG("Channel obtained.");
			return chdata->channel != NULL;
		});
		Cmd(FPUT, [=, &in]() mutable {
			return DataWrite(0, in, in.GetSize(), pick(progress));
		});
		SendRecvEof();
		Close();
	});
}
}