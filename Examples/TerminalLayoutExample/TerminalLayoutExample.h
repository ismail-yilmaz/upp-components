#ifndef _TerminalLayoutExample_TerminalLayoutExample_h
#define _TerminalLayoutExample_TerminalLayoutExample_h

#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows platform, PtyProcess class can use one of two backends:
// WinPty or the Windows 10 (tm) pseudoconsole  API. These  mutually
// exclusive backends can be enabled by setting WINPTY or WIN10 flag
// via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

using namespace Upp;

#define LAYOUTFILE <TerminalLayoutExample/TerminalLayoutExample.lay>
#include <CtrlCore/lay.h>

class TerminalExample : public WithTerminalExampleLayout<TopWindow> {
public:
	TerminalExample();
	void Serialize(Stream& s) override;
	void Run();
	
private:
	enum class WindowOp : int {
		FullScreen,
		Maximize,
		Minimize,
		Resize,
		Geometry
	};
	
	void	WindowAction(WindowOp action, Value arg = Null);
	void	MainMenu(Bar& bar);
	void	FileMenu(Bar& bar);
	void	ViewMenu(Bar& bar);
	void	ContextMenu(Bar& bar);
	
private:
	PtyProcess pty;
	MenuBar mainmenu;
};

#endif
