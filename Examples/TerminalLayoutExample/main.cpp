// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows platform PtyProcess class uses statically linked *winpty*
// library and the supplementary PtyAgent pacakges as its *default* pty
// backend. However, it also supports the Windows 10 (tm) pseudoconsole
// API via the WIN10 compiler flag. This flag can be enabled or disable
// easily via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

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

void TerminalExample::About()
{
	String txt;
	txt << "-----------------------------------\r\n"
		<< "# A terminal gui building example #\r\n"
		<< "# Using \033]8;;https:/www.ultimatepp.org\033\\"
		<< "\033[1;36mUltimate++\033[m\033]8;;\033\\ technology!    #\r\n"
		<< "-----------------------------------\r\n";
	term.Echo(txt);
}

void TerminalExample::Run()
{
	// a simple splash/greeting screen
	About();
	// Custon (high performance) event loop.
#if defined(PLATFORM_POSIX) && defined(IUTF8)
	// Set or change the initial terminal io flags on POSIX.
	pty.WhenAttrs = [=](termios& t) { t.c_iflag |= IUTF8; return true; };
#endif
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

	// Only keys...
	bar.AddKey(K_SHIFT|K_CTRL|K_ADD,	  [=] { FontZoom(1);     });
	bar.AddKey(K_SHIFT|K_CTRL|K_SUBTRACT, [=] { FontZoom(-1);    });
	bar.AddKey(K_SHIFT|K_CTRL|K_MULTIPLY, [=] { FontZoom(0);     });
	bar.AddKey(K_SHIFT|K_ALT|K_ADD,	      [=] { LineSpacing(1);  });
	bar.AddKey(K_SHIFT|K_ALT|K_SUBTRACT,  [=] { LineSpacing(-1); });
	bar.AddKey(K_SHIFT|K_ALT|K_MULTIPLY,  [=] { LineSpacing(0);  });
}

void TerminalExample::ContextMenu(Bar& bar)
{
	// Prepends the custom view menu to TerminalCtrl's standard menu.

	bar.Sub(t_("View"), [=](Bar& bar) { ViewMenu(bar); });
	bar.Separator();
	term.StdBar(bar);
	bar.AddKey(K_SHIFT|K_ALT_U, [=]{ EnterCodePoint(); });
}

void TerminalExample::FontZoom(int n)
{
	Font f = term.GetFont();
	int  y = decode(n, 0, GetStdFont().GetHeight(), f.GetHeight());
	term.SetFont(f.Height(clamp(y + n, 6, 128)));
}

void TerminalExample::LineSpacing(int n)
{
	term.SetPadding(Size(0,  decode(n, 0, 0, term.GetPadding().cy + n)));
}

void TerminalExample::EnterCodePoint()
{
	// Pop up the unicode codepoint input widget at cursor position.

	EditCodePoint q(*this);
	q.PopUp();
	if(!IsNull(~q)) {
		dword n = ScanInt(q.GetText(), nullptr, 16);
		term.Key(n, 1);
	}
}

void EditCodePoint::PopUp()
{
	if(!app.term.GetRect().Contains(app.term.GetCursorPoint()))
		return;
	Font f = app.term.GetFont();
	Size sz = app.term.GetCellSize();
	Point pt = app.term.GetScreenView().TopLeft() + app.term.GetCursorPoint();
	FrameLeft<DisplayCtrl> label;
	FrameRight<DisplayCtrl> preview;
	label.SetDisplay(StdRightDisplay());
	label.SetData(AttrText("U+").SetFont(f).Ink(SColorDisabled));
	preview.SetDisplay(StdCenterDisplay());
	AddFrame(label.Width(sz.cx * 2));
	AddFrame(preview.Width(sz.cx * 3));
	MaxChars(4).SetFont(f).SetFilter([](int c) { return IsXDigit(c) ? c : 0; });
	WhenEnter = app.Breaker();
	WhenAction = [=, &preview, &f] {
		AttrText txt;
		int n = ScanInt(GetText(), nullptr, 16);
		if(n >= 0x00 && n <= 0x1F)
			txt.Text("C0").Ink(SLtRed);
		else
		if(n >= 0x80 && n <= 0x9F)
			txt.Text("C1").Ink(SLtRed);
		else
			txt = WString(n, 1);
		preview.SetData(txt.SetFont(f));
	};
	SetRect(RectC(pt.x + 2, pt.y - 4, sz.cx * 11, sz.cy + 8));
	Ctrl::PopUp(&app, true, true, false, false);
	EventLoop(&app);
}

bool EditCodePoint::Key(dword key, int count)
{
	if(key == K_ESCAPE) {
		SetData(Null);
		app.Break();
		return true;
	}
	return EditString::Key(key, count);
}

void EditCodePoint::LostFocus()
{
	Key(K_ESCAPE, 1);
}

GUI_APP_MAIN
{
	TerminalExample upp_term;
	LoadFromFile(upp_term);
	upp_term.Run();
	StoreToFile(upp_term);
}
