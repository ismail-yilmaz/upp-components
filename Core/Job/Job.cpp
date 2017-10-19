#include "Job.h"

namespace Upp {

namespace JobGlobal {
thread_local int64	sWorkerId;
std::atomic<int64>	sWorkerCount;
std::atomic<int64>	sJobCancel;
Mutex				mListMutex;
Index<int64>		sCancelList;

int64 GetNewWorkerId()
{
	static int64 generator;
	if(generator == INT64_MAX)
		generator = 1;
	else
		++generator;
	return generator;
}

void UpdateList()
{
	if(!GetJobCount()) {
		Mutex::Lock __(JobGlobal::mListMutex);
		JobGlobal::sCancelList.Clear();
	}
	else
	if(IsJobCancelled()) {
		Mutex::Lock __(JobGlobal::mListMutex);
		auto i = JobGlobal::sCancelList.Find(JobGlobal::sWorkerId);
		if(i >= 0) {
			JobGlobal::sCancelList.Unlink(i); // A bit of optimization.
			JLOG("Job cancelled.");
		}
	}
}
}

int64 GetWorkerId()
{
	return JobGlobal::sWorkerId;
}

int64 GetJobCount()
{
	return JobGlobal::sWorkerCount;
}

void WaitForJobs()
{
	while(GetJobCount())
		Sleep(1);
}

void CancelJobs()
{
	JobGlobal::sJobCancel++;
	WaitForJobs();
	JobGlobal::sJobCancel--;
}

bool IsJobCancelled()
{
	if(Thread::IsShutdownThreads() || JobGlobal::sJobCancel)
		return true;
	Mutex::Lock __(JobGlobal::mListMutex);
	return JobGlobal::sCancelList.Find(JobGlobal::sWorkerId) >= 0;
}
}