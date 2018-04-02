#ifndef _SFtpGUI_SFtpGUI_h
#define _SFtpGUI_SFtpGUI_h

#include <CtrlLib/CtrlLib.h>
#include <SSH/SSH.h>

using namespace Upp;

#define LAYOUTFILE <SFtpGUI/SFtpGUI.lay>
#include <CtrlCore/lay.h>

class SFtpGUI : public WithSFtpGUILayout<TopWindow> {
    enum    OpCode { GET, PUT };
    SshSession session;
    One<SFtp>  browser;
    String workdir;
    String basedir;
    MenuBar menu;
    bool connected;

    void Error();
    void Sync();
    void Dirup();
    void Action();
    void Connect();
    void Disconnect();
    void LoadDirectory();
    void Upload();
    void Download(const String& f);
    void Transfer(OpCode cmd, const String& src, const String& dest);
    void Rename();
    void Delete();
    void MakeDir();
    void Menu(Bar& bar);

public:
    void Serialize(Stream& s);
    typedef SFtpGUI CLASSNAME;
    SFtpGUI();
};

#endif
