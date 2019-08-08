topic "SixelRenderer";
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
[ {{10000@(113.42.0) [s0;%% [*@7;4 SixelRenderer]]}}&]
[s0; &]
[s1;:Upp`:`:SixelRenderer`:`:class: [@(0.0.255)3 class][3 _][*3 SixelRenderer][3 _:_][@(0.0.255)3 p
rivate][3 _][*@3;3 NoCopy]&]
[s2;%% This helper class renders a [^https`:`/`/en`.wikipedia`.org`/wiki`/Sixel`?oldformat`=true^ s
ixel] image. It can handle both RGB and HSL color spaces, and 
has true color support. Sixel data must only contain the [/ payload 
]of a device control string (DCS).&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:SixelRenderer`:`:Get`(`): [_^Upp`:`:Image^ Image]_[* Get]()&]
[s5;:Upp`:`:SixelRenderer`:`:operator Image`(`): [* operator_Image]()&]
[s2;%% Processes the sixel data and returns the result as an [^topic`:`/`/Draw`/src`/Image`_en`-us`#Image`:`:class^ I
mage] object.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:SetSize`(Upp`:`:Size`): [_^Upp`:`:SixelRenderer^ SixelRend
erer][@(0.0.255) `&]_[* SetSize]([_^Upp`:`:Size^ Size]_[*@3 sz])&]
[s2;%% Suggests a canvas size to the renderer. This value will be 
overridden by the renderer if the sixel data contains a size 
information. The default canvas size is 640 x 480 pixels. Returns 
`*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:GetSize`(`)const: [_^Upp`:`:Size^ Size]_[* GetSize]()_[@(0.0.255) c
onst]&]
[s2;%% Returns the suggested or actual size of the canvas. Returns 
Null if it is not specified. In that case the renderer will default 
the canvas size to 640 x 480 pixels.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:SetPaper`(Upp`:`:Color`): [_^Upp`:`:SixelRenderer^ SixelRe
nderer][@(0.0.255) `&]_[* SetPaper]([_^Upp`:`:Color^ Color]_[*@3 c])&]
[s2;%% Sets the background color of image to [%-*@3 c]. Returns `*this 
for method chaining. The default background color is [^topic`:`/`/Draw`/src`/Colors`_en`-us`#SColorPaper`(`)^ S
ColorPaper].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:SetAspectRatio`(int`): [_^Upp`:`:SixelRenderer^ SixelRende
rer][@(0.0.255) `&]_[* SetAspectRatio]([@(0.0.255) int]_[*@3 r])&]
[s2;%% Suggests an aspect ratio for the image to be produced. This 
value will be overridden by the renderer if the sixel data contains 
an aspect ratio information. Returns `*this for method chaining. 
&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:GetAspectRatio`(`)const: [@(0.0.255) int]_[* GetAspectRatio](
)_[@(0.0.255) const]&]
[s2;%% Returns the suggested or actual aspect ratio of the image 
to be produced. &]
[s3; &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:NoColorHole`(bool`): [_^Upp`:`:SixelRenderer^ SixelRendere
r][@(0.0.255) `&]_[* NoColorHole]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% When [%-*@3 b] is true, the canvas background will be painted 
with the provided or default paper color.&]
[s0; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:SixelRenderer`:`:SixelRenderer`(const Upp`:`:String`&`): [* SixelRenderer](
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 data])&]
[s2;%% Constructs a sixel renderer object from [%-*@3 data].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:SixelRenderer`(const Upp`:`:String`&`,Upp`:`:Size`): [* Si
xelRenderer]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 data], 
[_^Upp`:`:Size^ Size]_[*@3 sz])&]
[s2;%% Constructs a sixel renderer object from [%-*@3 data]. The size 
argument is supplied only as a hint, and this value will be overridden 
by the renderer if the sixel data contains a size information. 
&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:SixelRenderer`(const Upp`:`:String`&`,Upp`:`:SixelRenderer`:`:Info`): [* S
ixelRenderer]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 data], 
[_^Upp`:`:SixelRenderer`:`:Info^ Info]_[*@3 info])&]
[s2;%% Constructs a sixel renderer object from [%-*@3 data]. [%-*@3 info] 
is used to suggest specific rendering flags to the sixel renderer. 
For more information, see [^topic`:`/`/Terminal`/src`/Upp`_SixelRenderer`_en`-us`#Upp`:`:SixelRenderer`:`:Info`:`:struct^ I
nfo] structure.&]
[s3;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Function List]]}}&]
[s3; &]
[s5;:Upp`:`:RenderSixelImage`(const Upp`:`:String`&`,const Upp`:`:Size`&`,Upp`:`:Color`): [_^Upp`:`:Image^ I
mage]_[* RenderSixelImage]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 s
ixeldata], [@(0.0.255) const]_[_^Upp`:`:Size^ Size][@(0.0.255) `&]_[*@3 sizehint], 
[_^Upp`:`:Color^ Color]_[*@3 paper])&]
[s2;%% This convenience function should be preferred for any standalone 
usage of SixelRenderer class. Unlike the SixelRenderer class, 
which accepts only the DCS [/ payload ]as its data, this function 
utilizes the [^topic`:`/`/Terminal`/src`/Upp`_VTInStream`_en`-us`#Upp`:`:VTInStream`:`:class^ V
TInStream] to filter out any control bytes or escape sequences 
from the provided [%-*@3 sixeldata]. [%-*@3 sizehint] parameter can 
be used to suggest a canvas size to the renderer. This value 
will be overridden by the renderer if the sixel data contains 
a size information. [%-*@3 paper] can be used to set the background 
color of the canvas. Again, the renderer can ignore the paper 
color, depending on the color hole flag of sixel data.&]
[s3;%% &]
[s3;%% &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 SixelRenderer`::Info]]}}&]
[s0;%% &]
[s1;:Upp`:`:SixelRenderer`:`:Info`:`:struct: [@(0.0.255)3 struct][3 _][*3 Info]&]
[s2;%% This simple member structure is used for passing basic rendering 
information to the SixelRenderer instances. Note that some of 
these informations are only suggestions. Renderer is free to 
override them.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:Info`:`:aspectratio: [@(0.0.255) int]_[* aspectratio]&]
[s2;%% The aspect ratio value can bu used to supply an aspect ratio 
hint to the SixelRenderer instances. This value will be overridden 
by the renderer if the sixel data already contains a size information&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:Info`:`:nohole: [@(0.0.255) bool]_[* nohole]&]
[s2;%% When this flag is true, the canvas background will be painted 
with the provided or default paper color.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:SixelRenderer`:`:Info`:`:size: [_^Upp`:`:Size^ Size]_[* size]&]
[s2;%% The size value can bu used to hint an initial canvas size 
to the SixelRenderer instances. This value will be overridden 
by the renderer if the sixel data already contains a size information&]
[s0;%% &]
[s4; &]
[s5;:Upp`:`:SixelInfo`:`:typedef: [* SixelInfo]&]
[s2;%% This is an alias for [^topic`:`/`/Terminal`/src`/Upp`_SixelRenderer`_en`-us`#Upp`:`:SixelRenderer`:`:Info`:`:struct^ S
ixelRenderer`::Info] structure.&]
[s3; &]
[s3;%% ]]