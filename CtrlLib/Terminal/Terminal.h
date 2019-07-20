#ifndef _Terminal_Terminal_h
#define _Terminal_Terminal_h

#include <CtrlLib/CtrlLib.h>

#include "Console.h"

namespace Upp {

class Terminal : public Console, public Ctrl {
public:
    enum TimerIds {
        TIMEID_REFRESH = Ctrl::TIMEID_COUNT,
        TIMEID_SIZEHINT,
        TIMEID_BLINK,
        TIMEID_COUNT
    };

    typedef Terminal CLASSNAME;

    Terminal();
    virtual ~Terminal()                                         {}

    Event<>     WhenResize;
    Event<Bar&> WhenBar;

    Terminal&   History(bool b = true)                          { GetDefaultPage().History(b); return *this; }
    Terminal&   NoHistory()                                     { return History(false); }
    Terminal&   ClearHistory()                                  { GetDefaultPage().EraseHistory(); return *this; }
    Terminal&   SetHistorySize(int sz)                          { GetDefaultPage().SetHistorySize(sz); return *this; }

    void        SetCharset(byte chrset);
    byte        GetCharset() const                              { return charset; }

    Terminal&   SetFont(Font f)                                 { font = f; Layout(); return *this; }
    Font        GetFont() const                                 { return font; }

    Terminal&   Ink(Color c)                                    { SetRefreshColor(COLOR_INK, c);   return *this; }
    Terminal&   Paper(Color c)                                  { SetRefreshColor(COLOR_PAPER, c); return *this; }
    Terminal&   SelectionInk(Color c)                           { SetRefreshColor(COLOR_INK_SELECTED, c); return *this; }
    Terminal&   SelectionPaper(Color c)                         { SetRefreshColor(COLOR_PAPER_SELECTED, c); return *this; }

    Terminal&   SetColor(int i, Color c)                        { colortable[i] = c; return *this; }
    void        SetRefreshColor(int i, Color c)                 { SetColor(i, c); Refresh(); }
    Color       GetColor(int i) const                           { return colortable[i]; }
    Terminal&   LightColors(bool b = true)                      { lightcolors = b; Refresh(); return *this; }
    Terminal&   NoLightColors()                                 { return LightColors(false); }
    Terminal&   AdjustColors(bool b = true)                     { adjustcolors = b; Refresh(); return *this; }
    Terminal&   NoAdjustColors()                                { return AdjustColors(false); }
    Terminal&   ResetColors();

    Terminal&   IntensifyBoldText(bool b = true)                { intensify = b; Refresh(); return *this; }
    Terminal&   NoIntensifyBoldText()                           { return IntensifyBoldText(false); }

    Terminal&   BlinkingText(bool b = true)                     { blinktext = b; RefreshDisplay(); return *this; }
    Terminal&   NoBlinkingText()                                { return BlinkingText(false); }
    Terminal&   BlinkInterval(int ms)                           { blinkinterval = clamp(ms, 100, 60000); return *this; }

    Terminal&   BlockCursor(bool blink = true)                  { caret.Block(blink); return *this; }
    Terminal&   BeamCursor(bool blink = true)                   { caret.Beam(blink);  return *this; }
    Terminal&   UnderlineCursor(bool blink = true)              { caret.Underline(blink); return *this; }
    Terminal&   LockCursor(bool b = true)                       { caret.Lock(b);  return *this; }
    Terminal&   UnlockCursor()                                  { caret.Unlock(); return *this; }

    Terminal&   NoBackground(bool b = true)                     { nobackground = b; Transparent(b); Refresh(); return *this; }

    Terminal&   ShowSizeHint(bool b = true)                     { sizehint = b; return *this; };
    Terminal&   HideSizeHint()                                  { return ShowSizeHint(false); }

    Terminal&   ShowScrollBar(bool b = true);
    Terminal&   HideScrollBar()                                 { return ShowScrollBar(false);  }
    Terminal&   SetScrollBarStyle(const ScrollBar::Style& s)    { sb.SetStyle(s); return *this; }

    Terminal&   AlternateScroll(bool b = true)                  { alternatescroll = b; return *this; }
    Terminal&   NoAlternateScroll()                             { return AlternateScroll(false); }

    Terminal&   MouseWheelStep(int lines)                       { wheelstep = max(1, lines); return *this; }

