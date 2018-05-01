topic "AnsiParser";
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
[ {{10000@(113.42.0) [s0;%% [*@7;4 AnsiParser]]}}&]
[s2; &]
[s1;:AnsiParser`:`:class: [@(0.0.255)3 class][3 _][*3 AnsiParser]&]
[s2;%% This class implements a simple yet flexible parser and filter 
for the [^https`:`/`/en`.wikipedia`.org`/wiki`/ANSI`_escape`_code`?oldformat`=true^ A
NSI escape and control sequences]. It is also a suitable tool 
for building more complex parsers.&]
[s2;%% AnsiParser class supports:&]
[s2;%% &]
[s2;i150;O0;%% Both ASCII and UTF`-8 character sets.&]
[s2;i150;O0;%% Parsing of escape and control sequences.&]
[s2;i150;O0;%% Parsing of operating system commands ([^http`:`/`/www`.xfree86`.org`/4`.5`.0`/ctlseqs`.html^ O
SC], xterm extension).&]
[s2;i150;O0;%% Working directly with streams&]
[s2;%% &]
[s2;%% AnsiParser is modal and moveable.&]
[s3; &]
[s0; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:AnsiParser`:`:GetControl`(`)const: [@(0.0.255) int]_[* GetControl]()_[@(0.0.255) const
]&]
[s2;%% Returns the first byte after the ESC character. &]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:GetParameters`(`)const: [_^Upp`:`:Vector^ Vector]<[_^Upp`:`:String^ St
ring]>_[* GetParameters]()_[@(0.0.255) const]&]
[s2;%% Returns the semicolon delimited parameter characters as a 
vector.&]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:GetIntermediate`(`)const: [_^Upp`:`:String^ String]_[* GetIntermediate
]()_[@(0.0.255) const]&]
[s2;%% Returns the intermediary bytes.&]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:GetTerminator`(`)const: [@(0.0.255) int]_[* GetTerminator]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the terminator byte for command sequence and some 
escape sequences.&]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:IsEsc`(int`)const: [@(0.0.255) bool]_[* IsEsc]([@(0.0.255) int]_[*@3 c])_[@(0.0.255) c
onst]&]
[s2;%% Returns true if [%-*@3 c] represents the ESC character.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsCsi`(int`)const: [@(0.0.255) bool]_[* IsCsi]([@(0.0.255) int]_[*@3 c])_[@(0.0.255) c
onst]&]
[s2;%% Returns true if [%-*@3 c] represents the command sequence introducer 
(CSI).&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsOsc`(int`)const: [@(0.0.255) bool]_[* IsOsc]([@(0.0.255) int]_[*@3 c])_[@(0.0.255) c
onst]&]
[s2;%% Returns true if [%-*@3 c] represents the operating system command 
introducer (OSCI)..&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsCmd`(int`)const: [@(0.0.255) bool]_[* IsCmd]([@(0.0.255) int]_[*@3 c])_[@(0.0.255) c
onst]&]
[s2;%% Returns true if [%-*@3 c] represents a CSI or OSCI.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsC0`(int`)const: [@(0.0.255) bool]_[* IsC0]([@(0.0.255) int]_[*@3 c])_[@(0.0.255) c
onst]&]
[s2;%% Returns true if [%-*@3 c] represents a 7`-bit control character.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsC1`(int`)const: [@(0.0.255) bool]_[* IsC1]([@(0.0.255) int]_[*@3 c])_[@(0.0.255) c
onst]&]
[s2;%% Returns true if [%-*@3 c] represents a 8`-bit control character.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsControl`(int`)const: [@(0.0.255) bool]_[* IsControl]([@(0.0.255) int]_
[*@3 c])_[@(0.0.255) const]&]
[s2;%% Returns true if [%-*@3 c] represents a 7`- or 8`-bit control 
character.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsIntermediate`(int`)const: [@(0.0.255) bool]_[* IsIntermediate]([@(0.0.255) i
nt]_[*@3 c])_[@(0.0.255) const]&]
[s2;%% Returns true if [%-*@3 c] represents a intermediary character.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsParameter`(int`)const: [@(0.0.255) bool]_[* IsParameter]([@(0.0.255) i
nt]_[*@3 c])_[@(0.0.255) const]&]
[s2;%% Returns true if [%-*@3 c] represents a parameter character.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:IsAlphabetic`(int`)const: [@(0.0.255) bool]_[* IsAlphabetic]([@(0.0.255) i
nt]_[*@3 c])_[@(0.0.255) const]&]
[s2;%% Returns true if [%-*@3 c] represetns a alphabetic character. 
Note that in ANSI notation, the alphabetic characters include 
more characters than A to Z.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:WhenEsc: [_^Upp`:`:Event^ Event]<>_[* WhenEsc]&]
[s2;%% Invoked when a valid escape sequence is encountered.&]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:WhenCsi: [_^Upp`:`:Event^ Event]<>_[* WhenCsi]&]
[s2;%% Invoked when a valid command sequence is encountered.&]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:WhenOsc: [_^Upp`:`:Event^ Event]<>_[* WhenOsc]&]
[s2;%% Invoked when a valid operating system command sequence is 
encountered.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:Parse`(int`): [@(0.0.255) int]_[* Parse]([@(0.0.255) int]_[*@3 c])&]
[s2;%% This method will process the input and return a negative value 
if the input is part of an escape sequence. Otherwise it will 
return the input unchanged (which means the input [%-*@3 c] is 
a plain character). Throws [^topic`:`/`/AnsiParser`/src`/AnsiParser`_en`-us`#AnsiParser`:`:Error`:`:struct^ A
nsiParser`::Error] on errors.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:Parse`(Upp`:`:Stream`&`,Upp`:`:Event`<int`>`): [@(0.0.255) void]_[* Pa
rse]([_^Upp`:`:Stream^ Stream][@(0.0.255) `&]_[*@3 in], [_^Upp`:`:Event^ Event]<[@(0.0.255) i
nt]>_[*@3 out])&]
[s2;%% This method will process the [%-*@3 in] stream and then output 
the plain characters to [%-*@3 out]. Throws [^topic`:`/`/AnsiParser`/src`/AnsiParser`_en`-us`#AnsiParser`:`:Error`:`:struct^ A
nsiParser`::Error] on errors.&]
[s3;%% &]
[s4; &]
[s5;:AnsiParser`:`:Reset`(`): [@(0.0.255) void]_[* Reset]()&]
[s2;%% Resets the parser.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:AnsiParser`:`:AnsiParser`(`): [* AnsiParser]()&]
[s2;%% Default constructor.&]
[s3; &]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 AnsiParser`::Error]]}}&]
[s3;%% &]
[s1;:AnsiParser`:`:Error`:`:struct: [@(0.0.255)3 struct][3 _][*3 Error][3 _:_][@(0.0.255)3 publi
c][3 _][*@3;3 Exc]&]
[s2;%% Type used as AnsiParser exception.&]
[s3; &]
[s4; &]
[s5;:AnsiParser`:`:Error`:`:Error`(const Upp`:`:String`&`): [* Error]([@(0.0.255) const]_
[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 reason])&]
[s2;%% Constructor. Sets the error message to [%-*@3 reason] .&]
[s3;%% &]
[s0;%% ]]