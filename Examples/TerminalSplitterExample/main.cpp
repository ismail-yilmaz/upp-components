#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a simple terminal splitter.
// It uses PtyProcess, therefore it is currently POSIX-only.

const char *nixshell = "/bin/bash";
const int  PANECOUNT = 4;  // You can increase the number of panes if you like.

using namespace Upp;

struct TerminalPane : Terminal, PtyProcess {
	TerminalPane()
	{
		SixelGraphics();
		WhenBell   = [=]()         { BeepExclamation();    };
		WhenOutput = [=](String s) { PtyProcess::Write(s); };
		WhenResize = [=]()         { PtyProcess::SetSize(GetPageSize()); };
		Start(nixshell, Environment(), GetHomeDirectory());	// Defaults to TERM=xterm
	}
	
	bool Do()
	{
		WriteUtf8(PtyProcess::Get());
		return PtyProcess::IsRunning();
	}
	
	bool Key(dword key, int count) override
	{
		// Let the parent handle the SHIFT + CTRL + T key.
		return key != K_SHIFT_CTRL_T ? Terminal::Key(key, count) : false;
	}
};

struct TerminalSplitterExample : public TopWindow {
	Splitter splitter;
	Array<TerminalPane> panes;
	
	void AddPane()
	{
		if(splitter.GetCount() < PANECOUNT)
			splitter.Add(panes.Add().SizePos());
	}

	bool Key(dword key, int count) override
	{
		if(key == K_SHIFT_CTRL_T) AddPane();
		return false;
	}

	void Run()
	{
		Title(t_("Terminal splitter. Press CTRL + T to split the view"));
		Sizeable().Zoomable().CenterScreen().SetRect(0, 0,  1024, 600);
		Add(splitter.Horz());
		AddPane();
		OpenMain();
		while(IsOpen() && !panes.IsEmpty()) {
			ProcessEvents();
			for(int i = 0; i < panes.GetCount(); i++) {
				TerminalPane& pane = panes[i];
				if(!pane.Do()) {
					splitter.Remove(pane);
					splitter.Layout();
					panes.Remove(i);
					i--;
				}
			}
			Sleep(10);
		}
	}
};

GUI_APP_MAIN
{
	TerminalSplitterExample().Run();
}