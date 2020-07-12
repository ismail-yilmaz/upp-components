topic "VTInStream";
[i448;a25;kKO9;2 $$1,0#37138531426314131252341829483380:class]
[l288;2 $$2,2#27521748481378242620020725143825:desc]
[0 $$3,0#96390100711032703541132217272105:end]
[H6;0 $$4,0#05600065144404261032431302351956:begin]
[i448;a25;kKO9;2 $$5,0#37138531426314131252341829483370:item]
[l288;a4;*@5;1 $$6,6#70004532496200323422659154056402:requirement]
[l288;i1121;b17;O9;~~~.1408;2 $$7,0#10431211400427159095818037425705:param]
[i448;b42;O9;2 $$8,8#61672508125594000341940100500538:tparam]
[b42;2 $$9,9#13035079074754324216151401829390:normal]
[b42;a42;2 $$10,10#45413000475342174754091244180557:text]
[2 $$0,0#00000000000000000000000000000000:Default]
[{_} 
[ {{10000@(113.42.0) [s0;%% [*@7;4 VTInStream]]}}&]
[s0; &]
[s1;:Upp`:`:VTInStream`:`:class: [@(0.0.255)3 class][3 _][*3 VTInStream][3 _:_][@(0.0.255)3 publ
ic][3 _][*@3;3 MemReadStream]&]
[s2;%% This class encapsulates a reliable and high`-performance VT500 
series “lexical” parser designed specifically for handling 
DEC `& ANSI escape sequences. It is implemented as a finite state 
machine built upon a very fast input stream object, and is based 
on [^https`:`/`/vt100`.net`/emu`/dec`_ansi`_parser^ the UML state 
diagram publicized by Paul`-Flo Williams]. &]
[s2;%% &]
[s2;%% VTInStream can handle UTF`-8 and non UTF`-8 characters, C0 
and C1 control bytes, and ESC, CSI, DCS, OSC, and APC sequences 
in both 7`-bits and 8`-bits forms, allows switching between UTF`-8 
and non UTF`-8 modes on`-the`-fly, and can be used as both a 
parser and a filter.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:VTInStream`:`:Parse`(const Upp`:`:String`&`,bool`): [@(0.0.255) void]_[* Pars
e]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 data], 
[@(0.0.255) bool]_[*@3 utf8])&]
[s5;:Upp`:`:VTInStream`:`:Parse`(const void`*`,int`,bool`): [@(0.0.255) void]_[* Parse]([@(0.0.255) c
onst]_[@(0.0.255) void]_`*[*@3 data], [@(0.0.255) int]_[*@3 size], [@(0.0.255) bool]_[*@3 utf
8])&]
[s2;%% Parses a [%-*@3 data] string, or a [%-*@3 data].buffer with given 
[%-*@3 size]. The parser will switch to UTF`-8 mode when the [%-*@3 utf8] 
flag is enabled and this flag can be enabled or disabled run`-time. 
Important note: VTInStream has no explicit error state. It will 
ignore the remaining bytes of the erroneous sequence if it encounters 
any. Still, it is possible to reset the parser to its initial 
state at any moment, using the [^topic`:`/`/Terminal`/src`/Upp`_VTInStream`_en`-us`#Upp`:`:VTInStream`:`:Reset`(`)^ R
eset()] method.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Reset`(`): [@(0.0.255) void]_[* Reset]()&]
[s2;%% Resets the parser to its initial state.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WasChr`(`)const: [@(0.0.255) bool]_[* WasChr]()_[@(0.0.255) const
]&]
[s2;%% Returns true if the last item was a character and not a sequence.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenChr: [_^Upp`:`:Event^ Event]<[@(0.0.255) int]>_[* WhenChr]&]
[s2;%% This event is dispatched when a UTF`-8 or non UTF`-8 character 
is received.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenCtl: [_^Upp`:`:Event^ Event]<[_^Upp`:`:byte^ byte]>_[* WhenCt
l]&]
[s2;%% This event is dispatched when a control byte (either in 0x00`-0x7F 
range or in 0x80`-0x9F range) is received. &]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenEsc: [_^Upp`:`:Event^ Event]<[@(0.0.255) const]_VTInStream`:
:Sequence[@(0.0.255) `&]>_[* WhenEsc]&]
[s2;%% This event is dispatched when an escape sequence is received.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenCsi: [_^Upp`:`:Event^ Event]<[@(0.0.255) const]_VTInStream`:
:Sequence[@(0.0.255) `&]>_[* WhenCsi]&]
[s2;%% This event is dispatched when a control function is received.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenDcs: [_^Upp`:`:Event^ Event]<[@(0.0.255) const]_VTInStream`:
:Sequence[@(0.0.255) `&]>_[* WhenDcs]&]
[s2;%% This event is dispatched when a device control string is received.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenOsc: [_^Upp`:`:Event^ Event]<[@(0.0.255) const]_VTInStream`:
:Sequence[@(0.0.255) `&]>_[* WhenOsc]&]
[s2;%% This event is dispatched when an operating system command 
is received.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:WhenApc: [_^Upp`:`:Event^ Event]<[@(0.0.255) const]_VTInStream`:
:Sequence[@(0.0.255) `&]>_[* WhenApc]&]
[s2;%% This event is dispatched when an application programming command 
is received.&]
[s3; &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:VTInStream`:`:VTInStream`(`): [* VTInStream]()&]
[s2;%% Default constructor.&]
[s3; &]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 VTInStream`::Sequence]]}}&]
[s0;%% &]
[s1;:Upp`:`:VTInStream`:`:Sequence`:`:struct: [@(0.0.255)3 struct][3 _][*3 Sequence]&]
[s2;%% This structure represents a ESC, CSI, DCS, or OSC sequence, 
depending on the VTInStream context.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:type: [_^Upp`:`:byte^ byte]_[* type]&]
[s2;%% Represents the type of the sequence. Currently the valid values 
are ESC, CSI, DCS, OSC, and APC.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:opcode: [_^Upp`:`:byte^ byte]_[* opcode]&]
[s2;%% Also known as the `"final`" or `"terminator`" byte. Usually 
represents the opcode of received function.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:mode: [_^Upp`:`:byte^ byte]_[* mode]&]
[s2;%% Contains the context`-dependent mode information of the given 
sequence. E.g. for private (DEC/xterm) functions or modes this 
is usually, but not always, a 0x3F (question mark).&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:parameters: [_^Upp`:`:Vector^ Vector]<[_^Upp`:`:String^ S
tring]>_[* parameters]&]
[s2;%% Contains the function parameters specific to CSI or DCS sequences, 
in a vectorized form. Note that this vector can contain empty 
strings that usually mean an implicit default value, depending 
on the sequence`'s context.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:intermediate: [_^Upp`:`:String^ String]_[* intermed
iate]&]
[s2;%% Contains the collected intermediate bytes specific to the 
given sequence.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:payload: [_^Upp`:`:String^ String]_[* payload]&]
[s2;%% Contains any addtional data to be parsed by client code. For 
example, operating system commands and the string component of 
the device control strings are handed to client code via this 
member variable.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:GetInt`(int`,int`)const: [@(0.0.255) int]_[* GetInt
]([@(0.0.255) int]_[*@3 n], [@(0.0.255) int]_[*@3 d]_`=_[@3 1])_[@(0.0.255) const]&]
[s2;%% A convenience method for parsing integer parameters. [%-*@3 n] 
is the index of parameter. The index is 1`-based. Namely, the 
index of the first parameter would be 1, and the index of the 
fifth parmeter would be 5, etc. A default value to be used instead 
of an erroneous, out`-of`-bounds, or empty (defaulted) sequence 
parameter can be provided with [%-*@3 d].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:GetStr`(int`)const: [_^Upp`:`:String^ String]_[* Ge
tStr]([@(0.0.255) int]_[*@3 n])_[@(0.0.255) const]&]
[s2;%% The same as above but for string parameters. [%-*@3 n ] is the 
index of the sequence parameter. Returns a nullifed string for 
erroneous, out`-of`-bounds, or empty (defaulted) sequence parameters.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:ToString`(`)const: [_^Upp`:`:String^ String]_[* ToS
tring]()_[@(0.0.255) const]&]
[s2;%% Returns the sequence as a String. Useful for diagnostics and 
logging.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:VTInStream`:`:Sequence`:`:Clear`(`): [@(0.0.255) void]_[* Clear]()&]
[s0;%% -|Clears the structure.]]