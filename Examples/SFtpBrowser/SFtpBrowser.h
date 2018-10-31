#ifndef _SFtpGUI_SFtpGUI_h
#define _SFtpGUI_SFtpGUI_h

#include <CtrlLib/CtrlLib.h>
#include <Core/SSH/SSH.h>

using namespace Upp;

#define LAYOUTFILE <SFtpBrowser/SFtpBrowser.lay>
#include <CtrlCore/lay.h>

#define IMAGECLASS Images
#define IMAGEFILE <SFtpBrowser/SFtpBrowser.iml>
#include <Draw/iml_header.h>

class SFtpBrowser : public WithBrowserLayout<TopWindow> {
    enum    Opcode { GET, PUT };
    WithSettingsLayout<TopWindow> settings;
    OpenFileButton prikeysel, pubkeysel;
    SelectDirButton dirsel;
    SFtpFileSystemInfo filesystem;
    SshSession session;
    One<SFtp>  browser;
    String     workdir;
    String     basedir;
    MenuBar    mainmenu;
    bool       sortbyext;
    bool       connected;
    
    void    SessionError()              { ErrorOK(DeQtf(session.GetErrorDesc())); }
    void    BrowserError()              { ErrorOK(DeQtf(browser->GetErrorDesc())); }
    void    UpdateGui()                 { ProcessEvents(); }
    void    Workdir(const String& s);
    String  GetWorkdir() const          { return dir.GetData().ToString(); }
    void    Close() final;
    void    Sync();
    void    Info();
    void    Summary();
    void    DirUp();
    void    Action();
    void    Connect();
    void    Disconnect();
    void    LoadDir();
    void    Upload();
    void    Download(const String& src);
    void    Transfer(int opcode, const String src, const String& dest);
    void    Rename();
    void    MakeDir();
    void    Delete();
    void    Settings();
    void    MainMenu(Bar& bar);
    void    FileMenu(Bar& bar);
    void    HelpMenu(Bar& bar);
    void    ContextMenu(Bar& bar);
    void    SortMenu(Bar& bar);
    
public:
    void    Serialize(Stream& s);
    typedef SFtpBrowser CLASSNAME;
    SFtpBrowser();
};


#endif
