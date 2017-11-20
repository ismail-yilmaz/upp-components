#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// This example demonstrates downloading multiple files from a SFTP server asynchronously.

void GetRemoteFiles(ArrayMap<String, SFtp>& downloaders)
{
	// Non-blocking downloader:
	while(!downloaders.IsEmpty())
		for(auto i = 0; i < downloaders.GetCount(); i++) {
			SocketWaitEvent we;
			auto& sftp = downloaders[i];
			sftp.AddTo(we);
			we.Wait(10);
			if(!sftp.Do()) {
				if(sftp.IsError())
					Cerr() << sftp.GetErrorDesc() << '\n';
				else {
					auto fname = AppendFileName(
						GetTempPath(),
						GetFileName(downloaders.GetKey(i))
					);
					FileOut fout(fname);
					if(fout) {
						Cout() << "Writing the transferred data to " << fname << "...\n";
						fout.Put((String) sftp.GetResult());
					}
					else Cerr() << "Unable to open local file " << fname << "!\n";
				}
				downloaders.Remove(i);
				break;
			}
		}
}

CONSOLE_APP_MAIN
{
	const char *host  = "test.rebex.net";	// A well known public (S)FTP test server.
	const char *user  = "demo";
	const char *pass  = "password";
	const char *dir   = "/pub/example/";
	const int  MAXDOWNLOADS  = 4;

	
	Ssh::Trace();
	SshSession session;
	if(session.Timeout(30000).Connect(host, 22, user, pass)) {
		auto sftp = session.CreateSFtp();
		SFtp::DirList ls;
		if(sftp.ListDir(dir, ls)) {
			for(auto& e : ls)
				Cout() << e << '\n';
			ArrayMap<String, SFtp> downloaders;
			for(auto i = 0; i < ls.GetCount(); i++) {
				if(!ls[i].IsFile())
					continue;
				if(downloaders.GetCount() == MAXDOWNLOADS)
					break;
				auto  file = AppendFileName(dir, ls[i].GetName());
				auto& sftp = downloaders.Add(file, new SFtp(session));
				sftp.NonBlocking()
				    .Timeout(0)
				    .Get(file);
				Cout() << "Downloading remote file " << file << "...\n";
			}
			GetRemoteFiles(downloaders);
		}
		else Cerr() << sftp.GetErrorDesc() << '\n';
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
