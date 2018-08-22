#ifndef _SshShellGUI_SshShellGUI_h
#define _SshShellGUI_SshShellGUI_h

#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <SSH/SSH.h>

using namespace Upp;

#define LAYOUTFILE <SshShellGUI/SshShellGUI.lay>
#include <CtrlCore/lay.h>

class SshConsole : public SshShell, public CodeEditor {
    bool Key(dword key, int count);
    void Output(const void *b, int l);
    
public:
    SshConsole(SshSession& session, bool x11);
    typedef SshConsole CLASSNAME;
};

class SshShellGUI : public TopWindow {
    SshSession        session;
    Array<SshConsole> shells;
    MenuBar           mainmenu;
    TabCtrl           tabs;
    bool              connected;

    void ShellFocus();
    SshConsole* GetFocusedShell();
       
public:
    void OpenShell(bool x11 = false);
    void RemoveTab(Ctrl& c);
    void MainMenu(Bar& bar);
    void FileMenu(Bar& bar);
    void HelpMenu(Bar& bar);
    void Run();
    
    SshShellGUI();
    typedef SshShellGUI CLASSNAME;
};

#endif
