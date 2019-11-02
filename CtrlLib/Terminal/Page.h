#ifndef _VTPage_h_
#define _VTPage_h_

#include <Core/Core.h>
#include "Cell.h"

namespace Upp {

// WARNING: VTLine's and VTPage's interfaces are not stable.
// They are subject to change. do not use them! (Not yet.)

class VTLine : public Moveable<VTLine, Vector<VTCell>> {
public:
    VTLine();

    void         Adjust(int cx, const VTCell& filler);
    void         ShiftLeft(int begin, int end, int n, const VTCell& filler);
    void         ShiftRight(int begin, int end, int n, const VTCell& filler);
    bool         Fill(int begin, int end, const VTCell& filler, dword flags = 0);

    void         Validate(bool b = true) const           { invalid = !b;    }
    void         Invalidate() const                      { invalid = true;  }
    inline bool  IsInvalid() const                       { return invalid;  }

    void         Wrap() const                            { wrapped = true;  }
    void         Unwrap() const                          { wrapped = false; }
    inline bool  IsWrapped() const                       { return wrapped;  }

private:
    mutable bool  invalid:1;
    mutable bool  wrapped:1;
};

class VTPage : Moveable<VTPage> {

    struct Cursor {
        int     x;
        int     y;
        bool    eol;
        bool    wrap;
        bool    relative;
        void    Clear()                             { x = y = 1; eol = wrap = relative = false; }
        operator Point() const                      { return Point(x, y); }
        void    operator=(const Point& p)           { x = p.x; y = p.y; }
        void    operator=(const Nuller&)            { Clear(); }
        Cursor()                                    { Clear(); }
        Cursor(int x_, int y_)                      { Clear(); x = max(1, x_) ; y = max(1, y_); }
    };

    enum Margins {
        MARGIN_NONE         = 0x00,
        MARGIN_LEFT         = 0x01,
        MARGIN_TOP          = 0x02,
        MARGIN_RIGHT        = 0x04,
        MARGIN_BOTTOM       = 0x08,
        MARGIN_HORZ         = MARGIN_LEFT|MARGIN_RIGHT,
        MARGIN_VERT         = MARGIN_TOP|MARGIN_BOTTOM,
        MARGIN_TOPLEFT      = MARGIN_TOP|MARGIN_LEFT,
        MARGIN_BOTTOMRIGHT  = MARGIN_BOTTOM|MARGIN_RIGHT,
        MARGIN_ALL          = MARGIN_HORZ|MARGIN_VERT
    };

    enum class   Element { CELL, LINE };

public:
    using Lines = Vector<VTLine>;

    enum Metrics : int {
        MINCOL      = 2,
        MINROW      = 2,
        STDCOL      = 80,
        STDROW      = 24,
        MAXCOL      = 132,
        MAXROW      = 24,
    };

public:
    Event<> WhenCursor;
    Event<> WhenScroll;

    VTPage&  PermitEvents(bool b = true)                 { events = b; return *this;     }
    VTPage&  ForbidEvents()                              { events = false; return *this; }

    VTPage& OriginMode(bool b = true);
    VTPage& WrapAround(bool b = true);
    VTPage& History(bool b = true);
    VTPage& Attributes(const VTCell& attrs)             { cellattrs = attrs; return *this; }

    bool    IsOriginMode() const                        { return cursor.relative; }
    bool    IsWrapAround() const                        { return cursor.wrap;     }

    void    SetCell(int x, int y, const VTCell& cell);
    void    SetCell(const Point& p, const VTCell& cell) { SetCell(p.x, p.y, cell);    }
    void    SetCell(const VTCell& cell)                 { SetCell(cursor.x, cursor.y, cell);}
    int     AddCell(const VTCell& cell);
    VTPage& InsertCell(const VTCell& cell);
    VTCell& GetCell(int x, int y)                       { return lines[y - 1][x - 1];         }
    VTCell& GetCell()                                   { return GetCell(cursor.x, cursor.y); }

