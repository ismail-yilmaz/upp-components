#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// This example demonstrates getting a directory listing, and using that list to download
// multiple files from the designated SFTP server asynchronously, using worker threads.

const char *dir = "/pub/example/";
const int  MAXDOWNLOADS  = 4;

void GetRemoteFiles(SshSession& session, const SFtp::DirList& ls)
{
	ArrayMap<String, AsyncWork<void>> workers;
	for(auto& e : ls) {
		if(!e.IsFile())
			continue;
		if(workers.GetCount() == MAXDOWNLOADS)
			break;
		auto source = AppendFileName(dir, e.GetName());
		auto target = AppendFileName(GetTempPath(), e.GetName());
		workers.Add(source) = pick(SFtp::AsyncGet(session, ~source, ~target));
		LOG("Downloading " << source << " to " << target);
	}
	while(!workers.IsEmpty())
		for(int i = 0; i < workers.GetCount(); i++) {
			auto& worker = workers[i];
			if(worker.IsFinished()) {
				try {
					worker.Get();
					LOG(workers.GetKey(i) << " successfully downloaded");
				}
				catch(Ssh::Error& e) {
					LOG(e);
				}
				workers.Remove(i);
				break;
			}
		}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
//	Ssh::Trace();

	SshSession session;
	if(session.Timeout(30000).Connect("sftp://demo:password@test.rebex.net:22")) {
		auto sftp = session.CreateSFtp();
		SFtp::DirList ls;
		if(sftp.ListDir(dir, ls)) {
			for(auto& e : ls)
				LOG(e);
			GetRemoteFiles(session, ls);
		}
		else LOG(sftp.GetErrorDesc());
	}
	else LOG(session.GetErrorDesc());
}
