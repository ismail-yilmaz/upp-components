#ifndef _FTP_Ftp_h_
#define _FTP_Ftp_h_

#include <Core/Core.h>
#include <plugin/pcre/Pcre.h>

#ifdef PLATFORM_WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

#ifdef PLATFORM_POSIX
#include <arpa/inet.h>
#endif

namespace Upp {

class Ftp : NoCopy {
    struct              Reply; // Forward declaration.

public:
    class DirEntry : Moveable<DirEntry> {
    public:
        enum class Style    { UNIX, DOS };
        DirEntry&           User(const String& user)                                { entry(USER) = user; return *this; }
        String              GetName() const                                         { return entry[FILE];  }
        String              GetRealName() const                                     { return entry[PATH];  }
        String              GetOwner() const                                        { return entry[OWNER]; }
        String              GetGroup() const                                        { return entry[GROUP]; }
        int64               GetSize() const                                         { return entry[SIZE];  }
        Time                GetLastModified() const                                 { return entry[TIME];  }
        Style               GetStyle() const                                        { return entry[STYLE].To<Ftp::DirEntry::Style>(); }
        String              GetEntry() const                                        { return entry[ENTRY]; }
        bool                IsDosStyle() const                                      { return GetStyle() == Style::DOS; }
        bool                IsUnixStyle() const                                     { return GetStyle() == Style::UNIX; }
        bool                IsFile() const                                          { return entry[TYPE] == "-"; }
        bool                IsDirectory() const                                     { return entry[TYPE] == "d"; }
        bool                IsSymLink() const                                       { return entry[TYPE] == "l"; }
        bool                IsReadOnly() const                                      { return IsReadable() && !IsWriteable(); }
        bool                IsReadable() const                                      { return entry[USER].IsNull() || entry[USER] != entry[OWNER] ? other[0] : owner[0]; }
        bool                IsWriteable() const                                     { return entry[USER].IsNull() || entry[USER] != entry[OWNER] ? other[1] : owner[1]; }
        bool                IsExecutable() const                                    { return entry[USER].IsNull() || entry[USER] != entry[OWNER] ? other[2] : owner[2]; }

        String              ToString() const                                        { return GetEntry(); }
        String              ToXml() const;

        bool                Parse(const String& dir_entry);

        DirEntry()      {}
        DirEntry(DirEntry&&) = default;
        DirEntry& operator=(DirEntry&&) = default;

    private:
        enum  Id            { USER, FILE, PATH, OWNER, GROUP, SIZE, TIME, STYLE, TYPE, ENTRY, UDATA };
        ValueMap            entry;
        Bits                owner, group, other;
    };

    using DirList = Vector<DirEntry>;

public:
    Ftp&                     Active(bool b = true, int port = 0)                    { mode  = b ? ACTIVE : PASSIVE; listen_port = port; return *this; }
    Ftp&                     Passive()                                              { return Active(false);  }
    Ftp&                     SSL(bool b = true)                                     { ssl = b; return *this; }
    Ftp&                     Utf8(bool b = true)                                    { utf = b; return *this; }
    Ftp&                     NonBlocking(bool b = true)                             { return Timeout(b ? 0 : Null); }
    Ftp&                     ChunkSize(int sz)                                      { chunk_size = clamp(sz, 1024, INT_MAX); return *this; }
    Ftp&                     Timeout(int ms);
    Ftp&                     WaitStep(int ms);

    bool                     Connect(const String& url);
    bool                     Connect(const String& host, int port);
    bool                     Connect(const String& host, int port, const String& user, const String& pwd);
    bool                     Login(const String& user, const String& pwd);
    void                     Disconnect();

    bool                     Do()                                                   { ASSERT(!IsBlocking()); return Do0(); }

