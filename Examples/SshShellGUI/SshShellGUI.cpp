#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <SSH/SSH.h>

// Note that this simple terminal does not process/filter-out the ANSI escape sequences.

using namespace Upp;

class SshConsole : public SshShell, public CodeEditor {
public:
	SshConsole(SshSession& session) : SshShell(session) {
		SetFont(Courier(14));
		NonBlocking();
		Run("ansi", 80, 24);

		WhenOutput = [=](const void* b, int l) {
			String out((const char*) b, l);
			if(out[0] == 0x08)
				Backspace();
			else
				Insert(GetLength(), out);
			SetCursor(GetLength());
		};
	}

	bool Key(dword key, int count) {
		switch(key) {
			case K_BACKSPACE:
				Send(0x08);
				break;
			case K_CTRL_C:
				Send(0x03);
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
	
	void Layout() {
		CodeEditor::Layout();
		PageSize(CodeEditor::GetPageSize()); // Updates the remote pty size.
	}
};

struct SshShellGUI : public TopWindow {
	SshShellGUI() {
		Title("A very simple SSH shell example with GUI")
		.Sizeable()
		.Zoomable();
	}

	void Run() {
		const char *url = "demo:password@test.rebex.net:22"; // A well-known public SSH test server.
		SshSession session;
		if(session.Connect(url)) {

			SshConsole console(session);
			Add(console.SizePos());

			Open();
			
			// Non-blocking event loop.
			while(IsOpen() && console.Do()) {
				ProcessEvents();
				SocketWaitEvent we;
				console.AddTo(we);
				we.Wait(10);
			}
			if(console.IsError())
				Exclamation(DeQtf(console.GetErrorDesc()));
		}
		else
			Exclamation(DeQtf(session.GetErrorDesc()));
	}
};


GUI_APP_MAIN
{
	Ssh::Trace();
	SshShellGUI().Run();
}
