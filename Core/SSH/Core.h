class Ssh {
public:
    bool                Do();
    bool				Wait(int ms = 10);
    void                Cancel()                                { if(ssh) ssh->status = CANCELLED; }

    int                 GetTimeout() const                      { return ssh->timeout; }

    bool                IsError() const                         { return ssh->status == FAILED; }
    int                 GetError() const                        { return ssh->error.Get<int>(); }
    String              GetErrorDesc() const                    { return ssh->error.Get<String>(); }
    
    int64               GetId() const                           { return ssh->oid;   }
    int                 GetType() const                         { return ssh->otype; }
    
    template <class T>  T&   To()                               { auto* t = dynamic_cast<T*>(this); ASSERT(t); return *t; }
    template <class T>  bool Is() const                         { return dynamic_cast<const T*>(this); }

    static void         Trace(bool b = true)                    { SSH::sTrace = b; }
    static void         TraceVerbose(bool b = true)             { Trace(b); SSH::sTraceVerbose = b; }

    Ssh();
    virtual ~Ssh();
    
    Ssh(Ssh&&) = default;
    Ssh& operator=(Ssh&&) = default;

    struct Error : Exc {
        int code;
        Error() : Exc(Null), code(-1) {}
        Error(const String& reason) : Exc(reason), code(-1) {}
        Error(int rc, const String& reason) : Exc(reason), code(rc) {}
    };
    enum Type  {
        SESSION,
        SFTP,
        CHANNEL,
        SCP,
        EXEC,
        SHELL
    };

protected:
    struct CoreData {
        BiVector<Tuple<int, Gate<>>> queue;
        LIBSSH2_SESSION*    session;
        TcpSocket*			socket;
        Tuple<int, String>  error;
        Event<>             event_proxy;
        int                 ccmd;
        bool                async;
        bool                init;
        int64               oid;
        int                 otype;
        int                 timeout;
        int                 start_time;
        int                 chunk_size;
        size_t              packet_length;
        int                 status;
    };
    One<CoreData> ssh;
    
    enum Status         { WORKING, FINISHED, CLEANUP, CANCELLED, FAILED };
 
    virtual bool        Init()                                  { return true; }
    virtual void        Exit() = 0;
    virtual bool        Cmd(int code, Function<bool()>&& fn);
    virtual bool        ComplexCmd(int code, Function<void()>&& fn);
    virtual void        Check();
    virtual bool        Cleanup(Error& e);
   
    int&                OpCode()                                { return ssh->queue.Head().Get<int>(); }
    bool                WouldBlock(int rc)                      { return rc == LIBSSH2_ERROR_EAGAIN; }
    bool                WouldBlock()                            { return ssh->session && WouldBlock(libssh2_session_last_errno(ssh->session)); }
    bool                IsTimeout() const                       { return ssh->timeout > 0 && msecs(ssh->start_time) >= ssh->timeout; }
    inline bool         IsComplexCmd() const                    { return ssh->ccmd != -1; }
    void                SetError(int rc, const String& reason = Null);
    
private:
    bool                _Do();
    static int64        GetNewId();
};