    Terminal&   KeyNavigation(bool b = true)                    { keynavigation = b; return *this; }
    Terminal&   NoKeyNavigation()                               { return KeyNavigation(); }

    Terminal&   DelayedRefresh(bool b = true)                   { delayed = b; return *this;    }
    Terminal&   NoDelayedRefresh()                              { return DelayedRefresh(false); }

    Terminal&   LazyResize(bool b = true)                       { lazyresize = b; return *this; }
    Terminal&   NoLazyResize()                                  { return LazyResize(false);     }

    Size        GetFontSize() const;
    Size        GetPageSize() const;

    Size        GetMinSize() const override                     { return AddFrameSize(Size(2, 2) * GetFontSize());   }
    Size        GetStdSize() const override                     { return AddFrameSize(Size(80, 24) * GetFontSize()); }
    Size        GetMaxSize() const override                     { return AddFrameSize(Size(132, 24) * GetFontSize());}

    void        Copy();
    void        Paste();
    void        SelectAll(bool history = false);

    void        StdBar(Bar& menu);

    void        Layout() override;
    void        Paint(Draw& w)  override;

    bool         Key(dword key, int count) override;
    virtual bool VTKey(dword key, int count);
    virtual bool UDKey(dword key, int count);
    virtual bool NavKey(dword key, int count);

    void        LeftDown(Point p, dword keyflags) override;
    void        LeftUp(Point p, dword keyflags) override;
    void        MiddleDown(Point p, dword keyflags) override;
    void        MiddleUp(Point p, dword keyflags) override;
    void        RightDown(Point p, dword keyflags) override;
    void        RightUp(Point p, dword keyflags) override;
    void        MouseMove(Point p, dword keyflags) override;
    void        MouseWheel(Point p, int zdelta, dword keyflags) override;
    void        VTMouseEvent(Point p, dword event, dword keyflags, int zdelta = 0);

    bool        IsTrackingEnabled() const;

    void        DragAndDrop(Point p, PasteClip& d) override;

    void        GotFocus() override;
    void        LostFocus() override;

    void        RefreshDisplay();

    Image       CursorImage(Point p, dword keyflags) override;

    void        State(int reason) override;

    void        Serialize(Stream& s) override;

private:
    void        PreParse() override;
    void        PostParse() override;

    void        SyncPage(bool notify = true);
    void        SwapPage() override;
    void        RefreshPage(bool full = false) override;

    void        DoDelayedRefresh();

    void        Blink(bool b);
    void        RefreshBlinkingText();

    Color       GetColorFromIndex(const VTCell& cell, int which) const;

    void        Paste(PasteClip& d);

    void        Scroll();
    void        SyncSb();

    void        SyncSize(bool notify = true);
    void        DoLazyResize();

    void        HintNewSize();
    void        RefreshSizeHint();
    Tuple<String, Rect> GetSizeHint(Rect r, Size sz);

    Rect        GetCaretRect();
    void        PlaceCaret(bool scroll = false);

    int         GetSbPos()                                      { return IsAlternatePage() ? 0 : sb; }

    Point       GetCursorPos()                                  { return --page->GetPos(); /* VT cursor position is 1-based */ }

    Point       GetMousePos(Point p);
    int         GetMouseSelPos(Point p);

    void        SetSelection(int l, int h);
    bool        GetSelection(int& l, int& h);
    void        ClearSelection();
    void        ReCalcSelection();
    bool        IsSelected(int pos);
    Rect        GetSelectionRect();
    String      GetSelectedText();

private:
    VScrollBar  sb;
    Scroller    scroller;
    Font        font            = Monospace();
    byte        charset;
    Rect        caretrect;
    int         anchor          = -1;
    int         selpos          = -1;
    bool        selection       = false;
    bool        delayed         = true;
    bool        resizing        = false;
    bool        lazyresize      = false;
    bool        sizehint        = true;
    bool        hinting         = false;
    bool        adjustcolors    = true;
    bool        lightcolors     = false;
    bool        intensify       = false;
    bool        nobackground    = false;
    bool        keynavigation   = true;
    bool        alternatescroll = false;
    bool        ignorescroll    = false;
    bool        userdefinedkeys = true;
    bool        blinktext       = true;
    bool        blinking        = false;
    int         blinkinterval   = 500;
    int         wheelstep       = GUI_WheelScrollLines();
};
}
#endif