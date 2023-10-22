#include <TabBar/TabBar.h>
#include <Core/SSH/SSH.h>
#include <Terminal/Terminal.h>

// This example demonstrates a multithreaded ssh terminal with tabs.
// It is using the  Core/SSH package, therefore it can be  compiled
// for, and run on, Windows and  POSIX-compliant operating  systems.

using namespace Upp;

String url = "demo:password@test.rebex.net:22";	// A well-known public SSH test server.
const int MAXTABCOUNT = 8; // You can increase the number of tabs if you like.

struct SshTerminalTab : TerminalCtrl, SshShell {
	TabBarCtrl& parent;
	SshTerminalTab(SshSession& session, TabBarCtrl& ctrl) : SshShell(session), parent(ctrl)
	{
		SshShell::Timeout(Null);
		SshShell::ChunkSize(65536);
		SshShell::WhenOutput = [=](const void *data, int size) { GuiLock __; TerminalCtrl::Write(data, size);};
		SshShell::WhenWait   = [=]()                           { if(CoWork::IsCanceled()) SshShell::Abort(); };
		TerminalCtrl::WhenOutput = [=](String data)            { SshShell::Send(data); };
		TerminalCtrl::WhenResize = [=]()                       { SshShell::PageSize(TerminalCtrl::GetPageSize()); };
		TerminalCtrl::InlineImages().Hyperlinks().WindowOps();
		parent.AddCtrl(TerminalCtrl::SizePos(), Format("Ssh Terminal #%d", SshShell::GetId()));
	}
	
	void Run(const String& termtype)
	{
		SshShell::Run(termtype, TerminalCtrl::GetPageSize());
		GuiLock __;
		parent.RemoveCtrl(*this);
	}

	bool Key(dword key, int count) override
	{
		// Let the parent handle the SHIFT + CTRL + T key.
		return key != K_SHIFT_CTRL_T ? TerminalCtrl::Key(key, count) : false;
	}
};

struct TabbedSshTerminal : TopWindow {
	TabBarCtrl            tabbar;
	SshSession            session;
	CoWorkNX              workers; // Same as CoWork, but can be used as a member.
	
	typedef TabbedSshTerminal CLASSNAME;

	bool Key(dword key, int cnt) override
	{
		if(key == K_SHIFT_CTRL_T) AddTab();
		return true;
	}

	void AddTab()
	{
		if(tabbar.GetCount() < MAXTABCOUNT) {
			workers & [=] {
				EnterGuiMutex();                       // Note: Ctrl derived classes can only be
				SshTerminalTab shell(session, tabbar); // initialized in main thread OR vith gui
				LeaveGuiMutex();                       // mutex. (GuiLock)
				shell.Run("xterm");
			};
		}
	}

	void CloseTab(Value key)
	{
		GuiLock __;
		auto *tab = dynamic_cast<SshTerminalTab*>(tabbar.GetCtrl(key));
		if(tab && tab->InProgress()) tab->Abort();
	}

	void FocusTab()
	{
		GuiLock __;
		tabbar.GetCurrentCtrl()->SetFocus();
	}

	void Run()
	{
		if(!EditTextNotNull(url, "SSH server", "Url"))
			Exit(1);

		session.WhenWait = [=] { if(IsMainThread()) ProcessEvents(); };

		if(!session.Timeout(10000).Connect(url)) {
			ErrorOK(DeQtf(session.GetErrorDesc()));
			Exit(1);
		}

		Title(t_("Tabbed terminals example (Press SHIFT + CTRL + T to open a new tab)"));
		Sizeable().Zoomable().CenterScreen().Add(tabbar.SortTabs().SizePos());
		SetRect(0, 0, 1024, 640);
		tabbar.WhenClose  = THISFN(CloseTab);
		tabbar.WhenAction = THISFN(FocusTab);
		AddTab();
		OpenMain();
		while(IsOpen() && !workers.IsFinished()) {
			ProcessEvents();
			GuiSleep(10);
		}
		GuiUnlock __;
		workers.Cancel();
	}
};

GUI_APP_MAIN
{
	TabbedSshTerminal().Run();
}
