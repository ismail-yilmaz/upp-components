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
		ChunkSize(1024*1024 * 10); // For better X11 tunneling experience.
		RunWithX11("ansi", 0, Size(80, 24));

		WhenOutput = [=](const void* b, int l) {
			WString out((const char*) b, l);
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
				Send(0x08 & 0xff);
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
			case K_F3:
				Send("export DISPLAY=:0\n");
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
	TabCtrl tabs;
	
	SshShellGUI() {
		Title("A very simple SSH shell example (in X11 tunneling mode!) with GUI")
		.Sizeable()
		.Zoomable();
	}

	void Run() {
		const char *url = "maldoror:succubus@localhost:22"; // A well-known public SSH test server.
		SshSession session;
		if(session.Connect(url)) {
			Array<SshConsole> consoles;
			auto& con1 = consoles.Add(new SshConsole(session));
			auto& con2 = consoles.Add(new SshConsole(session));
			
			Add(tabs.SizePos());
			tabs.Add(con1.SizePos(), "Ssh Console 1");
			tabs.Add(con2.SizePos(), "Ssh Console 2");

			Open();
			
			while(!consoles.IsEmpty()) {
				for(auto i = 0; i < consoles.GetCount(); i++) {
					ProcessEvents();
					if(!IsOpen())
						return;
					SocketWaitEvent we;
					consoles[i].AddTo(we);
					if(!consoles[i].Do()) {
						if(consoles[i].IsError())
							Exclamation(DeQtf(consoles[i].GetErrorDesc()));
						consoles.Remove(i);
						break;
					}
					we.Wait(10);
				}
			}
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
