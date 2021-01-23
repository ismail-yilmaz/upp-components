#ifndef _TerminalLayoutExample_TerminalLayoutExample_h
#define _TerminalLayoutExample_TerminalLayoutExample_h

#include <CtrlLib/CtrlLib.h>
#include <Terminal/Terminal.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows, the PtyProcess class requires at least Windows 10 (tm)
// for the new pseudoconsole API support. To enable this feature, you
// need to set the WIN10 flag in TheIDE's main package configurations
// dialog. (i.e. "GUI WIN10")

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
