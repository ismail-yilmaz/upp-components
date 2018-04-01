topic "Helper Functions";
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
[{_} 
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Convenience Functions and Helper Classes]]}}&]
[s2; &]
[s2; Below are the convenience functions and their related classes 
provided by the FTP package.&]
[s0; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Synchronous (Single`-threaded)]]}}&]
[s4; &]
[s5;:Upp`:`:FtpGet`(const Upp`:`:Ftp`:`:Request`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`>`,Upp`:`:Event`<`>`): [_^Upp`:`:Ftp`:`:Result^ F
tp`::Result]_[* FtpGet]([@(0.0.255) const]_[_^Upp`:`:Ftp`:`:Request^ Ftp`::Request][@(0.0.255) `&
]_[*@3 request], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64]>_
[*@3 progress]_`=_[@(0.0.255) false], [_^Upp`:`:Event^ Event]<>_[*@3 whenwait]_`=_CNULL)&]
[s2;%% Downloads the [%-*@3 request]ed file.[%-*@3  progress] function 
can be used for tracking the progress of the transfer. [%-*@3 whenwait] 
event can be used to update GUI in interactive applications. 
Returns an [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:class^ F
tp`::Result] object.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:FtpPut`(const Upp`:`:Ftp`:`:Request`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`>`,Upp`:`:Event`<`>`): [_^Upp`:`:Ftp`:`:Result^ F
tp`::Result]_[* FtpPut]([@(0.0.255) const]_[_^Upp`:`:Ftp`:`:Request^ Ftp`::Request][@(0.0.255) `&
]_[*@3 request], [_^Upp`:`:Gate^ Gate]<[_^Upp`:`:int64^ int64], [_^Upp`:`:int64^ int64]>_
[*@3 progress]_`=_[@(0.0.255) false], [_^Upp`:`:Event^ Event]<>_[*@3 whenwait]_`=_CNULL)&]
[s2;%% Uploads the [%-*@3 request]ed file. [%-*@3 progress] function 
can be used for tracking the progress of the transfer. [%-*@3 whenwait] 
event can be used to update GUI in interactive applications. 
Returns an [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:class^ F
tp`::Result] object.&]
[s3;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Asynchronous (Multi`-threaded)]]}}&]
[s4; &]
[s5;:Upp`:`:FtpAsyncGet`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`): [_^Upp`:`:Ftp`:`:Result^ F
tp`::Result]_[* FtpAsyncGet]([_^Upp`:`:Ftp`:`:Request^ Ftp`::Request][@(0.0.255) `&]_[*@3 r
equest], [_^Upp`:`:Event^ Event]<[_^Upp`:`:Ftp`:`:Result^ Ftp`::Result]>_[*@3 progress]_
`=_CNULL)&]
[s2;%% Downloads the [%-*@3 request]ed file. Returns an [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:class^ F
tp`::Result] object that contains a worker thread Id on success, 
or an error message on failure. [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:IsStarted`(`)const^ I
sStarted()] method should be used to check whether the worker 
thread is successfully created. [%-*@3 progress] event can be used 
for tracking and controlling the transfer progress. Note that 
ftp worker threads are limited to max. 256 slots.&]
[s6;%% Requires multithreading.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:FtpAsyncPut`(Upp`:`:Ftp`:`:Request`&`,Upp`:`:Event`<Upp`:`:Ftp`:`:Result`>`): [_^Upp`:`:Ftp`:`:Result^ F
tp`::Result]_[* FtpAsyncPut]([_^Upp`:`:Ftp`:`:Request^ Ftp`::Request][@(0.0.255) `&]_[*@3 r
equest], [_^Upp`:`:Event^ Event]<[_^Upp`:`:Ftp`:`:Result^ Ftp`::Result]>_[*@3 progress]_
`=_CNULL)&]
[s2;%% Uploads the [%-*@3 request]ed file. Returns an [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:class^ F
tp`::Result] object that contains a worker thread Id if successul, 
or an error message if failed. [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:IsStarted`(`)const^ I
sStarted()] method should be used to check whether the worker 
thread is successfully created. [%-*@3 progress] event can be used 
for tracking and controlling the transfer progress. Note that 
ftp worker threads are limited to max. 256 slots.&]
[s6;%% Requires multithreading.&]
[s3;%% &]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Ftp`::Request]]}}&]
[s0; &]
[s1;:Upp`:`:Ftp`:`:Request`:`:class: [@(0.0.255)3 class][3 _][*3 Request 
][3 :][*3  ][@(0.0.255)3 private][3 _][*@3;3 Moveable][3 <][_^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:Request`:`:class^3 F
tp`::Request][3 >]&]
[s2;%% This nested helper class represents a single Ftp file transfer 
request, and is intended to be used with the [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpGet`(const Upp`:`:Ftp`:`:Request`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`>`,Upp`:`:Event`<`>`)^ c
onvenience functions] provided by the FTP package.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s0; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:File`(const Upp`:`:String`&`,const Upp`:`:String`&`): [_^Upp`:`:Ftp`:`:Request^ R
equest][@(0.0.255) `&]_[* File]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 local], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 remote])&]
[s2;%% Sets the full path of the [%-*@3 local] file, and the [%-*@3 remote] 
file. Returns `*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Server`(const Upp`:`:String`&`,int`): [_^Upp`:`:Ftp`:`:Request^ R
equest][@(0.0.255) `&]_[* Server]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 host], [@(0.0.255) int]_[*@3 port]_`=_[@3 21])&]
[s2;%% Sets [%-*@3 host] and [%-*@3 port] to be connected. returns `*this 
for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Auth`(const Upp`:`:String`&`,const Upp`:`:String`&`): [_^Upp`:`:Ftp`:`:Request^ R
equest][@(0.0.255) `&]_[* Auth]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_
[*@3 user], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 password])&]
[s2;%% Sets [%-*@3 user] and [%-*@3 password]. Returns `*this for method 
chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Active`(bool`): [_^Upp`:`:Ftp`:`:Request^ Request][@(0.0.255) `&
]_[* Active]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Enables or disables FTP active connection mode. It is disabled 
by default. Returns `*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:SSL`(bool`): [_^Upp`:`:Ftp`:`:Request^ Request][@(0.0.255) `&
]_[* SSL]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Enables or disables FTPS mode. It is disabled by default. 
Returns `*this for method chaining.&]
[s6;%% Requires Core/SSL package.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Ascii`(bool`): [_^Upp`:`:Ftp`:`:Request^ Request][@(0.0.255) `&
]_[* Ascii]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Enables or disables ASCII transfer mode. It is isabled by 
default. Returns `*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Timeout`(int`): [_^Upp`:`:Ftp`:`:Request^ Request][@(0.0.255) `&
]_[* Timeout]([@(0.0.255) int]_[*@3 ms])&]
[s2;%% Sets socket timeout value. Default value is derived from [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Ftp`:`:class^ F
tp] class and is 60000 miliseconds (one minute). Returns `*this 
for method chaining..&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Priority`(int`): [_^Upp`:`:Ftp`:`:Request^ Request][@(0.0.255) `&
]_[* Priority]([@(0.0.255) int]_[*@3 percent])&]
[s2;%% Sets the ftp worker thread priority in [%-*@3 percent]. For 
more information, see: [^topic`:`/`/Core`/src`/Thread`$en`-us`#Thread`:`:Priority`(int`)^ T
hread`::Priority()]. Note that currently it is not possible to 
change worker priority on`-the`-fly.&]
[s6;%% Requires multithreading.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Restart`(Upp`:`:int64`): [_^Upp`:`:Ftp`:`:Request^ Reques
t][@(0.0.255) `&]_[* Restart]([_^Upp`:`:int64^ int64]_[*@3 pos]_`=_[@3 0])&]
[s2;%% Restarts the file transfer from given [%-*@3 pos], using REST 
command. Similar to [^topic`:`/`/Core`/src`/Stream`$en`-us`#Stream`:`:Seek`(int64`)^ S
tream`::Seek()]. Returns `*this for method chaining. Passing a 
negative value or zero (0) disables restart feature, causing 
the entire file to be transferred. Since the REST command is 
an extension ([^https`:`/`/tools`.ietf`.org`/html`/rfc3659^ RFC 
3959]) to the original file transfer protocol, you may want to 
check it`'s availability using [^topic`:`/`/FTP`/src`/Ftp`$en`-us`#Upp`:`:Ftp`:`:GetFeatures`(`)^ F
tp`::GetFeatures()] method.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:DeleteFailed`(bool`): [_^Upp`:`:Ftp`:`:Request^ Request][@(0.0.255) `&
]_[* DeleteFailed]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Deletes the downloaded local file on failure. Returns `*this 
for method chaining. Disabled by default. Note that to enable 
the transfer restart/resume feature, you must disable this feature.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:UserData`(const Upp`:`:Value`&`): [_^Upp`:`:Ftp`:`:Request^ R
equest][@(0.0.255) `&]_[* UserData]([@(0.0.255) const]_[_^Upp`:`:Value^ Value][@(0.0.255) `&
]_[*@3 v])&]
[s2;%% Allows passing user data to [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Result`:`:class^ F
tp`::Result] object. Returns `*this for method chaining.&]
[s6;%% Requires multithreading.&]
[s3;%% &]
[s0;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s0; &]
[s5;:Upp`:`:Ftp`:`:Request`:`:Request`(const Upp`:`:String`&`,int`): [* Request]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 host], [@(0.0.255) int]_[*@3 port]_`=_[@3 2
1])&]
[s2;%% Class constructor. Sets [%-*@3 host] and [%-*@3 port] to be connected.&]
[s3;%% &]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Ftp`::Result]]}}&]
[s0; &]
[s1;:Upp`:`:Ftp`:`:Result`:`:class: [@(0.0.255)3 class][3 _][*3 Result][3 _:_][@(0.0.255)3 priva
te][3 _][*@3;3 Moveable][3 <][_^Upp`:`:Ftp`:`:Request^3 Ftp`::Result][3 >_]&]
[s2;%% This nested helper class is used for tracking and controlling 
the transfer progress of any ftp object spawned via the [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:FtpGet`(const Upp`:`:Ftp`:`:Request`&`,Upp`:`:Gate`<Upp`:`:int64`,Upp`:`:int64`>`,Upp`:`:Event`<`>`)^ c
onvenience functions] provided by FTP package.&]
[s3;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s0;%% &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetName`(`)const: [_^Upp`:`:String^ String]_[* GetName]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the full path of the file to be transferred. &]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetError`(`)const: [@(0.0.255) int]_[* GetError]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the error code, if any.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetErrorDesc`(`)const: [_^Upp`:`:String^ String]_[* GetError
Desc]()_[@(0.0.255) const]&]
[s2;%% Returns the error description, if any.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetRestartPos`(`)const: [_^Upp`:`:int64^ int64]_[* GetRestar
tPos]()_[@(0.0.255) const]&]
[s2;%% Returns the current position in the data stream. Returned 
value represents the actual transferred data in bytes.  Using 
this value with the [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Request`:`:Restart`(Upp`:`:int64`)^ F
tp`::Request`::Restart()] method, a failed transfer can be resumed.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:IsSuccess`(`)const: [@(0.0.255) bool]_[* IsSuccess]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if file transfer was successful. &]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:IsFailure`(`)const: [@(0.0.255) bool]_[* IsFailure]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if file transfer has failed.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:IsRestarted`(`)const: [@(0.0.255) bool]_[* IsRestarted]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if a file transfer is successfully restarted.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:IsStarted`(`)const: [@(0.0.255) bool]_[* IsStarted]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if a worker thread is successfully started.&]
[s6;%% Requires multithreading.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:InProgress`(`)const: [@(0.0.255) bool]_[* InProgress]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if a file transfer is still in progress.&]
[s6;%% Requires multithreading.&]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetId`(`)const: [@(0.0.255) int]_[* GetId]()_[@(0.0.255) const
]&]
[s2;%% Returns the worker thread id. A worker thread Id is essentially 
an incremental counter ranging from 1 to 65536. It will automatically 
reset to 1 when it exceeds 65536. &]
[s6;%% Requires multithreading.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetTotal`(`)const: [_^Upp`:`:int64^ int64]_[* GetTotal]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the total size of the data to be transferred.&]
[s6;%% Requires multithreading.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetDone`(`)const: [_^Upp`:`:int64^ int64]_[* GetDone]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the actual size of the transferred data&]
[s6;%% Requires multithreading.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:GetUserData`(`)const: [@(0.0.255) const]_[_^Upp`:`:Value^ Va
lue][@(0.0.255) `&]_[* GetUserData]()_[@(0.0.255) const]&]
[s2;%% Returns user data passed by [^topic`:`/`/FTP`/src`/HelperFunctions`$en`-us`#Upp`:`:Ftp`:`:Request`:`:class^ F
tp`::Request] object.&]
[s6;%% Requires multithreading.&]
[s3; &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s0; &]
[s5;:Upp`:`:Ftp`:`:Result`:`:Result`(`): [* Result]()&]
[s2;%% Default constructor.&]
[s0;%% ]]