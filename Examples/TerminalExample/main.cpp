#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

using namespace Upp;

const char *nixshell = "/bin/bash";

struct TerminalExample : TopWindow {
	Terminal term;
	PtyProcess pty;
	
	TerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled).
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());
		term.WhenBell	= [=]()                { BeepExclamation();  };
		term.WhenTitle	= [=](String s)        { Title(s);           };
		term.WhenOutput	= [=](String s)        { pty.Write(s);       };
		term.WhenLink   = [=](const String& s) { PromptOK(DeQtf(s)); };
		term.WhenResize	= [=]()   { pty.SetSize(term.GetPageSize()); };
		term.InlineImages().Hyperlinks().WindowOps();
		pty.Start(nixshell, Environment(), GetHomeDirectory()); // defaults to TERM=xterm
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