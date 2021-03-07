#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal splitter example.

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

const int  MAXPANECOUNT = 4;  // You can increase the number of panes if you like.

using namespace Upp;

struct TerminalPane : TerminalCtrl, PtyProcess {
	Splitter& parent;
	TerminalPane(Splitter& ctrl) : parent(ctrl)
	{
		TerminalCtrl::InlineImages().Hyperlinks().WindowOps();
		TerminalCtrl::WhenBell   = [=]()         { BeepExclamation();    };
		TerminalCtrl::WhenOutput = [=](String s) { PtyProcess::Write(s); };
		TerminalCtrl::WhenResize = [=]()         { PtyProcess::SetSize(GetPageSize()); };
		PtyProcess::Start(GetEnv(tshell), Environment(), GetHomeDirectory());
		parent.Add(TerminalCtrl::SizePos());
	}
	
	void Do()
	{
		TerminalCtrl::WriteUtf8(PtyProcess::Get());
		if(!PtyProcess::IsRunning()) {
			parent.Remove(*this);
			parent.Layout();
		}
	}
	
	bool Key(dword key, int count) override
	{
		// Let the parent handle the SHIFT + CTRL + T key.
		return key != K_SHIFT_CTRL_T ? TerminalCtrl::Key(key, count) : false;
	}
};

struct TerminalSplitterExample : TopWindow {
	Splitter splitter;
	Array<TerminalPane> panes;
	
	bool Key(dword key, int count) override
	{
		if(key == K_SHIFT_CTRL_T) AddPane();
		return false;
	}

	void AddPane()
	{
		if(splitter.GetCount() < MAXPANECOUNT)
			panes.Create<TerminalPane>(splitter);
	}

	void Run()
	{
		Title(t_("Terminal splitter (Press CTRL + SHIFT + T to split the view)"));
		Sizeable().Zoomable().CenterScreen().SetRect(0, 0,  1024, 600);
		Add(splitter.Horz());
		AddPane();
		OpenMain();
		while(IsOpen() && splitter.GetCount()) {
			ProcessEvents();
			for(TerminalPane& pane : panes) pane.Do();
			Sleep(10);
		}
	}
};

GUI_APP_MAIN
{
	TerminalSplitterExample().Run();
}