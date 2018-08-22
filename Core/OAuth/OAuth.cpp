#include "OAuth.h"

namespace Upp {

#define LLOG(x)  RLOG("OAuth2: " << x)

static String MakeNonce()
{
	String s;
	
	s << "id=upp_oauth2.0.1\r\n"
	  << "time=" << GetSysTime() << "\r\n"
	  << "ticks=" << GetTickCount() << "\r\n"
	  << "random=" << Random64(UINT64_MAX) << "\r\n";

	return pick(Base64Encode(SHA256String(s)));
}

static void SendHttpResponse(TcpSocket& s, const String& msg)
{
	if(!HttpResponse(s, false, 200, "OK", "text/plain", msg))
		throw OAuth2Request::Error(s.GetErrorDesc());
}

String OAuth2Request::ToStr(const ValueMap& vm, bool noep)
{
	String out;
	for(int i = 0; i < vm.GetCount(); i++) {
		String k = vm.GetKey(i);
		if(noep && k.IsEqual("end_point"))
			continue;
		String v = vm[k];
		out << (i > 0 ? "&" : "") << k << "=" << v;
	}
	return pick(out);
}

bool OAuth2Request::Execute(Request t, const ValueMap& vm, String& result)
{
	HttpRequest httpcli;
	is_error = false;
	String s;

	UrlInfo u(callback_url);
	
	s << "redirect_uri=http://"
	  << (IsNull(u.host) ? "localhost" : u.host)
	  << ":"
	  << (IsNull(u.port) ? AsString(port) : u.port)
	  << "&"
	  << ToStr(vm, true)
	  << "&";

	if(t == Request::CODE && use_nonce) {
		nonce = MakeNonce();
		s << "&state=" << nonce;
	}

	if(!basic_auth) {
		s << "&client_id=" << client_id
		  << (t != Request::CODE ? "&client_secret=" << client_secret : "");
	}
	else
		httpcli.User(client_id, client_secret);
	
	httpcli.RequestTimeout(timeout);
	httpcli.WhenDo = Proxy(WhenWait);
	httpcli.WhenWait = Proxy(WhenWait);
	httpcli.ContentType("application/x-www-form-urlencoded");
	httpcli.SSL();
	httpcli.Url(AsString(vm["end_point"]));
	httpcli.Post(s);
	
	try {
		String r = httpcli.Execute();
		
		if(httpcli.IsError())
			throw Error(httpcli.GetErrorDesc());

		if(httpcli.GetStatusCode() >= 400)
			throw Error(httpcli.GetReasonPhrase());

		switch(t) {
			case Request::CODE: {
				String path = GetPage(r);
				result = SignIn(path);
				LLOG("Authorization code request is successful.");
				break;
			}
			case Request::TOKEN: {
				result = pick(r);
				LLOG("Access token request is successful.");
				break;
			}
			case Request::REFRESH: {
				result = pick(r);
				LLOG("Refresh token request is successful.");
				break;
			}
			default:
				NEVER();
		}
	}
	catch(const Error& e) {
		is_error = true;
		errormsg = e;
		LOG("Failed: " << e);
	}
	return !is_error;
}

String OAuth2Request::SignIn(const String& path)
{
	TcpSocket server;
	
	server.WhenWait = Proxy(WhenWait);
	
	if(!server.Listen(port))
		throw Error(server.GetErrorDesc());

	WhenAuth ? WhenAuth(path) : LaunchWebBrowser(path);
	
	TcpSocket socket;
	socket.Timeout(0);
	
	int ms = msecs();
	
	for(;;) {
		if(socket.Accept(server))
			break;
		if(!IsNull(timeout) && msecs(ms) >= timeout)
			throw Error("Operation timed out.");
		WhenWait();
		Sleep(1);
	}

	LLOG("Connection accepted.");

	HttpHeader h;
	if(!h.Read(socket.Timeout(timeout)))
		throw Error(socket.GetErrorDesc());
	
	UrlInfo u(h.GetURI());
	
	String err;
	
	if(u.parameters.Find("error") >= 0) {
		err = u["error"];
		goto Bailout;
	}
	if(use_nonce && (u.parameters.Find("state") < 0 || u["state"] != nonce)) {
		err = "Client-server state mismatch.";
		goto Bailout;
	}
	if(u.parameters.Find("code") < 0) {
		err = "Missing authorization code.";
		goto Bailout;
	}
	
	SendHttpResponse(socket, t_("OAuth2 authorization code granted."));
	
	if(WhenCheck) {
		ValueMap v;
		for(int i = 0; i < u.parameters.GetCount(); i++)
			v.Add(u.parameters.GetKey(i), u.parameters[i]);
		WhenCheck(pick(v));
	}
	
	return pick(u["code"]);
	
Bailout:
	SendHttpResponse(socket, t_("OAuth2 authorization code denied."));
	throw Error(err);
}

String OAuth2Request::GetPage(const String& s)
{
	String path = AppendFileName(GetTempPath(), Format("oauth2_auth_page_%d.html", (int64) GetTickCount()));
	if(!SaveFile(NativePath(path), s))
		throw Error("Unable to save the html page to temp file '" << path << "'");
	LLOG("Saving the html content to '" << path << "'");
	return pick(path);
}

OAuth2Request::OAuth2Request()
{
	port       = 0;
	timeout    = 120000;
	basic_auth = false;
	use_nonce  = false;
	is_error   = false;
}

ValueMap OAuth2CodeRequest(const String& endpoint, const String& scope)
{
	ValueMap vm;
	vm("end_point", endpoint)("response_type", "code")("scope", scope);
	return pick(vm);
}

ValueMap OAuth2TokenRequest(const String& endpoint, const String& code, const String& scope)
{
	ValueMap vm;
	vm("end_point", endpoint)("grant_type", "authorization_code")("code", code);
	if(!IsNull(scope)) vm("scope", scope);
	return pick(vm);
}

ValueMap OAuth2RefreshRequest(const String& endpoint, const String& token, const String& scope)
{
	ValueMap vm;
	vm("end_point", endpoint)("grant_type", "refresh_token")("refresh_token", token);
	if(!IsNull(scope)) vm("scope", scope);
	return pick(vm);
}
}
