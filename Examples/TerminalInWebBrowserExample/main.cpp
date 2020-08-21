#include <Terminal/Terminal.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example, running on the U++ Turtle backend.Turtle allows
// U++ GUI applications to run on modern web browsers  that  support
// HTML-5 canvas and websockets. Turtle can be switched on or off by
// a compile-time flag.

// On Windows, the PtyProcess class requires at least Windows 10 (tm)
// for the new pseudoconsole API support. To enable this feature, you
// need to set the WIN10 flag in TheIDE's main package configurations
// dialog. (i.e. "TURTLE WIN10")

#ifdef PLATFORM_POSIX
const char *tshell = "/bin/bash";
#elif PLATFORM_WIN32
const char *tshell = "cmd.exe"; // Alternatively you can use powershell...
#endif

using namespace Upp;

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
		term.InlineImages().Hyperlinks().WindowOps();
		
		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(tshell, Environment(), GetHomeDirectory()); // Defaults to TERM=xterm
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