    bool                     DirUp()                                                { return Run([=]{ StartCommand(OpCode::CDUP); }); }
    String                   GetDir()                                               { Run([=]{ StartCommand(OpCode::PWD); }); return pick(result); }
    bool                     SetDir(const String& path)                             { return Run([=]{ StartCommand(OpCode::CWD, EncodePath(path)); }); }
    bool                     MakeDir(const String& path)                            { return Run([=]{ StartCommand(OpCode::MKD, EncodePath(path)); }); }
    bool                     RemoveDir(const String& path)                          { return Run([=]{ StartCommand(OpCode::RMD, EncodePath(path)); }); }
    bool                     ListDir(const String& path, DirList& list);
    bool                     Rename(const String& oldpath, const String& newpath)   { return Run([=]{ StartCommand(OpCode::RNFR, Value() << EncodePath(oldpath) << EncodePath(newpath)); }); }
    bool                     Delete(const String& path)                             { return Run([=]{ StartCommand(OpCode::DELE, EncodePath(path)); }); }
    int64                    GetSize(const String& path)                            { Run([=]{ StartCommand(OpCode::SIZE, EncodePath(path)); }); return pick(result); }
    ValueMap                 GetFeatures()                                          { Run([=]{ StartCommand(OpCode::FEAT); }); return pick(result); }
    void                     Abort()                                                { aborted = true; }
    bool                     Noop()                                                 { return Run([=]{ StartCommand(OpCode::NOOP); }); }
    int                      SendCommand(const String& cmd)                         { Run([=]{ StartCommand(OpCode::RAW, cmd); }); return reply.code; }

    String                   Get(const String& path, bool ascii = false);
    bool                     Get(const String& path, Stream& s, bool ascii = false);
    bool                     Put(Stream& s, const String& path, bool ascii = false);
    bool                     Put(const String& s, const String& path, bool ascii = false);
    bool                     Append(Stream &s, const String& path, bool ascii = false);
    bool                     Append(const String& s, const String& path, bool ascii = false);

    Ftp&                     SetPos(int64 pos)                                      { if((restart = pos > 0 && pos <= INT64_MAX)) position = pos; return *this; }
    int64                    GetPos() const                                         { return position; }

    inline int64             GetId() const                                          { return uid; }
    Value                    GetResult() const                                      { return pick(result); }
    int                      GetCode() const                                        { return reply.code; }
    String                   GetReply() const                                       { return reply.text; }
    String                   GetReplyAsXml();
    bool                     IsWorking() const                                      { return status == WORKING; }
    bool                     IsBlocking() const                                     { return control_socket.GetTimeout() != 0; }
    bool                     IsError() const                                        { return status == FAILED || data_error; }
    int                      GetError() const                                       { return error.Get<int>(); }
    String                   GetErrorDesc() const                                   { return error.Get<String>(); }

    dword                    GetWaitEvents() const                                  { return events; }
    void                     AddTo(SocketWaitEvent& w)                              { ASSERT(sock); w.Add(*sock, GetWaitEvents()); }

    static bool              ParseDirList(const String& user, const String& in, Ftp::DirList& out);
    static void              Trace(bool b = true);

    Event<>                  WhenWait;
    Event<>                  WhenReply;
    Gate<const SSLInfo*>     WhenSSLInfo;
    Event<const void*, int>  WhenContent;
    Gate<int64, int64>       WhenProgress;

