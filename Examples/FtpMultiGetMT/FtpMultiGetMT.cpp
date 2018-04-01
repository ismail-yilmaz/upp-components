#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

// This example demonstrates getting a directory listing, and using that list to download
// multiple files from the designated FTP server asynchronously, using worker threads.

const char *url = "demo:password@test.rebex.net:21";
const char *dir = "/pub/example/";
const int  MAXDOWNLOADS  = 4;

void GetRemoteFiles(const Ftp::DirList& ls)
{
	ArrayMap<String, AsyncWork<String>> workers;
	for(auto& e : ls) {
		if(!e.IsFile())
			continue;
		if(workers.GetCount() == MAXDOWNLOADS)
			break;
		auto path = UnixPath(AppendFileName(dir, e.GetName()));
		workers.Add(path) = Ftp::AsyncGet(UnixPath(AppendFileName(url, path)));
		Cout() << "Downloading " << path << "\n";
	}
	while(!workers.IsEmpty())
		for(int i = 0; i < workers.GetCount(); i++) {
			auto& worker = workers[i];
			if(worker.IsFinished()) {
				try {
					worker.Get();
					Cout() << workers.GetKey(i) << " successfully downloaded\n";
				}
				catch(const Ftp::Error& e) {
					Cerr() << "Worker  failed. " << e << "\n";
				}
				workers.Remove(i);
				break;
			}
		}
}

CONSOLE_APP_MAIN
{
	Ftp::Trace();

	Ftp ftpclient;
	if(ftpclient.Connect(url)) {
		Ftp::DirList ls;
		if(ftpclient.ListDir(dir, ls)) {
			for(auto& e : ls)
				Cout() << e << "\n";
			GetRemoteFiles(ls);
			return;
		}
		else Cerr() << ftpclient.GetErrorDesc();
	}
	else Cerr() << ftpclient.GetErrorDesc();
}