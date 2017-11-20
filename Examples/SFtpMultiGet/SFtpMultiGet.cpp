#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// This example demonstrates getting a directory listing, and using that list to download
// multiple files from the designated SFTP server asynchronously.

const char *host  = "test.rebex.net";	// A well known public (S)FTP test server.
const char *user  = "demo";
const char *pass  = "password";
const char *dir   = "/pub/example/";
const int  MAXDOWNLOADS  = 4;

void SetDownloaders(SshSession& ssh, const SFtp::DirList& ls, ArrayMap<String, SFtp>& dls)
{
	for(auto i = 0; i < ls.GetCount(); i++) {
		if(!ls[i].IsFile())
			continue;
		if(dls.GetCount() == MAXDOWNLOADS)
			break;
		auto  file = AppendFileName(dir, ls[i].GetName());
		auto& sftp = dls.Add(file, new SFtp(ssh));
		sftp.NonBlocking()
		    .Timeout(0)
		    .Get(file);
		DLOG("Dowloading " << file);
	}
}

void GetRemoteFiles(ArrayMap<String, SFtp>& dls)
{
	// Non-blocking downloader.
	while(!dls.IsEmpty())
		for(int i = 0; i < dls.GetCount(); i++) {
			auto& sftp = dls[i];
			auto  file = AppendFileName(GetTempPath(), GetFileName(dls.GetKey(i)));
			SocketWaitEvent we;
			sftp.AddTo(we);
			we.Wait(10);
			if(sftp.Do())
				continue;
			if(sftp.IsError()) {
				DLOG("Unable to download " << dls.GetKey(i));
				DLOG(sftp.GetErrorDesc());
			}
			else {
				FileOut fout(file);
				if(!fout) {
					DLOG("Unable to write data to " << file);
					DLOG(fout.GetErrorText());
				}
				else {
					DLOG("Writing transferred data to " << file);
					fout.Put(sftp.GetResult().To<String>());
				}
			}
			dls.Remove(i);
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
				Cout() << e << '\n';
			ArrayMap<String, SFtp> downloaders;
			SetDownloaders(session, ls, downloaders);
			GetRemoteFiles(downloaders);
		}
		else Cerr() << sftp.GetErrorDesc() << '\n';
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
