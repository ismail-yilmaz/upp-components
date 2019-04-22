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
[s1; &]
[s1;:Upp`:`:PtyProcess`:`:class: [@(0.0.255)3 class][3 _][*3 PtyProcess][3 _:_][@(0.0.255)3 publ
ic][3 _][*@3;3 AProcess]&]
[s2;%% This class represents a pseudo`-terminal process on a local 
machine.&]
[s6;%% POSIX specific.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:PtyProcess`:`:ConvertCharset`(bool`): [_^Upp`:`:PtyProcess^ PtyProcess][@(0.0.255) `&
]_[* ConvertCharset]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s5;:Upp`:`:PtyProcess`:`:NoConvertCharset`(`): [_^Upp`:`:PtyProcess^ PtyProcess][@(0.0.255) `&
]_[* NoConvertCharset]()&]
[s2;%% Enables converting the character encoding from system to the 
pseudo`-terminal. Default setting is false.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:SetSize`(Upp`:`:Size`): [@(0.0.255) bool]_[* SetSize]([_^Upp`:`:Size^ S
ize]_[*@3 sz])&]
[s2;%% Sets or changes the size (in character cells) of terminal 
to [%-*@3 sz]. Calling this method before the start of a pty process 
is valid, and returns true. In that case, [%-*@3 sz] will be used 
as the initial size for the new pseudo`-terminal.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:GetSize`(`): [_^Upp`:`:Size^ Size]_[* GetSize]()&]
[s2;%% Returns the size (in character cells) of a running pty on 
success, Null on failure.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:SetAtrributes`(const termios`&`): [@(0.0.255) bool]_[* SetAtrri
butes]([@(0.0.255) const]_[_^termios^ termios][@(0.0.255) `&]_[*@3 t])&]
[s2;%% Sets or changes the terminal attributes to [%-*@3 t]. Returns 
true on success. Calling this method before the start of a pty 
process is valid, and returns true. In that case, [%-*@3 t] will 
be used as the initial attributes for the new pseudo`-terminal.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:GetAttributes`(termios`&`): [@(0.0.255) bool]_[* GetAttributes](
[_^termios^ termios][@(0.0.255) `&]_[*@3 t])&]
[s2;%% Fetches the attributes of a running pty. Return true on success.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:Start`(const char`*`,const char`*`,const char`*`): [@(0.0.255) b
ool]_[* Start]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmdline], 
[@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 env]_`=_nullptr, [@(0.0.255) const]_[@(0.0.255) c
har]_`*[*@3 cd]_`=_nullptr)&]
[s0;l288;%% Starts a new pseudo`-terminal process defined by [%-*@3 cmdline]. 
[%-*@3 env] can provide a new environment for the process, if NULL, 
then the new process inherits caller`'s environment. [%-*@3 cd] 
can be used to specify the new current directory for the process.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:Start`(const char`*`,const Upp`:`:Vector`<Upp`:`:String`>`*`,const char`*`,const char`*`): [@(0.0.255) b
ool]_[* Start]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmd], [@(0.0.255) const]_[_^Upp`:`:Vector^ V
ector]<[_^Upp`:`:String^ String]>_`*[*@3 args], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 e
nv]_`=_nullptr, [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cd]_`=_nullptr)&]
[s2;%% Starts a new process defined by [%-*@3 cmd], [%-*@3 args]. [%-*@3 env] 
can provide a new environment for the process, if NULL, then 
the new process inherits caller`'s environment. This variant 
passes individual arguments instead of whole commandline, this 
has advantage that arguments are in POSIX passed directly to 
execv, without parsing the commandline. [%-*@3 cd] can be used 
to specify the new current directory for the process.&]
[s3;%% &]
[s0; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(`): [* PtyProcess]()&]
[s2;%% Default constructor.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(const char`*`,const char`*`): [* PtyProcess]([@(0.0.255) c
onst]_[@(0.0.255) char]_`*[*@3 cmdline], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 env]_`=
_nullptr)&]
[s2;%% Constructor. Calls Start method with the given [%-*@3 cmdline] 
and [%-*@3 env].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:PtyProcess`:`:PtyProcess`(const char`*`,const Upp`:`:Vector`<Upp`:`:String`>`*`,const char`*`): [* P
tyProcess]([@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 cmd], [@(0.0.255) const]_[_^Upp`:`:Vector^ V
ector]<[_^Upp`:`:String^ String]>_`*[*@3 args], [@(0.0.255) const]_[@(0.0.255) char]_`*[*@3 e
nv]_`=_nullptr)&]
[s2;%%  Constructor. Calls Start method with the given [%-*@3 cmd], 
[%-*@3 args ]and [%-*@3 env].&]
[s3;%% &]
[s0;%% ]]