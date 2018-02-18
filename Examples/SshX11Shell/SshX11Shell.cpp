#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// Note: this example uses some default values.
// You'll need to change them to run this example.

CONSOLE_APP_MAIN
{
	const char *url = "username@password:localhost:22";

	Ssh::Trace();
	SshSession session;
	if(session.Connect(url)) {
		auto shell = session.CreateShell();
		session.WhenX11 = [&](SshX11Connection* x11conn) { shell.AcceptX11(x11conn); };
		if(!shell.ForwardX11().Console("xterm"))
			Cerr() << shell.GetErrorDesc() << '\n';
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}
