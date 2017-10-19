#ifndef _FTP_Ftp_h_
#define _FTP_Ftp_h_

#include <Core/Core.h>

#ifdef PLATFORM_WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#endif

#ifdef PLATFORM_POSIX
#include <arpa/inet.h>
#endif

//Std C++
#include <bitset>

namespace Upp {

class Ftp : private NoCopy {
public:
    class DirEntry final : MoveableAndDeepCopyOption<Ftp::DirEntry> {
    public:
        enum class Style { UNIX, DOS };
        DirEntry&       User(const String& user)                        { entry(USER) = user; return *this; }
        String          GetName() const                                 { return entry[FILE];  }
        String          GetOwner() const                                { return entry[OWNER]; }
        String          GetGroup() const                                { return entry[GROUP]; }
        int64           GetSize() const                                 { return entry[SIZE];  }
        Time            GetLastModified() const                         { return entry[TIME];  }
        Style           GetStyle() const                                { return entry[STYLE].To<Ftp::DirEntry::Style>(); }
        String          GetEntry() const                                { return entry[ENTRY]; }
        bool            IsFile() const                                  { return entry[TYPE] == "file"; }
        bool            IsDirectory() const                             { return entry[TYPE] == "dir"; }
        bool            IsSymLink() const                               { return entry[TYPE] == "link"; }
        bool            IsReadable() const                              { return entry[USER].IsNull() || entry[USER] != entry[OWNER] ? other[0] : owner[0]; }
        bool            IsWriteable() const                             { return entry[USER].IsNull() || entry[USER] != entry[OWNER] ? other[1] : owner[1]; }
        bool            IsExecutable() const                            { return entry[USER].IsNull() || entry[USER] != entry[OWNER] ? other[2] : owner[2]; }

        bool            Parse(const String& dir_entry);

        DirEntry()      {}
        DirEntry(const DirEntry& src, int) { Clone(clone(src)); }

    private:
        enum  Id        { USER, FILE, OWNER, GROUP, SIZE, TIME, STYLE, TYPE, ENTRY, UDATA };
        ValueMap        entry;
        std::bitset<3>  owner, group, other;
        void            Clone(const DirEntry& src);
     };
    typedef Vector<DirEntry> DirList;

    class Request final : Moveable<Ftp::Request>, DeepCopyOption<Ftp::Request> {
    public:
        Request&        Server(const String& host, int port = 21)        { info("host") = host; info("port") = port; return *this; }
        Request&        File(const String& local, const String& remote)  { info("local_file") = local; info("remote_file") = remote; return *this; }
        Request&        Auth(const String& user, const String& password) { info("user") = user; info("password") = password; return *this; }
        Request&        Active(bool b = true)                            { info("active") = b; return *this; }
        Request&        SSL(bool b = true)                               { info("ssl") = b; return *this; }
        Request&        Ascii(bool b = true)                             { info("ascii") = b; return *this; }
        Request&        Timeout(int ms)                                  { info("timeout") = ms; return *this; }
        Request&        Priority(int percent)                            { info("priority") = percent; return *this; }
        Request&        Restart(int64 pos = 0)                           { info("rest") = pos; return *this; }
        Request&        DeleteFailed(bool b = true)                      { info("del") = b; return *this; }
        Request&        UserData(const Value& v)                         { info("data") = pick(v); return *this; }
        const Value&    operator[](const Value& key) const               { return info[key]; }

        Request(const String& host, int port = 21) : Request()           { Server(host, port); }
        Request() { info("timeout") = int(60000); info("priority") = int(-1); info("rest") = int64(0); }
        Request(const Request& src, int) { info = clone(src.info); }

    private:
        ValueMap info;
    };

    class Result final : Moveable<Ftp::Result> {
    public:
        String          GetName() const                                 { return file; }
        int             GetError() const                                { return rc; }
        String          GetErrorDesc() const                            { return msg; }
        int64           GetRestartPos() const                           { return pos; }
        bool            IsStarted() const                               { return status == Status::STARTED;  }
        bool            IsRestarted() const                             { return status == Status::RESTARTED; }
        bool            IsSuccess() const                               { return status == Status::SUCCESS;  }
        bool            IsFailure() const                               { return status == Status::FAILURE;  }
        bool            InProgress() const                              { return status == Status::PROGRESS; }
        int             GetId() const                                   { return id;  }
        int64           GetTotal() const                                { return total; }
        int64           GetDone() const                                 { return done; }
        const Value&    GetUserData() const                             { return data; }
        Value&          operator()()                                    { return data; }

        Result()        { id = 0; rc = 0; total = 0; done = 0; pos = 0; };

    private:
        enum class Status { STARTED, SUCCESS, FAILURE, PROGRESS, RESTARTED };
        int    id;
        int    rc;
        bool   resume;
        int64  total;
        int64  done;
        int64  pos;
        String file;
        String msg;
        Status status;
        Value  data;
        friend Ftp::Result FtpIO(const Ftp::Request& request, Gate<int64, int64> progress, Event<> whenwait, bool put);
        friend Ftp::Result FtpAsyncIO(Ftp::Request request, Event<Ftp::Result> progress, bool put);
    };

public:
    typedef             Ftp CLASSNAME;

