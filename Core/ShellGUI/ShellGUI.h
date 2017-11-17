#ifndef _ShellGUI_ShellGUI_h
#define _ShellGUI_ShellGUI_h

#include <CtrlLib/CtrlLib.h>
#include <SSH/SSH.h>
#include <CodeEditor/CodeEditor.h>

using namespace Upp;

#define LAYOUTFILE <ShellGUI/ShellGUI.lay>
#include <CtrlCore/lay.h>

class ShellGUI : public WithShellGUILayout<TopWindow> {
	SshSession session;
public:
	typedef ShellGUI CLASSNAME;
	void Run();
	ShellGUI();
};

#endif
