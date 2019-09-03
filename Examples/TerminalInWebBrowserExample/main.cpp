#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example is basically the the same as TerminalExample.
// Except that this examples uses the U++ Turtle technology.
// Turtle is a web technology that allows an application to
// run in a web browser that supports canvas, and Websockets
// Turtle can be switched on or off by a compile-time flag.

using namespace Upp;

const char *nixshell = "/bin/bash";

struct TerminalExample : TopWindow {
	Terminal  term;
	PtyProcess pty;					// This class is completely optional
	
	TerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());

		term.WhenBell   = [=]()			{ BeepExclamation(); };
		term.WhenTitle  = [=](String s)	{ Title(s);	};
		term.WhenResize = [=]()			{ pty.SetSize(term.GetPageSize()); };
		term.WhenOutput = [=](String s)	{ PutGet(s); };

		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(nixshell, Environment(), GetHomeDirectory()); // Defaults to TERM=xterm
	}
	
	void PutGet(String out = Null)
	{
		term.WriteUtf8(pty.Get());
		pty.Write(out);
		if(!pty.IsRunning())
			Break();
	}
};

void Main()
{
	TerminalExample().Run();
}

#ifdef flagTURTLE
CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	MemoryLimitKb(100000000);
	Ctrl::host = "localhost";
	Ctrl::port = 8888;
	Ctrl::connection_limit = 15;		// Maximum number of concurrent users (preventing DDoS)

#ifdef _DEBUG
	Ctrl::debugmode = true;				// Only single session in debug (no forking)
#endif
	if(Ctrl::StartSession()) {
		Main();
		Ctrl::EndSession();
	}
	LOG("Session Finished");
}
#else
GUI_APP_MAIN
{
	Main();
}
#endif
