#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;


CONSOLE_APP_MAIN
{
	const char *host = "test.rebex.net";	// A well-known public (S)Ftp test serverç
	const char *user = "demo";
	const char *pass = "password";
	
	Ssh::Trace();
	SshSession session;
	session.WhenKeyboard = [=](String title, String instructions, String prompt) mutable {
		Cout() << prompt;		// Ignore title and instructions.
		return ReadSecret();		// Rebex does not provide them anyway.
	};
	if(session.Timeout(30000).KeyboardAuth().Connect(host, 22, user, pass))
		Cout() << "\nSuccessfully connected to " << host << ", using keyboard-interactive method.\n";
	else Cerr() << session.GetErrorDesc() << '\n';
}
