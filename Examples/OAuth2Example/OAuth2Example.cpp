#include <Core/Core.h>
#include <OAuth/OAuth.h>

// OAuth2Example:
// This example will try to obtain an access token for Google Drive access, using the Google OAuth2 api.
// Note: You'll need to replace the client id and secret values with the ones you obtain from Google.

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);

	const char *CLIENT_ID      = "client-id";
	const char *CLIENT_SECRET  = "client-secret";
	const char *CLIENT_SCOPE   = "https://www.googleapis.com/auth/drive";
	const char *AUTH_ENDPOINT  = "https://accounts.google.com/o/oauth2/auth";
	const char *TOKEN_ENDPOINT = "https://www.googleapis.com/oauth2/v4/token";

	String auth_code, token_response;
	
	OAuth2Request oauth2(CLIENT_ID, CLIENT_SECRET);
	
	oauth2.Port(3245)
		  .UseNonce();
	
	if(oauth2.GetAuthCode(OAuth2CodeRequest(AUTH_ENDPOINT, CLIENT_SCOPE), auth_code)) {
		DUMP(auth_code);
		if(oauth2.GetAccessToken(OAuth2TokenRequest(TOKEN_ENDPOINT, auth_code), token_response))
			DUMP(token_response);
	}
	else LOG(oauth2.GetErrorDesc());
}
