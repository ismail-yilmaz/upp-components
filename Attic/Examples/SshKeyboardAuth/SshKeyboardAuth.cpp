#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
//	Ssh::Trace();

	SshSession session;
	session.WhenKeyboard = [=](String title, String instructions, String prompt) mutable
	{
		Cout() << prompt;       // Ignore title and instructions.
		return ReadSecret();    // Rebex does not provide them anyway.
	};
	if(session.Timeout(30000).KeyboardAuth().Connect("demo@test.rebex.net:22"))
		LOG("Successfully connected to server, using keyboard-interactive method.");
	else
		LOG(session.GetErrorDesc());
}