    VTPage& MoveTo(int x, int y)                        { return MoveHorz(x).MoveVert(y); }
    VTPage& MoveTo(Point pt)                            { return MoveTo(pt.x, pt.y);    }
    VTPage& MoveToLine(int n)                           { return MoveVert(n);           }
    VTPage& MoveToColumn(int n)                         { return MoveHorz(n);           }
    VTPage& MoveUp(int n = 1)                           { return MoveVert(-n, true);    }
    VTPage& MoveDown(int n = 1)                         { return MoveVert(n, true);     }
    VTPage& MoveRight(int n = 1)                        { return MoveHorz(n, true);     }
    VTPage& MoveLeft(int n = 1)                         { return MoveHorz(-n, true);    }
    VTPage& MoveHome()                                  { return MoveHorz(1);           }
    VTPage& MoveEnd()                                   { return MoveHorz(size.cx);     }
    VTPage& MoveTopLeft()                               { return MoveTo(1, 1);          }
    VTPage& MoveBottomRight()                           { return MoveTo(size);          }

    VTPage& ScrollUp(int n = 1)                         { LineInsert(margin.top, n, cellattrs); return *this; }
    VTPage& ScrollDown(int n = 1)                       { LineRemove(margin.top, n, cellattrs); return *this; }
    VTPage& ScrollLeft(int n = 1)                       { CellInsert(margin.left, n, cellattrs, true); return *this; }
    VTPage& ScrollRight(int n = 1)                      { CellRemove(margin.left, n, cellattrs, true); return *this; }

    VTPage& InsertLines(int n)                          { return InsertLines(cursor.y, n); }
    VTPage& InsertLines(int pos, int n)                 { if(IsContained()) LineInsert(pos, n, cellattrs); return MoveHome(); }
    VTPage& RemoveLines(int n)                          { return RemoveLines(cursor.y, n); }
    VTPage& RemoveLines(int pos, int n)                 { if(IsContained()) LineRemove(pos, n, cellattrs); return MoveHome(); }

    VTPage& NextLine(int n = 1)                         { return MoveVert(n,  true, IsContained()); }
    VTPage& PrevLine(int n = 1)                         { return MoveVert(-n, true, IsContained()); }
    VTPage& NewLine(int n = 1)                          { return MoveHome().NextLine(n);          }
    VTPage& NextColumn(int n = 1)                       { return MoveHorz(n,  true, IsContained()); }
    VTPage& PrevColumn(int n = 1)                       { return MoveHorz(-n, true, IsContained()); }

    VTPage& InsertCells(int n)                          { return InsertCells(cursor.x, n); }
    VTPage& InsertCells(int pos, int n)                 { if(IsHContained()) CellInsert(pos, n, cellattrs); return *this; }
    VTPage& RemoveCells(int n)                          { return RemoveCells(cursor.x, n); }
    VTPage& RemoveCells(int pos, int n)                 { if(IsHContained()) CellRemove(pos, n, cellattrs); return *this; }
    VTPage& EraseCells(int n, dword flags = 0)          { return FillLine(cursor.y, cursor.x, cursor.x + n - 1, cellattrs, flags); }
    VTPage& RepeatCell(int n);

    VTPage& NextTab(int n = 1);
    VTPage& PrevTab(int n = 1);
    VTPage& SetTabAt(int i, bool b = true)              { SetTabstop(i, b); return *this; }
    VTPage& SetTab(bool b = true)                       { SetTabstop(cursor.x, b); return *this; }
    VTPage& SetTabs(int tsz);
    VTPage& ClearTabs()                                 { tabs.Clear(); tabsync = false; return *this; }
    void    GetTabs(Vector<int>& tabstops);

    VTPage& EraseLine(dword flags = 0);
    VTPage& EraseLeft(dword flags = 0);
    VTPage& EraseRight(dword flags = 0);

    VTPage& ErasePage(dword flags = 0);
    VTPage& EraseBefore(dword flags = 0);
    VTPage& EraseAfter(dword flags = 0);

    VTPage& FillLine(int pos, const VTCell& filler, dword flags);
    VTPage& FillLine(int pos, int b, int e, const VTCell& filler, dword flags);
    VTPage& FillRect(const Rect& r, const VTCell& filler, dword flags);
    VTPage& FillRect(const Rect& r, dword chr);
    VTPage& EraseRect(const Rect& r, dword flags = 0)   { return FillRect(r, cellattrs, flags); }
    VTPage& CopyRect(const Rect& r, const Point& pt);

    void    GetRectArea(Rect r, Event<const VTCell&, const Point&> consumer);
    void    GetRelRectArea(Rect r, Event<const VTCell&, const Point&> consumer);

    VTPage& FillStream(const Rect& r, const VTCell& filler, dword flags);
    VTPage& FillStream(const Rect& r, dword chr);

	VTPage& AddImage(Size sz, dword imageid, bool scroll, bool relpos = false);
		
