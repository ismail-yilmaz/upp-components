#ifndef _VTPage_h_
#define _VTPage_h_

#include <Core/Core.h>
#include "Cell.h"

namespace Upp {

class VTLine : public Moveable<VTLine, Vector<VTCell>> {
public:
    VTLine();
    void            Adjust(int cx, const VTCell& filler);
    void            ShiftLeft(int begin, int end, int n, const VTCell& filler);
    void            ShiftRight(int begin, int end, int n, const VTCell& filler);
    bool            Fill(int begin, int end, const VTCell& filler, dword flags = 0);

    void            Validate(bool b = true)  const          { invalid = !b;    }
    void            Invalidate() const                      { invalid = true;  }
    bool            IsInvalid() const                       { return invalid;  }

    void            Wrap(bool b = true) const               { wrapped = b;     }
    void            Unwrap() const                          { wrapped = false; }
    bool            IsWrapped() const                       { return wrapped;  }

    static const VTLine& Void();
    bool IsVoid() const                                     { return this == &Void(); }
    
    String          ToString() const;
    WString         ToWString() const;

    using Range = SubRangeOf<Vector<VTCell>>;
    using ConstRange = const SubRangeOf<const Vector<VTCell>>;

private:
    mutable bool invalid:1;
    mutable bool wrapped:1;
};

WString AsWString(VTLine::ConstRange& cellrange, bool tspaces = true);

class VTPage : Moveable<VTPage> {
    struct Cursor
    {
        enum Flags : dword {
            Absolute        = 1 << 0,
            Relative        = 1 << 1,
            Marginal        = 1 << 2,
            Displaceable    = 1 << 3,
            Scroller        = 1 << 4,
            ReWrapper       = 1 << 5
        };

        int      x, y;
        bool     eol;
        bool     displaced;
        String   ToString() const;
        void     Clear()                                    { x = y = 1; eol = displaced = false; }
        operator Point() const                              { return Point(x, y); }
        void     operator=(const Point& p)                  { x = p.x; y = p.y;   }
        Point    operator+(int n) const                     { return Point(x + n, y + n); }
        Point    operator-(int n) const                     { return Point(x - n, y - n); }
        Cursor&  operator++()                               { x++; y++; return *this; }
        Cursor&  operator--()                               { x--; y--; return *this; }
        void     operator=(const Nuller&)                   { Clear(); }
        Cursor()                                            { Clear(); }
        Cursor(int col, int row)                            { Clear(); x = max(1, col); y = max(1, row); }
    };

public:
    using Lines = Vector<VTLine>;
    using Saved = BiVector<VTLine>;
    
    VTPage();
    VTPage(Size sz) : VTPage()                              { SetSize(sz); }
    virtual ~VTPage()                                       {}

    Event<>         WhenUpdate;

    VTPage&         Displaced(bool b = true);
    bool            IsDisplaced() const                     { return cursor.displaced; }

    VTPage&         AutoWrap(bool b = true);
    bool            IsAutoWrapping() const                  { return autowrap; }

    VTPage&         ReverseWrap(bool b = true);
    bool            IsReverseWrapping() const               { return reversewrap; }

    VTPage&         History(bool b = true);
    bool            HasHistory() const                      { return history; }
    const Saved&    GetHistory() const                      { return saved;   }
    void            EraseHistory();
    void            SetHistorySize(int sz);
    int             GetHistorySize() const                  { return historysize; };

    VTPage&         Attributes(const VTCell& attrs)         { cellattrs = attrs; return *this; }
    const VTCell&   GetAttributes() const                   { return cellattrs; }

    VTPage&         Reset();
    
    VTPage&         Backup();
    VTPage&         Discard();
    VTPage&         Restore();
    
    VTPage&         SetSize(Size sz);
    VTPage&         SetSize(int cx, int cy)                 { return SetSize(Size(cx, cy)); }
    Size            GetSize() const                         { return size; }

    Point           GetPos() const;
    Point           GetRelPos() const;

    Rect            GetView() const                         { return Rect(1, 1, size.cx, size.cy); }
    bool            ViewContains(const Point& pt) const;
    bool            ViewContains(const Rect& r) const       { return ViewContains(r.TopLeft()) && ViewContains(r.BottomRight()); }

    Rect            GetMargins() const                      { return margins; }
    bool            MarginsContain(const Point& pt) const;
    bool            MarginsContain(const Rect& r) const     { return MarginsContain(r.TopLeft()) && MarginsContain(r.BottomRight()); }

