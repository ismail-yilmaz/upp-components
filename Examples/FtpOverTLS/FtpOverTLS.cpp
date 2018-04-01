#include <Core/Core.h>
#include <FTP/Ftp.h>

using namespace Upp;

bool CheckSSLCertificate(const SSLInfo* info)
{
	DUMP(info->cert_issuer);
	DUMP(info->cert_notafter);
	DUMP(info->cert_notbefore);
	DUMP(info->cert_subject);
	DUMP(info->cert_version);
	DUMP(info->cert_avail);
	DUMP(info->cert_verified);
	DUMP(info->cipher);
	
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
		LOG("Secure connection established.");
	else
		LOG(ftpclient.GetErrorDesc());
	
}
