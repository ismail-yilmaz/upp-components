#include "SSH.h"

// WARNING: Below functions are experimental, and subject to change.

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG("SSH: Worker thread #" << worker.GetId() << " " << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

static void SFtpAsyncIO(int direction, SshSession& session, const String& path, Stream& io, Gate<int64, int64> progress)
{
	SFtp worker(session);
	LLOG("Starting file transfer over SFtp...");
	worker.NonBlocking();
	if(direction == 0)
		worker.Get(path, io, pick(progress));
	else
		worker.Put(io, path, pick(progress));
	auto cancelled = false;
	while(worker.Do())
		if(!cancelled && CoWork::IsCanceled()) {
			LLOG("Transfer cancelled.");
			worker.Cancel();
			cancelled = true;
		}
	if(worker.IsError()) {
		LLOG("Transfer failed. " << worker.GetErrorDesc());
		throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
	}
}

AsyncWork<String> SFtp::AsyncGet(SshSession& session, const String& path, Gate<int64, int64> progress)
{
	auto work = Async([=, path = String(path), &session]{
		StringStream ss;
		ss.Create();
		SFtpAsyncIO(0, session, path, ss, pick(progress));
		return pick(ss.GetResult());
	});
	return pick(work);
}

AsyncWork<void> SFtp::AsyncGet(SshSession& session,	const char* source, const char* target, Gate<int64, int64> progress)
{
	auto work = Async([=, source = String(source), target = String(target), &session]{
		FileOut fout(target);
		if(!fout) {
			auto error = Format("Unable to open local file '%s' for writing.", target);
			throw Ssh::Error(error);
		}
		SFtpAsyncIO(0, session, source, fout, pick(progress));
	});
	return pick(work);
}

AsyncWork<void> SFtp::AsyncPut(SshSession& session, String&& data, const String& target, Gate<int64, int64> progress)
{
	auto work = Async([=, data = pick(data), target = String(target), &session]{
		StringStream ss(pick(data));
		SFtpAsyncIO(1, session, target, ss, pick(progress));
	});
	return pick(work);
}

AsyncWork<void> SFtp::AsyncPut(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress)
{
	auto work = Async([=, source = String(source), target = String(target), &session]{
		FileIn fin(source);
		if(fin.IsError()) {
			auto error = Format("Unable to open local file '%s' for reading.", source);
			throw Ssh::Error(error);
		}
		SFtpAsyncIO(1, session, target, fin, pick(progress));
	});
	return pick(work);
}

static void ScpAsyncIO(int direction, SshSession& session, const String& path, Stream& io, Gate<int64, int64> progress)
{
	Scp worker(session);
	LLOG("Starting file transfer over Scp... ");
	worker.NonBlocking();
	if(direction == 0)
		worker.Get(io, path, pick(progress));
	else
		worker.Put(io, path, pick(progress));
	auto cancelled = false;
	while(worker.Do())
		if(!cancelled && CoWork::IsCanceled()) {
			LLOG("Transfer cancelled.");
			worker.Cancel();
			cancelled = true;
		}
	if(worker.IsError()) {
		LLOG("Transfer failed. " << worker.GetErrorDesc());
		throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
	}
}

AsyncWork<String> Scp::AsyncGet(SshSession& session, const String& path, Gate<int64, int64> progress)
{
	auto work = Async([=, path = String(path), &session]{
		StringStream ss;
		ss.Create();
		ScpAsyncIO(0, session, path, ss, pick(progress));
		return pick(ss.GetResult());
	});
	return pick(work);
}

AsyncWork<void> Scp::AsyncGet(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress)
{
	auto work = Async([=, source = String(source), target = String(target), &session]{
		FileOut fout(target);
		if(!fout) {
			auto error = Format("Unable to open local file '%s' for writing.", target);
			throw Ssh::Error(error);
		}
		ScpAsyncIO(0, session, source, fout, pick(progress));
	});
	return pick(work);
}

AsyncWork<void> Scp::AsyncPut(SshSession& session, String&& data, const String& target, Gate<int64, int64> progress)
{
	auto work = Async([=, data = pick(data), target = String(target), &session]{
		StringStream ss(pick(data));
		ScpAsyncIO(1, session, target, ss, pick(progress));
	});
	return pick(work);
}

AsyncWork<void> Scp::AsyncPut(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress)
{
	auto work = Async([=, source = String(source), target = String(target), &session]{
		FileIn fin(source);
		if(!fin) {
			auto error = Format("Unable to open local file '%s' for reading.", source);
			throw Ssh::Error(error);
		}
		ScpAsyncIO(1, session, target, fin, pick(progress));
	});
	return pick(work);
}

AsyncWork<Tuple<int, String, String>> SshExec::Async(SshSession& session, const String& cmd)
{
	auto work = Upp::Async([=, cmd = String(cmd), &session]{
		SshExec worker(session);
		LLOG("Starting remote  command execution... ");
		worker.NonBlocking();
		auto cancelled = false;
		StringStream out, err;
		worker.Execute(cmd, out, err);
		while(worker.Do())
			if(!cancelled && CoWork::IsCanceled()) {
				LLOG("Transfer cancelled.");
				worker.Cancel();
				cancelled = true;
			}
		if(worker.IsError()) {
			LLOG("Remote command execution failed. " << worker.GetErrorDesc());
			throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
		}
		return MakeTuple<int, String, String>(
			worker.GetExitCode(),
			pick(out.GetResult()),
			pick(err.GetResult())
			);
	});
	return pick(work);
}

AsyncWork<void> SshShell::AsyncRun(SshSession& session, String terminal, Size pagesize, Event<SshShell&> in, Event<const String&> out)
{
	auto work = Async([=, &session, in = pick(in), out = pick(out)]{
		SshShell worker(session);
		LLOG("Starting SSH shell (in generic mode)...");
		worker.WhenInput = [&worker, &in]() {
			in(worker);
		};
		worker.WhenOutput = [&out](const void* buf, int len) {
			String s((const char*) buf, len);
			out(s);
		};
		worker.NonBlocking();
		worker.Run(terminal, pagesize);
		auto cancelled = false;
		while(worker.Do())
			if(!cancelled && CoWork::IsCanceled()) {
				LLOG("Shell session cancelled.");
				worker.Cancel();
				cancelled = true;
			}
		if(worker.IsError()) {
			LLOG("Shell failed. " << worker.GetErrorDesc());
			throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
		}
	});
}
}