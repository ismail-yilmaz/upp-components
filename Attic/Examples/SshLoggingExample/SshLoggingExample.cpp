#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// To activate verbose logging, set the LIBSSH2TRACE flag.
// (e.g. via TheIDE->main configuration settings)

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	// Ssh::Trace();
	
	Ssh::TraceVerbose(
//		LIBSSH2_TRACE_SOCKET    |
		LIBSSH2_TRACE_KEX       |
		LIBSSH2_TRACE_AUTH      |
		LIBSSH2_TRACE_CONN      |
//		LIBSSH2_TRACE_SCP       |
//		LIBSSH2_TRACE_SFTP      |
//		LIBSSH2_TRACE_PUBLICKEY |
		LIBSSH2_TRACE_ERROR
	);
	SshSession session;
	auto b = session.Timeout(30000).Connect("demo:password@test.rebex.net:22");
	LOG((b ? "Successfully connected to SSH2 server." : session.GetErrorDesc() << '\n'));
}
