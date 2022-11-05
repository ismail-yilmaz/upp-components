#include <Core/Core.h>
#include <CoRoutines/CoRoutines.h>

// Note that this example requires at least C++20 with coroutines support.

using namespace Upp;

const String str = "Hello, world!";

CoRoutine<void> PrintString()
{
	String s;
	for(char c : str) {
		s << c;
		co_await CoSuspend();
	}
	RLOG("CoRoutine PrintString(): Done! Result: " << s);
}

CoRoutine<String> ReturnString()
{
	String ret;
	for(char c : str) {
		ret << c;
		co_await CoSuspend();
	}
	co_return ret;
}

CoRoutine<Vector<int>> ReturnPickVector()
{
	Vector<int> v;
	for(int i = 90; i <= 100; i++) {
		v.Add(i);
		co_await CoSuspend();
	}
	co_return v;
}

CoGenerator<int> GenerateNumber()
{
	for(int i = 0;; i++)
		co_yield i;
}

CoGenerator<int> GenerateException()
{
	for(int i = 0;; i++) {
		if(i == 5)
			throw Exc("GenerateException(): exception thrown at i: " << AsString(i));
		co_yield i;
	}
}


CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);

	auto co  = PrintString();
	auto co1 = ReturnString();
	auto co2 = GenerateNumber();
	auto co3 = ReturnPickVector();
	auto co4 = GenerateException();
	
	try {
		while(co.Do())
			RLOG("CoRoutine PrintString(): Running...");

		while(co1.Do())
			RLOG("CoRoutine ReturnString(): Running...");
		RLOG("CoRoutine ReturnString(): Done! Result: " << co1.Get());
	
		for(int n = 0; n < 10; n = co2.Next())
			RLOG("CoRoutine GenerateNumber(), yielding: " << n);
		
		while(co3.Do())
			; // NOP
		RLOG("CoRoutine ReturnPickVector(): v: " << co3.Pick());
	

		for(int n = 0; n < 10; n = co4.Next())
			RLOG("CoRoutine GenerateException(), yielding: " << n);

	}
    catch(const Exc& e) {
        RLOG(e);
    }
    catch(...) {
        RLOG("Unhandled exception");
    }
}
