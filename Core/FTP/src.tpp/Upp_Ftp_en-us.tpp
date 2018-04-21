topic "Ftp";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[i448;a25;kKO9;2 $$3,0#37138531426314131252341829483370:item]
[0 $$4,0#96390100711032703541132217272105:end]
[H6;0 $$5,0#05600065144404261032431302351956:begin]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[ $$0,0#00000000000000000000000000000000:Default]
[{_}%EN-US 
[ {{10000@(113.42.0) [s0; [*@7;4 Ftp]]}}&]
[s1;%- &]
[s1;:Upp`:`:Ftp`:`:class:%- [@(0.0.255)3 class][3 _][*3 Ftp ][3 :][*3  ][@(0.0.255)3 private][3 _][*@3;3 N
oCopy]&]
[s2; This class provides an easy`-to`-use interface to the client 
side of the File Transfer Protocol as specified in [^http`:`/`/www`.ietf`.org`/rfc`/rfc959`.txt^ R
FC 959], with support for a number of advanced capabilities. 
Ftp class supports:&]
[s2; &]
[s2;i150;O0; Time`-constrained blocking, and non blocking operation 
modes.&]
[s2;i150;O0; Multithreaded file transfers, using worker threads.&]
[s2;i150;O0; IPv6 connections and NATs, as specified in [^http`:`/`/tools`.ietf`.org`/html`/rfc2428^ R
FC 2428.]&]
[s2;i150;O0; Ftp over TLS/SSL (FTPS), as specified in [^http`:`/`/tools`.ietf`.org`/html`/rfc2228^ R
FC 2228].&]
[s2;i150;O0; Feature negotiation mechanism as specifien in [^http`:`/`/tools`.ietf`.org`/html`/rfc2389^ R
FC 2389].&]
[s2;i150;O0; Parsing of UNIX and DOS style directory listings.&]
[s2;i150;O0; Extending the existing functionality of Ftp class.&]
[s2;i150;O0; Transfer restart/resume mechanism, as specified in [^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ R
FC 3959].&]
[s2;i150;O0; UTF`-8 encoded path names. As specified in [^https`:`/`/tools`.ietf`.org`/html`/draft`-ietf`-ftpext`-utf`-8`-option`-00^ t
his draft].&]
[s1;%- &]
[ {{10000F(128)G(128)@1 [s0; [* URL specification]]}}&]
[s4;%- &]
[s2; Ftp class can use a URL scheme. Syntax of the ftp URL is as 
follows:&]
[s2; &]
[s2;   [C@3 `[ftp`|ftps`]://`[user:password`@`]host`[:port`]`[/path`]`[?arg`=value`]][C .]&]
[s2; &]
[ {{4133:5867<288;>544;l/26r/26t/14b/14@1-1 [s0;= [*2 Optional Arguments]]
::l/25r/25t/15b/15@2 [s0;%- ]
::l/26r/26t/14b/14@(178) [s0;= [2 Argument]]
:: [s0;= [2 Possible Value(s)]]
::l/25r/25t/15b/15@2 [s0;= [*C2 timeout]]
:: [s0; [C2 A string representing the timeout value in miliseconds (int)]]
:: [s0;= [*C2 chunksize]]
:: [s0; [C2 A string representing the chunk size for binary data transfers 
(int)]]
:: [s0;= [*C2 type]]
:: [s0; [C2 ascii, binary]]
:: [s0;= [*C2 mode]]
:: [s0; [C2 active, passive]]
:: [s0;= [*C2 utf8]]
:: [s0; [C2 on, off]]
:: [s0;= [*C2 pos]]
:: [s0; [C2 A string representing the restart position of the data transfer 
(int64)]]}}&]
[s0;%- &]
[s2;%- Note that the [C@3 path] variable and [%%*C type ]argument are 
only used with the multithreaded transfer methods.&]
[s1;%- &]
[ {{10000F(128)G(128)@1 [s0; [* Public Method List]]}}&]
[s4;%- &]
[s3;:Ftp`:`:Trace`(bool`):%- [@(0.0.255) static] [@(0.0.255) void]_[* Trace]([@(0.0.255) bool
]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2; Enables logging of Ftp client&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Active`(bool`,int`):%- [_^Upp`:`:Ftp^ Ftp][@(0.0.255) `&]_[* Active]([@(0.0.255) b
ool]_[*@3 b]_`=_[@(0.0.255) true], [@(0.0.255) int]_[*@3 port]_`=_[@3 0])&]
[s2; Enables or disables the FTP active data transfer mode, using 
EPRT or PORT command. Returns `*this for method chaining. [%-*@3 port] 
can be used to specify a port for incoming ftp data connection. 
Alternatively, to use a randomized port number between 49152 
and 65535, set the [%-*@3 port] parameter to 0 (default behaviour).&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:Passive`(`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* Passive]()&]
[s2; Enables FTP passive data transfer mode, using EPSV or PASV command. 
Same as Active(false). Returns `*this for method chaining. This 
is the default transfer mode.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:SSL`(bool`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* SSL]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) t
rue])&]
[s2; Activates a session`-wide [^http`:`/`/en`.wikipedia`.org`/wiki`/FTPS^ FTPS] 
mode (TLS/SSL) through `"explicit`" ftps request. Returns `*this 
for method chaining. This method must be invoked before login, 
and cannot be used in active data connection mode. Data retrieval 
methods will fail if FTPS is enabled in active mode.&]
[s6; Requires Core/SSL package.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Utf8`(bool`):%- [_^Upp`:`:Ftp^ Ftp][@(0.0.255) `&]_[* Utf8]([@(0.0.255) bo
ol]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2; Activates a session`-wide UTF`-8 encoding of path names. This 
method can only be invoked before login. Server may reject this 
request. In that case the login will fail. Returns `*this for 
method chaining..&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:NonBlocking`(bool`):%- [_^Upp`:`:Ftp^ Ftp][@(0.0.255) `&]_[* NonBlocking
]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%- Activates or deactivates the non`-blocking mode. Default is 
blocking mode. Returns `*this for methods chaining.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:ChunkSize`(int`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* ChunkSize]([@(0.0.255) int]_[*@3 s
ize])&]
[s2; Sets data chunk [%-*@3 size] for binary transfers. Default size 
is 65536 bytes (64K). Returns `*this for method chaining.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:Timeout`(int`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* Timeout]([@(0.0.255) int]_[*@3 ms])
&]
[s2; Sets socket timeout value. Setting the timeout value to 0 will 
put the ftp object into non`-blocking mode (same as NonBlocking(true)). 
Returns `*this for method chaining.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:WaitStep`(int`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* WaitStep]([@(0.0.255) int]_[*@3 m
s])&]
[s2; Sets the periodicity of calling [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:WhenWait^ W
henWait] in millisecond between calls. Default value is 10ms 
(100hz). Returns `*this for method chaining.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Do`(`):%- [@(0.0.255) bool]_[* Do]()&]
[s2; Progresses the requests in non`-blocking mode. This method cannot 
be called in blocking mode. Returns true if the request is still 
in progress.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Connect`(const Upp`:`:String`&`):%- [@(0.0.255) bool]_[* Connect]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 url])&]
[s3;:Upp`:`:Ftp`:`:Connect`(const Upp`:`:String`&`,int`,const Upp`:`:String`&`,const Upp`:`:String`&`):%- [@(0.0.255) b
ool]_[* Connect]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], 
[@(0.0.255) int]_[*@3 port], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 u
sername], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 password])&]
[s2; Connects and logs into an ftp server specified at [%-*@3 host] 
and [%-*@3 port], using [%-*@3 username] and [%-*@3 password]. Returns 
true on success. Ftp client will attempt anonymous login when 
user credentials are not specified. Alternatively, an ftp [%-*@3 url] 
can also be used. &]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:Connect`(const String`&`,int`):%- [@(0.0.255) bool]_[* Connect]([@(0.0.255) con
st]_[_^String^ String][@(0.0.255) `&]_[*@3 host], [@(0.0.255) int]_[*@3 port]_`=_[@3 21])&]
[s2; Connects to an ftp server specified at [%-*@3 host ][%-$2 and] [%-*@3 port]. 
Returns true on success. This method does not perform an automatic 
login, and is meant to be used with [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:Login`(const Upp`:`:String`&`,const Upp`:`:String`&`)^ L
ogin()] method.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Login`(const Upp`:`:String`&`,const Upp`:`:String`&`):%- [@(0.0.255) b
ool]_[* Login]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 username], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 password])&]
[s2; Logs into an already connected ftp server, using [%-*@3 username] 
and [%-*@3 password]. Returns true on success. Ftp client will 
attempt anonymous login when user name or password is not specified.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:Disconnect`(`):%- [@(0.0.255) void]_[* Disconnect]()&]
[s2; Disconnects from the ftp server.&]
[s4;%- &]
[s5;%- &]
[s3;:Ftp`:`:GetDir`(`):%- [_^String^ String]_[* GetDir]()&]
[s2; Returns the remote working directory. Returns String`::GetVoid() 
on failure. In non`-blocking mode, the result of this operation 
can be obtained using [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetResult`(`)const^ G
etResult()] method.&]
[s4;%- &]
[s5;%- &]
[s3;:Ftp`:`:SetDir`(const String`&`):%- [@(0.0.255) bool]_[* SetDir]([@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 path])&]
[s2; Sets the remote working directory to [%-*@3 path]. Returns true 
on success.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:DirUp`(`):%- [@(0.0.255) bool]_[* DirUp]()&]
[s2; Sets the remote working directory to one directory up. Returns 
true on success.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:ListDir`(const Upp`:`:String`&`,Upp`:`:Ftp`:`:DirList`&`):%- [@(0.0.255) b
ool]_[* ListDir]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[_^Upp`:`:Ftp`:`:DirList^ DirList][@(0.0.255) `&]_[*@3 list])&]
[s2; Retrieves the directory listing of remote [%-*@3 path] into [%-*@3 list]. 
Returns true on success. When path parameter is null or empty, 
this method get the content of the remote working directory. 
Note that this method will return true even if the directory 
is empty.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:MakeDir`(const String`&`):%- [@(0.0.255) bool]_[* MakeDir]([@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 path])&]
[s2; Creates a remote directory at [%-*@3 path]. Returns true on success.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:RemoveDir`(const String`&`):%- [@(0.0.255) bool]_[* RemoveDir]([@(0.0.255) cons
t]_[_^String^ String][@(0.0.255) `&]_[*@3 path])&]
[s2; Removes a remote directory at [%-*@3 path]. Returns true on success.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:Rename`(const String`&`,const String`&`):%- [@(0.0.255) bool]_[* Rename]([@(0.0.255) c
onst]_[_^String^ String][@(0.0.255) `&]_[*@3 oldname], [@(0.0.255) const]_[_^String^ String
][@(0.0.255) `&]_[*@3 newname])&]
[s2; Renames a remote file. Returns true on success.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:Delete`(const String`&`):%- [@(0.0.255) bool]_[* Delete]([@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 path])&]
[s2; Deletes the remote file specified at [%-*@3 path]. Returns true 
on success.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetSize`(const Upp`:`:String`&`):%- [_^Upp`:`:int64^ int64]_[* GetSize
]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path])&]
[s2; Returns the size of remote [%-*@3 path] in octets, or `-1 on failure. 
This method implements the ftp SIZE command as defined in [^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ R
FC 3959]. It may not be available on every server. In non`-blocking 
mode, the result of this operation can be obtained using [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetResult`(`)const^ G
etResult()] method.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetFeatures`(`):%- [_^Upp`:`:ValueMap^ ValueMap]_[* GetFeatures]()&]
[s2; Returns a list of features supported by the ftp server, on success, 
and an empty list, on failure. Features supported by the server 
are represented by `"keys`", and their parameters, which can 
be multiple or none, are represented by `"values`". Note that 
all keys and values are lowercase strings. In non`-blocking mode, 
the result of this operation can be obtained using [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetResult`(`)const^ G
etResult()] method.&]
[s4;%- &]
[s5;%- &]
[s3;:Ftp`:`:Noop`(`):%- [@(0.0.255) bool]_[* Noop]()&]
[s2; Sends a `"NOOP`" command to the server. Returns true if the 
command is accepted. Useful for keeping connections alive.&]
[s4;%- &]
[s5;%- &]
[s3;:Ftp`:`:Abort`(`):%- [@(0.0.255) void]_[* Abort]()&]
[s2; Aborts any data transfer in progress.&]
[s4;%- &]
[s5;%- &]
[s3;:Ftp`:`:SendCommand`(const String`&`):%- [@(0.0.255) int]_[* SendCommand]([@(0.0.255) c
onst]_[_^String^ String][@(0.0.255) `&]_[*@3 cmd])&]
[s2; Sends a raw command to the ftp server. A CRLF (`"`\r`\n`") is 
automatically appended to every command. Returns protocol specific 
reply codes. Note that this method will not fail on ftp protocol 
errors. In case of internal errors, IsError(), GetError() and 
GetErrorDesc() methods should be used to determine the cause 
of error. This is a low level method to simplify extending the 
functionality of the Ftp class. Server replies and/or internal 
error codes and messages can be obtained using the relevant methods. 
In non`-blocking mode, the result of this operation can be obtained 
using [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetResult`(`)const^ Ge
tResult()] method.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:SetPos`(Upp`:`:int64`):%- [_^Upp`:`:Ftp^ Ftp][@(0.0.255) `&]_[* SetPos](
[_^Upp`:`:int64^ int64]_[*@3 pos])&]
[s2; This method sets the current position in the data stream to 
[%-*@3 pos], and restarts the transfer. Returns `*this for method 
chaining. Passing a negative value or zero (0) disables restart 
feature, causing the entire file to be transferred. [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:Get`(const Upp`:`:String`&`,bool`)^ G
et()], [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:Put`(Upp`:`:Stream`&`,const Upp`:`:String`&`,bool`)^ P
ut()], and [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:Append`(Upp`:`:Stream`&`,const Upp`:`:String`&`,bool`)^ A
ppend()] methods take advantage of the transfer restart feature. 
However, since the REST command is an extension ([^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ R
FC 3959]) to the original file transfer protocol, you may want 
to check it`'s availability, using the [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetFeatures`(`)^ G
etFeatures()] method.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetPos`(`)const:%- [_^Upp`:`:int64^ int64]_[* GetPos]()_[@(0.0.255) cons
t]&]
[s2; Returns the current position in the data stream. Returned value 
represents the actual transferred data in bytes. A previously 
failed transfer can be resumed by using this value with the [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:SetPos`(Upp`:`:int64`)^ S
etPos()] method.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetId`(`)const:%- [_^Upp`:`:int64^ int64]_[* GetId]()_[@(0.0.255) const]&]
[s2; Returns the unique id of the Ftp object. Each Ftp object is 
assigned a unique id on construction.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetResult`(`)const:%- [_^Upp`:`:Value^ Value]_[* GetResult]()_[@(0.0.255) c
onst]&]
[s2; This method returns the result of last operation if that operation 
returns a value (e.g. this method will return an int64 value 
for GetSize(), and a string value for Get(), Intented to be used 
in non`-blocking mode.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:IsWorking`(`)const:%- [@(0.0.255) bool]_[* IsWorking]()_[@(0.0.255) cons
t]&]
[s2; Returns true if a command or data transfer is currently in progress.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:IsBlocking`(`)const:%- [@(0.0.255) bool]_[* IsBlocking]()_[@(0.0.255) co
nst]&]
[s2; Returns true if the Ftp object is in blocking mode.&]
[s4;%- &]
[s5;%- &]
[s3;:Ftp`:`:GetCode`(`)const:%- [@(0.0.255) int]_[* GetCode]()_[@(0.0.255) const]&]
[s2; Returns last server reply code.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:GetReply`(`)const:%- [_^String^ String]_[* GetReply]()_[@(0.0.255) const]&]
[s2; Returns last server reply message.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetReplyAsXml`(`):%- [_^Upp`:`:String^ String]_[* GetReplyAsXml]()&]
[s2; Returns the last server reply message in a simple XML format. 
The XML format is as follows:&]
[s2; &]
[ {{1765:8235h1;l/26r/26t/14b/14@1 [s2; [* XML Tag]]
:: [s2; [* Description]]
::l/25r/25t/15b/15@2 [s2; [C@3 <reply>]]
:: [s2; This tag represents a single reply. A reply has two attributes: 
[C@3 type] and [C@3 code]. The type of reply can be either [C@3 `"protocol`"] 
or [C@3 `"internal`"]. Typical protocol reply codes are listed 
in [^http`:`/`/www`.ietf`.org`/rfc`/rfc959`.txt^ RFC 959]. Typical 
internal reply (error) code is `"`-1`". A reply contains at least 
one [C@3 <line>] tag.]
:: [s2; [C@3 <line>]]
:: [s2; This tag represents a single line of reply. A line contains 
a single line of message or it can be empty.]}}&]
[s2; &]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:IsError`(`)const:%- [@(0.0.255) bool]_[* IsError]()_[@(0.0.255) const]&]
[s2; Returns true if there was an error.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetError`(`)const:%- [@(0.0.255) int]_[* GetError]()_[@(0.0.255) const]&]
[s2; Returns the last error code, if any.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetErrorDesc`(`)const:%- [_^Upp`:`:String^ String]_[* GetErrorDesc]()_
[@(0.0.255) const]&]
[s2; Returns the last error message, if any.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:GetWaitEvents`(`)const:%- [_^Upp`:`:dword^ dword]_[* GetWaitEvents]()_
[@(0.0.255) const]&]
[s0;l288; [2 Returns a combination of WAIT`_READ and WAIT`_WRITE flags 
to indicate what is]&]
[s2; blocking the operation of the Ftp object. Can be used with SocketWaitEvent.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AddTo`(Upp`:`:SocketWaitEvent`&`):%- [@(0.0.255) void]_[* AddTo]([_^Upp`:`:SocketWaitEvent^ S
ocketWaitEvent][@(0.0.255) `&]_[*@3 w])&]
[s2; Adds the Ftp object to SocketWaitEvent for waiting on it.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:WhenWait:%- [_^Upp`:`:Event^ Event]<>_[* WhenWait]&]
[s2; If this event is defined, it is invoked periodically while the 
ftp client performs any socket operations, with the frequency 
of 100Hz. This is intended to give user feedback in interactive 
applications.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:WhenReply:%- [_^Upp`:`:Event^ Event]<>_[* WhenReply]&]
[s2; This event will be invoked whenever a new server reply is available.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:WhenSSLInfo:%- [_^Upp`:`:Gate^ Gate]<[@(0.0.255) const]_SSLInfo[@(0.0.255) `*
]>_[* WhenSSLInfo]&]
[s2; When defined, this event will provide information about established 
(after handshake) SSL connection or NULL if such information 
is not available. Returning true will halt the attempted connection.&]
[s6; Requires Core/SSL package.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:WhenContent:%- [_^Upp`:`:Event^ Event]<[@(0.0.255) const 
void`*], [@(0.0.255) int]>_[* WhenContent]&]
[s2; Defines a consumer function for Ftp data content. If defined, 
Ftp class uses this output event instead of storing the output 
content in the String or stream that is provided with the data 
transfer methods.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:WhenProgress:%- [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[* WhenProgress]&]
[s2; If defined, this gate allows tracking of data transfers. The 
first parameter provides the amount of data that has already 
been transferred. The second parameter may provide the total 
amount of data to be transferred, but is allowed to be 0. Returning 
true will abort the current data transfer.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Get`(const Upp`:`:String`&`,bool`):%- [_^Upp`:`:String^ String]_[* Get
]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[@(0.0.255) bool]_[*@3 ascii])&]
[s2; Downloads the remote file at [%-*@3 path] as string. Data can 
be transferred as [%-*@3 ascii] or binary. In non`-blocking mode, 
the result of this operation can be obtained using [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetResult`(`)const^ G
etResult()] method.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Get`(const Upp`:`:String`&`,Upp`:`:Stream`&`,bool`):%- [@(0.0.255) b
ool]_[* Get]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 s], [@(0.0.255) bool]_[*@3 ascii])&]
[s2; Downloads the remote file at [%-*@3 path] to stream [%-*@3 s]. Data 
can be transferred as [%-*@3 ascii] or binary. In non`-blocking 
mode, the result of this operation can be obtained using [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:GetResult`(`)const^ G
etResult()] method.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Put`(Upp`:`:Stream`&`,const Upp`:`:String`&`,bool`):%- [@(0.0.255) b
ool]_[* Put]([_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 s], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 path], [@(0.0.255) bool]_[*@3 ascii])&]
[s3;:Upp`:`:Ftp`:`:Put`(const Upp`:`:String`&`,const Upp`:`:String`&`,bool`):%- [@(0.0.255) b
ool]_[* Put]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 s], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[@(0.0.255) bool]_[*@3 ascii])&]
[s2; Uploads the content of [%-*@3 s] to the remote [%-*@3 path]. Data 
can be transferred as [%-*@3 ascii] or binary.&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Append`(Upp`:`:Stream`&`,const Upp`:`:String`&`,bool`):%- [@(0.0.255) b
ool]_[* Append]([_^Upp`:`:Stream^ Stream]_`&[*@3 s], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 path], [@(0.0.255) bool]_[*@3 ascii])&]
[s3;:Upp`:`:Ftp`:`:Append`(const Upp`:`:String`&`,const Upp`:`:String`&`,bool`):%- [@(0.0.255) b
ool]_[* Append]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 s], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[@(0.0.255) bool]_[*@3 ascii])&]
[s2; Appends the content of [%-*@3 s] to the remote [%-*@3 path]. Data 
can be transferred as [%-*@3 ascii] or binary.&]
[s4; &]
[ {{10000F(128)G(128)@1 [s0; [* Multithreaded Transfer Methods]]}}&]
[s2;%- &]
[s2;#%- The following convenience methods comply to the url specification 
listed above, and use high performance worker threads to transfer 
data. They all throw [^topic`:`/`/FTP`/src`/Upp`_Ftp`_en`-us`#Upp`:`:Ftp`:`:Error`:`:struct^ F
tp`::Error] on failure. In all of these methods, except AsyncConsumerGet(), 
[*@3 progress][%%  gate can be used to track the progress of the 
transfer: The first parameter of this gate indicates a unique 
id. The second parameter provides the amount of data that has 
already been transferred. The third parameter may provide the 
total amount of data to be transferred, but is allowed to be 
0. Returning true will abort the current data transfer.] &]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncGet`(const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[_^Upp`:`:String^ String]>_[* AsyncGet]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ i
nt64], [_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Downloads the remote file pointed by the ftp [%-*@3 url] as string. 
&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncGet`(const Upp`:`:String`&`,Upp`:`:Stream`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncGet]([@(0.0.255) const]_
[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 url], [_^Upp`:`:Stream^ Stream][@(0.0.255) `&
]_[*@3 out], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Downloads the remote file pointed by the ftp [%-*@3 url] to [%-*@3 out]. 
&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncPut`(Upp`:`:String`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncPut]([_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 in], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s3;:Upp`:`:Ftp`:`:AsyncPut`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncPut]([_^Upp`:`:Stream^ S
tream][@(0.0.255) `&]_[*@3 in], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Uploads [%-*@3 in] to the path pointed by the ftp [%-*@3 url]. &]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncAppend`(Upp`:`:String`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncAppend]([_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 in], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s3;:Upp`:`:Ftp`:`:AsyncAppend`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncAppend]([_^Upp`:`:Stream^ S
tream][@(0.0.255) `&]_[*@3 in], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Appends [%-*@3 in] to the object pointed by the ftp [%-*@3 url]. 
&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncGetToFile`(const Upp`:`:String`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncGetToFile]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 url], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 dest], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Downloads the remote file pointed by [%-*@3 url] to local file 
pointed by [%-*@3 dest]. Note that, in case of failure this method 
[/ does not] automatically delete the partially downloaded file.&]
[s4;  &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncPutFromFile`(const Upp`:`:String`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncPutFromFile]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 src], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Uploads the local file pointed by [%-*@3 src] to  the remote path 
pointed by [%-*@3 url]. &]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncAppendFromFile`(const Upp`:`:String`&`,const Upp`:`:String`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`,Upp`:`:int64`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncAppendFromFile]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 src], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 url], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64]>_[*@3 progress])&]
[s2; Appends the local file pointed by [%-*@3 src] to the remote path 
pointed by [%-*@3 url].&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:AsyncConsumerGet`(const Upp`:`:String`&`,Upp`:`:Event`<Upp`:`:int64`,const void`*`,int`>`):%- [@(0.0.255) s
tatic ][_^Upp`:`:AsyncWork^ AsyncWork]<[@(0.0.255) void]>_[* AsyncConsumerGet]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 url], [_^Upp`:`:Event^ Event]<[_^Upp`:`:int64^ i
nt64], [@(0.0.255) const]_[@(0.0.255) void`*], [@(0.0.255) int]>_[*@3 consumer])&]
[s2; Downloads the remote file pointed by the ftp [%-*@3 url], using 
a user`-defined [%-*@3 consumer ]function. The first parameter 
of the consumer function is the unique id of the given ftp worker.&]
[s4; &]
[s4; &]
[ {{10000F(128)G(128)@1 [s0; [* Constructor Detail]]}}&]
[s4;%- &]
[s3;:Ftp`:`:Ftp`(`):%- [* Ftp]()&]
[s2; Default constructor.&]
[s4; &]
[s5;%- &]
[s3;:Ftp`:`:`~Ftp`(`):%- [@(0.0.255) `~][* Ftp]()&]
[s2; Default destructor. Invokes [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:Disconnect`(`)^ D
isconnect()].&]
[s4;%- &]
[s0;%- &]
[ {{10000@(113.42.0) [s0; [*@7;4 Ftp`::Error]]}}&]
[s4; &]
[s1;:Upp`:`:Ftp`:`:Error`:`:struct:%- [@(0.0.255)3 struct][3 _][*3 Error][3 _:_][@(0.0.255)3 pub
lic][3 _][*@3;3 Exc]&]
[s2; Type used as Ftp exception. This helper struct is externally 
used by FTP worker threads, or within the Ftp events, to halt 
jobs, and report errors.&]
[s2;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Error`:`:code:%- [@(0.0.255) int]_[* code]&]
[s2; The error code returned by the halted job or event.&]
[s4;%- &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Error`:`:Error`(const Upp`:`:String`&`):%- [* Error]([@(0.0.255) const
]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 reason])&]
[s2; Constructor. Sets the error code to `-1, and the error message 
to [%-*@3 reason] .&]
[s4; &]
[s5;%- &]
[s3;:Upp`:`:Ftp`:`:Error`:`:Error`(int`,const Upp`:`:String`&`):%- [* Error]([@(0.0.255) i
nt]_[*@3 rc], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 reason])&]
[s2; Constructor overload. Sets the error code to [%-*@3 rc] and the 
error message to [%-*@3 reason] .&]
[s4; &]
[s4;%- ]]