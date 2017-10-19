#include "SSH.h"

// WARNING: Below functions are experimental, and subject to change.

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG("SFtp async worker #" << worker.GetId() << " " << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

AsyncWork<String> SFtpGet(SshSession& session, const String& path, Gate<int64, int64> progress)
{
	auto work = Async([=, &session]{
		SFtp worker(session);
		worker.NonBlocking().Get(path, pick(progress));
		LLOG("Downloading file " << path);
		auto cancelled = false;
		while(worker.Do())
			
			if(!cancelled && CoWork::IsCanceled()) {
				LLOG("Job cancelled.");
				worker.Cancel();
				cancelled = true;
			}
		if(worker.IsError()) {
			LLOG("File download failed. " << worker.GetErrorDesc());
			throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
		}
		return pick(worker.GetResult().To<String>());
				
	});
	return pick(work);
}

AsyncWork<void> SFtpGet(SshSession& session, const String& source, const String& target, Gate<int64, int64> progress)
{
	auto work = Async([=, &session]{
		SFtp worker(session);
		FileOut fout(target);
		if(!fout) {
			auto error = Format("Unable to open local file '%s' for writing.", target);
			LLOG(error);
			throw Ssh::Error(error);
		}
		worker.NonBlocking().Get(source, fout, pick(progress));
		auto cancelled = false;
		while(worker.Do())
			if(!cancelled && CoWork::IsCanceled()) {
				LLOG("Job cancelled.");
				worker.Cancel();
				cancelled = true;
			}
		if(worker.IsError()) {
			LLOG("File download failed. " << worker.GetErrorDesc());
			throw Ssh::Error(worker.GetError(), worker.GetErrorDesc());
		}
	});
	return pick(work);
}
}