    VTPage& SetVertMargins(int t, int b);
    VTPage& SetHorzMargins(int l, int r);
    VTPage& SetMargins(const Rect& r)                   { return SetMargins(r.left, r.top, r.right, r.bottom); }
    VTPage& SetMargins(int l, int t, int r, int b);
    Rect    GetMargins() const                          { return margin; }

    VTPage& SetSize(Size sz);
    Size    GetSize() const                             { return size;   }
    Size    GetRelSize() const                          { return margin.GetSize() + 1; }
    Rect    GetRect() const                             { return Rect(1, 1, size.cx, size.cy); }
    Rect    GetRelRect() const                          { return cursor.relative ? margin : GetRect(); }
    Point   GetPos() const;
    Point   GetRelPos() const;

    void    ClearEol()                                  { cursor.eol = false;}
    bool    IsEol() const                               { return cursor.eol; }

    VTPage& Backup()                                    { backup = cursor; return *this; }
    VTPage& Restore()                                   { cursor = backup; return *this; }
    VTPage& Reset();

    void    Invalidate()                                { for(auto& line : lines) line.Invalidate(); }
    void    Invalidate(int begin, int end);

    void    Shrink();

    const Lines& GetHistory() const                     { return saved; }
    void  EraseHistory()                                { saved.Clear(); Shrink(); if(events) WhenScroll(); }
    bool  HasHistory() const                            { return scrollback; }
    void  SetHistorySize(int sz)                        { historysize = max(1, sz); AdjustHistorySize(); if(events) WhenScroll(); }

    // For easy enumeration
    int   GetLineCount() const                           { return lines.GetCount() + saved.GetCount(); }
    const VTLine& GetLine(int i) const;

    const VTLine* begin() const                           { return lines.begin(); }
    VTLine*       begin()                                 { return lines.begin(); }
    const VTLine* end() const                             { return lines.end();   }
    VTLine*       end()                                   { return lines.end();   }

    virtual void Serialize(Stream& s);

    VTPage();
    VTPage(Size sz) : VTPage()                          { SetSize(sz);       }
    virtual ~VTPage()                                   {}

private:
    VTPage& MoveHorz(int col, bool rel = false, bool scrl = false);
    VTPage& MoveVert(int row, bool rel = false, bool scrl = false);
    void    LineInsert(int pos, int n, const VTCell& attrs);
    void    LineRemove(int pos, int n, const VTCell& attrs);
    void    CellInsert(int pos, int n, const VTCell& attrs, bool pan = false);
    void    CellRemove(int pos, int n, const VTCell& attrs, bool pan = false);
    void    RectFill(const Rect& r, const VTCell& filler, dword flags = 0);
    void    UnwindHistory(const Size& prevsize);
    void    RewindHistory(const Size& prevsize);
    bool    AddToHistory(int pos);
    void    AdjustHistorySize();
    int     AdjustCol(int& col, bool rel, bool scrl) const;
    int     AdjustRow(int& row, bool rel, bool scrl) const;
    Point   AdjustPoint(Point pt, dword delta = MARGIN_ALL);
    Rect    AdjustRect(Rect r, dword delta = MARGIN_ALL);
    int     GetRelX(int x) const                        { return cursor.relative ?  x - margin.left + 1 : x; }
    int     GetRelY(int y) const                        { return cursor.relative ?  y - margin.top  + 1 : y; }
    Point   GetPoint(Point pt);
    Rect    GetAreaRect(const Rect& r);
    int     AdjustHStep(int pos, int n) const           { return clamp(n, 0, margin.right - pos + 1);  }
    int     AdjustVStep(int pos, int n) const           { return clamp(n, 0, margin.bottom - pos + 1); }
    bool    IsHContained() const                        { return cursor.x >= margin.left && cursor.x <= margin.right;  }
    bool    IsVContained() const                        { return cursor.y >= margin.top  && cursor.y <= margin.bottom; }
    bool    IsContained() const                         { return IsHContained() && IsVContained();   }
    void    SetEol(bool b)                              { cursor.eol = b; }
    int     SetTabstop(int col, bool b)                 { tabs.Set(col, b); return ++col; }
    bool    IsTabstop(int col)                          { return tabs[col]; }

    Lines   lines;
    Lines   saved;
    Cursor  cursor;
    Cursor  backup;
    Size    size;
    Rect    margin;
    Bits    tabs;
    int     tabsize;
    int     historysize;
    bool    tabsync;
    bool    events;
    bool    scrollback;
    VTCell  cellattrs;
};
}
#endif
