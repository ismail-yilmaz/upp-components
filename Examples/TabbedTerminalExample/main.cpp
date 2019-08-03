#include <TabBar/TabBar.h>
#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a simple tabbed terminal.
// It uses PtyProcess, therefore it is currently POSIX-only.

const char *nixshell = "/bin/bash";
const int MAXTABS    = 10;

using namespace Upp;

struct TerminalTab : Terminal, PtyProcess {
	TerminalTab()
	{
		WhenBell   = [=] { BeepExclamation(); };
		WhenResize = [=] { PtyProcess::SetSize(GetPageSize()); };
		WhenOutput = [=](String s) { Do(s);   };
		PtyProcess::Start(nixshell, Environment(), GetHomeDirectory());
	}

	bool Do(String out = Null)
	{
		WriteUtf8(PtyProcess::Get());
		PtyProcess::Write(out);
		return PtyProcess::IsRunning();
	}
	
	bool Key(dword key, int count) override
	{
		return key != K_SHIFT_CTRL_T ? Terminal::Key(key, count) : false;
	}
};

class TabbedTerminalExample : public TopWindow {
	Array<TerminalTab> terminals;
	TabBarCtrl tabs;
public:
	typedef TabbedTerminalExample CLASSNAME;

	bool Key(dword key, int cnt) override
	{
		if(key == K_SHIFT_CTRL_T && terminals.GetCount() < MAXTABS) {
			AddNewTab();
			return true;
		}
		return false;
	}

	void AddNewTab()
	{
		TerminalTab& tt = terminals.Add();
		int64 key = (int64) GetTickCount();
		tabs.AddCtrl(tt.SizePos(), key,	Format("Terminal #%d", terminals.GetCount()));
	}

	void CloseTab(Value key)
	{
		Ctrl *c = tabs.GetCtrl(key);
		if(c)
			for(int i = 0; i < terminals.GetCount(); i++)
				if(&terminals[i] == c) {
					terminals.Remove(i);
					break;
				}
	}

	void FocusTab()
	{
		tabs.GetCurrentCtrl()->SetFocus();
	}

	void Run()
	{
		Title(t_("Tabbed terminal example (Press SHIFT+CTRL+T to open a new tab)"));
		SetRect(0, 0, 1024, 640);
		Sizeable().Zoomable().CenterScreen().Add(tabs.SortTabs().SizePos());
		tabs.WhenClose  = THISFN(CloseTab);
		tabs.WhenAction = THISFN(FocusTab);
		AddNewTab();
		OpenMain();
		while(IsOpen() && !terminals.IsEmpty()) {
			ProcessEvents();
			for(int i = 0; i < terminals.GetCount(); i++) {
				auto& tt = terminals[i];
				if(!tt.Do()) {
					tabs.RemoveCtrl(tt);
					terminals.Remove(i);
					i--;
				}
			}
			Sleep(10);
		}
	}
};

GUI_APP_MAIN
{
	TabbedTerminalExample().Run();
}
