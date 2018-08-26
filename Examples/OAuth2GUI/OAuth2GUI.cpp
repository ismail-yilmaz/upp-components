#include <CtrlLib/CtrlLib.h>
#include <ChromiumBrowser/ChromiumBrowser.h>
#include <OAuth/OAuth.h>

using namespace Upp;

#define LAYOUTFILE <OAuth2GUI/OAuth2GUI.lay>
#include <CtrlCore/lay.h>

// Note: This authorization dialog can be called within any U++ GUI application.

class GoogleDriveAuthDialog : public OAuth2Request, public TopWindow {
	ChromiumBrowser browser; // We'll use the U++/CEF wrapper.
public:
	GoogleDriveAuthDialog()
	{
		SetRect(0, 0, 640, 540);
		CenterScreen();
		Title(t_("Google Drive Authorization"));
		Add(browser.SizePos());
	
		WhenWait  = [=] { ProcessEvents(); };
		WhenAuth  = [=](String url) { browser.StartPage("file://" + url); Open(); };
		WhenClose = [=] { throw OAuth2Request::Error("Authentication cancelled."); };
	}

	bool Authorize(int port = 3245)
	{
		const char *CLIENT_ID      = "client-id";
		const char *CLIENT_SECRET  = "client-secret";
		const char *CLIENT_SCOPE   = "https://www.googleapis.com/auth/drive";
		const char *AUTH_ENDPOINT  = "https://accounts.google.com/o/oauth2/auth";
		const char *TOKEN_ENDPOINT = "https://www.googleapis.com/oauth2/v4/token";
		
		Port(port);
		UseNonce();
		Credentials(CLIENT_ID, CLIENT_SECRET);
	
		if(GetAuthCode(OAuth2CodeRequest(AUTH_ENDPOINT, CLIENT_SCOPE), code))
			GetAccessToken(OAuth2TokenRequest(TOKEN_ENDPOINT, code), response);
		if(IsOpen())
			Close();
		return !IsError();
	}

	String	code;
	String	response;
};

struct OAuth2App : public WithMainLayout<TopWindow> {
	GoogleDriveAuthDialog gauth;
	OAuth2App()
	{
		CtrlLayout(*this, t_("OAuth2 Test"));
		auth.WhenAction = [=]
		{
			if(gauth.Authorize())
				PromptOK(DeQtf(gauth.response));
			else ErrorOK(DeQtf(gauth.GetErrorDesc()));
		};
	}
};

GUI_APP_MAIN
{
	StdLogSetup(LOG_FILE);
	SetLanguage(SetLNGCharset(GetSystemLNG(), CHARSET_UTF8));
	if (ChromiumBrowser::IsChildProcess()){
		ChromiumBrowser::ChildProcess();
	}else{
		OAuth2App().Run();
	}
}
