#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This  example  demonstrates a  simple terminal  splitter.
// It uses PtyProcess, therefore it is currently POSIX-only.

const char *nixshell = "/bin/bash";
const int  MAXPANECOUNT = 4;  // You can increase the number of panes if you like.

using namespace Upp;

struct TerminalPane : Terminal, PtyProcess {
	Splitter& parent;
	TerminalPane(Splitter& ctrl) : parent(ctrl)
	{
		Terminal::InlineImages().Hyperlinks().WindowOps();
		Terminal::WhenBell   = [=]()         { BeepExclamation();    };
		Terminal::WhenOutput = [=](String s) { PtyProcess::Write(s); };
		Terminal::WhenResize = [=]()         { PtyProcess::SetSize(GetPageSize()); };
		PtyProcess::Start(nixshell, Environment(), GetHomeDirectory());	// Defaults to TERM=xterm
		parent.Add(Terminal::SizePos());
	}
	
	void Do()
	{
		Terminal::WriteUtf8(PtyProcess::Get());
		if(!PtyProcess::IsRunning()) {
			parent.Remove(*this);
			parent.Layout();
		}
	}
	
	bool Key(dword key, int count) override
	{
		// Let the parent handle the SHIFT + CTRL + T key.
		return key != K_SHIFT_CTRL_T ? Terminal::Key(key, count) : false;
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