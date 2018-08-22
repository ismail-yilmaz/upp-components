#ifndef _OAuth_Oauth_h_
#define _OAuth_Oauth_h_

#include <Core/Core.h>
#include <Core/SSL/SSL.h>

namespace Upp {

class OAuth2Request {
public:
    OAuth2Request&  Credentials(const String& id, const String& secret)         { client_id = id; client_secret = secret; return *this; }
    OAuth2Request&  BasicAuth(bool b = true)                                    { basic_auth = b; return *this; }
    OAuth2Request&  UseNonce(bool b = true)                                     { use_nonce = b; return *this;  }
    OAuth2Request&  Port(int prt)                                               { port = prt; return *this;     }
    OAuth2Request&  Timeout(int ms)                                             { timeout = clamp(ms, 10, INT_MAX); return *this;   }
    OAuth2Request&  CallbackUrl(const String& url)                              { callback_url = url; return *this; }
    
    bool            GetAuthCode(const ValueMap& request, String& code)          { return Execute(Request::CODE, request, code); }
    bool            GetAccessToken(const ValueMap& request, String& response)   { return Execute(Request::TOKEN, request, response); }
    bool            GetRefreshToken(const ValueMap& request, String& response)  { return Execute(Request::REFRESH, request, response); }
    
    bool            IsError() const                                             { return is_error; }
    String          GetErrorDesc() const                                        { return errormsg; }

    Event<>         WhenWait;
    Event<String>   WhenAuth;
    Event<ValueMap> WhenCheck;

    struct Error : Exc {
        Error(const String& reason) : Exc(reason)                               {}
    };

    OAuth2Request();
    OAuth2Request(const String& id, const String& secret) : OAuth2Request()     { Credentials(id, secret); }
    virtual ~OAuth2Request()                                                    {}
    
private:
    enum class Request { CODE, TOKEN, REFRESH };
    
    String          client_id;
    String          client_secret;
    String          callback_url;
    String          nonce;
    String          errormsg;
    bool            is_error;
    bool            basic_auth;
    bool            use_nonce;
    int             port;
    int             timeout;
    
    bool            Execute(Request r, const ValueMap& vm, String& result);
    String          SignIn(const String& path);
    String          GetPage(const String& s);
    String          ToStr(const ValueMap& vm, bool noep = false);
};

ValueMap OAuth2CodeRequest(const String& endpoint, const String& scope);
ValueMap OAuth2TokenRequest(const String& endpoint, const String& code, const String& scope = Null);
ValueMap OAuth2RefreshRequest(const String& endpoint, const String& token, const String& scope = Null);
}
#endif
