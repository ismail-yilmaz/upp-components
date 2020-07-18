#ifndef _Job_Job_h_
#define _Job_Job_h_

#include <Core/Core.h>

namespace Upp {

class JobWorker : NoCopy {
public:
    JobWorker();
    ~JobWorker();
    bool                Start(Event<>&& fn);
    void                Stop();
    void                Loop();
    void                Wait();
    void                Cancel()                    { cancel = true; Wait();  Rethrow(); }
    static bool         IsCanceled()                { return ptr && ptr->cancel; }
    void                Rethrow()                   { Mutex::Lock __(lock); if(exc) std::rethrow_exception(exc); }
    inline bool         Is(int stat) const          { return status == stat; }
    
    enum State : int { IDLE, WORKING, SHUTDOWN };
    
private:
    Thread              work;
    Event<>             cb;
    Mutex               lock;
    ConditionVariable   cv, wv;
    std::exception_ptr  exc;
    std::atomic<int>    status;
    std::atomic<bool>   cancel;
    static thread_local JobWorker *ptr;
};

template<class T>
class Job {
private:
    template<class R>
    struct Result {
        JobWorker v;
        R ret;
        template<class Function, class... Args>
        bool       Start(Function&& f, Args&&... args)  { return v.Start([=]{ ret = f(args...);}); }
        const R&   Get()                                { v.Wait(); v.Rethrow(); return ret; }
        R          Pick()                               { v.Wait(); v.Rethrow(); return pick(ret); }
    };

    struct VoidResult {
        JobWorker v;
        template<class Function, class... Args>
        bool       Start(Function&& f, Args&&... args)  { return v.Start([=]{ f(args...); }); }
        void       Get()                                { v.Wait(); v.Rethrow(); }
        void       Pick()                               {}
    };

    using ResType = typename std::conditional<std::is_void<T>::value, VoidResult, Result<T>>::type;
    One<ResType> worker;
    
public:
    template<class Function, class... Args>
    bool    Do(Function&& f, Args&&... args)            { return worker && worker->Start(f, args...); }
    bool    IsFinished() const                          { return !worker || !worker->v.Is(JobWorker::WORKING);  }
    void    Cancel()                                    { ASSERT(worker); worker->v.Cancel(); }
    static bool IsCanceled()                            { return JobWorker::IsCanceled(); }
    T       Get()                                       { ASSERT(worker); return worker->Get(); }
    T       operator~()                                 { return Get(); }
    T       Pick()                                      { ASSERT(worker); return worker->Pick(); }
    Job()                                               { worker.Create(); }
    ~Job()                                              { if(worker) worker.Clear();  }

    Job& operator=(Job&&) = default;
    Job(Job&&) = default;
};
}
#endif