    VTPage&         SetHorzMargins(int l, int r);
    VTPage&         SetVertMargins(int t, int b);
    VTPage&         SetMargins(const Rect& r);
    VTPage&         ResetMargins();

    VTPage&         SetCell(int x, int y, const VTCell& cell);
    VTPage&         SetCell(Point pt, const VTCell& cell)   { return SetCell(pt.x, pt.y, cell); }
    VTPage&         SetCell(const VTCell& cell)             { return SetCell(cursor, cell); }
    const VTCell&   GetCell(int x, int y) const;
    const VTCell&   GetCell(Point pt) const                 { return GetCell(pt.x, pt.y);   }
    const VTCell&   GetCell() const                         { return GetCell(cursor);       }
    int             AddCell(const VTCell& cell)             { return CellAdd(cell, cell.GetWidth()); }
    VTPage&         InsertCell(const VTCell& cell);
    VTPage&         RepeatCell(int n);

    VTPage&         MoveTo(int x, int y);
    VTPage&         MoveTo(Point pt)                        { return MoveTo(pt.x, pt.y); }
    VTPage&         MoveToLine(int n, bool relative = false);
    VTPage&         MoveToColumn(int n, bool relative = false);

    VTPage&         MoveUp(int n = 1);
    VTPage&         MoveDown(int n = 1);
    VTPage&         MoveRight(int n = 1);
    VTPage&         MoveLeft(int n = 1);
    VTPage&         MoveHome();
    VTPage&         MoveEnd();
    VTPage&         MoveTopLeft();
    VTPage&         MoveBottomRight();
    
    VTPage&         EraseLine(dword flags = 0)              { LineFill(cursor.y, 1, size.cx, cellattrs, flags); return *this;  }
    VTPage&         EraseLeft(dword flags = 0)              { LineFill(cursor.y, 1, cursor.x, cellattrs, flags); return *this; }
    VTPage&         EraseRight(dword flags = 0)             { LineFill(cursor.y, cursor.x, size.cx, cellattrs, flags); return *this; }

    VTPage&         ErasePage(dword flags = 0)              { RectFill(GetView(), cellattrs, flags); return *this; }
    VTPage&         EraseBefore(dword flags = 0)            { RectFill(Rect(1, 1, size.cx, cursor.y - 1), cellattrs, flags); return EraseLeft(flags); }
    VTPage&         EraseAfter(dword flags = 0)             { RectFill(Rect(1, cursor.y + 1, size.cx, size.cy), cellattrs, flags); return EraseRight(flags); }

    VTPage&         ScrollUp(int n = 1);
    VTPage&         ScrollDown(int n = 1);
    VTPage&         ScrollLeft(int n = 1);
    VTPage&         ScrollRight(int n = 1);
    
    VTPage&         PanLeft(int n = 1);
    VTPage&         PanRight(int n = 1);

    VTPage&         InsertLines(int pos, int n);
    VTPage&         InsertLines(int n)                      { return InsertLines(cursor.y, n); }
    VTPage&         RemoveLines(int pos, int n);
    VTPage&         RemoveLines(int n)                      { return RemoveLines(cursor.y, n); }

    VTPage&         InsertCells(int pos, int n);
    VTPage&         InsertCells(int n)                      { return InsertCells(cursor.x, n); }
    VTPage&         RemoveCells(int pos, int n);
    VTPage&         RemoveCells(int n)                      { return RemoveCells(cursor.x, n); }
    VTPage&         EraseCells(int n, dword flags = 0);

    VTPage&         NextLine(int n = 1);
    VTPage&         PrevLine(int n = 1);
    VTPage&         NewLine(int n = 1)                      { return NextLine(n).MoveHome(); }
    VTPage&         NextColumn(int n = 1);
    VTPage&         PrevColumn(int n = 1);

    VTPage&         FillRect(const Rect& r, const VTCell& filler, dword flags = 0);
    VTPage&         FillRect(const Rect& r, dword chr);
    VTPage&         CopyRect(const Point& pt, const Rect& r, dword flags = 0);
    VTPage&         EraseRect(const Rect& r, dword flags = 0);
    void            GetRectArea(const Rect& r, Event<Point> consumer, bool displaced = false);

    VTPage&         FillStream(const Rect& r, const VTCell& filler, dword flags);
    VTPage&         FillStream(const Rect& r, dword chr);

    VTPage&         AddImage(Size sz, dword imageid, bool scroll, bool relpos = false);

