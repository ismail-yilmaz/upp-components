topic "PtyProcess";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 PtyProcess]]}}&]
[s0; &]
[s1;:Upp`:`:PtyProcess`:`:class: [@(0.0.255)3 class][3 _][*3 PtyProcess][3 _:_][@(0.0.255)3 publ
ic][3 _][*@3;3 AProcess]&]
[s2;%% This class represents a pseudo`-terminal process on a local 
machine.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:PtyProcess`:`:ConvertCharset`(bool`): [_^Upp`:`:PtyProcess^ PtyProcess][@(0.0.255) `&
]_[* ConvertCharset]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s5;:Upp`:`:PtyProcess`:`:NoConvertCharset`(`): [_^Upp`:`:PtyProcess^ PtyProcess][@(0.0.255) `&
]_[* NoConvertCharset]()&]
[s2;%% Determines if PtyProcess should convert encoding from system 
to the pseudo`-terminal. Default setting is false.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:SetSize`(Upp`:`:Size`): [@(0.0.255) bool]_[* SetSize]([_^Upp`:`:Size^ S
ize]_[*@3 sz])&]
[s5;:Upp`:`:PtyProcess`:`:SetSize`(int`,int`): [@(0.0.255) bool]_[* SetSize]([@(0.0.255) in
t]_[*@3 col], [@(0.0.255) int]_[*@3 row])&]
[s2;%% Setst the page size of the active pty. Returns true on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:GetSize`(`): [_^Upp`:`:Size^ Size]_[* GetSize]()&]
[s2;%% Returns the page size of the active pty on success and Null 
on failure.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:SetAttrs`(const termios`&`): [@(0.0.255) bool]_[* SetAttrs]([@(0.0.255) c
onst]_[_^termios^ termios][@(0.0.255) `&]_[*@3 t])&]
[s6; POSIX specific.&]
[s2;%% Sets the attributes of the active pty to [%-*@3 t]. Returns 
true on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:GetAttrs`(termios`&`): [@(0.0.255) bool]_[* GetAttrs]([_^termios^ t
ermios][@(0.0.255) `&]_[*@3 t])&]
[s6; POSIX specific.&]
[s2;%% Fetches the attributes of the active pty into [%-*@3 t]. Return 
true on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:WhenAttrs: [_^Upp`:`:Gate^ Gate]<termios[@(0.0.255) `&]>_[* WhenA
ttrs]&]
[s6;%% POSIX specific&]
[s2;%% This event can be used to set or change the [/ initial attributes] 
of the pty. Returning true will set the attributes, and returning 
false will cancel the operation.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:Start`(const char`*`,const char`*`,const char`*`): [@(0.0.255) b
ool]_[* Start]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmdline], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 env]_`=_nullptr, [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 cd]_`=_nullptr)&]
[s2;# [@N Starts a new pseudo`-terminal process defined by ][*@3 cmdline][@N . 
][*@3 env ][@N can provide a new environment for the process. The 
new proces will inherit the caller`'s environment if the ][*@3 env 
][@N argument is NULL. Additionally, o][%% n POSIX systems ][@N if 
the ][*@3 env ][%% argument doesn`'t contain a `"][%%C TERM][%% `" key, 
then the `"][%%C TERM`=xterm][%% `" key/value pair will be used as 
a fallback.. ][*@3 cd ][@N can be used to specify the new current 
directory for the process.]&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:Start`(const char`*`,const Upp`:`:Vector`<Upp`:`:String`>`*`,const char`*`,const char`*`): [@(0.0.255) b
ool]_[* Start]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmd], [@(0.0.255) const]_[_^Upp`:`:Vector^ V
ector]<[_^Upp`:`:String^ String]>_`*[*@3 args], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 e
nv]_`=_nullptr, [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cd]_`=_nullptr)&]
[s2;%% Starts a new pseudo`-terminal process defined by [%-*@3 cmd] 
and [%-*@3 args]. [%-*@3 env] can provide a new environment for the 
process. [%-@N The new proces will inherit the caller`'s environment 
if the ][%-*@3 env ][%-@N argument is NULL. Additionally, o]n POSIX 
systems [%-@N if the ][%-*@3 env ]argument doesn`'t contain a `"[C TERM]`" 
key, then the `"[C TERM`=xterm]`" key/value pair will be used as 
a fallback. This variant passes individual arguments instead 
of whole commandline. On POSIX this has the advantage of passing 
the commands directly to execv, without parsing the command line. 
[%-*@3 cd] can be used to specify the new current directory for 
the process.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:Start`(const char`*`,const Upp`:`:VectorMap`<Upp`:`:String`,Upp`:`:String`>`&`,const char`*`): [@(0.0.255) b
ool]_[* Start]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmdline], 
[@(0.0.255) const]_[_^Upp`:`:VectorMap^ VectorMap]<[_^Upp`:`:String^ String], 
[_^Upp`:`:String^ String]>`&_[*@3 env], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cd])&]
[s2; [%% Start a new pseudo terminal process defined by ][*@3 cmdline][%% . 
][*@3 env][%%  is a map of key/value pairs that can provide a new 
environment for the process. On POSIX systems if the ][*@3 env][%%  
argument doesn`'t contain a `"][%%C TERM][%% `" key, then the `"][%%C TERM`=xterm][%% `" 
key/value pair will be used as a fallback. ][*@3 cd][%%  ][@N can be 
used to specify the new current directory for the process.]&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(`): [* PtyProcess]()&]
[s2;%% Default constructor.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(const char`*`,const Upp`:`:VectorMap`<Upp`:`:String`,Upp`:`:String`>`&`,const char`*`): [* P
tyProcess]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmdline], [@(0.0.255) const]_[_^Upp`:`:VectorMap^ V
ectorMap]<[_^Upp`:`:String^ String], [_^Upp`:`:String^ String]>`&_[*@3 env], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cd]_`=_nullptr)&]
[s2;%%  Equivalent of default constructor and then invoking Start([%-*@3 cmdline], 
[%-*@3 env], [%-*@3 cd]).&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(const char`*`,const char`*`,const char`*`): [* Pt
yProcess]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmdline], [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 env]_`=_nullptr, [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cd]_`=_nullptr)&]
[s2;%% Equivalent of default constructor and then invoking Start([%-*@3 cmdline], 
[%-*@3 env], [%-*@3 cd]).&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(const char`*`,const Upp`:`:Vector`<Upp`:`:String`>`*`,const char`*`,const char`*`): [* P
tyProcess]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmd], [@(0.0.255) const]_[_^Upp`:`:Vector^ V
ector]<[_^Upp`:`:String^ String]>_`*[*@3 args], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 e
nv]_`=_nullptr, [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cd]_`=_nullptr)&]
[s2;%% Equivalent of default constructor and then invoking Start([%-*@3 cmd], 
[%-*@3 args], [%-*@3 env], [%-*@3 cd]).&]
[s3; ]]