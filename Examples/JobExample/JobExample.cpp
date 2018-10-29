#include <Core/Core.h>
#include <Job/Job.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE|LOG_COUT);
	
	Job<void>   job1;
	Job<String> job2;
	
	{
		job1.Do([=]{ LOG("Hello world!"); });
		job1.Get();
	}
	{
		job1.Do([=]{ throw Exc("Error"); });
		try {
			job1.Get();
		}
		catch(...) {
			LOG("Exception caught!");
		}
	}
	{
		job1.Do([=]{
			while(!Job<void>::IsCancelled())
				Sleep(1);
			LOG("Worker canceled.");
		});
		Sleep(1000);
		//job1.Cancel(); // Worker will automatically cancel the job when it goes out of scope.
	}
	{
		job2.Do([=]{ return "Hello again!"; });
		LOG(~job2);
	}
}
