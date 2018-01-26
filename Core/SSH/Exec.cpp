#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

int SshExec::Execute(const String& cmd, Stream& out, Stream& err)
{
	ComplexCmd(CHEXEC, [=, &out, &err]() mutable {
		SshChannel::Open();
		Cmd(CHSHELL, [=] { return Lock(); });
		SshChannel::Exec(cmd);
		Cmd(CHSHELL, [=] { Unlock(); return true; });
		SshChannel::Get(out, ssh->chunk_size);
		SshChannel::GetStdErr(err);
		SshChannel::SendRecvEof();
		SshChannel::Close();
		SshChannel::CloseWait();
		Cmd(CHRC,  [=]() mutable { SshChannel::GetExitCode(); return true; });
		Cmd(CHSIG, [=]() mutable { SshChannel::GetExitSignal(); return true; });
		
	});
	return exitcode;
}
}