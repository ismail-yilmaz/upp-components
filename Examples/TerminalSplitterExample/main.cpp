#include <Terminal/Terminal.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal splitter example.

// On Windows, the PtyProcess class requires at least Windows 10 (tm)
// for the new pseudoconsole API support. To enable this feature, you
// need to set the WIN10 flag in TheIDE's main package configurations
// dialog. (i.e. "GUI WIN10")

#ifdef PLATFORM_POSIX
const char *tshell = "/bin/bash";
#elif PLATFORM_WIN32
const char *tshell = "cmd.exe"; // Alternatively you can use powershell...
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
		PtyProcess::Start(tshell, Environment(), GetHomeDirectory());
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