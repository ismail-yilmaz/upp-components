#ifndef _StackCtrl_StackCtrl_h_
#define _StackCtrl_StackCtrl_h_

#include <CtrlLib/CtrlLib.h>

namespace Upp {
    
class StackCtrl : public ParentCtrl {
public:
    StackCtrl() : activectrl(nullptr), wheel(false) {}
    virtual ~StackCtrl() {}
    
    StackCtrl&  Wheel(bool b = true)    { wheel = b; return *this; }
    
    StackCtrl&  Add(Ctrl& ctrl);
    StackCtrl&  Insert(int i, Ctrl& ctrl);
        
    int         GetCount() const        { return list.GetCount(); }
    int         GetCursor() const       { return list.Find(activectrl);  }
    Ctrl&       Get(int i)              { ASSERT(i >= 0 && i < GetCount()); return *list[i]; }
    Ctrl&       operator[](int i)       { return Get(i); }

    int         Find(Ctrl& ctrl)        { return list.Find(&ctrl); }
    
    void        Goto(int i)             { if(i >= 0 && i < GetCount()) Activate(list[i]); }
    void        Goto(Ctrl& ctrl)        { Goto(list.Find(&ctrl)); }
    void        Prev();
    void        Next();
    void        GoBegin()               { Goto(0); }
    void        GoEnd()                 { Goto(GetCount() - 1); }

    void        ChildRemoved(Ctrl *ctrl) override;
    
private:
    void        Activate(Ctrl* ctrl);

    Index<Ctrl*> list;
    Ctrl*        activectrl;    // cursor
    bool         wheel:1;
};

}
#endif
