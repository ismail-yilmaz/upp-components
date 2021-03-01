#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

using namespace Upp;

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows platform, PtyProcess class can use one of two backends:
// WinPty or the Windows 10 (tm) pseudoconsole  API. These  mutually
// exclusive backends can be enabled by setting WINPTY or WIN10 flag
// via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

#ifdef PLATFORM_POSIX
const char *tshell = "SHELL";
#elif PLATFORM_WIN32
const char *tshell = "ComSpec"; // Alternatively you can use powershell...
#endif

struct TerminalExample : TopWindow {
	TerminalCtrl term;
	PtyProcess   pty;
	
	TerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled).
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());
		term.WhenBell   = [=]()                { BeepExclamation();  };
		term.WhenTitle  = [=](String s)        { Title(s);           };
		term.WhenOutput = [=](String s)        { pty.Write(s);       };
		term.WhenLink   = [=](const String& s) { PromptOK(DeQtf(s)); };
		term.WhenResize = [=]()                { pty.SetSize(term.GetPageSize()); };
		term.InlineImages().Hyperlinks().WindowOps();
		pty.Start(GetEnv(tshell), Environment(), GetHomeDirectory());
		SetTimeCallback(-1, [=] ()
		{
			term.WriteUtf8(pty.Get());
			 if(!pty.IsRunning())
				Break();
		});
	}
};

GUI_APP_MAIN
{
	TerminalExample().Run();
}