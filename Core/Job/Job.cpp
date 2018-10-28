#include "Job.h"

namespace Upp {
	
#define LLOG(x)  // RLOG("Worker #" << this << ": " << x)

thread_local JobWorker *JobWorker::ptr = nullptr;

JobWorker::JobWorker()
{
	cb  = Null;
	exc = nullptr;
	status = IDLE;
	if(!work.RunNice([=]{ JobWorker::ptr = this; Loop(); }))
		throw Exc("Couldn't create new job.");
	LLOG("Initialized.");
}

JobWorker::~JobWorker()
{
	cancel = true;
	Wait();
	status = SHUTDOWN;
	cv.Signal();
	LLOG("Shut down signal sent...");
	work.Wait();
	LLOG("Joined.");
}

bool JobWorker::Start(Event<>&& fn)
{
	if(Is(WORKING)) {
		LLOG("Couldn't start working. Worker is busy.");
		return false;
	}
	lock.Enter();
	cb = pick(fn);
	exc = nullptr;
	lock.Leave();
	status = WORKING;
	LLOG("Starting to work...");
	cv.Signal();
	return true;
}

void JobWorker::Loop()
{
	Mutex::Lock __(lock);
	for(;;) {
		while(!Is(WORKING)) {
			if(Is(SHUTDOWN)) {
				LLOG("Shut down signal received. Shutting down...");
				return;
			}
			LLOG("Waiting for work.");
			cv.Wait(lock);
		}
		LLOG("Waiting is ended.");
		try {
			LLOG("Working...");
			cb();
			LLOG("Finished!");
		}
		catch(...) { //exception propagation.
			LLOG("Failed. Exception raised!");
			exc = std::current_exception();
		}
		cancel = false;
		status = IDLE;
		wv.Signal();
	}
}

void JobWorker::Wait()
{
	Mutex::Lock __(lock);
	while(Is(WORKING)) {
		LLOG("Waiting for worker to finish...");
		wv.Wait(lock);
	}
}
}
