#include "ShellGUI.h"

ShellGUI::ShellGUI()
{
	CtrlLayout(*this, "SshShell");
	session.WhenDo = [=] { ProcessEvents(); };
	Ssh::Trace();
}

static bool bb = false;

void ShellGUI::Run()
{
	if(!session.Connect("127.0.0.1", 22, "maldoror", "succubus")) {
		Exclamation(session.GetErrorDesc());
		Exit(1);
	}
	auto console = session.CreateShell();
	console.NonBlocking();
	console.RunGUI("vanilla", 80, 24);
	console.WhenOutput = [=](String out) { for(auto e : out) cmd.Put(e); };
	input.WhenEnter = [=, &console]      { String s(~input); s << "\n"; console.SetText(s); };
	cmd.WhenUpdate = [=] { cmd.MoveTextEnd(); };
	Open();
	while(IsOpen()) {
		ProcessEvents();
		console.Do();
//			break;
		
	}
}

GUI_APP_MAIN
{
	ShellGUI().Run();
}
