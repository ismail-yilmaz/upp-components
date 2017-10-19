topic "NetProxy";
[2 $$0,0#00000000000000000000000000000000:Default]
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[{_}%EN-US 
[ {{10000@(113.42.0) [s0; [*@7;4 NetProxy]]}}&]
[s1;@(0.0.255)3%- &]
[s1;:Upp`:`:NetProxy`:`:class:%- [@(0.0.255)3 class][3 _][*3 NetProxy]&]
[s2; This delegate class encapsulates two widely used network proxy 
protocols: Http tunneling and SOCKS.&]
[s2; &]
[s2;i150;O0; Uses HTTP`_CONNECT method for [^https`:`/`/en`.wikipedia`.org`/wiki`/HTTP`_tunnel`?oldformat`=true^ h
ttp tunneling].&]
[s2;i150;O0; Encapsulates SOCKS proxy protocol version 4/4a, and 
version 5, as defined in [^https`:`/`/www`.ietf`.org`/rfc`/rfc1928`.txt^ RFC 
1928] and [^https`:`/`/www`.ietf`.org`/rfc`/rfc1929`.txt^ RFC 1929]. 
&]
[s2;i150;O0; In SOCKS mode, NetProxy can work with both IPv4 and 
IPv6 address families.&]
[s2;i150;O0; In SOCKS mode, NetProxy allows BIND requests.&]
[s2;i150;O0; In SOCKS mode, NetProxy allows remote name lookups (DNS).&]
[s2;i150;O0; Supports both synchronous and asynchronous operation 
modes.&]
[s2;i150;O0; Allows SSL connections to the target machine (not to 
proxy itself) in both Http and SOCKS modes.&]
[s2; &]
[s2; Note: In SOCKS mode, NetProxy currently does not allow UDP association.&]
[s3;%- &]
[ {{10000F(128)G(128)@1 [s0; [* Public Method List]]}}&]
[s3;%- &]
[s5;:Upp`:`:NetProxy`:`:Attach`(Upp`:`:TcpSocket`&`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&
]_[* Attach]([_^Upp`:`:TcpSocket^ TcpSocket][@(0.0.255) `&]_[*@3 sock])&]
[s2; Sets client socket. Returns `*this for methods chaining.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Timeout`(int`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&]_[* T
imeout]([@(0.0.255) int]_[*@3 ms])&]
[s2; Sets timeout interval for overall operation. Default timeout 
value is 60000 miliseconds (one minute). Returns `*this for method 
chaining.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:NonBlocking`(bool`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&
]_[* NonBlocking]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2; Enables or disables non`-blocking mode.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:SSL`(bool`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&]_[* SSL](
[@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2; Enables or disables SSL connection. Returns `*this for method 
chaining. This switch will cause Netproxy client to start an 
SSL session only with the specified target machine, and not with 
the proxy server itself. Initial connection and subsequent authorization 
requests to the proxy server will be sent over an unencrypted 
channel.&]
[s6; Requires Core/SSL package.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Lookup`(bool`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&]_[* L
ookup]([@(0.0.255) bool]_[@3 b]_`=_[@(0.0.255) true])&]
[s2; Enables or disables remote domain name resolving feature of 
SOCKS protocol. Returns `*this for method chaining. This feature 
is introduced in SOCKS protocol version 4a, and not supported 
by every socks server. Disabled by default.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Http`(`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&]_[* Http]()
&]
[s5;:Upp`:`:NetProxy`:`:Socks4`(`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&]_[* Socks
4]()&]
[s5;:Upp`:`:NetProxy`:`:Socks5`(`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&]_[* Socks
5]()&]
[s2; Sets the proxy protocol. NetProxy supports Socks protocol version 
4/4a/5, and http tunneling. Default is http tunneling. Returns 
`*this for methods chaining.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Auth`(const Upp`:`:String`&`):%- [_^Upp`:`:NetProxy^ NetProxy][@(0.0.255) `&
]_[* Auth]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 user])&]
[s5;:Upp`:`:NetProxy`:`:Auth`(const Upp`:`:String`&`,const Upp`:`:String`&`):%- [_^Upp`:`:NetProxy^ N
etProxy][@(0.0.255) `&]_[* Auth]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 user], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 pass])&]
[s2; Sets user credentials. Returns `*this for methods chaining.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Proxy`(const Upp`:`:String`&`,int`):%- [_^Upp`:`:NetProxy^ NetP
roxy][@(0.0.255) `&]_[* Proxy]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 host], [@(0.0.255) int]_[*@3 port])&]
[s2; Sets th [%-*@3 host] name and [%-*@3 port] number of the proxy server 
to be connected. Returns `*this for methods chaining.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Connect`(const Upp`:`:String`&`,int`):%- [@(0.0.255) bool]_[* Con
nect]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], 
[@(0.0.255) int]_[*@3 port])&]
[s5;:Upp`:`:NetProxy`:`:Connect`(int`,const Upp`:`:String`&`,int`):%- [@(0.0.255) bool]_
[* Connect]([@(0.0.255) int]_[*@3 type], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 host], [@(0.0.255) int]_[*@3 port])&]
[s2; Attempts a proxy connection to target machine at [%-*@3 host:port]. 
Preferred proxy type can be specified using the [%-*@3 type] parameter 
which can be one of the following: HTTP, SOCKS4, SOCKS5. Returns 
true on success. In non`-blocking mode this method will return 
immediately. Once this happens the operation can be performed 
asynchronously by using [^topic`:`/`/NetProxy`/src`/Upp`_NetProxy`$en`-us`#Upp`:`:NetProxy`:`:Do`(`)^ D
o()] method.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Bind`(const Upp`:`:String`&`,int`):%- [@(0.0.255) bool]_[* Bind](
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], 
[@(0.0.255) int]_[*@3 port])&]
[s5;:Upp`:`:NetProxy`:`:Bind`(int`,const Upp`:`:String`&`,int`):%- [@(0.0.255) bool]_[* B
ind]([@(0.0.255) int]_[*@3 type], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 host], [@(0.0.255) int]_[*@3 port])&]
[s2; Attempts a Socks BIND (`"accept`") request for incoming connections 
from the target machine designated at [%-*@3 host:port] .Preferred 
proxy type can be specified using the [%-*@3 type] parameter which 
can be one of the following: SOCKS4, SOCKS5. Returns true on success. 
In non`-blocking mode this method will return immediately. Once 
this happens the operation can be performed asynchronously by 
using [^topic`:`/`/NetProxy`/src`/Upp`_NetProxy`$en`-us`#Upp`:`:NetProxy`:`:Do`(`)^ D
o()] method.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Do`(`):%- [@(0.0.255) bool]_[* Do]()&]
[s2; Progresses the scheduled proxy operation in a non`-blocking 
way. Returns true if the processing is not finished.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:WhenDo:%- [_^Upp`:`:Event^ Event]<>_[* WhenDo]&]
[s2; Dispatched each time [^topic`:`/`/NetProxy`/src`/Upp`_NetProxy`$en`-us`#Upp`:`:NetProxy`:`:Do`(`)^ D
o()] method exits. Useful for updating information or showing 
progress in GUI.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:WhenBound:%- [_^Upp`:`:Event^ Event]<[_^Upp`:`:String^ String], 
[@(0.0.255) int]>_[* WhenBound]&]
[s2; This event will be dispatched after a successful Socks BIND 
request. It will contain the IP and port information to be passed 
to the target machine (peer) that made the connection request.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:GetSocket`(`):%- [_^Upp`:`:TcpSocket^ TcpSocket][@(0.0.255) `&]_[* G
etSocket]()&]
[s2; Returns a reference to the delegated TcpSocket.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:IsError`(`)const:%- [@(0.0.255) bool]_[* IsError]()_[@(0.0.255) con
st]&]
[s2; Returns true if the operation has failed.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:GetError`(`)const:%- [@(0.0.255) int]_[* GetError]()_[@(0.0.255) co
nst]&]
[s2; Returns last error code, if any.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:GetErrorDesc`(`)const:%- [_^Upp`:`:String^ String]_[* GetErrorDes
c]()_[@(0.0.255) const]&]
[s2; Returns last error message, if any.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:Trace`(bool`):%- [@(0.0.255) static void]_[* Trace]([@(0.0.255) boo
l]_[*@3 b])&]
[s2; Enables or disables logging of NetProxy.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:TraceVerbose`(bool`):%- [@(0.0.255) static 
void]_[* TraceVerbose]([@(0.0.255) bool]_[*@3 b])&]
[s2; Same as [^topic`:`/`/NetProxy`/src`/Upp`_NetProxy`$en`-us`#Upp`:`:NetProxy`:`:Trace`(bool`)^ T
race()] but with packet information.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0; [* Constructor detail]]}}&]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:NetProxy`(`):%- [* NetProxy]()&]
[s2; Default constructor.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:NetProxy`(Upp`:`:TcpSocket`&`):%- [* NetProxy]([_^Upp`:`:TcpSocket^ T
cpSocket][@(0.0.255) `&]_[*@3 sock])&]
[s2; Constructor overload. Sets client socket to [%-*@3 sock].&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:NetProxy`:`:NetProxy`(Upp`:`:TcpSocket`&`,const Upp`:`:String`&`,int`):%- [* N
etProxy]([_^Upp`:`:TcpSocket^ TcpSocket][@(0.0.255) `&]_[*@3 sock], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 proxy], 
[@(0.0.255) int]_[*@3 port])&]
[s2; Constructor overload.  Sets the client socket and proxy info.&]
[s0; &]
[s3;%- &]
[ {{3434:828:5738h17;t/8b/8@3-2 [s0;=%- [*@7$3;3 NetProxy Error Codes and Descriptions]]
::t/15b/15@2-1 [s0;%- ]
:: [s0;%- ]
::t/8b/8@(191) [s0;=%- Error]
:: [s0;= Code]
:: [s0;= Description]
::t/15b/15@2 [s0;%- NO`_SOCKET`_ATTACHED]
:: [s0;= 10000]
:: [s0; No client to serve (No socket attached).]
:: [s0;%- HOST`_NOT`_SPECIFIED]
:: [s0;= 10001]
:: [s0; Proxy address or port not specified.]
:: [s0;%- TARGET`_NOT`_SPECIFIED]
:: [s0;= 10002]
:: [s0; Target address or port not specified.]
:: [s0;%- DNS`_FAILED]
:: [s0;= 10003]
:: [s0; Couldn`'t resolve address.]
:: [s0;%- CONNECTION`_FAILED]
:: [s0;= 10004]
:: [s0; Couldn`'t connect to proxy server.]
:: [s0;%- SSL`_FAILED]
:: [s0;= 10005]
:: [s0; Couldn`'t start SSL negotioation.]
:: [s0;%- INVALID`_PACKET]
:: [s0;= 10006]
:: [s0; Invalid packet received.]
:: [s0;%- SOCKET`_FAILURE]
:: [s0;= 10007]
:: [s0; Socket error occured.]
:: [s0;%- ABORTED]
:: [s0;= 10008]
:: [s0; Operation was aborted.]
:: [s0;%- CONNECTION`_TIMED`_OUT]
:: [s0;= 10009]
:: [s0; Connection timed out.]
:: [s0;%- HTTPCONNECT`_FAILED]
:: [s0;= 10010]
:: [s0; Http CONNECT method failed.]
:: [s0; HTTPCONNECT`_NOBIND]
:: [s0;= 10011]
:: [s0; BINDing is not possible in Http tunneling. Consider using Socks 
protocol.]
:: [s0; SOCKS4`_REQUEST`_FAILED]
:: [s0;= 091]
:: [s0; Request rejected or failed.]
:: [s0; SOCKS4`_CLIENT`_NOT`_REACHABLE]
:: [s0;= 092]
:: [s0; Request failed. Client is not running identd (or not reachable 
from the server).]
:: [s0; SOCKS4`_AUTHENTICATION`_FAILED]
:: [s0;= 093]
:: [s0; Request failed. Cilent`'s identd could not confirm the user 
ID string in the request.]
:: [s0; SOCKS4`_ADDRESS`_TYPE`_NOT`_SUPPORTED]
:: [s0;= 094]
:: [s0; Socks4 protocol doesn`'t support IP version 6 address family. 
Considers using Socks5 protocol instead.]
:: [s0; SOCKS5`_GENERAL`_FAILURE]
:: [s0;= 001]
:: [s0; General failure.]
:: [s0; SOCKS5`_CONNECTION`_NOT`_ALLOWED]
:: [s0;= 002]
:: [s0; Connection not allowed by the ruleset.]
:: [s0; SOCKS5`_NETWORK`_UNREACHABLE]
:: [s0;= 003]
:: [s0; Network unreachable.]
:: [s0; SOCKS5`_TARGET`_UNREACHABLE]
:: [s0;= 004]
:: [s0; Target machine unreachable.]
:: [s0; SOCKS5`_CONNECTION`_REFUSED]
:: [s0;= 005]
:: [s0; Connection refused by the destination host.]
:: [s0; SOCKS5`_TTL`_EXPIRED,]
:: [s0;= 006]
:: [s0; TTL expired.]
:: [s0; SOCKS5`_COMMAND`_NOT`_SUPPORTED]
:: [s0;= 007]
:: [s0; Command not supported / protocol error.]
:: [s0; SOCKS5`_ADDRESS`_TYPE`_NOT`_SUPPORTED]
:: [s0;= 008]
:: [s0; Address type not supported.]
:: [s0; SOCKS5`_INVALID`_AUTHENTICATION`_METHOD]
:: [s0;= 255]
:: [s0; Invalid authentication method. No acceptable methods were offered.]
:: [s0; SOCKS5`_AUTHENTICATION`_FAILED]
:: [s0;= 256]
:: [s0; Authentication failed.]}}&]
[s0; ]]