#ifndef _VTPage_h_
#define _VTPage_h_

#include <Core/Core.h>
#include "Cell.h"

namespace Upp {

// WARNING: VTPage's interface is not yet stable. It is subject to change!

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
    struct Line : Moveable<Line, Vector<VTCell>> {
        bool    wrapped;
        mutable bool invalid;
        void    Adjust(int cx, const VTCell& attrs);
        void    ShiftLeft(int b, int e, int n, const VTCell& attrs);
        void    ShiftRight(int b, int e, int n, const VTCell& attrs);
        void    Validate() const                    { invalid = false; }
        void    Invalidate() const                  { invalid = true;  }
        bool    IsInvalid() const                   { return invalid;  }
        Line()                                      { wrapped = false; invalid = true; }
    };

    using Lines = Vector<Line>;

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

    VTPage& OriginMode(bool b = true);
    VTPage& WrapAround(bool b = true);
    VTPage& History(bool b = true);
    VTPage& Attributes(const VTCell& attrs)             { cellattrs = attrs; return *this; }

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

    VTPage& ScrollUp(int n = 1)                         { Insert(Element::LINE, margin.top, n); return *this; }
    VTPage& ScrollDown(int n = 1)                       { Remove(Element::LINE, margin.top, n); return *this; }
    VTPage& ScrollLeft(int n = 1)                       { Insert(Element::CELL, margin.left, n, true); return *this; }
    VTPage& ScrollRight(int n = 1)                      { Remove(Element::CELL, margin.left, n, true); return *this; }

    VTPage& InsertLines(int n)                          { return InsertLines(cursor.y, n); }
    VTPage& InsertLines(int pos, int n)                 { if(IsContained()) Insert(Element::LINE, pos, n); return MoveHome(); }
    VTPage& RemoveLines(int n)                          { return RemoveLines(cursor.y, n); }
    VTPage& RemoveLines(int pos, int n)                 { if(IsContained()) Remove(Element::LINE, pos, n); return MoveHome(); }

    VTPage& NextLine(int n = 1)                         { return MoveVert(n,  true, IsContained()); }
    VTPage& PrevLine(int n = 1)                         { return MoveVert(-n, true, IsContained()); }
    VTPage& NewLine(int n = 1)                          { return MoveHome().NextLine(n);          }
    VTPage& NextColumn(int n = 1)                       { return MoveHorz(n,  true, IsContained()); }
    VTPage& PrevColumn(int n = 1)                       { return MoveHorz(-n, true, IsContained()); }

    VTPage& InsertCells(int n)                          { return InsertCells(cursor.x, n); }
    VTPage& InsertCells(int pos, int n)                 { if(IsHContained()) Insert(Element::CELL, pos, n); return *this; }
    VTPage& RemoveCells(int n)                          { return RemoveCells(cursor.x, n); }
    VTPage& RemoveCells(int pos, int n)                 { if(IsHContained()) Remove(Element::CELL, pos, n); return *this; }
    VTPage& EraseCells(int n, dword flags = 0)          { LineFill(cursor.y, cursor.x, cursor.x + n - 1, cellattrs, flags); return *this; }
    VTPage& RepeatCell(int n);

    VTPage& NextTab(int n = 1);
    VTPage& PrevTab(int n = 1);
    VTPage& SetTab(bool b = true)                       { SetTabstop(cursor.x, b); return *this; }
    VTPage& SetTabs(int tsz);
    VTPage& ClearTabs()                                 { tabs.Clear(); tabsync = false; return *this; }

    VTPage& EraseLine(dword flags = 0);
    VTPage& EraseLeft(dword flags = 0);
    VTPage& EraseRight(dword flags = 0);

    VTPage& ErasePage(dword flags = 0);
    VTPage& EraseBefore(dword flags = 0);
    VTPage& EraseAfter(dword flags = 0);

    VTPage& CopyRect(const Rect& r, const Point& pt);
    VTPage& FillRect(const Rect& r, const VTCell& filler, dword flags);
    VTPage& FillRect(const Rect& r, int chr);
    VTPage& EraseRect(const Rect& r, dword flags = 0)   { return FillRect(r, cellattrs, flags); }

    void    GetRectArea(Rect r, Event<const VTCell&, const Point&> consumer);
    void    GetRelRectArea(Rect r, Event<const VTCell&, const Point&> consumer);

    VTPage& FillStream(const Rect& r, const VTCell& filler, dword flags);
    VTPage& FillStream(const Rect& r, int chr);

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
    void  EraseHistory()                                { saved.Clear(); Shrink(); WhenScroll(); }
    bool  HasHistory() const                            { return scrollback; }
    void  SetHistorySize(int sz)                        { historysize = max(1, sz); AdjustHistorySize(); WhenScroll(); }

    // For easy enumeration
    int  GetLineCount() const                           { return lines.GetCount() + saved.GetCount(); }
    const Line& GetLine(int i) const;

    const Line* begin() const                           { return lines.begin(); }
    Line*       begin()                                 { return lines.begin(); }
    const Line* end() const                             { return lines.end();   }
    Line*       end()                                   { return lines.end();   }

    virtual void Serialize(Stream& s);

    VTPage();
    VTPage(Size sz) : VTPage()                          { SetSize(sz);       }
    virtual ~VTPage()                                   {}

private:
    VTPage& MoveHorz(int col, bool rel = false, bool scrl = false);
    VTPage& MoveVert(int row, bool rel = false, bool scrl = false);
    void    Insert(Element t, int pos, int n, bool pan = false);
    void    Remove(Element t, int pos, int n, bool pan = false);
    void    LineFill(int pos, int b, int e, const VTCell& filler, dword flags = 0);
    void    RectFill(const Rect& r, const VTCell& filler, dword flags = 0);
    void    Sync();
    void    SyncPageWithHistory();
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
    bool    scrollback;
    VTCell  cellattrs;
};
}
#endif
