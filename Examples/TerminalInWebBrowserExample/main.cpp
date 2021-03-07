#include <Turtle/Turtle.h>
#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example, running on the U++ Turtle backend.Turtle allows
// U++ GUI applications to run on modern web browsers  that  support
// HTML-5 canvas and websockets. Turtle can be switched on or off by
// a compile-time flag.

// On Windows platform PtyProcess class uses statically linked *winpty*
// library and the supplementary PtyAgent pacakges as its *default* pty
// backend. However, it also supports the Windows 10 (tm) pseudoconsole
// API via the WIN10 compiler flag. This flag can be enabled or disable
// easily via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

#ifdef PLATFORM_POSIX
const char *tshell = "SHELL";
#elif PLATFORM_WIN32
const char *tshell = "ComSpec"; // Alternatively you can use powershell...
#endif

using namespace Upp;

struct TerminalExample : TopWindow {
	TerminalCtrl  term;
	PtyProcess    pty;  // This class is completely optional
	
	TerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());

		term.WhenBell   = [=]()         { BeepExclamation(); };
		term.WhenTitle  = [=](String s) { Title(s);	};
		term.WhenResize = [=]()         { pty.SetSize(term.GetPageSize()); };
		term.WhenOutput = [=](String s) { PutGet(s); };
		term.InlineImages().Hyperlinks().WindowOps();
		
		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(GetEnv(tshell), Environment(), GetHomeDirectory());
	}
	
	void PutGet(String out = Null)
	{
		term.WriteUtf8(pty.Get());
		pty.Write(out);
		if(!pty.IsRunning())
			Break();
	}
};


void AppMainLoop()
{
	// "Main" stuff should go in here...
	TerminalExample().Run();
}

CONSOLE_APP_MAIN
{

#ifdef _DEBUG
	TurtleServer::DebugMode();
#endif

	// MemoryLimitKb(100000000); // Can aid preventing DDoS attacks.

	TurtleServer guiserver;
	guiserver.Host("localhost");
	guiserver.HtmlPort(8888);
	guiserver.MaxConnections(15);
	RunTurtleGui(guiserver, AppMainLoop);
}