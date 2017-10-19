#ifndef _Job_Job_h
#define _Job_Job_h

#include <Core/Core.h>

namespace Upp {

template<class T> struct JobResult {
    One<T>  data;
    T&      Get()                           { return *data; }
    void    Set(Function<T()>&  fn)         { *data = pick(fn()); }
    void    New()                           { data = MakeOne<T>(); }
};

template<> struct JobResult<void> {
    void    Get()                           { return void(); }
    void    Set(Function<void()>&  fn)      { fn(); }
    void    New()                           {}
};

template<class T>
class JobBase : public Pte<JobBase<T>>, NoCopy {
public:
    bool    Start(Function<T()>&& fn);
    bool    operator&(Function<T()>&& fn)   { return Start(pick(fn)); }
    void    Finish();
    bool    IsFinished();
    void    Cancel();

    void    Clear()                         { result.New(); exc = NULL; }
    void    operator=(const Nuller&)        { Clear(); }

#ifdef _MULTITHREADED
    Thread  GetWorker()                     { return worker; }
#endif
    int64   GetWorkerId()                   { return wid; }

    bool    IsError()                       { Rethrow(); return status == FAILED; }
    int     GetError() const                { return error.Get<int>(); }
    String  GetErrorDesc() const            { return error.Get<String>(); }

    JobBase()                               { Init(); }
    JobBase(Function<T()>&& fn) : JobBase() { Start(pick(fn)); }
    virtual ~JobBase()                      { Exit(); }

protected:
    JobResult<T>	result;
    Mutex		lock;
    inline void		Rethrow();

private:
#ifdef _MULTITHREADED
    Thread              worker;
    ConditionVariable   cv;
#endif
    Function<T()>       work;
    int64               wid;
    volatile Atomic     status;
    Tuple<int, String>  error;
    std::exception_ptr  exc;

    enum Status         { IDLE, WORKING, FINISHED, FAILED, EXIT };

    void Init();
    void Exit();
    void Work();
};

template<class T>
class Job : public JobBase<T> {
public:
    T        GetResult();
    const T& operator~();
    Job()                                   {}
    Job(Function<T()>&& fn) : Job()         { Start(fn); }
    virtual ~Job()                          {}
};

// Void specialization
template<>
class Job<void> : public JobBase<void> {
public:
    void    GetResult()                     { Rethrow(); return void(); }
    void    operator~()                     { return GetResult(); }
    Job()                                   {}
    Job(Function<void()>&& fn) : Job()      { Start(pick(fn)); }
    virtual ~ Job()							{}
};

struct JobError : Exc {
    int code;
    JobError() : Exc(Null), code(-1) {}
    JobError(const String& reason) : Exc(reason), code(-1) {}
    JobError(int rc, const String& reason) : Exc(reason), code(rc) {}
};

extern int64   GetWorkerId();
extern int64   GetJobCount();
extern void    WaitForJobs();
extern void    CancelJobs();
extern bool    IsJobCancelled();

#include "Job.hpp"
}
#endif