    VTPage&         SetTabAt(int col, bool b = true)         { SetTabStop(col, b); return *this; }
    VTPage&         SetTab(bool b = true)                    { SetTabStop(cursor.x, b); return *this; }
    VTPage&         SetTabs(int tsz);
    void            GetTabs(Vector<int>& tabstops);
    VTPage&         NextTab(int n = 1);
    VTPage&         PrevTab(int n = 1);
    VTPage&         ClearTabs()                              { tabs.Clear(); tabsync = false; return *this; }

    void            SetEol(bool b = true)                    { cursor.eol = b;     }
    void            ClearEol()                               { cursor.eol = false; }
    bool            IsEol() const                            { return cursor.eol;  }

    void            Invalidate()                             { for(auto& line : lines) line.Invalidate(); }
    void            Invalidate(int begin, int end);

    // Index: 0-based.
    const VTLine&   FetchLine(int i) const;
    const VTLine&   operator[](int i) const                  { return FetchLine(i); }
    int             GetLineCount() const                     { return lines.GetCount() + saved.GetCount(); }

    // Point: 0-based.
    const VTCell&   FetchCell(const Point& pt) const;
    const VTCell&   operator()(const Point& pt) const        { return FetchCell(pt);  }

    // Rect: 0-based.
    bool            FetchRange(const Rect& r, Gate<const VTLine&, VTLine::ConstRange&> consumer, bool rect = false) const;

    const VTLine*    begin() const                           { return lines.begin(); }
    VTLine*          begin()                                 { return lines.begin(); }
    const VTLine*    end() const                             { return lines.end();   }
    VTLine*          end()                                   { return lines.end();   }

    virtual void    Serialize(Stream& s);
    virtual void    Jsonize(JsonIO& jio);
    virtual void    Xmlize(XmlIO& xio);

private:
    bool            HorzMarginsExist() const                                        { return margins.Width()  < size.cx - 1; }
    bool            VertMarginsExist() const                                        { return margins.Height() < size.cy - 1; }
    bool            HorzMarginsContain(int col) const                               { return margins.left <= col && col <= margins.right; }
    bool            VertMarginsContain(int row) const                               { return margins.top <= row && row <= margins.bottom; }
    Point           Bind(const Rect& r, const Point& p) const;
    VTPage&         MoveHorz(int pos, dword flags = Cursor::Absolute);
    VTPage&         MoveVert(int pos, dword flags = Cursor::Absolute);
    int             GetNextRowPos(int n, int offset, bool rel = false) const        { return rel ? n + cursor.y : offset + n - 1; }
    int             GetNextColPos(int n, int offset, bool rel = false) const        { return rel ? n + cursor.x : offset + n - 1; }
    int             GetAbsCol(int col) const                                        { return cursor.displaced ? col - margins.left + 1 : col; }
    int             GetAbsRow(int row) const                                        { return cursor.displaced ? row - margins.top + 1 : row;  }
    VTPage&         RewrapCursor(int n);
    int             LineInsert(int pos, int n, const VTCell& attrs);
    int             LineRemove(int pos, int n, const VTCell& attrs);
    int             CellAdd(const VTCell& cell, int width = 1);
    int             CellInsert(int pos, int n, const VTCell& attrs, bool pan);
    int             CellRemove(int pos, int n, const VTCell& attrs, bool pan);
    int             SetTabStop(int col, bool b);
    bool            IsTabStop(int col) const                                        { return tabs[col]; }
    void            AdjustHistorySize();
    bool            SaveToHistory(int pos);
    void            UnwindHistory(const Size& prevsize);
    void            RewindHistory(const Size& prevsize);
    Rect            AdjustRect(const Rect& r, bool displaced = true);
    void            RectFill(const Rect& r, const VTCell& filler, dword flags = 0);
    void            RectCopy(const Point& p, const Rect& r, const Rect& rr, dword flags = 0);
    void            LineFill(int pos, int begin, int end, const VTCell& filler, dword flags = 0);

private:
    Lines           lines;
    Saved           saved;
    Cursor          cursor;
    Cursor          backup;
    Size            size;
    Rect            margins;
    Bits            tabs;
    int             tabsize;
    int             historysize;
    bool            history;
    bool            autowrap;
    bool            reversewrap;
    bool            tabsync;
    VTCell          cellattrs;
};

WString AsWString(const VTPage& page, const Rect& r, bool rectsel = false, bool tspaces = true);

}
#endif
