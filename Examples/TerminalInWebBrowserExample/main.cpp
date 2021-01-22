// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example, running on the U++ Turtle backend.Turtle allows
// U++ GUI applications to run on modern web browsers  that  support
// HTML-5 canvas and websockets. Turtle can be switched on or off by
// a compile-time flag.

// On Windows, the PtyProcess class requires at least Windows 10 (tm)
// for the new pseudoconsole API support. To enable this feature, you
// need to set the WIN10 flag in TheIDE's main package configurations
// dialog. (i.e. "TURTLEGUI WIN10")

#ifdef flagTURTLEGUI
#include <Turtle/Turtle.h>
#else
#include <CtrlLib/CtrlLib.h>
#endif

#include <Terminal/Terminal.h>

#ifdef PLATFORM_POSIX
const char *tshell = "/bin/bash";
#elif PLATFORM_WIN32
const char *tshell = "cmd.exe"; // Alternatively, you can use powershell...
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
		pty.Start(tshell, Environment(), GetHomeDirectory());
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

#ifdef flagTURTLEGUI

CONSOLE_APP_MAIN
{

#ifdef _DEBUG
	TurtleServer::DebugMode();
#endif

	// MemoryLimitKb(100000000); // Can aid preventing DDoS attacks.

	TurtleServer guiserver;
	guiserver.Host("localhost");
	guiserver.Port(8888);
	guiserver.MaxConnections(15);
	RunTurtleGui(guiserver, AppMainLoop);
}

#else

GUI_APP_MAIN
{
	AppMainLoop();
}

#endif

