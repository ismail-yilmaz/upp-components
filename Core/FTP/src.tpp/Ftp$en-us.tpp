topic "Ftp";
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
[ {{10000@(113.42.0) [s0; [*@7;4 Ftp]]}}&]
[s1;%- &]
[s1;:Upp`:`:Ftp`:`:class:%- [@(0.0.255)3 class][3 _][*3 Ftp ][3 :][*3  ][@(0.0.255)3 private][3 _][*@3;3 N
oCopy]&]
[s2; This class provides a client side interface to the File Transfer 
Protocol (FTP) as specified in [^http`:`/`/www`.ietf`.org`/rfc`/rfc959`.txt^ RFC 
959], with several advanced capabilities: &]
[s2; &]
[s2;i150;O0; Support for IPv6 and NATs, as specified in [^http`:`/`/tools`.ietf`.org`/html`/rfc2428^ R
FC 2428.]&]
[s2;i150;O0; Support for ftp over TLS/SSL (FTPS), as specified in 
[^http`:`/`/tools`.ietf`.org`/html`/rfc2228^ RFC 2228].&]
[s2;i150;O0; Support for [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetFeatures`(`)^ f
eature negotiation mechanism] as specifien in [^http`:`/`/tools`.ietf`.org`/html`/rfc2389^ R
FC 2389].&]
[s2;i150;O0; Support for [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpAsyncGet`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`)^ m
ultithreading], using worker threads. &]
[s2;i150;O0; Support for [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:DirEntry`:`:class^ p
arsing] UNIX and DOS style directory listings.&]
[s2;i150;O0; Support for [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:SendCommand`(const String`&`)^ e
xtending] the functionality of Ftp class.&]
[s2;i150;O0; Support for transfer restart mechanism, as specified 
in [^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ RFC 3959].&]
[s1;%- &]
[ {{10000F(128)G(128)@1 [s0; [* Public Method List]]}}&]
[s0;%- &]
[s5;:Ftp`:`:User`(const String`&`,const String`&`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* User](
[@(0.0.255) const]_[_^String^ String][@(0.0.255) `&]_[*@3 user], [@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 pass])&]
[s2; Sets username and password. Returns `*this for method chaining. 
Ftp client will attempt anonymous login if user id or password 
is unspecified.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:SSL`(bool`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* SSL]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) t
rue])&]
[s2; Activates a session`-wide [^http`:`/`/en`.wikipedia`.org`/wiki`/FTPS^ FTPS] 
mode (TLS/SSL) through `"explicit`" ftps request. Returns `*this 
for method chaining. This method must be invoked before a [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:Connect`(const String`&`,int`)^ C
onnect]() call and cannot be used in active data connection mode. 
Data retrieval methods will fail if FTPS is enabled in active 
mode.&]
[s6; Requires Core/SSL package.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:Active`(bool`):%- [_^Upp`:`:Ftp^ Ftp][@(0.0.255) `&]_[* Active]([@(0.0.255) b
ool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2; Switches the data transfer mode to active, using EPRT or PORT 
command. Returns `*this for method chaining. Note that the active 
mode uses a port range between 49152 and 65535.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Passive`(`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* Passive]()&]
[s2; Switches the data transfer mode to passive, using EPSV or PASV 
command. Returns `*this for method chaining. This is the default 
transfer mode.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Timeout`(int`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* Timeout]([@(0.0.255) int]_[*@3 ms])
&]
[s2; Sets socket timeout value. Default value is 60000 miliseconds 
(one minute). Returns `*this for method chaining.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:WaitStep`(int`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* WaitStep]([@(0.0.255) int]_[*@3 m
s])&]
[s2; Sets the periodicity of calling [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:WhenWait^ W
henWait] in millisecond between calls. Default value is 10ms 
(100hz). Returns `*this for method chaining.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:ChunkSize`(int`):%- [_^Ftp^ Ftp][@(0.0.255) `&]_[* ChunkSize]([@(0.0.255) int]_[*@3 s
ize])&]
[s2; Sets data chunk [%-*@3 size] for binary transfers. Default size 
is 65536 bytes (64K). Returns `*this for method chaining.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:Restart`(Upp`:`:int64`):%- [_^Upp`:`:Ftp^ Ftp][@(0.0.255) `&]_[* Restart
]([_^Upp`:`:int64^ int64]_[*@3 pos])&]
[s2; Restarts the file transfer from given [%-*@3 pos], using REST 
command. Similar to [^topic`:`/`/Core`/src`/Stream`$en`-us`#Stream`:`:Seek`(int64`)^ S
tream`::Seek()]. Returns `*this for method chaining. Passing a 
negative value or zero (0) disables restart feature, causing 
the entire file to be transferred. [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetResumePos`(`)const^ G
etRestartPos()] can be used to obtain actual position. [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Get`(const Upp`:`:String`&`,Upp`:`:Stream`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`)^ G
et()], [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Put`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`)^ P
ut()], and [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Append`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`)^ A
ppend()] methods take advantage of the transfer restart feature. 
However, since the REST command is an extension ([^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ R
FC 3959]) to the original file transfer protocol, you may want 
to check it`'s availability using [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetFeatures`(`)^ G
etFeatures()] method. &]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Connect`(const String`&`,int`):%- [@(0.0.255) bool]_[* Connect]([@(0.0.255) con
st]_[_^String^ String][@(0.0.255) `&]_[*@3 host], [@(0.0.255) int]_[*@3 port]_`=_[@3 21])&]
[s2; Connects to a ftp server specified at [%-*@3 host ][%-$2 and] [%-*@3 port]. 
Returns true on success.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Disconnect`(`):%- [@(0.0.255) void]_[* Disconnect]()&]
[s2; Disconnects from the ftp server.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:GetDir`(`):%- [_^String^ String]_[* GetDir]()&]
[s2; Returns the remote working directory. Returns String`::GetVoid() 
on failure.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:SetDir`(const String`&`):%- [@(0.0.255) bool]_[* SetDir]([@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 path])&]
[s2; Sets the remote working directory to [%-*@3 path]. Returns true 
on success.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:DirUp`(`):%- [@(0.0.255) bool]_[* DirUp]()&]
[s2; Sets the remote working directory to one directory up. Returns 
true on success.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:ListDir`(const Upp`:`:String`&`,Upp`:`:Ftp`:`:DirList`&`,Upp`:`:Gate1`<Upp`:`:String`>`):%- [@(0.0.255) b
ool]_[* ListDir]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[_^Upp`:`:Ftp`:`:DirList^ DirList][@(0.0.255) `&]_[*@3 list], [_^Upp`:`:Gate1^ Gate1]<[_^Upp`:`:String^ S
tring]>_[*@3 progress]_`=_[@(0.0.255) false])&]
[s2; Retrieves the directory listing of [%-*@3 path]. When path is 
empty or Null, this method will retrieve a directory listing 
of the remote working directory. Returns true on success. Note 
that this method will return true even when the remote directory 
is empty. [%-*@3 progress] function can be used to track progress 
of the operation, and to get the single, raw directory (String) 
entries one by one; returning true will cancel the operation.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:MakeDir`(const String`&`):%- [@(0.0.255) bool]_[* MakeDir]([@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 path])&]
[s2; Creates a remote directory at [%-*@3 path]. Returns true on success.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:RemoveDir`(const String`&`):%- [@(0.0.255) bool]_[* RemoveDir]([@(0.0.255) cons
t]_[_^String^ String][@(0.0.255) `&]_[*@3 path])&]
[s2; Removes a remote directory at [%-*@3 path]. Returns true on success.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:Get`(const Upp`:`:String`&`,Upp`:`:Stream`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`):%- [@(0.0.255) b
ool]_[* Get]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path], 
[_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 out], [_^Upp`:`:Gate2^ Gate2]<[_^Upp`:`:int64^ i
nt64], [_^Upp`:`:int64^ int64]>_[*@3 progress]_`=_[@(0.0.255) false], 
[@(0.0.255) bool]_[*@3 ascii]_`=_[@(0.0.255) false])&]
[s2; Downloads the remote file specified at [%-*@3 path]  and writes 
it into [%-*@3 out]. Returns true on success. [%-*@3 progress] function 
can be used to track the progress of the download; returning 
true will abort the operation. Data can be downloaded as [%-*@3 ascii] 
or binary. This method can take advantage of file restart (resume) 
feature. See [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Restart`(Upp`:`:int64`)^ R
estart()] and GetRestartPos() methods.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:Put`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`):%- [@(0.0.255) b
ool]_[* Put]([_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 in], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 path], [_^Upp`:`:Gate2^ Gate2]<[_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress]_`=_[@(0.0.255) false], [@(0.0.255) bool]_[*@3 ascii
]_`=_[@(0.0.255) false])&]
[s2; Uploads the local file [%-*@3 in] to the remote [%-*@3 path]. Returns 
true on success. [%-*@3 progress]  function can be used to track 
the progress of the upload; returning true will abort the operation. 
Data can be uploaded as [%-*@3 ascii] or binary. This method can 
take advantage of file restart (resume) feature. See [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Restart`(Upp`:`:int64`)^ R
estart()] and [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetRestartPos`(`)const^ G
etRestartPos()] methods. [* Warning:] Restarting an upload using 
REST command is not very reliable. Consider using [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Append`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`)^ A
ppend()] instead.&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:Append`(Upp`:`:Stream`&`,const Upp`:`:String`&`,Upp`:`:Gate2`<Upp`:`:int64`,Upp`:`:int64`>`,bool`):%- [@(0.0.255) b
ool]_[* Append]([_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 in], [@(0.0.255) const]_[_^Upp`:`:String^ S
tring][@(0.0.255) `&]_[*@3 path], [_^Upp`:`:Gate2^ Gate2]<[_^Upp`:`:int64^ int64], 
[_^Upp`:`:int64^ int64]>_[*@3 progress]_`=_[@(0.0.255) false], [@(0.0.255) bool]_[*@3 ascii
]_`=_[@(0.0.255) false])&]
[s2; Appends the local file [%-*@3 in] to the remote file at remote 
[%-*@3 path]. Returns true on success. [%-*@3 progress] function 
can be used to track the progress of the upload; returning true 
will abort the operation. Data can be uploaded as [%-*@3 ascii] 
or binary. This method can take advantage of file restart (resume) 
feature. See [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Restart`(Upp`:`:int64`)^ R
estart()] and [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetRestartPos`(`)const^ G
etRestartPos()] methods.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Info`(const String`&`,Ftp`:`:DirEntry`&`):%- [@(0.0.255) bool]_[* Info]([@(0.0.255) c
onst]_[_^String^ String][@(0.0.255) `&]_[*@3 path], [_^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:DirEntry`:`:class^ D
irEntry][@(0.0.255) `&]_[*@3 info])&]
[s2; Retrieves the directory listing [%-*@3 info] of [%-*@3 path]. Returns 
true on success.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Rename`(const String`&`,const String`&`):%- [@(0.0.255) bool]_[* Rename]([@(0.0.255) c
onst]_[_^String^ String][@(0.0.255) `&]_[*@3 oldname], [@(0.0.255) const]_[_^String^ String
][@(0.0.255) `&]_[*@3 newname])&]
[s2; Renames a remote file. Returns true on success.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Delete`(const String`&`):%- [@(0.0.255) bool]_[* Delete]([@(0.0.255) const]_[_^String^ S
tring][@(0.0.255) `&]_[*@3 path])&]
[s2; Deletes the remote file specified at [%-*@3 path]. Returns true 
on success.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Noop`(`):%- [@(0.0.255) bool]_[* Noop]()&]
[s2; Sends a `"NOOP`" command to the FTP server. Returns true if 
the command is accepted. Useful for keeping connections alive.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:Abort`(`):%- [@(0.0.255) void]_[* Abort]()&]
[s2; Aborts any download or upload in progress.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:SendCommand`(const String`&`):%- [@(0.0.255) int]_[* SendCommand]([@(0.0.255) c
onst]_[_^String^ String][@(0.0.255) `&]_[*@3 cmd])&]
[s2; Sends a raw command to the ftp server. Returns `-1 for internal 
errors, and other values for protocol specific error and success 
messages. A CRLF (`"`\r`\n`") is automatically appended to every 
command.This is a low level method to simplify extending the 
functionality of the Ftp class. Server replies and/or internal 
error codes and messages can be obtained using the [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:GetCode`(`)const^ G
etCode()], [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:GetReply`(`)const^ GetReply()
] or [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetReplyAsXml`(`)^ GetReplyA
sXml()] methods.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:InProgress`(`)const:%- [@(0.0.255) bool]_[* InProgress]()_[@(0.0.255) const]&]
[s2; Returns true if a file transfer is in progress. Only the [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:Abort`(`)^ A
bort()] command should be called while a transfer is in progress.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:GetSize`(const Upp`:`:String`&`):%- [_^Upp`:`:int64^ int64]_[* GetSize
]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 path])&]
[s2; Returns the size of remote [%-*@3 path] in octets, or `-1 on failure. 
This method implements the ftp SIZE command as defined in [^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ R
FC 3959]&]
[s3; &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:GetRestartPos`(`)const:%- [_^Upp`:`:int64^ int64]_[* GetRestartPos]()_
[@(0.0.255) const]&]
[s2; Returns the current position in the data stream. Returned value 
represents the actual transferred data in bytes. Using this value 
with the [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Restart`(Upp`:`:int64`)^ R
estart()] method, a failed transfer can be resumed.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:GetFeatures`(`):%- [_^Upp`:`:ValueMap^ ValueMap]_[* GetFeatures]()&]
[s2; Returns a list of features supported by the ftp server, on success, 
and an empty list, on failure. Features supported by the server 
are represented by `"keys`", and their parameters, which can 
be multiple or none, are represented by `"values`". Note that 
all keys and values are lowercase strings.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:GetSocket`(`):%- [_^TcpSocket^ TcpSocket][@(0.0.255) `&]_[* GetSocket]()&]
[s2; Returns a reference to the ftp control socket.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:GetCode`(`)const:%- [@(0.0.255) int]_[* GetCode]()_[@(0.0.255) const]&]
[s2; Returns last server reply code and returns `-1 for internal 
errors.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:GetReply`(`)const:%- [_^String^ String]_[* GetReply]()_[@(0.0.255) const]&]
[s2; Returns last server reply message, or internal error message.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:GetReplyAsXml`(`):%- [_^Upp`:`:String^ String]_[* GetReplyAsXml]()&]
[s2; Returns last server reply message, or internal error message 
in a simple XML format. The XML format is as follows:&]
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
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:WhenWait:%- [_^Callback^ Callback]_[* WhenWait]&]
[s2; If this callback is defined, it is invoked periodically while 
the ftp client performs any socket operations, with the frequency 
of 100Hz. This is intended to give user feedback in interactive 
applications.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:GetWorkerCount`(`):%- [@(0.0.255) static] [@(0.0.255) int]_[* GetWorkerC
ount]()&]
[s2; Returns the number of running ftp worker threads. See [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpAsyncGet`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`)^ F
tpAsyncGet()] and [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpAsyncPut`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`)^ F
tpAsyncPut]() for more information.&]
[s6; Requires multithreading.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:AbortWorker`(int`):%- [@(0.0.255) static] [@(0.0.255) void]_[* AbortWork
er]([@(0.0.255) int]_[*@3 id])&]
[s2; Sets the abort flag on for the ftp worker thread with the given 
[%-*@3 id]. Note that [^topic`:`/`/Core`/src`/Thread`$en`-us`#Thread`:`:ShutdownThreads`(`)^ S
hutdownThreads() ]can be used to abort all running worker threads 
at once. See [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpAsyncGet`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`)^ F
tpAsyncGet()] and [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpAsyncPut`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`)^ F
tpAsyncPut()] for more information.&]
[s6; Requires multithreading.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:Trace`(bool`):%- [@(0.0.255) static] [@(0.0.255) void]_[* Trace]([@(0.0.255) bool
]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2; Enables logging of Ftp client&]
[s3; &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0; [* Constructor Detail]]}}&]
[s3;%- &]
[s5;:Ftp`:`:Ftp`(`):%- [* Ftp]()&]
[s2; Default constructor.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:`~Ftp`(`):%- [@(0.0.255) `~][* Ftp]()&]
[s2; Default destructor. Invokes [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:Disconnect`(`)^ D
isconnect()].&]
[s3;%- &]
[s0;%- &]
[ {{10000@(113.42.0) [s0; [*@7;4 Ftp`::DirEntry]]}}&]
[s3; &]
[s1;:Ftp`:`:DirEntry`:`:class:%- [@(0.0.255)3 class][3 _][*3 DirEntry][3 _:_][@(0.0.255)3 privat
e][3 _][*@3;3 Moveable][3 <][_^Ftp`:`:DirEntry^3 Ftp`::DirEntry][3 >_]&]
[s2; This nested class is intended to simplify the parsing of directory 
entries (files, directories, symbolic links) returned by the 
[^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:class^ Ftp] class. It 
can handle both UNIX and DOS style directory listings.&]
[s2; &]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:User`(const String`&`):%- [_^Ftp`:`:DirEntry^ DirEntry][@(0.0.255) `&
]_[* User]([@(0.0.255) const]_[_^String^ String][@(0.0.255) `&]_[*@3 u])&]
[s2; Sets user to [%-*@3 u]. If not set or set to Null, public permissions 
can be queried.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:GetName`(`)const:%- [_^String^ String]_[* GetName]()_[@(0.0.255) co
nst]&]
[s2; Returns the name of the directory entry.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:GetOwner`(`)const:%- [_^String^ String]_[* GetOwner]()_[@(0.0.255) c
onst]&]
[s2; Returns the owner of the directory entry. &]
[s6; Only applicable to UNIX style directory listing.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:GetGroup`(`)const:%- [_^String^ String]_[* GetGroup]()_[@(0.0.255) c
onst]&]
[s2; Returns the group of the directory entry. &]
[s6; Only applicable to UNIX style directory listing.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:GetSize`(`)const:%- [_^int64^ int64]_[* GetSize]()_[@(0.0.255) cons
t]&]
[s2; Returns the size of the entry.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:GetLastModified`(`)const:%- [_^Time^ Time]_[* GetLastModified]()_
[@(0.0.255) const]&]
[s2; Returns the last modification time of the entry.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:GetStyle`(`)const:%- [_ DirEntry`::Style]_[* GetStyle]()_[@(0.0.255) c
onst]&]
[s2; Returns the directory listing style. Currently [^topic`:`/`/trunk`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:UNIX^ U
NIX]  and [^topic`:`/`/trunk`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:DOS^ D
OS] style directory listings are supported. Returns UNDEFINED 
for unsupported directory listing styles.&]
[s3; &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:GetEntry`(`)const:%- [_^String^ String]_[* GetEntry]()_[@(0.0.255) c
onst]&]
[s2; Returns the raw directory listing string.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:IsFile`(`)const:%- [@(0.0.255) bool]_[* IsFile]()_[@(0.0.255) const
]&]
[s2; Returns true if the entry is a file.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:IsDirectory`(`)const:%- [@(0.0.255) bool]_[* IsDirectory]()_[@(0.0.255) c
onst]&]
[s2; Returns true if the entry is a directory.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:IsSymLink`(`)const:%- [@(0.0.255) bool]_[* IsSymLink]()_[@(0.0.255) c
onst]&]
[s2; Returns true if the entry is a symbolic link.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:IsReadable`(`)const:%- [@(0.0.255) bool]_[* IsReadable]()_[@(0.0.255) c
onst]&]
[s2; Returns true if the directory entry is readable by the user. 
If user is not set, this method will return the public read permission. 
&]
[s6; Only applicable to UNIX style directory listing.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:IsWriteable`(`)const:%- [@(0.0.255) bool]_[* IsWriteable]()_[@(0.0.255) c
onst]&]
[s2; Returns true if  the directory entry is writeable by the user. 
If user is not set, this method will return the public write 
permission. &]
[s6; Only applicable to UNIX style directory listing.&]
[s3;%- &]
[s4;%- &]
[s5;:Ftp`:`:DirEntry`:`:IsExecutable`(`)const:%- [@(0.0.255) bool]_[* IsExecutable]()_[@(0.0.255) c
onst]&]
[s2; Returns true if  the directory entry is executable by the user. 
If user is not set, this method will return the public execute 
permission. &]
[s6; Only applicable to UNIX style directory listing.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:UNIX:%- [@(0.0.255) enum]_DirEntry`::Style`::[*@3 U
NIX]&]
[s2; Represents a UNIX style directory listing.&]
[s3;%- &]
[s4;%- &]
[s5;:Upp`:`:Ftp`:`:DirEntry`:`:Style`:`:DOS:%- [@(0.0.255) enum]_DirEntry`::Style`::[*@3 DO
S]&]
[s2; Represents a DOS style directory listing.&]
[s3; &]
[s0; &]
[ {{10000F(128)G(128)@1 [s0; [* Constructor detail]]}}&]
[s0;%- &]
[s5;:Ftp`:`:DirEntry`:`:DirEntry`(`):%- [* DirEntry]()&]
[s2; Default constructor.&]
[s3;%- &]
[ {{10000@(113.42.0) [s0; [*@7;4 Ftp`::DirList]]}}&]
[s0;%- &]
[s5;:Ftp`:`:DirList`:`:typedef:%- [@(0.0.255) typedef]_[_^Vector^ Vector]<[_^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:DirEntry`:`:class^ D
irEntry]>_[* DirList]&]
[s2; Ftp`::DirList is a [^topic`:`/`/Core`/src`/Vector`$en`-us^ Vector] 
type container, containing [^topic`:`/`/FTP`/src`/FtpDirEntry`$en`-us`#Ftp`:`:DirEntry`:`:class^ F
tp`::DirEntry] elements.&]
[s3;%- &]
[s0;%- &]
[ {{10000F(128)G(128)@1 [s0; [* Miscellaneous]]}}&]
[s0;%- &]
[s5;:Upp`:`:ParseFtpDirEntry`(const Upp`:`:String`&`,Upp`:`:Ftp`:`:DirList`&`):%- [@(0.0.255) b
ool]_[* ParseFtpDirEntry]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 i
n], [_^Upp`:`:Ftp`:`:DirList^ Ftp`::DirList][@(0.0.255) `&]_[*@3 out])&]
[s2; This helper function parses a UNIX or DOS style directory list 
into a [^topic`:`/`/FTP`/src`/FtpDirEntry`$en`-us`#Ftp`:`:DirList`:`:typedef^ Ftp`:
:DirList] structure. Returns true on success.&]
[s3;%- ]]