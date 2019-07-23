#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a barebone terminal multiplexer.
// It uses PtyProcess, therefore it is currently POSIX-only.

const char *nixshell = "/bin/bash";
const int  PANECOUNT = 2;						// You can increase the number of panes if you like.

using namespace Upp;

struct TerminalPane : Terminal, PtyProcess {
	TerminalPane()
	{
		WhenBell   = [=]()			{ BeepExclamation(); };
		WhenResize = [=]()			{ PtyProcess::SetSize(GetPageSize()); };
		WhenOutput = [=](String s)	{ Do(s); };
		PtyProcess::Start(nixshell, Environment(), GetHomeDirectory());
	};
	
	bool Do(String out = Null)
	{
		WriteUtf8(PtyProcess::Get());
		PtyProcess::Write(out);
		return PtyProcess::IsRunning();
	}
};

class TerminalMultiplexerExample : public TopWindow {
	Splitter splitter;
	Array<TerminalPane> terminals;				// Let's dynamically create the TerminalPane instances.
public:
	
	void SetupSplitter()
	{
		Add(splitter.Horz());
		for(int i = 0; i < PANECOUNT; i++)
			splitter.Add(terminals.Add().SizePos());
	}
	
	void RemovePane(int i, Ctrl& c)
	{
		splitter.Remove(c);
		splitter.RefreshLayoutDeep();
		terminals.Remove(i);
	}
	
	void Run()
	{
		Title(t_("Terminal Multiplexing Example"));
		SetRect(0, 0, 1024, 600);
		Sizeable().Zoomable().CenterScreen();
		SetupSplitter();
		OpenMain();
		while(IsOpen() && !terminals.IsEmpty()) {
			for(int i = 0; i < terminals.GetCount(); i++) {
				ProcessEvents();
				TerminalPane& pane = terminals[i];
				if(!pane.Do()) {
					RemovePane(i, pane);
					i--;
				}
			}
			Sleep(1);
		}
	}
};

GUI_APP_MAIN
{
	TerminalMultiplexerExample().Run();
}
