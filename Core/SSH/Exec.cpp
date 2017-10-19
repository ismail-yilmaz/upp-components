#include "SSH.h"

namespace Upp {

int SshExec::Execute(const String& cmd, Stream& out, Stream& err)
{
	ComplexCmd(EXEC, [=, &out, &err]() mutable {
		SshChannel::Open();
		SshChannel::Exec(cmd);
		SshChannel::Read(out, core->chunk_size);
		SshChannel::ReadStdErr(err);
		SshChannel::Close();
		Cmd(CHRC,  [=]() mutable { SshChannel::GetExitCode(); return true; });
		Cmd(CHSIG, [=]() mutable { SshChannel::GetExitSignal(); return true; });
	});
	return chdata->code;
}
}