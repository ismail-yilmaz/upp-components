#ifndef _TrackerCtrlCtrl_TrackerCtrlCtrl_h
#define _TrackerCtrlCtrl_TrackerCtrlCtrl_h

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

#define LAYOUTFILE <TrackerCtrl/TrackerCtrl.lay>
#include <CtrlCore/lay.h>

class TrackerCtrl : public WithTrackerCtrlLayout<ParentCtrl> {
public:
    struct Item : Moveable<Item> {
        enum Status { STARTED = 1, PROGRESS, SUCCESS, FAILURE,  RESTARTED };
        enum Type   { GET, PUT };
        Value   id;
        int     type;
        int     status;
        String  source;
        String  destination;
        int64   size;
        int64   done;
        String  message;
        int     error;
        Value   data;
        ValueArray  operator()() const;
        bool    IsStarted() const               { return status == STARTED; }
        bool    IsRestarted() const             { return status == RESTARTED; }
        bool    InProgress() const              { return status == PROGRESS; }
        bool    IsSuccess() const               { return status == SUCCESS; }
        bool    IsFailure() const               { return status == FAILURE; }
        Item()  { id = 0; size = 0; done = 0; status = 0; error = 0; type = GET; }
    };

public:
    TrackerCtrl& StdMenu(bool b = true)         { stdmenu = b; return *this; }
    TrackerCtrl& StdToolBar(bool b = true)      { stdbar = b; return *this; }
    TrackerCtrl& ShowActive()                   { filter.SetIndex(ACTIVE); Filter(); return *this; }
    TrackerCtrl& ShowFinished()                 { filter.SetIndex(FINISHED); Filter(); return *this; }
    TrackerCtrl& ShowFailed()                   { filter.SetIndex(FAILED); Filter(); return *this; }
    TrackerCtrl& ShowAll()                      { filter.SetIndex(ALL); Filter(); return *this; }
    TrackerCtrl& ShowStatusBar(bool b = true)   { if(b) AddFrame(status); else RemoveFrame(status); return *this; }
    TrackerCtrl& HideStatusBar()                { return ShowStatusBar(false); }

    const Item& Get() const                     { return Get(GetCursor()); }
    const Item& Get(int i) const                { return items.Get(itemlist.Get(i, ID)).item; }
    inline int GetCursor() const                { return itemlist.GetCursor();}
    inline bool IsCursor() const                { return itemlist.IsCursor(); }

    void Track(const Item& t);
    void Serialize(Stream& s);

    Event<Bar&> WhenBar;
    Event<>     WhenRetry;
    Event<>     WhenCancel;

    TrackerCtrl();
    virtual ~TrackerCtrl() {}
    typedef TrackerCtrl CLASSNAME;

private:
    struct TI : Moveable<TI> {
        Item  item;
        Time  start;
        Time  end;
        Time  elapsed;
        ValueArray operator()() const;
        TI() {}
    };

    void  Add(const Item& t, bool put = false);
    TI&   GetAdd(const Item& t);
    void  Set(const Item& t, int i, const Value& v);
    void  Filter();
    void  Sync(const TI& ti);
    void  Summary();
    void  Populate(int i, One<Ctrl>& ctrl);

    void  MainToolBar(Bar& bar);
    void  ContextMenu(Bar& bar);
    void  ColumnMenu(Bar& bar);
    void  FilterMenu(Bar& bar);
    void  ActionMenu(Bar& bar);

    int64 tsize;
    int64 tdone;
    StatusBar status;
    ToolBar toolbar;
    DropList filter;
    VectorMap<Value, TI> items;
    enum Columns  { ID, SOURCE, DESTINATION, PROGRESS, SIZE, STATUS, STIME, ELAPSED, FTIME };
    enum Filter   { ACTIVE, FINISHED, FAILED, ALL };
    bool stdmenu;
    bool stdbar;
};

TrackerCtrl::Item ConvertResult(Ftp::Result r, bool put = false);
#endif
