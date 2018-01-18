class SshChannel : public Ssh {
public:
    SshChannel&         Timeout(int ms)                                             { ssh->timeout = ms; return *this; }
    SshChannel&         NonBlocking(bool b = true)                                  { ssh->async = b; return *this ;}
    SshChannel&         ChunkSize(int sz)                                           { ssh->chunk_size = clamp(sz, 128, INT_MAX); return *this; }


    LIBSSH2_CHANNEL*    GetHandle() const                                           { return *channel; }
    Value               GetResult()                                                 { return pick(result); }

    bool                Open();
    bool                Close();
    bool                CloseWait();
    bool                Request(const String& request, const String& params = Null);
    bool                Exec(const String& cmdline)                                 { return Request("exec", cmdline); }
    bool                Shell()                                                     { return Request("shell", Null); }
    bool                Subsystem(const String& subsystem)                          { return Request("subsystem", subsystem); }
    bool                SetEnv(const String& variable, const String& value);
    bool                Terminal(const String& term, int width, int height);
    bool                Terminal(const String& term, Size sz)                       { return Terminal(term, sz.cx, sz.cy); }
    
    String              Get(int64 size, int sid = 0);
    int64               Get(Stream& out, int64 size, int sid = 0);
    String              GetLine(int maxlen = 65536, int sid = 0);
    int                 GetNow(int sid = 0);
    int                 GetNow(void* buffer, int sid = 0);
    String              GetStdErr(int64 size = 65536)                               { return Get(size, SSH_EXTENDED_DATA_STDERR); }
    int64               GetStdErr(Stream& out, int64 size = 65536)                  { return Get(out, size, SSH_EXTENDED_DATA_STDERR); }
    int64               Put(const String& s, int sid = 0);
    int64               Put(Stream& in, int64 size = 65536, int sid = 0);
    bool                PutNow(char c, int sid = 0);
    int                 PutNow(const void* buffer, int64 size, int sid = 0);
    int64               PutStdErr(const String& err)                                { return Put(err, SSH_EXTENDED_DATA_STDERR); }
    int64               PutStdErr(Stream& err)                                      { return Put(err, err.GetSize(), SSH_EXTENDED_DATA_STDERR); }

    bool                SendEof();
    bool                RecvEof();
    bool                SendRecvEof();
    bool                IsEof();

    bool                SetTerminalSize(int width, int height);
    bool                SetTerminalSize(Size sz)                                    { return SetTerminalSize(sz.cx, sz.cy); }
    bool                SetReadWindowSize(int64 size, bool force = false);
    int64               GetReadWindowSize();
    int64               GetWriteWindowSize();
    int                 GetExitCode();
    String              GetExitSignal();

    bool                IsNullInstance() const                                      { return !channel || !ssh; }
    inline operator     bool() const                                                { return IsNullInstance();}

    Gate<int64, int64>      WhenProgress;

    SshChannel() : Ssh(false) {}
    SshChannel(SshSession& session);
    virtual ~SshChannel();

    SshChannel(SshChannel&&) = default;
    SshChannel& operator=(SshChannel&&) = default;

protected:
    virtual bool        Init() override;
    void                Exit();
    virtual bool        Cleanup(Error& e) override;

    int                 AdjustChunkSize(int64 sz);
    void                Clear();

    int                 Read(void *buffer, int64 len, int sid = 0);
    int                 Read(int sid = 0);
    bool                ReadString(String& s, int64 len, int sid = 0, bool nb = false);
    bool                ReadStream(Stream& s, int64 len, int sid = 0, bool nb = false);
    
    int                 Write(const void* buffer, int64 len, int sid = 0);
    bool                Write(char c, int sid = 0);
    bool                WriteString(const String& s, int64 len, int sid = 0, bool nb = false);
    bool                WriteStream(Stream& s, int64 len, int sid = 0, bool nb = false);

    bool                SendEof0();
    bool                RecvEof0();

    bool                SetWndSz(int64 size, bool force = false);

    int                 SetPtySz(int w, int h);
    int                 SetPtySz(Size sz)                                           { return SetPtySz(sz.cx, sz.cy); }
    
    dword               EventWait(int fd, dword events, int tv = 10);
    bool                ProcessEvents(String& input);
    virtual void        ReadWrite(String& in, const void* out, int out_len);

    One<LIBSSH2_CHANNEL*>   channel;
    LIBSSH2_LISTENER*       listener;
    libssh2_struct_stat     filestat;
    Value                   result;
    int                     exitcode;
    String                  exitsignal;
    int64                   done;
    int64                   total;
    
    enum OpCodes {
        CHINIT,
        CHEXIT,
        CHOPEN,
        CHREQUEST,
        CHSETENV,
        CHREAD,
        CHWRITE,
        CHCLOSE,
        CHWAIT,
        CHEOF,
        CHSTDERR,
        CHRC,
        CHSIG,
        CHWNDSZ,
        CHTRMSZ,
        CHEXEC,
        CHSHELL
    };
};