    // Multithreaded transfer methods.
    static AsyncWork<String> AsyncGet(const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncGet(const String& url, Stream& out, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncPut(String& in,  const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncPut(Stream& in,  const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncAppend(String& in, const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncAppend(Stream& in, const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncGetToFile(const String& url, const String& dest, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncPutFromFile(const String& src, const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncAppendFromFile(const String& src, const String& url, Gate<int64, int64, int64> progress = Null);
    static AsyncWork<void>   AsyncConsumerGet(const String& url, Event<int64, const void*, int> consumer);
    
    Ftp();
    virtual ~Ftp();

    struct Error : Exc {
        int code;
        Error(const String& reason) : code(-1), Exc(reason) {}
        Error(int rc, const String& reason) : code(rc), Exc(reason) {}
        Error(const Reply& reply) : code(reply.code), Exc(reply.text) {}
    };

private:
    enum Mode           { ACTIVE, PASSIVE };
    enum State          { IDLE, WORKING, FINISHED, FAILED };

    enum class OpCode   {
        NONE, DNS,  CONN, PRXY,
        HELO, USER, PASS, ACCT,
        PORT, EPRT, PASV, EPSV,
        PWD,  CWD,  CDUP, MKD,
        RMD,  TYPE, RNFR, RNTO,
        NOOP, QUIT, SSL,  SSL3,
        TLS,  PBSZ, PROT, LIST,
        DELE, RAW,  SIZE, REST,
        RETR, APPE, STOR, MODE,
        FEAT, OPTS, EXIT
    };

    struct Reply {
        enum Flags {
            WAIT    = 0x01,
            SUCCESS = 0x02,
            PENDING = 0x04,
            FAILURE = 0x08,
            HALT    = 0x10,
            ANY     = 0x80
        };
        String          text;
        int             code;
        int             prevcode;
        bool            multiline;

        inline bool     IsWait() const                                          { return code >= 100 && code <= 199; }
        inline bool     IsSuccess() const                                       { return code >= 200 && code <= 299; }
        inline bool     IsPending() const                                       { return code >= 300 && code <= 399; }
        inline bool     IsFailure() const                                       { return code >= 400 && code <= 499; }
        inline bool     IsError() const                                         { return code >= 500 && code <= 599; }
        inline bool     IsMultiline() const                                     { return multiline; }
        void            Clear()                                                 { text = Null; code = 0; multiline = false; }

        const String&   operator*() const                                       { return text; }
        inline Reply&   operator<<(int n)                                       { prevcode = n == 0 ? code : n; code = n; return *this; }
        inline Reply&   operator<<(const String& s)                             { text << pick(s); return *this; }
        inline void     operator=(const Nuller&)                                { Clear(); }

        Reply()                                                                 { Clear(); prevcode = 0; }
        Reply(Reply&&) = default;
        Reply& operator=(Reply&&) = default;
    };

    struct ConnectionInfo {
        enum  class Type { LOCAL, PEER };
        String          addr;
        int             port;
        int             family;
        void            Clear()                                                 { addr = Null; port = 0; family = AF_INET; }
        ConnectionInfo()                                                        { Clear(); }
    };

    TcpSocket           control_socket, data_socket, listener, *sock;
    IpAddrInfo          ipinfo;
    BiVector<Gate<>>    queue;
    Reply               reply;
    ConnectionInfo      connection;
    OpCode              opcode;
    Mode                mode;
    MemReadStream       wstream;
    int                 listen_port;
    Value               result;
    int                 chunk_size;
    String              uname;
    int64               uid;
    String              packet;
    int                 status;
    int64               done;
    int64               total;
    int64               position;
    Tuple<int, String>  error;
    bool                restart;
    bool                connected;
    bool                aborted;
    bool                data_error;
    bool                utf;
    bool                ssl;
    dword               events;

    void                StartConnect(TcpSocket& s, bool dataconn = false);
    void                StartLogin(const String& u, const String& p);
    void                StartSSL(int size);
    void                StartCommand(const OpCode& code, const Value& req = Null);
    void                StartAccept(const OpCode& code, const Value& req);
    void                StartTransfer(const OpCode& code, const Value& req, bool ascii = false);
    void                StartRestart(const OpCode& code, int64 pos);
    void                StartAbort();
    static void         StartAsync(const OpCode& code, const String& url, Stream& io, Gate<int64,
                                    int64, int64> progress, Event<int64, const void*, int> consumer = Null);


    void                CheckAbort()                                                { if(aborted) throw Error(""); }
    void                Check(TcpSocket& s);
    void                CloseSockets();
    int                 GetReplyCode(const String& s);
    bool                PutGet(const String& s, byte r = Reply::SUCCESS);
    bool                Run(const Event<>& cmd);
    bool                Do0();
    bool                WriteData(Stream& s);
    bool                ReadAscii(const Event<const void*, int>& fn = Null, bool log = false);
    bool                ReadBinary(const Event<const void*, int>& fn = Null);
    bool                SendData(const OpCode& code, const Value& req, Stream& s, bool ascii = false);
    bool                RecvData(const OpCode& code, const Value& req, const Event<const void*, int>&& fn, bool ascii = false, bool log = false);
    bool                IsDataEof();
    void                ScanFileSize();
    void                ParseFeatureList();
    void                GetSSLInfo(TcpSocket& s);
    void                SetPassiveMode(const OpCode& code);
    void                SetActiveMode(const OpCode& code);
    ConnectionInfo      GetConnectionInfo(ConnectionInfo::Type t);
    static String       EncodePath(const String& path);
    static String       DecodePath(const String& path);
    static String       GetDelimitedString(const String& s, int delim1, int delim2);
};
}
#endif

