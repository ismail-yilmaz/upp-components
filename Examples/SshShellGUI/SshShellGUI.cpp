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
		Run("ansi", Size(80, 24));

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
		tabs.WhenSet = [=] { tabs.GetItem(tabs.Get()).GetSlave()->SetFocus(); };
	}

	void Run() {
		const char *url = "demo:password@test.rebex.net:22"; // A well-known public SSH test server.
		SshSession session;
		if(session.Connect(url)) {
			Array<SshConsole> consoles;
			Add(tabs.SizePos());
			for(auto i = 0; i < 5; i++) {
				auto& con = consoles.Add(new SshConsole(session));
				tabs.Add(con.SizePos(), Format("Ssh Console %d", con.GetId()));
			}
			Open();
			
			while(!consoles.IsEmpty()) {
				for(auto i = 0; i < consoles.GetCount(); i++) {
					ProcessEvents();
					if(!IsOpen())
						return;
					if(!consoles[i].Do()) {
						if(consoles[i].IsError())
							Exclamation(DeQtf(consoles[i].GetErrorDesc()));
						consoles.Remove(i);
						break;
					}
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
