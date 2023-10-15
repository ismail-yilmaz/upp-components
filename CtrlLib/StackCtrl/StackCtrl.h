#ifndef _StackCtrl_StackCtrl_h_
#define _StackCtrl_StackCtrl_h_

#include <CtrlLib/CtrlLib.h>

namespace Upp {

class StackCtrl : public ParentCtrl {
public:
    StackCtrl();
    virtual ~StackCtrl() {}

    StackCtrl&  Wheel(bool b = true)          { wheel = b; return *this; }
    StackCtrl&  Animation(int ms = 150)       { duration = clamp(ms, 0, 1000); return *this; }
    StackCtrl&  Horz()                        { vertical = false; return *this; }
    StackCtrl&  Vert()                        { vertical = true;  return *this; }

    StackCtrl&  Add(Ctrl& ctrl);
    StackCtrl&  Insert(int i, Ctrl& ctrl);
    void        Remove(Ctrl& ctrl);
    void        Remove(int i)           { if(i >= 0 && i < GetCount()) Remove(*list[i]); }

    int         GetCount() const        { return list.GetCount(); }
    int         GetCursor() const       { return list.Find(activectrl);  }
    Ctrl&       Get(int i)              { ASSERT(i >= 0 && i < GetCount()); return *list[i]; }
    Ctrl&       operator[](int i)       { return Get(i); }

	Ctrl*       GetActiveCtrl() const   { return list.Find(activectrl) >= 0 ? activectrl : nullptr; }
		
    int         Find(Ctrl& ctrl) const  { return list.Find(&ctrl); }

    void        Goto(int i)             { if(i >= 0 && i < GetCount()) Activate(list[i]); }
    void        Goto(Ctrl& ctrl)        { Goto(list.Find(&ctrl)); }
    void        Prev();
    void        Next();
    void        GoBegin()               { Goto(0); }
    void        GoEnd()                 { Goto(GetCount() - 1); }

	void		Serialize(Stream& s) override;
	
private:
    void        Activate(Ctrl* ctrl);

    // Animation related stuff.
    enum class Direction { Left, Right, Up, Down };
    bool ScrollCtrl(Direction direction, Ctrl& ctrl, Rect r, Rect target, int time);
    void Animate(Ctrl *next);

    Index<Ctrl*> list;
    Ctrl*        activectrl;    // cursor
    bool         wheel;
    bool         vertical;
    int          duration;
};

}
#endif
