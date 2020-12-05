#include <Core/SSH/SSH.h>
#include <Terminal/Terminal.h>

using namespace Upp;

String url = "demo:password@test.rebex.net:22";	// A well-known public SSH test server.

struct SshTerminal : TerminalCtrl, SshShell {
	SshTerminal(SshSession& session) : SshShell(session)
	{
		SshShell::Timeout(Null);
		SshShell::ChunkSize(65536);
		SshShell::WhenOutput = [=](const void *data, int size) { TerminalCtrl::Write(data, size); };
		TerminalCtrl::WhenOutput = [=](String data)            { SshShell::Send(data); };
		TerminalCtrl::WhenResize = [=]()                       { SshShell::PageSize(TerminalCtrl::GetPageSize()); };
		TerminalCtrl::InlineImages().Hyperlinks().WindowOps();
	}

	void Run(const String& termtype)
	{
		SshShell::Run(termtype, TerminalCtrl::GetPageSize());
		if(SshShell::IsError())
			ErrorOK(DeQtf(GetErrorDesc()));
	}
};

struct SshTerminalExample : TopWindow {
	void Run()
	{
		if(!EditTextNotNull(url, "SSH server", "Url"))
			return;
		SshSession session;
		session.WhenWait = [=]{ ProcessEvents(); };
		if(!session.Timeout(10000).Connect(url)) {
			ErrorOK(DeQtf(session.GetErrorDesc()));
			return;
		}
		SshTerminal term(session);
		SetRect(term.GetStdSize()); // 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());
		OpenMain();
		term.Run("xterm");
	}
};

GUI_APP_MAIN
{
	SshTerminalExample().Run();
}
