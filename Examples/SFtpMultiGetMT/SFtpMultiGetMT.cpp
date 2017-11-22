#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// This example demonstrates getting a directory listing, and using that list to download
// multiple files from the designated SFTP server asynchronously, using worker threads.

const char *host  = "test.rebex.net";	// A well known public (S)FTP test server.
const char *user  = "demo";
const char *pass  = "password";
const char *dir   = "/pub/example/";
const int  MAXDOWNLOADS  = 4;

void GetRemoteFiles(SshSession& session, const SFtp::DirList& ls)
{
	// Multithreaded downloader.
	ArrayMap<String, AsyncWork<String>> workers;
	for(auto& e : ls) {
		if(!e.IsFile())
			continue;
		if(workers.GetCount() == MAXDOWNLOADS)
			break;
		auto file = AppendFileName(dir, e.GetName());
		workers.Add(file) = pick(SFtp::AsyncGet(session, file));
		LOG("Downloading " << file);
	}
	while(!workers.IsEmpty())
		for(int i = 0; i < workers.GetCount(); i++) {
			if(!workers[i].IsFinished()) {
				Sleep(1);
				continue;
			}
			auto file = AppendFileName(GetTempPath(), GetFileName(workers.GetKey(i)));
			try {
				FileOut fout(file);
				if(fout) {
					LOG("Writing data to " << file);
					fout.Put(workers[i].Get());
				}
				else {
					LOG("Unable to open " << file);
					LOG(fout.GetErrorText());
				}
			}
			catch(Ssh::Error& e) {
				LOG(e);
			}
			workers.Remove(i);
			break;
		}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
//	Ssh::Trace();

	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
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
