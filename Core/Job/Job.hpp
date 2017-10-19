#define JLOG(x)	   //   RLOG("Job(): Worker #" << GetWorkerId() << ": " << x)
//#define JLOG(x)	  RLOG("Job(): Worker #" << GetWorkerId() << ": " << x)

namespace JobGlobal {
extern thread_local int64	sWorkerId;
extern std::atomic<int64>	sWorkerCount;
extern std::atomic<int64>	sJobCancel;
extern Mutex				mListMutex;
extern Index<int64>			sCancelList;
extern int64				GetNewWorkerId();
extern void					UpdateList();
}

template<class T>
void JobBase<T>::Init()
{
	work = Null;
	status = IDLE;
	wid = JobGlobal::GetNewWorkerId();
	error = MakeTuple<int, String>(0, Null);
	Clear();
#ifdef _MULTITHREADED
	auto b = worker.Run([=]() mutable {
		JobGlobal::sWorkerId  = wid;
		Work();
	});
	if(!b)  {
		status = FAILED;
		throw JobError("Worker initializetion failed!");
	}
#else
	JobGlobal::sWorkerId = wid;  // Set id to actual worker.
#endif
	JLOG("Initialized.");
}

template<class T>
void JobBase<T>::Exit()
{
#ifdef _MULTITHREADED
	if(!IsFinished()) // Let workers know that we want to shutdown.
		Cancel();
	while(!IsFinished());
	{
		lock.Enter();
		status = EXIT;
		lock.Leave();
		cv.Signal();
	}
	JLOG("Exit signal sent...");
	worker.Wait();
#endif
}

template<class T>
void JobBase<T>::Work()
{
#ifdef _MULTITHREADED
	Mutex::Lock __(lock);
	for(;;) {
		while(status != WORKING) { // Takes care of spurious wake-ups
			if(status == EXIT) {
				JLOG("Exit signal received. Shutting down job.");
				return;
			}
			JLOG("Waiting for work.");
			cv.Wait(lock);
			JLOG("Waiting is ended.");
		}
#endif
		JobGlobal::sWorkerCount++;
		try {
			JLOG("Working...");
			result.Set(work);
			status = FINISHED;
			JLOG("Finished!");
		}
		catch(JobError& e) {
			status = FAILED;
			error = MakeTuple<int, String>(e.code, e);
			JLOG(Format("Failed. (Error code = %d) %s", e.code, e));
		}
		catch(...) { //exception propagation.
			status = FAILED;
			JLOG("Failed. Exception raised!");
			exc = std::current_exception();
		}
		JobGlobal::sWorkerCount--;
		JobGlobal::UpdateList();
#ifdef _MULTITHREADED
	}
#endif
}

template<class T>
bool JobBase<T>::Start(Function<T()>&& fn)
{
	auto b  = IsFinished();
	if(b) {
		JLOG("Starting to work.");
		work = pick(fn);
		lock.Enter();
		status = WORKING;
		lock.Leave();
#ifdef _MULTITHREADED
		cv.Signal();
#else
		JobGlobal::sWorkerId = wid;  // Set id to actual worker with each call.
		Work();
#endif
	}
	else JLOG("Couldn't start working. Worker is busy.");
	return b;
}

template<class T>
void JobBase<T>::Cancel()
{
	Mutex::Lock __(JobGlobal::mListMutex);
	JobGlobal::sCancelList.FindAdd(wid);
}

template<class T>
void JobBase<T>::Finish()
{
	while(!IsFinished())
		;	// Sleep(1);
}

template<class T>
bool JobBase<T>::IsFinished()
{
	return status != WORKING;
}

template<class T>
void JobBase<T>::Rethrow()
{
	Mutex::Lock __(lock);
	if(exc)	std::rethrow_exception(exc);
}

template<class T>
T Job<T>::GetResult()
{
	return operator~();
}

template<class T>
const T& Job<T>::operator~()
{
	JobBase<T>::Rethrow();
	Mutex::Lock __(JobBase<T>::lock);
	return JobBase<T>::result.Get();
}