// Channels.
class Scp : public SshChannel {
public:
    bool    Get(Stream& out, const String& path, Gate<int64, int64> progress = Null);
    bool    operator()(Stream& out, const String& path, Gate<int64, int64> progress = Null)                 { return Get(out, path, progress); }
    String  Get(const String& path, Gate<int64, int64> progress = Null);
    String  operator()(const String& path, Gate<int64, int64> progress = Null)                              { return Get(path, progress); }
    bool    Put(Stream& in, const String& path, long mode, Gate<int64, int64> progress = Null);
    bool    operator()(Stream& in, const String& path, long mode, Gate<int64, int64> progress = Null)       { return Put(in, path, mode, progress); }
    bool    Put(const String& in, const String& path, long mode, Gate<int64, int64> progress = Null);
    bool    operator()(const String& in, const String& path, long mode, Gate<int64, int64> progress = Null) { return Put(in, path, mode, progress); }

    static  AsyncWork<String> AsyncGet(SshSession& session, const String& path, Gate<int64, int64> progress = Null);
    static  AsyncWork<void>   AsyncGet(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress = Null);
    static  AsyncWork<void>   AsyncPut(SshSession& session, String&& data, const String& target, Gate<int64, int64> progress = Null);
    static  AsyncWork<void>   AsyncPut(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress = Null);

    Scp(SshSession& session) : SshChannel(session)                                                      { ssh->otype = SCP; }

private:
    virtual bool Init() override                                                                        { return true; }
    bool Open(int opcode, const String& path, int64 size, long mode);
    enum OpCodes { FGET, FPUT };
};

class SshExec : public SshChannel {
public:
    int         Execute(const String& cmd, Stream& out, Stream& err);
    int         operator()(const String& cmd, Stream& out, Stream& err)                                 { return Execute(cmd, out, err); }

    static      AsyncWork<Tuple<int, String, String>> Async(SshSession& session, const String& cmd);

    SshExec(SshSession& session) : SshChannel(session)                                                  { ssh->otype = EXEC; };

private:
    virtual bool Init() override                                                                        { return true; }
};

class SshTunnel : public SshChannel {
public:
    bool        Connect(const String& host, int port);
    bool        Connect(const String& url);
    bool        Listen(int port, int listen_count = 5)                                                  { return Listen(Null, port, NULL, listen_count); }
    bool        Listen(const String& host, int port, int* bound_port, int listen_count = 5);
    bool        Accept(SshTunnel& listener);

    SshTunnel(SshSession& session) : SshChannel(session)                                                { ssh->otype = TCPTUNNEL; mode = -1; }
    SshTunnel() : SshChannel()                                                                          {}
    
private:
    virtual bool Init() override                                                                        { return true; }
    void         Validate();
    int mode;
    enum OpCodes { CONNECT, LISTEN, ACCEPT };
};

class SshShell : public SshChannel {
public:
    bool        Run(const String& terminal, Size pagesize);
    bool        Run(const String& terminal, int width, int height)                                      { return Run(terminal, {width, height}); }

    bool        Console(const String& terminal);
    
    void        Send(int c)                     { queue.Cat(c);   }
    void        Send(const char* s)             { Send(String(s));}
    void        Send(const String& s)           { queue.Cat(s);   }
    
    SshShell&   PageSize(Size sz)               { if((resized = sz != psize)) psize = sz; return *this;}
    Size        GetPageSize() const             { return psize; }
    Size        GetConsolePageSize();
    
    Event<>                  WhenInput;
    Event<const void*, int>  WhenOutput;

    static AsyncWork<void> AsyncRun(SshSession& session, String terminal, Size pagesize,
                                                Event<SshShell&> in, Event<const String&> out);
    
    SshShell(SshSession& session);
    virtual ~SshShell();
    
    SshShell(SshShell&&) = default;
    SshShell& operator=(SshShell&&) = default;

protected:
    void    Resize();
    void    ConsoleInit();
    void    ConsoleRead();
    void    ConsoleWrite(const void* buffer, int len);
    void    ConsoleRawMode(bool b = true);
    void    ReadWrite(String& in, const void* out, int out_len);
    
private:
    String  queue;
    Size    psize;
    int     mode;
    bool    resized;
#ifdef PLATFORM_WIN32
    DWORD   tflags;
    HANDLE  stdinput;
    HANDLE  stdoutput;
#elif  PLATFORM_POSIX
    termios tflags;
#endif
    enum Modes { GENERIC, CONSOLE };
};
