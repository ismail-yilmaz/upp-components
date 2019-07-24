#include <CtrlLib/CtrlLib.h>
#include <TabBar/TabBar.h>
#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a simple tabbed terminal.
// It uses PtyProcess, therefore it is currently POSIX-only.

const int MAXTABS = 10;

using namespace Upp;

class TerminalTab : public Terminal {
	PtyProcess pty;
public:
	TerminalTab()
	{
		WhenBell   = [=]		{ BeepExclamation(); };
		WhenResize = [=]		{ pty.SetSize(GetPageSize()); };
		WhenOutput = [=](String s)	{ Do(s);   };
		pty.Start("/bin/bash", Environment(), GetHomeDirectory());
	}

	bool Do(String out = Null)
	{
		Write(pty.Get(), true);
		pty.Write(out);
		return pty.IsRunning();
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
		tt.WantFocus();
		tabs.AddCtrl(tt.SizePos(),  (int64) GetTickCount(), Format("Terminal #%d", terminals.GetCount()));
	}

	void CloseTab(Value key)
	{
		Ctrl *c = tabs.GetCtrl(key);
		if(c)
			for(int i = 0; i < terminals.GetCount(); i++)
				if(&terminals[i] == c)
					terminals.Remove(i);
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
			Sleep(1);
		}
	}
};

GUI_APP_MAIN
{
	TabbedTerminalExample().Run();
}