    Ftp&                User(const String& user, const String& pass)        { user_id = user; user_password = pass; return *this; }
    Ftp&                SSL(bool b = true)                                  { ftps = b; return *this; }
    Ftp&                Active(bool b = true)                               { transfer_mode = b ? ACTIVE : PASSIVE; return *this; }
    Ftp&                Passive()                                           { Active(false); return *this; }
    Ftp&                Timeout(int ms)                                     { control_socket.Timeout(ms); return *this; }
    Ftp&                WaitStep(int ms)                                    { control_socket.WaitStep(ms); return *this; }
    Ftp&                ChunkSize(int size)                                 { if(size > 0) chunk_size = size; return *this; }
    Ftp&                Restart(int64 pos)                                   { position = pos; return *this; }

    bool                Connect(const String& host, int port = 21);
    void                Disconnect();

    String              GetDir();
    bool                SetDir(const String& path);
    bool                DirUp();
    bool                ListDir(const String& path, DirList& list, Gate1<String> progress = false);
    bool                MakeDir(const String& path);
    bool                RemoveDir(const String& path);

    bool                Get(const String& path, Stream& out, Gate2<int64, int64> progress = false, bool ascii = false);
    bool                Put(Stream& in, const String& path, Gate2<int64, int64> progress = false, bool ascii = false);
    bool                Append(Stream& in, const String& path, Gate2<int64, int64> progress = false, bool ascii = false);
    bool                Info(const String& path, DirEntry& info);
    bool                Rename(const String& oldname, const String& newname);
    bool                Delete(const String& path);

    bool                Noop();
    void                Abort();
    ValueMap            GetFeatures();
    int                 SendCommand(const String& cmd)                      { PutGet(cmd); return reply_code; }

    bool                InProgress() const                                  { return progress; }

    Event<>             WhenWait;

    int64               GetSize(const String& path);
    int64               GetRestartPos() const                               { return position; }
    TcpSocket&          GetSocket()                                         { return control_socket; }
    int                 GetCode() const                                     { return reply_code; }
    String              GetReply() const                                    { return reply; }
    String              GetReplyAsXml();

    static int          GetWorkerCount();
    static void         AbortWorker(int id);
    static bool         IsWorkerAborted(int id);

    static void         Trace(bool b = true);

private:
    enum Commands       { GET, PUT, APPEND };
    enum Address        { LOCAL, PEER };
    enum Encryption     { NONE, SSL3, TLS };
    enum Type           { ASCII, BINARY };
    enum Mode           { ACTIVE, PASSIVE };

    TcpSocket           control_socket, *target;
    String              ftp_host;
    int                 ftp_port;
    String              user_id;
    String              user_password;
    String              reply;
    int                 reply_code;
    int                 data_type;
    int                 transfer_mode;
    int                 chunk_size;
    int64               position;
    bool                ftps;
    bool                progress;
    bool                aborted;

    Gate1<String>       WhenList;
    Gate2<int64, int64> WhenData;

    bool                ReplyIsWait() const                                 { return reply_code >= 100 && reply_code <= 199; }
    bool                ReplyIsSuccess() const                              { return reply_code >= 200 && reply_code <= 299; }
    bool                ReplyIsPending() const                              { return reply_code >= 300 && reply_code <= 399; }
    bool                ReplyIsFailure() const                              { return reply_code >= 400 && reply_code <= 499; }
    bool                ReplyIsError() const                                { return reply_code >= 500 || reply_code == -1;  }

    String              DecodePath(const String& path);
    String              EncodePath(const String& path);

    int                 GetRandomPort(int min, int max);
    bool                GetSockAddr(int type, int& family, String& ip, int& port);
    bool                SetError(const String& e);

    bool                InitFtps(int size);
    int                 GetReplyCode(const String& s);
    bool                PutGet(const String& s, bool nolog = false);
    bool                SetDataType(int type);
    bool                SetTransferMode(String& addr, int& port, int& family);
    bool                TransferData(int cmd, const String& request, Stream& file, int64 size, int type);
    bool                ResumeTransfer(int cmd, Stream& s);
    bool                GetData(TcpSocket& socket, Stream& out, int64 sz, int type, bool log = false);
    bool                PutData(TcpSocket& socket, Stream& in, int type);
    bool                SocketAccept(TcpSocket& s, const String& host, int port, int family);
    bool                SocketConnect(TcpSocket& s, const String& host, int port, bool data = false);

public:
    Ftp();
    virtual ~Ftp();
};

// Single-threaded convenience functions.
Ftp::Result FtpGet(const Ftp::Request& request, Gate<int64, int64> progress = false, Event<> whenwait = CNULL);
Ftp::Result FtpPut(const Ftp::Request& request, Gate<int64, int64> progress = false, Event<> whenwait = CNULL);
Ftp::Result FtpIO(const Ftp::Request& request, Gate<int64, int64> progress = false, Event<> whenwait = CNULL, bool put = false);

// Multi-threaded convenience functions.
Ftp::Result FtpAsyncGet(Ftp::Request& request, Event<Ftp::Result> progress = CNULL);
Ftp::Result FtpAsyncPut(Ftp::Request& request, Event<Ftp::Result> progress = CNULL);
Ftp::Result FtpAsyncIO(Ftp::Request request, Event<Ftp::Result> progress = CNULL, bool put = false);

// Misc
bool ParseFtpDirEntry(const String& in, Ftp::DirList& out);
}
#endif
