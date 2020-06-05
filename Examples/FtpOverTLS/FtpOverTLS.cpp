#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

bool CheckSSLCertificate(const SSLInfo* info)
{
	RDUMP(info->cert_issuer);
	RDUMP(info->cert_notafter);
	RDUMP(info->cert_notbefore);
	RDUMP(info->cert_subject);
	RDUMP(info->cert_version);
	RDUMP(info->cert_avail);
	RDUMP(info->cert_verified);
	RDUMP(info->cipher);
	
	Cout() << "Would you like to continue? (Y/N):";
	return ToLower(ReadStdIn()).StartsWith("n");
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
//	Ftp::Trace();

	Ftp ftpclient;
	ftpclient.WhenSSLInfo = callback(CheckSSLCertificate);
	if(ftpclient.Timeout(30000).Connect("ftps://demo:password@test.rebex.net:21"))
		RLOG("Secure connection established.");
	else
		RLOG(ftpclient.GetErrorDesc());
	
}
