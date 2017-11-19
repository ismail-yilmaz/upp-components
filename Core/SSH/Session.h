class SshSession : public Ssh {
public:
  enum Methods {
        METHOD_EXCHANGE = 0,
        METHOD_HOSTKEY,
        METHOD_CENCRYPTION,
        METHOD_SENCRYPTION,
        METHOD_CMAC,
        METHOD_SMAC,
        METHOD_CCOMPRESSION,
        METHOD_SCOMPRESSION
    };

public:
    SshSession&         Timeout(int ms)                         { ssh->timeout = ms; return *this; }
    SshSession&         NonBlocking(bool b = true)              { ssh->async = b; return *this ;}

    SshSession&         Keys(const String& prikey, const String& pubkey, const String& phrase = Null);
    SshSession&         Method(int type, Value method)          { session->iomethods(type) = pick(method); return *this; }
    SshSession&         Methods(ValueMap methods)               { session->iomethods = pick(methods); return *this; }

    SshSession&         PasswordAuth()                          { session->authmethod = PASSWORD;  return *this; }
    SshSession&         PublicKeyAuth()                         { session->authmethod = PUBLICKEY; return *this; }
    SshSession&         KeyboardAuth()                          { session->authmethod = KEYBOARD;  return *this; }
    SshSession&         AgentAuth()                             { session->authmethod = SSHAGENT;  return *this; }

    LIBSSH2_SESSION*    GetHandle()                             { return ssh->session; }
    String              GetBanner() const                       { return ssh->session ? pick(String(libssh2_session_banner_get(ssh->session))) : Null; }
    String              GetFingerprint() const                  { return session->fingerprint; }
    Vector<String>      GetAuthMethods()                        { return pick(Split(session->authmethods, ' ')); }
    TcpSocket&          GetSocket()                             { return session->socket;  }
    ValueMap            GetMethods();

    SFtp                CreateSFtp();
    SshChannel          CreateChannel();
    SshExec             CreateExec();
    Scp                 CreateScp();

    bool                Connect(const String& host, int port, const String& user, const String& password);
    void                Disconnect();

    Event<>             WhenDo;
    Event<>             WhenConfig;
    Event<>             WhenAuth;
    Gate<>              WhenVerify;
    Gate<>              WhenProxy;
    Function<String(String, String, String)>  WhenKeyboard;

    SshSession();
    virtual ~SshSession();

    SshSession(SshSession&&) = default;
    SshSession& operator=(SshSession&&) = default;

private:
    virtual void        Exit() override;
    virtual void        Check() override;
    String              GetMethodNames(int type);
    int                 TryAgent(const String& username);
    void                FreeAgent(SshAgent* agent);

    struct SessionData {
        TcpSocket       socket;
        IpAddrInfo      ipinfo;
        String          fingerprint;
        String          authmethods;
        int             authmethod;
        String          prikey;
        String          pubkey;
        String          phrase;
        ValueMap        iomethods;
        bool            connected;
    };
    One<SessionData> session;

    enum AuthMethod     { PASSWORD, PUBLICKEY, KEYBOARD, SSHAGENT };
    enum HostkeyType    { RSAKEY, DSSKEY };
    enum OpCodes        { CONNECT, LOGIN, DISCONNECT };
};
