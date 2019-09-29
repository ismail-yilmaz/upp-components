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

struct TabbedTerminal : TopWindow {
	TabBarCtrl tabbar;
	Array<TerminalTab> tabs;

	typedef TabbedTerminal CLASSNAME;

	bool Key(dword key, int cnt) override
	{
		if(key == K_SHIFT_CTRL_T) AddTab();
		return true;
	}

	void AddTab()
	{
		if(tabs.GetCount() < MAXTABS) {
			TerminalTab& tt = tabs.Add();
			int64 key = (int64) GetTickCount();
			tabbar.AddCtrl(tt.SizePos(), key, Format("Terminal #%d", tabs.GetCount()));
		}
	}

	void CloseTab(Value key)
	{
		Ctrl *c = tabbar.GetCtrl(key);
		if(c)
			for(int i = 0; i < tabs.GetCount(); i++)
				if(&tabs[i] == c) {
					tabs.Remove(i);
					break;
				}
	}

	void FocusTab()
	{
		tabbar.GetCurrentCtrl()->SetFocus();
	}

	void Run()
	{
		Title(t_("Tabbed terminals example (Press SHIFT+CTRL+T to open a new tab)"));
		Sizeable().Zoomable().CenterScreen().Add(tabbar.SortTabs().SizePos());
		SetRect(0, 0, 1024, 640);
		tabbar.WhenClose  = THISFN(CloseTab);
		tabbar.WhenAction = THISFN(FocusTab);
		AddTab();
		OpenMain();
		while(IsOpen() && !tabs.IsEmpty()) {
			ProcessEvents();
			for(int i = 0; i < tabs.GetCount(); i++) {
				TerminalTab& tt = tabs[i];
				if(!tt.Do()) {
					tabbar.RemoveCtrl(tt);
					tabs.Remove(i);
					i--;
				}
			}
			Sleep(10);
		}
	}
};

GUI_APP_MAIN
{
	TabbedTerminal().Run();
}
