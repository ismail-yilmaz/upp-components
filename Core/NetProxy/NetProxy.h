#ifndef _NetProxy_NetProxy_h
#define _NetProxy_NetProxy_h

#include <Core/Core.h>
#ifdef PLATFORM_POSIX
#include <arpa/inet.h>
#endif

namespace Upp {

class NetProxy {

    enum status         { IDLE, WORKING, FINISHED, FAILED };
    enum ProxyCommands  { CONNECT = 0x01, BIND = 0x02 };
    enum Socks5Packets  { SOCKS5_HELO, SOCKS5_AUTH, SOCKS5_REQUEST };

    TcpSocket*          socket;
    IpAddrInfo          ipinfo;
    String              packet;
    int                 packet_length;
    int                 proxy_type;
    String              proxy_host;
    int                 proxy_port;
    String              proxy_user;
    String              proxy_password;
    String              target_host;
    int                 target_port;
    int                 status;
    int                 start_time;
    int                 timeout;
    int                 timeout_backup;
    byte                command;
    int                 packet_type;
    bool                async;
    bool                lookup;
    bool                ssl;
    bool                bound;
    dword               events;
    Tuple<int, String>  error;
    Function<bool()>    IsEof;
    BiVector<Gate<>>    queue;
    struct sockaddr_storage bound_addr;

    struct Error : Exc {
        int code;
        Error() : code(-1), Exc(Null) {}
        Error(const String& reason) : code(-1), Exc(reason) {}
        Error(int rc, const String& reason) : code(rc), Exc(reason) {}
    };

    union  Reply {
        struct Helo {
            byte version;
            byte method;
        };
        struct Auth {
            byte version;
            byte status;
        };
        struct Socks4 {
            byte    version;
            byte    status;
            uint16  port;
            uint32  address;

        };
        struct Socks5 {
            byte    version;
            byte    status;
            byte    reserved;
            byte    addrtype;
            union {
                byte   ipv4[4];
                byte   ipv6[16];
                byte   namelen;
            };
        };
    };

    static const char*  GetMsg(int code);
    void                SetError(int code)                              { throw Error(code, GetMsg(code)); }
    bool                Init();
    bool                Exit();
    bool                Dns();
    bool                Connect();
    bool                Get();
    bool                Put();
    void                PutGet();
    void                StartSSL();
    void                Check();
    bool                Run();
    bool                IsTimeout()                                     { return msecs(start_time) >= timeout; }
    void                ParseBoundAddr();

    void                HttpcConnect();
    bool                HttpcRequest();
    bool                HttpcParseReply();
    bool                HttpcIsEof();

    bool                SocksStart();
    bool                SocksCommand(int cmd);

    void                Socks4Connect(int cmd);
    bool                Socks4Request();
    bool                Socks4ParseReply();
    bool                Socks4IsEof();

    void                Socks5Connect(int cmd);
    bool                Socks5Request();
    bool                Socks5ParseReply();
    bool                Socks5IsEof();

public:
    NetProxy&           Attach(TcpSocket& sock)                         { socket  = &sock; return *this; }
    NetProxy&           Timeout(int ms)                                 { if(ms < 100) timeout = 100; else timeout = ms; return *this; }
    NetProxy&           Lookup(bool b = true)                           { lookup  = b; return *this; }
    NetProxy&           NonBlocking(bool b = true)                      { async   = b; return *this; }
    NetProxy&           SSL(bool b = true)                              { ssl     = b; return *this; }
    NetProxy&           Auth(const String& user)                        { return Auth(user, Null);   }
    NetProxy&           Auth(const String& user, const String& pass)    { proxy_user = user; proxy_password = pass; return *this; }
    NetProxy&           Proxy(const String& host, int port)             { proxy_host = host; proxy_port = port; return *this; }

    NetProxy&           Http()                                          { proxy_type = HTTP;   return *this; }
    NetProxy&           Socks4()                                        { proxy_type = SOCKS4; return *this; }
    NetProxy&           Socks5()                                        { proxy_type = SOCKS5; return *this; }

    bool                Connect(const String& host, int port)           { return Connect(proxy_type, host, port); }
    bool                Connect(int type, const String& host, int port);

    bool                Bind(const String& host, int port)              { return Bind(proxy_type, host, port); }
    bool                Bind(int type, const String& host, int port);

    bool                Do();
    Event<>             WhenDo;

    Event<String, int>  WhenBound;

    TcpSocket&          GetSocket()                                     { ASSERT(socket); return *socket; }
    dword               GetWaitEvents() const                           { return events; }
    NetProxy&           AddTo(SocketWaitEvent& e)                       { ASSERT(socket); e.Add(*socket, events); return *this; }
    
    bool                IsError() const                                 { return status == FAILED; }
    int                 GetError() const                                { return error.Get<int>(); }
    String              GetErrorDesc() const                            { return error.Get<String>(); }

    static void         Trace(bool b = true);
    static void         TraceVerbose(bool b = true);

    NetProxy();
    NetProxy(TcpSocket& sock) : NetProxy()                                      { socket = &sock; }
    NetProxy(TcpSocket& sock, const String& proxy, int port) : NetProxy(sock)   { Proxy(proxy, port); }
    virtual ~NetProxy();

    enum ProxyTypes     { SOCKS4 = 0x04, SOCKS5, HTTP };
    enum ErrorCodes {
        // NetProxy errors.
        NO_SOCKET_ATTACHED = 10000,
        HOST_NOT_SPECIFIED,
        TARGET_NOT_SPECIFIED,
        DNS_FAILED,
        CONNECTION_FAILED,
        SSL_FAILED,
        INVALID_PACKET,
        SOCKET_FAILURE, ABORTED,
        CONNECTION_TIMED_OUT,

        // HTTP_CONNECT
        HTTPCONNECT_FAILED = 10010,
        HTTPCONNECT_NOBIND,

        // Socks4 Protocol errors.
        SOCKS4_REQUEST_FAILED = 91,
        SOCKS4_CLIENT_NOT_REACHABLE,
        SOCKS4_AUTHENTICATION_FAILED,
        SOCKS4_ADDRESS_TYPE_NOT_SUPPORTED,

       // Socks5 protocol errors.
        SOCKS5_GENERAL_FAILURE = 1,
        SOCKS5_CONNECTION_NOT_ALLOWED,
        SOCKS5_NETWORK_UNREACHABLE,
        SOCKS5_TARGET_UNREACHABLE,
        SOCKS5_CONNECTION_REFUSED,
        SOCKS5_TTL_EXPIRED,
        SOCKS5_COMMAND_NOT_SUPPORTED,
        SOCKS5_ADDRESS_TYPE_NOT_SUPPORTED,
        SOCKS5_INVALID_AUTHENTICATION_METHOD = 255,
        SOCKS5_AUTHENTICATION_FAILED = 256
    };
};
}
#endif