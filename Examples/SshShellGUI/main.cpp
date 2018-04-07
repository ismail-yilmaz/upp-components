#include "SshShellGUI.h"

// Note: This example uses defaulted values for X11 forwarding.
// You may need to change them.

bool SshConsole::Key(dword key, int count)
{
	auto* gui = static_cast<SshShellGUI*>(GetTopCtrl());
	
	switch(key) {
		case K_BACKSPACE:
				Send(0x08 & 0xff);
				break;
			case K_CTRL_C:
				Send(0x03);
				break;
			case K_CTRL_T:
				gui->OpenShell(false);
				break;
			case K_ESCAPE:
				Send("exit\r\n");
				break;
			case K_F1:
				Send("help\r\n");
				break;
			case K_F2:
				Send("ls -l\r\n");
				break;
			default:
				if(key >= 0 && key < 65535)
					Send((char)key);
	}
	return true;
}

void SshConsole::Output(const void *b, int l)
{
	WString out((const char*) b, l);
	if(out[0] == 0x08)
		Backspace();
	else
		Insert(GetLength(), out);
	SetCursor(GetLength());
}

SshConsole::SshConsole(SshSession& session,  bool x11) : SshShell(session)
{
	SetFont(Courier(14));
	WhenOutput = THISFN(Output);
	NonBlocking();
	if(x11)
		ForwardX11();
	Run("xterm", Size(80, 24));
}

void SshShellGUI::OpenShell(bool x11)
{
	if(!connected)
		return;
	auto& sh = shells.Add(new SshConsole(session, x11));
	auto t = Format("Shell #%d%[0:"";1: (X11)]s", sh.GetId(), x11);
	auto i = tabs.Add(sh.SizePos(), CtrlImg::menu_window(), t);
	auto n = tabs.Find(*i.GetSlave());
	if(n >= 0)
		tabs.Set(n);
}

void SshShellGUI::X11ShellFocus()
{
	session.WhenX11 = [=](SshX11Connection* x11conn) {
		if(!tabs.GetCount())
			return;
		static_cast<SshConsole*>(tabs.GetItem(tabs.Get()).GetSlave())->AcceptX11(x11conn);
	};
}

void SshShellGUI::RemoveTab(Ctrl& c)
{
	auto i = tabs.Find(c);
	if(i >= 0)
		tabs.Remove(i);
}

void SshShellGUI::MainMenu(Bar& bar)
{
	bar.Sub("File", THISFN(FileMenu));
	bar.Sub("Help", THISFN(HelpMenu));
	bar.SetFrame(BottomSeparatorFrame());
}

void SshShellGUI::FileMenu(Bar& bar)
{
	bar.Add("Open new shell", [=]{ OpenShell(false); });
#ifdef PLATFORM_POSIX
	bar.Add("Open new shell (with X11)", [=]{ OpenShell(true); });
#endif
	bar.Separator();
	bar.Add("Exit", THISFN(Close));
}

void SshShellGUI::HelpMenu(Bar& bar)
{
	bar.Add("About", [=] {
	const char* txt =	"[ [2 This example demonstrates the following features of ]"
				"[^https`:`/`/github`.com`/ismail`-yilmaz`/upp`-components`/tree`/master`/Core`/SSH^2 SSH package"
				"][2  for ][^https`:`/`/www`.ultimatepp`.org`/index`.html^2 Ultimate`+`+]&][2 &][l192;i150;O0; [2"
				" Multiple, simultaneous shell instances]&][l192;i150;O0; [2 X11 forwarding support]&][l192;i150"
				";O0; [2 Non`-blocking GUI]&][2 &][ [1 SSH package uses ][^https`:`/`/www`.libssh2`.org`/^1 libss"
				"h2][1 , a client`-side C library implementing the SSH2 protocol.]&][@3 &][ ]]";
		PromptOK(txt);
	});
}

void SshShellGUI::Run()
{
	String url;
	if(!EditText(url, "Enter the URL to connect to [Format: user:password@host:port]", "URL"))
		return;
	if((connected = session.Timeout(60000).Connect(url))) {
		OpenMain();
		while(IsOpen()) {
			ProcessEvents();
			for(auto i = 0; i < shells.GetCount(); i++) {
				auto& shell = shells[i];
				if(shell.Do()) {
					GuiSleep(10);
					continue;
				}
				if(shell.IsError())
					ErrorOK(shell.GetErrorDesc());
				RemoveTab(shell);
				shells.Remove(i);
				break;
			}
		}
	}
	else ErrorOK(session.GetErrorDesc());
}

SshShellGUI::SshShellGUI()
{
	Sizeable();
	Zoomable();
	AddFrame(mainmenu);
	mainmenu.Set(THISFN(MainMenu));
	Add(tabs.SizePos());
	tabs.WhenSet     = [=] { X11ShellFocus(); };
	session.WhenWait = [=] { ProcessEvents(); };
	connected = false;
}

GUI_APP_MAIN
{
	Ssh::Trace();
	SshShellGUI().Run();
}
