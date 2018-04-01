#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	Ftp::Trace();
	try {
		Cout() << Ftp::AsyncGet("ftp://demo:password@test.rebex.net:21/readme.txt").Get();
	}
	catch(Ftp::Error& e) {
		Cerr() << e << '\n';
	}
}