#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	Ftp::Trace();
	Ftp ftpclient;
	if(ftpclient.Connect("ftp://demo:password@test.rebex.net:21")) {
		ftpclient.NonBlocking().Get("readme.txt");
		while(ftpclient.Do()) {
			SocketWaitEvent we;
			ftpclient.AddTo(we);
			we.Wait(10);
		}
		Cout() << (ftpclient.IsError()
						? ftpclient.GetErrorDesc()
						: ftpclient.GetResult().To<String>()) << '\n';
	}
	else Cout() << ftpclient.GetErrorDesc() << '\n';
		
}
