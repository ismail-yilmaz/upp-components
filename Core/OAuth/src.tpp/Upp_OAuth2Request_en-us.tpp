topic "OAuth2Request";
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
[ {{10000@(113.42.0) [s0;%% [*@7;4 OAuth2Request]]}}&]
[s3; &]
[s1;:Upp`:`:OAuth2Request`:`:class: [@(0.0.255)3 class][3 _][*3 OAuth2Request]&]
[s0;l288;%% This class encapsulates the [^https`:`/`/tools`.ietf`.org`/html`/rfc6749`#section`-4`.1^ a
uthorization code grant] flow of [^https`:`/`/tools`.ietf`.org`/html`/rfc6749^ OAut
h2] authorization protocol. It provides the native clients (such 
as desktop applications) with a means for safely accessing protected 
resources.&]
[s3;%% &]
[s6; Requires Core/SSL package.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:OAuth2Request`:`:Credentials`(const Upp`:`:String`&`,const Upp`:`:String`&`): [_^Upp`:`:OAuth2Request^ O
Auth2Request][@(0.0.255) `&]_[* Credentials]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 id], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 secret])&]
[s2;%% Sets the client [%-*@3 id] and [%-*@3 secret] for the request. 
Returns `*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:BasicAuth`(bool`): [_^Upp`:`:OAuth2Request^ OAuth2Request][@(0.0.255) `&
]_[* BasicAuth]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Enables the basic authentication mode. In this mode the client 
id and secret will be sent as a user`-password pair in the authorization 
field of HTTP header. Returns `*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:UseNonce`(bool`): [_^Upp`:`:OAuth2Request^ OAuth2Request][@(0.0.255) `&
]_[* UseNonce]([@(0.0.255) bool]_[*@3 b]_`=_[@(0.0.255) true])&]
[s2;%% Adss an optional opaque value to the code requests. This may 
help preventing [^https`:`/`/tools`.ietf`.org`/html`/rfc6749`#section`-10`.12^ cros
s`-site request forgeries]. Returns `*this for method chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:Port`(int`): [_^Upp`:`:OAuth2Request^ OAuth2Request][@(0.0.255) `&
]_[* Port]([@(0.0.255) int]_[*@3 prt])&]
[s2;%% Sets the listen port for `"localhost`" redirections to [%-*@3 prt]. 
Returns `*this for method chaining. &]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:Timeout`(int`): [_^Upp`:`:OAuth2Request^ OAuth2Request][@(0.0.255) `&
]_[* Timeout]([@(0.0.255) int]_[*@3 ms])&]
[s2;%% Sets the connection timeout value in miliseconds. Default 
timeout value is 120.000 ms (2 minutes). Returns `*this for method 
chaining.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:CallbackUrl`(const Upp`:`:String`&`): [_^Upp`:`:OAuth2Request^ O
Auth2Request][@(0.0.255) `&]_[* CallbackUrl]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 url])&]
[s2;%% Sets the redirection [%-*@3 url]. Returns `*this for method 
chaining. The reason for this method is that some oauth2`-based 
services don`'t allow redirections to localhost. In such cases 
a URI on a trusted server which will then redirect the response 
to localhost can be used as a workaround. Setting the [%-*@3 url 
]to Null cancels the extra redirection.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:GetAuthCode`(const Upp`:`:ValueMap`&`,Upp`:`:String`&`): [@(0.0.255) b
ool]_[* GetAuthCode]([@(0.0.255) const]_[_^Upp`:`:ValueMap^ ValueMap][@(0.0.255) `&]_[*@3 r
equest], [_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 code])&]
[s2;%% Requests an authorization [%-*@3 code ]from the authorization 
server. The [^topic`:`/`/OAuth`/src`/Upp`_OAuth2Request`_en`-us`#Upp`:`:OAuth2CodeRequest`(const Upp`:`:String`&`,const Upp`:`:String`&`)^ O
Auth2CodeRequest()] helper function can be used to create a minimal 
[%-*@3 request]. Returns true on success. Note that some services 
provide additional parameters with the auth code response. These 
parameters can be examined and obtained using the [^topic`:`/`/OAuth`/src`/Upp`_OAuth2Request`_en`-us`#Upp`:`:OAuth2Request`:`:WhenCheck^ W
henCheck] event.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:GetAccessToken`(const Upp`:`:ValueMap`&`,Upp`:`:String`&`): [@(0.0.255) b
ool]_[* GetAccessToken]([@(0.0.255) const]_[_^Upp`:`:ValueMap^ ValueMap][@(0.0.255) `&]_[*@3 r
equest], [_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 response])&]
[s2;%% Requests an access token from the authorization server. The 
[^topic`:`/`/OAuth`/src`/Upp`_OAuth2Request`_en`-us`#Upp`:`:OAuth2TokenRequest`(const Upp`:`:String`&`,const Upp`:`:String`&`,const Upp`:`:String`&`)^ O
Auth2TokenRequest()] helper function can be used to create a 
minimal [%-*@3 request]. If the request is successful this method 
will return true and the [%-*@3 response ]should be filled  with 
a [^topic`:`/`/Core`/src`/JSON`_en`-us^ JSON] structure that contains 
the token. Note that, however, some services don`'t comply to 
the OAuth2 standard, and may send the token in plain text or 
wrap it up in some other type of structure.  &]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:GetRefreshToken`(const Upp`:`:ValueMap`&`,Upp`:`:String`&`): [@(0.0.255) b
ool]_[* GetRefreshToken]([@(0.0.255) const]_[_^Upp`:`:ValueMap^ ValueMap][@(0.0.255) `&]_
[*@3 request], [_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 response])&]
[s2;%% Requests a refresh token from the authorization server. The 
[^topic`:`/`/OAuth`/src`/Upp`_OAuth2Request`_en`-us`#Upp`:`:OAuth2RefreshRequest`(const Upp`:`:String`&`,const Upp`:`:String`&`,const Upp`:`:String`&`)^ O
Auth2RefreshRequest()] helper function can be used to create 
a minimal [%-*@3 request]. If the request is successful this method 
will return true and the [%-*@3 response ]should be filled with 
a [^topic`:`/`/Core`/src`/JSON`_en`-us^ JSON] structure that contains 
the token. Note that, however, some services don`'t comply to 
the OAuth2 standard, and may send the token in plain text or 
wrap it up in some other type of structure.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:IsError`(`)const: [@(0.0.255) bool]_[* IsError]()_[@(0.0.255) c
onst]&]
[s2;%% Returns true if there was an error.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:GetErrorDesc`(`)const: [_^Upp`:`:String^ String]_[* GetError
Desc]()_[@(0.0.255) const]&]
[s2;%% Returns the last error message, if any.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:WhenWait: [_^Upp`:`:Event^ Event]<>_[* WhenWait]&]
[s2;%% If this callback is defined, it is invoked periodically while 
any socket operation is performed. This is intended to give user 
feedback in interactive applications.,&]
[s3; &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:WhenAuth: [_^Upp`:`:Event^ Event]<[_^Upp`:`:String^ String]>
_[* WhenAuth]&]
[s2;%% By default, the user sign`-in URL is opened in the default 
web browser. If you don`'t want to use the default web browser 
(e.g. if you want to use an embedded web browser component `-such 
as CEF`- for sign`-ins in your application) you should define 
this event.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:WhenCheck: [_^Upp`:`:Event^ Event]<[_^Upp`:`:ValueMap^ Value
Map]>_[* WhenCheck]&]
[s2;%% If defined, this event will be emitted immediately after a 
successful authorization code request (i.e. redirection). Some 
services provide additional parameters in the response for this 
request. In such cases the parameter list (consisted of key`-value 
pairs) of the response can be examined using this event.&]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:OAuth2Request`:`:OAuth2Request`(`): [* OAuth2Request]()&]
[s2;%% Default constructor.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:OAuth2Request`(const Upp`:`:String`&`,const Upp`:`:String`&`): [* O
Auth2Request]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 id], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 secret])&]
[s2;%% Constructs an OAuth2 client with the client [%-*@3 id] and [%-*@3 secret].&]
[s3;%% &]
[ {{10000F(128)G(128)@1 [s0;%% [* Function List]]}}&]
[s3; &]
[s5;:Upp`:`:OAuth2CodeRequest`(const Upp`:`:String`&`,const Upp`:`:String`&`): [_^Upp`:`:ValueMap^ V
alueMap]_[* OAuth2CodeRequest]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 endpoint], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 scope])&]
[s2;%% Creates a minimal authorization code request for the authorization 
[%-*@3 endpoint] with [%-*@3 scope].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2TokenRequest`(const Upp`:`:String`&`,const Upp`:`:String`&`,const Upp`:`:String`&`): [_^Upp`:`:ValueMap^ V
alueMap]_[* OAuth2TokenRequest]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 endpoint], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 code], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 scope]_`=_Null)&]
[s2;%% Creates a minimal access token request for the token [%-*@3 endpoint] 
with [%-*@3 scope], and a formerly obtained authorization [%-*@3 code].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:OAuth2RefreshRequest`(const Upp`:`:String`&`,const Upp`:`:String`&`,const Upp`:`:String`&`): [_^Upp`:`:ValueMap^ V
alueMap]_[* OAuth2RefreshRequest]([@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&
]_[*@3 endpoint], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 token], 
[@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 scope]_`=_Null)&]
[s2;%% Creates a minimal refresh token request for the token [%-*@3 endpoint] 
with [%-*@3 scope], and a formerly obtained authorization access 
[%-*@3 token].&]
[s3;%% &]
[s0; &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 OAuth2Request:Error]]}}&]
[s3; &]
[s1;:Upp`:`:OAuth2Request`:`:Error`:`:struct: [@(0.0.255)3 struct][3 _][*3 Error][3 _:_][@(0.0.255)3 p
ublic][3 _][*@3;3 Exc]&]
[s2;%% Type used as the OAuth2Request exception. This helper is internally 
used by the OAuth2Request class, but can also be used externally 
by client code to halt jobs, and report errors (e.g. in worker 
threads).&]
[s3; &]
[s4; &]
[s5;:Upp`:`:OAuth2Request`:`:Error`:`:Error`(const Upp`:`:String`&`): [* Error]([@(0.0.255) c
onst]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 reason])&]
[s2;%% Constructor. Sets the error message to [%-*@3 reason].&]
[s3;%% &]
[s0;%% ]]