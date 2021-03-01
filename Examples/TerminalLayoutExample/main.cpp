// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows platform, PtyProcess class can use one of two backends:
// WinPty or the Windows 10 (tm) pseudoconsole  API. These  mutually
// exclusive backends can be enabled by setting WINPTY or WIN10 flag
// via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

#include "TerminalLayoutExample.h"

#ifdef PLATFORM_POSIX
const char *tshell = "SHELL";
#elif PLATFORM_WIN32
const char *tshell = "ComSpec"; // Alternatively you can use powershell...
#endif

TerminalExample::TerminalExample()
{
	CtrlLayout(*this, "Terminal Layout Example");
#ifdef PLATFORM_COCOA	// MacOS menubar fix.
	SetMainMenu([=](Bar& bar) { MainMenu(bar); });
#else
	AddFrame(mainmenu);
#endif
	mainmenu.Set([=](Bar& bar) { MainMenu(bar); });
	Sizeable().Zoomable().SetRect(AddFrameSize(term.GetStdSize()));
	term.WhenBell                 = [=]()                { BeepExclamation(); };
	term.WhenBar                  = [=](Bar& bar)        { ContextMenu(bar);  };
	term.WhenTitle                = [=](String s)        { Title(s);          };
	term.WhenOutput               = [=](String s)        { pty.Write(s);      };
	term.WhenResize               = [=]()                { pty.SetSize(term.GetPageSize()); };
	term.WhenLink                 = [=](const String& s) { PromptOK(DeQtf(s));              };
	term.WhenWindowMinimize       = [=](bool b)          { WindowAction(WindowOp::Minimize, b);   };
	term.WhenWindowMaximize       = [=](bool b)          { WindowAction(WindowOp::Maximize, b);   };
	term.WhenWindowFullScreen     = [=](int i)           { WindowAction(WindowOp::FullScreen, i); };
	term.WhenWindowGeometryChange = [=](Rect r)          { WindowAction(WindowOp::Geometry, r);   };
}

void TerminalExample::Serialize(Stream& s)
{
	// Binary serialization support. It is also possible to use JSON or XML for persistence.

	term.Serialize(s);
	SerializePlacement(s);
}

void TerminalExample::Run()
{
	// Custon (high performance) event loop.

	pty.Start(GetEnv(tshell), Environment(), GetHomeDirectory());
	OpenMain();
	while(IsOpen() && pty.IsRunning()) {
		ProcessEvents();
		String s = pty.Get();
		int    l = s.GetLength();
		term.WriteUtf8(s);
		Sleep(l >= 1024 ? 1024 * 10 / l : 10); // Scale to workload.
	}
}

void TerminalExample::WindowAction(WindowOp action, Value arg)
{
	// xterm's window ops.

	if(IsFullScreen() && (action != WindowOp::FullScreen))
		FullScreen(false);

	switch(action) {
	case WindowOp::Geometry:
		SetRect(arg);
		break;
	case WindowOp::Minimize:
		IsMinimized() ? Overlap() : Minimize();
		break;
	case WindowOp::Maximize:
		IsMaximized() ? Overlap() : Maximize();
		break;
	case WindowOp::Resize:
		SetRect(Rect(GetRect().TopLeft(), AddFrameSize(arg)));
		break;
	case WindowOp::FullScreen:
		FullScreen(!int(arg) ? !IsFullScreen() : int(arg) > 0);
		break;
	default:
		return;
	}
	
	term.SetFocus();
}

void TerminalExample::MainMenu(Bar& bar)
{
	bar.Sub(t_("File"), [=](Bar& bar)    { FileMenu(bar); });
	bar.Sub(t_("Edit"), [=](Bar& bar)    { term.EditBar(bar); });
	bar.Sub(t_("View"), [=](Bar& bar)    { ViewMenu(bar); });
	bar.Sub(t_("Options"), [=](Bar& bar) { term.OptionsBar(bar); });
}

void TerminalExample::FileMenu(Bar& bar)
{
	bar.Add(t_("Exit"), [=] { Close(); });
}

void TerminalExample::ViewMenu(Bar& bar)
{
	// Custom menu to define and toggle custom terminal display sizes.

	bar.Add(t_("Toggle full screen"),
		[=]{ WindowAction(WindowOp::FullScreen, 0); })
		.Key(K_SHIFT_CTRL_F7)
		.Check(IsFullScreen());
	bar.Add(t_("Maximize window"),
		[=]{ WindowAction(WindowOp::Maximize); })
		.Key(K_SHIFT_CTRL_F6)
		.Check(IsMaximized());
	bar.Add(t_("Minimize window"),
		[=]{ WindowAction(WindowOp::Minimize); })
		.Key(K_SHIFT_CTRL_F5)
		.Check(IsMinimized());
	bar.Separator();
	bar.Add("80 x 24",
		[=]{ WindowAction(WindowOp::Resize, term.PageSizeToClient(80, 24));  })
		.Key(K_SHIFT_CTRL_F1);
	bar.Add("80 x 48",
		[=]{ WindowAction(WindowOp::Resize, term.PageSizeToClient(80, 48));  })
		.Key(K_SHIFT_CTRL_F2);
	bar.Add("132 x 24",
		[=]{ WindowAction(WindowOp::Resize, term.PageSizeToClient(132, 24)); })
		.Key(K_SHIFT_CTRL_F3);
	bar.Add("132 x 48",
		[=]{ WindowAction(WindowOp::Resize, term.PageSizeToClient(132, 48)); })
		.Key(K_SHIFT_CTRL_F4);
}

void TerminalExample::ContextMenu(Bar& bar)
{
	// Prepends the custom view menu to TerminalCtrl's standard menu.

	bar.Sub(t_("View"), [=](Bar& bar) { ViewMenu(bar); });
	bar.Separator();
	term.StdBar(bar);
}

GUI_APP_MAIN
{
	TerminalExample upp_term;
	LoadFromFile(upp_term);
	upp_term.Run();
	StoreToFile(upp_term);
}
