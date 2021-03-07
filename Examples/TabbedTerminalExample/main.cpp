#include <TabBar/TabBar.h>
#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// tabbed terminal example.

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

const int MAXTABS = 10;

using namespace Upp;

struct TerminalTab : TerminalCtrl, PtyProcess {
	TerminalTab()
	{
		InlineImages().Hyperlinks().WindowOps();
		WhenBell   = [=]()         { BeepExclamation();    };
		WhenOutput = [=](String s) { PtyProcess::Write(s); };
		WhenResize = [=]()         { PtyProcess::SetSize(GetPageSize()); };
		Start(GetEnv(tshell), Environment(), GetHomeDirectory());
	}
	
	bool Do()
	{
		WriteUtf8(PtyProcess::Get());
		return PtyProcess::IsRunning();
	}
	
	bool Key(dword key, int count) override
	{
		// Let the parent handle the SHIFT + CTRL + T key.
		return key != K_SHIFT_CTRL_T ? TerminalCtrl::Key(key, count) : false;
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
