topic "Job";
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
[ {{10000@(113.42.0) [s0;%% [*@7;4 Job]]}}&]
[s3; &]
[s1;:noref: [@(0.0.255)3 template][3 _<][@(0.0.255)3 class][3 _][*@4;3 T][3 >]&]
[s1;:Upp`:`:Job`:`:class: [@(0.0.255) class]_[* Job]&]
[s0;#l288;%% This template class implements a scope`-bound, lightweight, 
single worker thread based on [^https`:`/`/en`.wikipedia`.org`/wiki`/Resource`_acquisition`_is`_initialization`?oldformat`=true^ R
AII] principle. It provides a return semantics for result gathering, 
functionally similar to the promise`-future pattern, including 
void type specialization, and exception propagation. &]
[s0;#l288;%% &]
[s0;#l288;%% Note that Job is meant to be a hybrid between the so`-called 
`"dedicated thread`" and `"worker thread`" models. While this 
class is a decent general`-purpose multithreading tool, for high 
performance parallelization scenarios [^topic`:`/`/Core`/src`/CoWork`$en`-us`#CoWork`:`:class^ C
oWork] would be a more suitable option. Job is mainly designed 
to allow applications and libraries to gain an easily managable, 
optional non`-blocking behavior where high latency is expected 
such as network operations and file I/O, and a safe, container`-style 
access to the data processed by the worker threads is preferred. 
&]
[s2;#%% &]
[s3; &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:Job`:`:Do`(Upp`:`:Function`&`&`,Args`&`&`.`.`.args`): [@(0.0.255) template]_
<[@(0.0.255) class]_[*@4 Function], [@(0.0.255) class...]_[*@4 Args]>_[@(0.0.255) bool]_[* Do
]([*@4 Function][@(0.0.255) `&`&]_[*@3 f], [*@4 Args][@(0.0.255) `&`&...]_args)&]
[s2;%% Starts job [%-*@3 f] with parameters args in another thread. 
Returns true if the work is successfully started. A Job can run 
only one [%-*@3 f] at a time. Invoking this method while a work 
is in progress simply returns false.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Job`:`:IsFinished`(`)const: [@(0.0.255) bool]_[* IsFinished]()_[@(0.0.255) cons
t]&]
[s2;%% Returns true if the work is finished. This method is non`-blocking. 
It returns false if the work is not finished.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Job`:`:Cancel`(`): [@(0.0.255) void]_[* Cancel]()&]
[s2;%% Cancels the job in progress and waits for the worker to finish. 
Rethrows the exception thrown in worker thread.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Job`:`:IsCanceled`(`): [@(0.0.255) static] [@(0.0.255) bool]_[* IsCanceled]()&]
[s2;%% Returns true if the job is canceled. Intended to be called 
[*/ within ]the worker thread.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Job`:`:Get`(`): [*@4 T]_[* Get]()&]
[s5;:Upp`:`:Job`:`:operator`~`(`): [*@4 T]_[* operator`~]()&]
[s2;%% Waits for the work to be finished (if necessary), then returns 
the result. If there was an exception, it is rethrown.&]
[s3; &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:Job`:`:Job`(`): [* Job]()&]
[s2;%% Default constructor. Job class relies on the [^https`:`/`/en`.wikipedia`.org`/wiki`/Resource`_acquisition`_is`_initialization`?oldformat`=true^ R
AII] (resource acquisition is initialization) principle: Worker 
thread is initialized on class construction, and shut down during 
class destruction. In case of acquisiton errors, Job instances 
will throw [^topic`:`/`/Core`/src`/Exc`_en`-us`#Exc`:`:class^ Exc].&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Job`:`:`~Job`(`): [@(0.0.255) `~][* Job]()&]
[s2;%% Destructor. Cancels the running job and waits for the worker 
to `"join`" with its caller.&]
[s3;%% ]]