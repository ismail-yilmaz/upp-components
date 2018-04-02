#ifndef _FtpGUI_FtpGUI_h
#define _FtpGUI_FtpGUI_h

#include <CtrlLib/CtrlLib.h>
#include <FTP/Ftp.h>

using namespace Upp;

#define LAYOUTFILE <FtpGUI/FtpGUI.lay>
#include <CtrlCore/lay.h>

class FtpGUI : public WithFtpGUILayout<TopWindow> {
    enum    OpCode { GET, PUT };
    MenuBar menu;
    Ftp     browser;
    String  workdir;
    String  basedir;
    bool    connected;
   
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
    FtpGUI();
    typedef FtpGUI CLASSNAME;
};

#endif
