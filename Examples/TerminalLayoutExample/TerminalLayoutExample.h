#ifndef _TerminalLayoutExample_TerminalLayoutExample_h
#define _TerminalLayoutExample_TerminalLayoutExample_h

#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

// This example demonstrates a simple, cross-platform (POSIX/Windows)
// terminal example.

// On Windows platform PtyProcess class uses statically linked *winpty*
// library and the supplementary PtyAgent pacakges as its *default* pty
// backend. However, it also supports the Windows 10 (tm) pseudoconsole
// API via the WIN10 compiler flag. This flag can be enabled or disable
// easily via TheIDE's main package configuration dialog. (E.g: "GUI WIN10")

using namespace Upp;

#define LAYOUTFILE <examples/TerminalLayoutExample/TerminalLayoutExample.lay>
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
    
    void    WindowAction(WindowOp action, Value arg = Null);
    void    MainMenu(Bar& bar);
    void    FileMenu(Bar& bar);
    void    ViewMenu(Bar& bar);
    void    ContextMenu(Bar& bar);

    void    FontZoom(int n);
    void    LineSpacing(int n);
    void    EnterCodePoint();
    void    About();
    
private:
    PtyProcess pty;
    MenuBar mainmenu;
};

// Helper popup for unicode codepoint input.

class EditCodePoint : public EditString {
public:
    EditCodePoint(TerminalExample& t) : app(t) {}

    void    PopUp();
    bool    Key(dword key, int count) override;
    void    LostFocus() override;

private:
    TerminalExample& app;
};

#endif
