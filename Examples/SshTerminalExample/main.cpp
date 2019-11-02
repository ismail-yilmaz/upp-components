#include <Core/SSH/SSH.h>
#include <Terminal/Terminal.h>

using namespace Upp;

String url = "demo:password@test.rebex.net:22";	// A well-known public SSH test server.

struct SshTerminal : Terminal, SshShell {
	SshTerminal(SshSession& session) : SshShell(session)
	{
		SshShell::Timeout(Null);
		SshShell::ChunkSize(65536);
		SshShell::WhenOutput = [=](const void *data, int size) { Terminal::Write(data, size);                 };
		Terminal::WhenOutput = [=](String data)                { SshShell::Send(data);                        };
		Terminal::WhenResize = [=]()                           { SshShell::PageSize(Terminal::GetPageSize()); };
		Terminal::InlineImages();
	}

	void Run(const String& termtype)
	{
		SshShell::Run(termtype, Terminal::GetPageSize());
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
