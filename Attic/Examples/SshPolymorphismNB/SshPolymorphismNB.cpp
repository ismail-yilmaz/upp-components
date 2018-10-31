#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ssh::Trace();

	StringStream file, list;
	SshSession session;
	if(session.Timeout(30000).Connect("demo:password@test.rebex.net:22")) {
		Array<Ssh> channels;
		session.NonBlocking();
		{
			channels.Add(new SFtp(session)).To<SFtp>()
				.Get("readme.txt", file);
			channels.Add(new SshExec(session)).To<SshExec>()
				.Execute("ls -l /pub/example", list, NilStream());
		}
		while(!channels.IsEmpty()) {
			for(int i = 0; i < channels.GetCount(); i++) {
				auto& ch = channels[i];
				if(ch.Do()) {
					SocketWaitEvent we;
					ch.AddTo(we);
					we.Wait(1000);
					continue;
				}
				if(ch.IsError()) {
					LOG(ch.GetErrorDesc());
				}
				else {
					switch(ch.GetType()) {
						case Ssh::SFTP:
							LOG(file.GetResult());
							break;
						case Ssh::EXEC:
							LOG(list.GetResult());
							break;
					}
				}
				channels.Remove(i);
				break;
			}
		}
	}
	else
		LOG(session.GetErrorDesc());
}
