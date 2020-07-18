#include <Core/Core.h>
#include <Job/Job.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
	
	{
		Job<String> job1;
		job1.Do([=]{ return "Hello world!"; });
		RLOG(~job1);
	}
	
	Job<void> job2;

	job2.Do([=]{ throw Exc("Error"); });
	try {
		job2.Get();
	}
	catch(...) {
		RLOG("Exception caught!");
	}

	job2.Do([=]{
		while(!Job<void>::IsCanceled())
			Sleep(1);
		RLOG("Job2 is canceled.");
	});
	
	{
		Job<Vector<int>> job3;
		job3.Do([=]{ return MakeIota(100); });
		RDUMP(job3.Pick());
	}

	Sleep(1000);
	// job2.Cancel(); // Worker will automatically cancel the job when it goes out of scope.
}
