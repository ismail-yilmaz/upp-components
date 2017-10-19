topic "Job";
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
[ {{10000@(113.42.0) [s0;%% [*@7;4 Job]]}}&]
[s3; &]
[s1;:noref: [@(0.0.255)3 template][3 _<][@(0.0.255)3 class][3 _][*@4;3 T][3 >]&]
[s1;:Upp`:`:Job`:`:class: [@(0.0.255) class]_[* Job]_:_[@(0.0.255) public]_[*@3 Pte]<[* Job]<[*@4 T
]>>, [@(0.0.255) private]_[*@3 NoCopy]&]
[s0;#l288;%% This template class implements a scope bound, single 
worker thread based on [^https`:`/`/en`.wikipedia`.org`/wiki`/Resource`_acquisition`_is`_initialization`?oldformat`=true^ R
AII] principle. It provides a return semantics for result gathering, 
functionally similar to promise/future pattern (including void 
type specialization). Also it provides a convenient error management 
and exception propagation mechanisms for worker threads, and 
it is compatible with single`-threaded mode.&]
[s2;#%% &]
[s2;#%% Note that while Job is a general purpose multithreading tool, 
for high performance loop parallelization scenarios [^topic`:`/`/Core`/src`/CoWork`$en`-us`#CoWork`:`:class^ C
oWork] would be a more suitable option. This class is mainly 
designed to allow applications and libraries to gain an easily 
managable, optional non`-blocking behavior where high latency 
is expected such as network operations and file I/O, and a safe, 
container`-style access to the data processed by the worker threads 
is preferred.&]
[s3; &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Public Method List]]}}&]
[s3; &]
[s5;:Upp`:`:JobBase`:`:Start`(Upp`:`:Function`<T`(`)`>`&`&`): [@(0.0.255) bool]_[* Start](
[_^Upp`:`:Function^ Function]<[*@4 T]()>`&`&_[*@3 fn])&]
[s5;:Upp`:`:JobBase`:`:operator`&`(Upp`:`:Function`<T`(`)`>`&`&`): [@(0.0.255) bool]_[* o
perator`&]([_^Upp`:`:Function^ Function]<[*@4 T]()>`&`&_[*@3 fn])&]
[s2;%% Starts callback [%-*@3 fn] asynchronously, using a worker thread. 
In a multithreaded environment this method returns true if the 
job is successfully started. In a single`-threaded environment 
it will return true if the job is finished. Result of the job 
can be obtained by using [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetResult`(`)^ G
etResult()] method. Specific error message, if any, can be obtained 
by using [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetError`(`)const^ G
etError()] and [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetErrorDesc`(`)const^ G
etErrorDesc()] methods. As a design decision, no scheduling is 
available: A Job can run only one [%-*@3 fn] at a time. Invoking 
this method while the job is in progress simply returns false. 
&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:Finish`(`): [@(0.0.255) void]_[* Finish]()&]
[s2;%% Waits until the job is finished. Provides per`-job external 
blocking. Note that finished jobs are not `"joined`". &]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:IsFinished`(`): [@(0.0.255) bool]_[* IsFinished]()&]
[s2;%% Returns true if the job is finished. Basically a non`-blocking 
variant of [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:Finish`(`)^ Finis
h()]. &]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:Cancel`(`): [@(0.0.255) void]_[* Cancel]()&]
[s2;%% Sets on the cancellation flag for the job. In order for this 
cancellation mechanism to work, the flag should be checked from 
within the executed callback by using the [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:IsJobCancelled`(`)^ I
sJobCancelled()] global function, and further action should be 
taken accordingly. Note that this method does not wait for the 
job to finish. &]
[s6;%% Requires multithreaded or non`-blocking calls.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Job`:`:GetResult`(`): [*@4 T]_[* GetResult]()&]
[s2;%% Returns the result of the job. This method will propagate 
any exception raised by the worker to its caller. See also [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:Clear`(`)^ C
lear()].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Job`:`:operator`~`(`): [@(0.0.255) const ][*@4 T`&]_[* operator`~]()&]
[s2;%% Provides constant reference access to the result of the job. 
This method will propagate any exception raised by the worker 
to its caller. See also [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:Clear`(`)^ C
lear()].&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:Clear`(`): [@(0.0.255) void]_[* Clear]()&]
[s5;:Upp`:`:JobBase`:`:operator`=`(const Upp`:`:Nuller`&`): [@(0.0.255) void]_[* operator
`=]([@(0.0.255) const]_[_^Upp`:`:Nuller^ Nuller][@(0.0.255) `&])&]
[s2;%% Clears data. Useful for clearing the previous work`'s result. 
It should also be invoked to revalidate the job after its data 
is `"picked`".&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:GetWorker`(`): [_^Upp`:`:Thread^ Thread][@(0.0.255) `&]_[* GetWorker
()]&]
[s2;%% Returns a reference to the worker thread associated with the 
Job instance. It is possible to inspect and modify the properties 
(such as priority, etc.) of the worker, using this method. Please 
keep in mind that workers are job bound. Detaching a worker from 
its job is illegal, and will lead to crash.&]
[s6;%% Requires multithreading.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:GetWorkerId`(`): [_^Upp`:`:int64^ int64]_[* GetWorkerId]()&]
[s2;%% Returns the worker id of the job.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:IsError`(`): [@(0.0.255) bool]_[* IsError]()&]
[s2;%% Returns true when the job has failed. Specific error code 
and message, if any, can be obtained by using [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetError`(`)const^ G
etError()] and [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetErrorDesc`(`)const^ G
etErrorDesc()] methods. This method will propagate any exception 
raised by the worker to its caller.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:GetError`(`)const: [@(0.0.255) int]_[* GetError]()_[@(0.0.255) const
]&]
[s2;%% Returns last error code, if any. (`-1 is considered a generic 
error.)&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobBase`:`:GetErrorDesc`(`)const: [_^Upp`:`:String^ String]_[* GetErrorDesc](
)_[@(0.0.255) const]&]
[s2;%% Returns last error message, if any.&]
[s3;%% &]
[s3; &]
[ {{10000F(128)G(128)@1 [s0;%% [* Constructor detail]]}}&]
[s3; &]
[s5;:Upp`:`:Job`:`:Job`(`): [* Job]()&]
[s2;%% Default constructor. Job class relies on the [^https`:`/`/en`.wikipedia`.org`/wiki`/Resource`_acquisition`_is`_initialization`?oldformat`=true^ R
AII] (resource acquisition is initialization) principle: Worker 
thread is initialized on class construction, and shut down during 
class destruction. In case of acquisiton errors, Job instances 
will throw [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:JobError`:`:struct^ JobE
rror].&]
[s3; &]
[s4; &]
[s5;:Upp`:`:Job`:`:Job`(Upp`:`:Function`<T`(`)`>`&`&`): [* Job]([_^Upp`:`:Function^ Funct
ion]<[*@4 T]()>`&`&_[*@3 fn])&]
[s2;%% Constructs a [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:class^ Job 
]instance and then calls [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:Start`(Upp`:`:Function`<T`(`)`>`)^ S
tart()]. In case of acquisition errors, Job instances will throw 
[^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:JobError`:`:struct^ JobError].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:Job`:`:`~Job`(`): [@(0.0.255) `~][* Job]()&]
[s2;%% Destructor. Notifies workers about shutdown, calls [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:Finish`(`)^ F
inish()] and waits for the worker to `"join`".&]
[s3;%% &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 JobError]]}}&]
[s0; &]
[s1;:Upp`:`:JobError`:`:struct: [@(0.0.255)3 struct][3 _][*3 JobError][3 _:_][@(0.0.255)3 public
][3 _][*@3;3 Exc]&]
[s2;%% Type used as [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:class^ Job] 
exception. This helper struct is meant to be used by [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:class^ J
ob] workers (callbacks) to halt their jobs, and report errors. 
Error code and message sent throwing this exception can be retrieved 
using [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetError`(`)const^ Job
`::GetError()] and [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:GetErrorDesc`(`)const^ J
ob`::GetErrorDesc()] methods.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobError`:`:JobError`(`): [* JobError]()&]
[s2;%% Default constructor. Sets the error code to `-1, and error 
message to Null.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:JobError`:`:JobError`(const Upp`:`:String`&`): [* JobError]([@(0.0.255) const
]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 reason])&]
[s2;%% Constructor overload. Sets error code to `-1, and error message 
to [%-*@3 reason].&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:JobError`:`:JobError`(int`,const Upp`:`:String`&`): [* JobError]([@(0.0.255) i
nt]_[*@3 rc], [@(0.0.255) const]_[_^Upp`:`:String^ String][@(0.0.255) `&]_[*@3 reason])&]
[s2;%% Constructor overload. Sets the error code to [%-*@3 rc], and 
the error message to [%-*@3 reason].&]
[s3;%% &]
[s0;%% &]
[ {{10000@(113.42.0) [s0;%% [*@7;4 Global Functions]]}}&]
[s2; &]
[s2; Below are the helper functions that can be used globally in 
conjunction with jobs, or from within the [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:Job`:`:class^ J
ob] workers (callbacks).&]
[s3; &]
[s4; &]
[s5;:Upp`:`:GetWorkerId`(`): [_^Upp`:`:int64^ int64]_[* GetWorkerId]()&]
[s2;%% Returns the worker id of a given job. This function is meant 
to be called from within the executed callbacks.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:GetJobCount`(`): [_^Upp`:`:int64^ int64]_[* GetJobCount]()&]
[s2;%% Returns the number of running jobs.&]
[s3;%% &]
[s4; &]
[s5;:Upp`:`:WaitForJobs`(`): [@(0.0.255) void]_[* WaitForJobs]()&]
[s2;%% Waits for all running jobs to finish.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:CancelJobs`(`): [@(0.0.255) void]_[* CancelJobs]()&]
[s2;%% Sets the `"Cancel`" flag on for all running jobs, waits before 
them terminate, and then sets the flag off again. It is meant 
to be used together with [^topic`:`/`/Job`/src`/Upp`_Job`$en`-us`#Upp`:`:IsJobCancelled`(`)^ I
sJobCancelled()] function to terminate long running Jobs. Main 
thread calls CancelJobs(), jobs test IsJobCancelled() and if 
true, exit. &]
[s6;%% Requires multithreading or non`-blocking calls.&]
[s3; &]
[s4; &]
[s5;:Upp`:`:IsJobCancelled`(`): [@(0.0.255) bool]_[* IsJobCancelled]()&]
[s2;%% Returns true if the job is canceled. Intended to be called 
by the workers to verify their jobs`'s status. This method will 
first check the global shutdown state of all threads. If this 
fails, it will check the global cancellation state of all running 
jobs. Finally, if the first two checks fail, it will check the 
cancellation flag of the job itself.&]
[s6;%% Requires multithreading or non`-blocking calls.&]
[s3;%% &]
[s0;%% ]]