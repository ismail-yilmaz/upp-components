#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

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
		term.Paper(Black());
		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(nixshell, Environment(), GetHomeDirectory()); // Defaults to TERM=xterm
	}
	
	void PutGet(String out = Null)
	{
		term.CheckWriteUtf8(pty.Get());
		pty.Write(out);
		if(!pty.IsRunning())
			Break();
	}
};

GUI_APP_MAIN
{
	TerminalExample().Run();
}
