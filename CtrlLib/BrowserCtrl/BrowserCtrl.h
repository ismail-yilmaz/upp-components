#ifndef BROWSERCTRL_H
#define BROWSERCTRL_H

#include <CtrlLib/CtrlLib.h>

namespace Upp {

#define LAYOUTFILE <BrowserCtrl/BrowserCtrl.lay>
#include <CtrlCore/lay.h>

class BrowserCtrl : public WithFileSelLayout<ParentCtrl> {
public:
    BrowserCtrl& BaseDir(const String& s)       { return ActiveDir(basedir = s); }
    BrowserCtrl& ActiveDir(const String& s);
    BrowserCtrl& AddPath(const String& s)       { return ActiveDir(s); }
    BrowserCtrl& Multi(bool b = true)           { filelist.Multi(b); return *this; }
    BrowserCtrl& SortByExt(bool b = true)       { sortbyext = b; return *this; }
    BrowserCtrl& DnD(bool b = true)             { dnd = b; return *this; }
    BrowserCtrl& Ask(bool b = true)             { ask = b; return *this; }
    BrowserCtrl& StdMenu(bool b = true)         { stdmenu = b; return *this; }
    BrowserCtrl& DirUpOption(bool b = true)     { dirup.Show(b); return *this; }
    BrowserCtrl& MkDirOption(bool b = true)     { mkdir.Show(b); return *this; }
    BrowserCtrl& Columns(int n)                 { filelist.Columns(n); return *this ; }
    BrowserCtrl& Renaming(bool b = true)        { filelist.Renaming(b); return *this; }
    BrowserCtrl& AccelKey(bool b = true)        { filelist.AccelKey(b); return *this; }
    BrowserCtrl& JustName(bool b = true)        { filelist.JustName(b); return *this; }
    BrowserCtrl& ShowInfoPanel(bool b = true);
    BrowserCtrl& HideInfoPanel()                { return ShowInfoPanel(false); }

    String GetBaseDir() const                   { return basedir; }
    bool IsBaseDir()                            { return !basedir.IsEmpty() && basedir != GetActiveDir(); }
    String GetActiveDir() const                 { return dir.GetData().ToString(); }
    const Vector<String>& GetSelected() const   { return selected; }
    FileList& GetFileList()                     { return filelist; }
    const FileList::File& GetFileInfo() const   { return filelist.Get(filelist.GetCursor()); }

    void KillCursor()                           { filelist.KillCursor(); }
    bool IsCursor() const                       { return filelist.IsCursor(); }

    bool Load();
    void Action();
    void Rename();
    void MkDir();
    void Delete();
    void DirUp();
    void Select();
    void Clear();

    Event<Bar&> WhenBar;
    Event<>     WhenSel;
//  Event<>     WhenAction;
    Gate<>      WhenDirUp;
    Gate<>      WhenLoad;
    Event<>     WhenDrag;
    Event<PasteClip> WhenDrop;
    Event<>     WhenDropFiles;
    Gate<const String&> WhenDelete;
    Gate<const String&> WhenMkDir;
    Gate<const String&, const String&> WhenRename;


    virtual void Serialize(Stream& s);

    BrowserCtrl();
    virtual ~BrowserCtrl() {}
    typedef BrowserCtrl CLASSNAME;

private:
    Vector<String> selected;
    String  basedir;
    String  path;
    bool    ask;
    bool    dnd;
    bool    sortbyext;
    bool    stdmenu;

    void    Reload(const String& s = Null);
    void    Info();
    void    Summary();
    void    Relabel(const String& s1, const String& s2);
    void    Drag();
    void    Drop(PasteClip& d);
    String  GetFullPath(const String& s);
    void    Sync();
    void    KillCurSel();
    void    ContextMenu(Bar& bar);
    void	SortMenu(Bar& bar);
};
}
#endif
