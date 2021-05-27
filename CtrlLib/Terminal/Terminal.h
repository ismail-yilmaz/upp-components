#ifndef _Terminal_TerminalCtrl_h
#define _Terminal_TerminalCtrl_h

#include <CtrlLib/CtrlLib.h>
#include <plugin/jpg/jpg.h>

#include "Parser.h"
#include "Page.h"
#include "Sixel.h"

namespace Upp {

class TerminalCtrl : public Ctrl {
public:
    const int ANSI_COLOR_COUNT = 16;    // Actually, ANSI + aixterm colors.

    enum Colors
    {
        COLOR_BLACK = 0,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE,
        COLOR_LTBLACK,
        COLOR_LTRED,
        COLOR_LTGREEN,
        COLOR_LTYELLOW,
        COLOR_LTBLUE,
        COLOR_LTMAGENTA,
        COLOR_LTCYAN,
        COLOR_LTWHITE,
        COLOR_INK,
        COLOR_INK_SELECTED,
        COLOR_PAPER,
        COLOR_PAPER_SELECTED,
        MAX_COLOR_COUNT
    };

    enum ConformanceLevels
    {
        LEVEL_0 = 0,
        LEVEL_1,
        LEVEL_2,
        LEVEL_3,
        LEVEL_4
    //  LEVEL_5
    };

    enum LEDs
    {
        LED_NUMLOCK  = 0,
        LED_CAPSLOCK,
        LED_SCRLOCK,
        LED_ALL
    };

   enum TimerIds
   {
        TIMEID_REFRESH = Ctrl::TIMEID_COUNT,
        TIMEID_SIZEHINT,
        TIMEID_BLINK,
        TIMEID_COUNT
    };

    // Inline image data structure.
    struct InlineImage : ValueType<InlineImage, 999, Moveable<InlineImage> > {
        Image       image;
        Size        cellsize;
        Size        fontsize;
        Rect        paintrect;
        operator    Value() const                               { return RichValue<TerminalCtrl::InlineImage>(*this); }
    };

    TerminalCtrl();
    virtual ~TerminalCtrl();

    Event<>              WhenBell;
    Event<>              WhenResize;
    Event<Size>          WhenSetSize;
    Event<Bar&>          WhenBar;
    Event<String>        WhenTitle;
    Event<String>        WhenOutput;
    Event<int, bool>     WhenLED;
    Gate<PasteClip&>     WhenClip;
    Event<const String&> WhenLink;
    Event<const String&> WhenImage;

    // Window Ops support.
    Event<bool>          WhenWindowMinimize;
    Event<bool>          WhenWindowMaximize;
    Event<int>           WhenWindowFullScreen;
    Event<Rect>          WhenWindowGeometryChange;

    // APC support.
    Event<const String&> WhenApplicationCommand;

    void            Write(const void *data, int size, bool utf8 = true);
    void            Write(const String& s, bool utf8 = true)        { Write(~s, s.GetLength(), utf8); }
    void            WriteUtf8(const String& s)                      { Write(s, true);         }

    TerminalCtrl&   Echo(const String& s);
    
    TerminalCtrl&   SetLevel(int level)                             { SetEmulation(level); return *this; }
    bool            IsLevel0() const                                { return !modes[DECANM]; }
    bool            IsLevel1() const                                { return modes[DECANM] && clevel >= LEVEL_1; }
    bool            IsLevel2() const                                { return modes[DECANM] && clevel >= LEVEL_2; }
    bool            IsLevel3() const                                { return modes[DECANM] && clevel >= LEVEL_3; }
    bool            IsLevel4() const                                { return modes[DECANM] && clevel >= LEVEL_4; }

    TerminalCtrl&   Set8BitMode(bool b = true)                      { eightbit = b; return *this; }
    TerminalCtrl&   No8BitMode()                                    { return Set8BitMode(false); }
    bool            Is8BitMode() const                              { return IsLevel2() && eightbit; }
    bool            Is7BitMode() const                              { return !Is8BitMode(); }

    bool            IsUtf8Mode() const                              { return charset == CHARSET_UNICODE && !legacycharsets; }

    void            HardReset()                                     { Reset(true);  }
    void            SoftReset()                                     { Reset(false); }

    TerminalCtrl&   History(bool b = true)                          { dpage.History(b); return *this; }
    TerminalCtrl&   NoHistory()                                     { return History(false); }
    TerminalCtrl&   ClearHistory()                                  { dpage.EraseHistory(); return *this; }
    bool            HasHistory() const                              { return dpage.HasHistory(); }

    TerminalCtrl&   SetHistorySize(int sz)                          { dpage.SetHistorySize(sz); return *this; }
    int             GetHistorySize() const                          { return dpage.GetHistorySize(); }

    TerminalCtrl&   SetFont(Font f);
    Font            GetFont() const                                 { return font; }

    TerminalCtrl&   SetPadding(Size sz);
    Size            GetPadding() const                              { return padding; }

    void            SetCharset(byte cs)                             { charset = ResolveCharset(cs); }
    byte            GetCharset() const                              { return charset;    }

    TerminalCtrl&   Ink(Color c)                                    { SetRefreshColor(COLOR_INK, c); return *this; }
    TerminalCtrl&   Paper(Color c)                                  { SetRefreshColor(COLOR_PAPER, c); return *this; }
    TerminalCtrl&   SelectionInk(Color c)                           { SetRefreshColor(COLOR_INK_SELECTED, c); return *this; }
    TerminalCtrl&   SelectionPaper(Color c)                         { SetRefreshColor(COLOR_PAPER_SELECTED, c); return *this; }

    TerminalCtrl&   SetColor(int i, Color c)                        { colortable[i] = c; return *this; }
    void            SetRefreshColor(int i, Color c)                 { SetColor(i, c); Refresh(); }
    Color           GetColor(int i) const                           { return colortable[i]; }

    TerminalCtrl&   DynamicColors(bool b = true)                    { dynamiccolors = b; return *this; }
    TerminalCtrl&   NoDynamicColors()                               { return DynamicColors(false); }
    bool            HasDynamicColors() const                        { return dynamiccolors; }

    TerminalCtrl&   LightColors(bool b = true)                      { lightcolors = b; Refresh(); return *this; }
    TerminalCtrl&   NoLightColors()                                 { return LightColors(false); }
    bool            HasLightColors() const                          { return lightcolors; }

    TerminalCtrl&   AdjustColors(bool b = true)                     { adjustcolors = b; Refresh(); return *this; }
    TerminalCtrl&   NoAdjustColors()                                { return AdjustColors(false); }
    bool            HasAdjustedColors() const                       { return adjustcolors; }

    TerminalCtrl&   ResetColors();

    TerminalCtrl&   IntensifyBoldText(bool b = true)                { intensify = b; Refresh(); return *this; }
    TerminalCtrl&   NoIntensifyBoldText()                           { return IntensifyBoldText(false); }
    bool            HasIntensifiedBoldText() const                  { return intensify; }

    TerminalCtrl&   BlinkingText(bool b = true)                     { blinkingtext = b; RefreshDisplay(); return *this; }
    TerminalCtrl&   NoBlinkingText()                                { return BlinkingText(false); }
    bool            HasBlinkingText() const                         { return blinkingtext; }

    TerminalCtrl&   BlinkInterval(int ms)                           { blinkinterval = clamp(ms, 100, 60000); return *this; }

    TerminalCtrl&   SetCursorStyle(int style, bool blink = true)    { caret.Set(style, blink); return *this;}
    int             GetCursorStyle() const                          { return caret.GetStyle(); }
    TerminalCtrl&   BlockCursor(bool blink = true)                  { caret.Block(blink); return *this; }
    TerminalCtrl&   BeamCursor(bool blink = true)                   { caret.Beam(blink);  return *this; }
    TerminalCtrl&   UnderlineCursor(bool blink = true)              { caret.Underline(blink); return *this; }
    TerminalCtrl&   BlinkingCursor(bool b = true)                   { caret.Blink(b); return *this; }
    TerminalCtrl&   NoBlinkingCursor()                              { return BlinkingCursor(false); }
    bool            IsCursorBlinking() const                        { return caret.IsBlinking();    }
    TerminalCtrl&   LockCursor(bool b = true)                       { caret.Lock(b);  return *this; }
    TerminalCtrl&   UnlockCursor()                                  { caret.Unlock(); return *this; }
    bool            IsCursorLocked() const                          { return caret.IsLocked();      }
    Point           GetCursorPoint() const;

    TerminalCtrl&   NoBackground(bool b = true)                     { nobackground = b; Transparent(b); Refresh(); return *this; }
    bool            HasBackground() const                           { return !nobackground; }

    TerminalCtrl&   ShowSizeHint(bool b = true)                     { sizehint = b; return *this; }
    TerminalCtrl&   HideSizeHint()                                  { return ShowSizeHint(false); }
    bool            HasSizeHint() const                             { return sizehint; }
    
    TerminalCtrl&   ShowScrollBar(bool b = true);
    TerminalCtrl&   HideScrollBar()                                 { return ShowScrollBar(false);  }
    bool            HasScrollBar() const                            { return sb.IsChild();          }
    TerminalCtrl&   SetScrollBarStyle(const ScrollBar::Style& s)    { sb.SetStyle(s); return *this; }

    TerminalCtrl&   AlternateScroll(bool b = true)                  { alternatescroll = b; return *this; }
    TerminalCtrl&   NoAlternateScroll()                             { return AlternateScroll(false); }
    bool            HasAlternateScroll() const                      { return alternatescroll; }

    TerminalCtrl&   MouseWheelStep(int lines)                       { wheelstep = max(1, lines); return *this; }
    int             GetMouseWheelStep() const                       { return wheelstep; }

    TerminalCtrl&   AutoHideMouseCursor(bool b = true)              { hidemousecursor = b; return *this; }
    TerminalCtrl&   NoAutoHideMouseCurosr()                         { return AutoHideMouseCursor(false); }
    bool            IsMouseCursorAutoHidden() const                 { return hidemousecursor; }

    TerminalCtrl&   KeyNavigation(bool b = true)                    { keynavigation = b; return *this; }
    TerminalCtrl&   NoKeyNavigation()                               { return KeyNavigation(false); }
    bool            HasKeyNavigation() const                        { return keynavigation; }

    TerminalCtrl&   InlineImages(bool b = true)                     { sixelimages = jexerimages = iterm2images = b; return *this; }
    TerminalCtrl&   NoInlineImages()                                { return InlineImages(false);  }
    bool            HasInlineImages() const                         { return sixelimages || jexerimages || iterm2images; }

    TerminalCtrl&   SixelGraphics(bool b = true)                    { sixelimages = b; return *this; }
    TerminalCtrl&   NoSixelGraphics()                               { return SixelGraphics(false); }
    bool            HasSixelGraphics() const                        { return sixelimages; }

    TerminalCtrl&   JexerGraphics(bool b = true)                    { jexerimages = b; return *this; }
    TerminalCtrl&   NoJexerGraphics()                               { return JexerGraphics(false); }
    bool            HasJexerGraphics() const                        { return jexerimages; }

    TerminalCtrl&   iTerm2Graphics(bool b = true)                   { iterm2images = b; return *this; }
    TerminalCtrl&   NoiTerm2Graphics(bool b = true)                 { return iTerm2Graphics(false); }
    bool            HasiTerm2Graphics() const                       { return iterm2images; }

    TerminalCtrl&   Hyperlinks(bool b = true)                       { hyperlinks = b; return *this; }
    TerminalCtrl&   NoHyperlinks()                                  { return Hyperlinks(false);     }
    bool            HasHyperlinks() const                           { return hyperlinks; }

    TerminalCtrl&   ReverseWrap(bool b = true)                      { XTrewrapm((reversewrap = b)); return *this; }
    TerminalCtrl&   NoReverseWrap()                                 { return ReverseWrap(false); }
    bool            HasReverseWrap() const                          { return reversewrap; }

    TerminalCtrl&   DelayedRefresh(bool b = true)                   { delayedrefresh = b; return *this; }
    TerminalCtrl&   NoDelayedRefresh()                              { return DelayedRefresh(false); }
    bool            IsDelayingRefresh() const                       { return delayedrefresh; }

    TerminalCtrl&   LazyResize(bool b = true)                       { lazyresize = b; return *this; }
    TerminalCtrl&   NoLazyResize()                                  { return LazyResize(false);     }
    bool            IsLazyResizing() const                          { return lazyresize; }

    TerminalCtrl&   WindowOps(bool b = true)                        { windowactions = windowreports = b; return *this; }
    TerminalCtrl&   NoWindowOps()                                   { return WindowOps(false);      }
    bool            HasWindowOps() const                            { return windowactions || windowreports; }

    TerminalCtrl&   WindowReports(bool b = true)                    { windowreports = b; return *this; }
    TerminalCtrl&   NoWindowReports()                               { return WindowReports(false);  }
    bool            HasWindowReports() const                        { return windowreports; }

    TerminalCtrl&   WindowActions(bool b = true)                    { windowactions = b; return *this; }
    TerminalCtrl&   NoWindowActions()                               { return WindowActions(false);  }
    bool            HasWindowActions() const                        { return windowactions; }

    TerminalCtrl&   PermitClipboardAccess(bool b = true)            { return PermitClipboardRead(b).PermitClipboardWrite(b); }
    TerminalCtrl&   ForbidClipboardAccess()                         { clipaccess = CLIP_NONE;  return *this; }
    bool            IsClipboardAccessPermitted() const              { return clipaccess != CLIP_NONE; }

    TerminalCtrl&   PermitClipboardRead(bool b = true)              { if(b) clipaccess |= CLIP_READ; else clipaccess &= ~CLIP_READ; return *this; }
    TerminalCtrl&   ForbidClipboardRead()                           { return PermitClipboardRead(false); }
    bool            IsClipboardReadPermitted() const                { return clipaccess & CLIP_READ; }

    TerminalCtrl&   PermitClipboardWrite(bool b = true)             { if(b) clipaccess |= CLIP_WRITE; else clipaccess &= ~CLIP_WRITE; return *this; }
    TerminalCtrl&   ForbidClipboardWrite()                          { return PermitClipboardWrite(false); }
    bool            IsClipboardWritePermitted() const               { return clipaccess & CLIP_WRITE; }
 
    TerminalCtrl&   PCStyleFunctionKeys(bool b = true)              { pcstylefunctionkeys = b; return *this; }
    TerminalCtrl&   NoPCStyleFunctionKeys()                         { return PCStyleFunctionKeys(false); }
    bool            HasPCStyleFunctionKeys() const                  { return pcstylefunctionkeys; }
    
    TerminalCtrl&   SetImageDisplay(const Display& d)               { imgdisplay = &d; return *this; }
    const Display&  GetImageDisplay() const                         { return *imgdisplay; }

    TerminalCtrl&   UDK(bool b = true)                              { userdefinedkeys = b; return *this;  }
    TerminalCtrl&   NoUDK()                                         { return UDK(false);     }
    bool            HasUDK() const                                  { return userdefinedkeys; }
    TerminalCtrl&   LockUDK(bool b = true)                          { userdefinedkeyslocked = b;  return *this; }
    TerminalCtrl&   UnlockUDK()                                     { return LockUDK(false); }
    bool            IsUDKLocked() const                             { return userdefinedkeyslocked; }

    Size            GetFontSize() const                             { return Size(max(font.GetWidth('M'), font.GetWidth('W')), font.GetCy()); }
    Size            GetCellSize() const                             { return GetFontSize() + padding * 2; }
    Size            GetPageSize() const                             { Size csz = GetCellSize(); return clamp(GetSize() / csz, Size(1, 1), GetScreenSize() / csz); }

    Size            PageSizeToClient(Size sz) const                 { return AddFrameSize(sz * GetCellSize()); }
    Size            PageSizeToClient(int col, int row) const        { return PageSizeToClient(Size(col, row)); }

    Size            GetMinSize() const override                     { return PageSizeToClient(Size(2, 2)); }
    Size            GetStdSize() const override                     { return PageSizeToClient(Size(80, 24)); }
    Size            GetMaxSize() const override                     { return PageSizeToClient(Size(132, 24)); }

    void            Copy()                                          { Copy(GetSelectedText()); }
    void            Copy(const WString& s);
    void            Paste()                                         { DragAndDrop(Null, Clipboard()); }
    void            Paste(const WString& s, bool filter = false);
    void            SelectAll(bool history = false);
    bool            IsSelection() const                             { return !IsNull(anchor) && anchor != selpos && seltype != SEL_NONE; }

    String          GetSelectionData(const String& fmt) const override;
    
    void            StdBar(Bar& menu);
    void            EditBar(Bar& menu);
    void            LinksBar(Bar& menu);
    void            ImagesBar(Bar& menu);
    void            OptionsBar(Bar& menu);

    void            Layout() override                               { SyncSize(true); SyncSb(); }

    void            Paint(Draw& w)  override                        { Paint0(w); }
    void            PaintPage(Draw& w)                              { Paint0(w, true); }

    bool            Key(dword key, int count) override;
    virtual bool    VTKey(dword key, int count);
    virtual bool    UDKey(dword key, int count);
    virtual bool    NavKey(dword key, int count);

    TerminalCtrl&   MetaEscapesKeys(bool b = true)                  { if(b) metakeyflags |= MKEY_ESCAPE; else metakeyflags &= ~MKEY_ESCAPE; return *this; }
    TerminalCtrl&   MetaShiftsKeys(bool b = true)                   { if(b) metakeyflags |= MKEY_SHIFT;  else metakeyflags &= ~MKEY_SHIFT;  return *this; }
    TerminalCtrl&   MetaKeyDoesNothing()                            { modes.Set(XTALTESCM, false); metakeyflags = MKEY_NONE; return *this; }

    void            LeftDown(Point pt, dword keyflags) override;
    void            LeftUp(Point pt, dword keyflags) override;
    void            LeftDrag(Point pt, dword keyflags) override;
    void            LeftDouble(Point pt, dword keyflags) override;
    void            LeftTriple(Point pt, dword keyflags) override;
    void            MiddleDown(Point pt, dword keyflags) override;
    void            MiddleUp(Point pt, dword keyflags) override;
    void            RightDown(Point pt, dword keyflags) override;
    void            RightUp(Point pt, dword keyflags) override;
    void            MouseMove(Point pt, dword keyflags) override;
    void            MouseWheel(Point pt, int zdelta, dword keyflags) override;
    Image           MouseEvent(int event, Point pt, int zdelta, dword keyflags) override;
    void            VTMouseEvent(Point pt, dword event, dword keyflags, int zdelta = 0);

    bool            IsMouseOverImage() const                        { Point pt = GetMouseViewPos(); return IsMouseOverImage(ClientToPagePos(pt)); }
    bool            IsMouseOverHyperlink() const                    { Point pt = GetMouseViewPos(); return IsMouseOverHyperlink(ClientToPagePos(pt)); }

    bool            IsTracking() const;

    const VTCell&   GetCellAtMousePos() const                       { Point pt = GetMouseViewPos(); return page->FetchCell(ClientToPagePos(pt));; }
    const VTCell&   GetCellAtCursorPos() const                      { return page->GetCell(); };

    String          GetHyperlinkUri()                               { return GetHyperlinkURI(mousepos, true); }
    Image           GetInlineImage()                                { return GetInlineImage(mousepos, true);  }

    void            DragAndDrop(Point pt, PasteClip& d) override;

    void            GotFocus() override                             { if(modes[XTFOCUSM]) PutCSI('I'); Refresh(); }
    void            LostFocus() override                            { if(modes[XTFOCUSM]) PutCSI('O'); Refresh(); }

    void            RefreshDisplay();

    Image           CursorImage(Point p, dword keyflags) override;

    void            AnswerBackMessage(const String& s)              { answerback = s; }

    void            State(int reason) override;

    void            Serialize(Stream& s) override;
    void            Jsonize(JsonIO& jio) override;
    void            Xmlize(XmlIO& xio) override;

    static void     ClearImageCache();
    static void     SetImageCacheMaxSize(int maxsize, int maxcount);

    static void     ClearHyperlinkCache();
    static void     SetHyperlinkCacheMaxSize(int maxcount);

private:
    void        InitParser(VTInStream& vts);
    
    void        PreParse()                                      { /*ScheduleRefresh();*/ }
    void        PostParse()                                     { ScheduleRefresh(); }

    void        SyncPage(bool notify = true);
    void        SwapPage();

    void        ScheduleRefresh();

    void        Blink(bool b);

    void        Scroll();
    void        SyncSb();

    void        SyncSize(bool notify = true);

    Tuple<String, Rect> GetSizeHint(Rect r, Size sz);
    void        RefreshSizeHint()                               { Refresh(GetSizeHint(GetView(), GetPageSize()).b.Inflated(8)); }

    Rect        GetCaretRect();
    void        PlaceCaret(bool scroll = false);

    int         GetSbPos() const                                { return IsAlternatePage() ? 0 : sb; }

    Point       GetCursorPos() const                            { return --page->GetPos(); /* VT cursor position is 1-based */ }

    Point       ClientToPagePos(Point pt) const;
    Point       SelectionToPagePos(Point pt) const;

    void        SetSelection(Point  pl, Point ph, dword selflag);
    bool        GetSelection(Point& pl, Point& ph) const;
    Rect        GetSelectionRect() const;
    void        ClearSelection();
    bool        IsSelected(Point pt) const;
    WString     GetSelectedText() const;
    void        GetLineSelection(const Point& pt, Point& pl, Point& ph) const;
    bool        GetWordSelection(const Point& pt, Point& pl, Point& ph) const;
    void        GetWordPosL(const VTLine& line, Point& pl) const;
    void        GetWordPosH(const VTLine& line, Point& ph) const;

    bool        IsMouseOverImage(Point pt) const                { return !IsSelected(pt) && page->FetchCell(pt).IsImage(); }
    bool        IsMouseOverHyperlink(Point pt) const            { return !IsSelected(pt) && page->FetchCell(pt).IsHyperlink(); }

    void        HighlightHyperlink(Point pt);

    String      GetHyperlinkURI(Point pt, bool modifier);
    Image       GetInlineImage(Point pt, bool modifier);

private:
    using       ImagePart  = Tuple<dword, Point, Rect>;
    using       ImageParts = Vector<ImagePart>;

    struct ImageString : Moveable<ImageString> {
        String  data;
        Size    size;
        bool    encoded:1;
        bool    keepratio:1;
        dword   GetHashValue() const                            { return FoldHash(CombineHash(data, size, encoded, keepratio)); }
        void    SetNull()                                       { data = Null; size = Null; encoded = keepratio = true; }
        bool    IsNullInstance() const                          { return Upp::IsNull(data); }
        ImageString()                                           { SetNull(); }
        ImageString(const Nuller&)                              { SetNull(); }
        ImageString(String&& s)                                 { SetNull(); data = s;  }
    };

    struct InlineImageMaker : LRUCache<InlineImage>::Maker {
        dword   id;
        const   Size& fontsize;
        const   ImageString& imgs;
        String  Key() const override;
        int     Make(InlineImage& imagedata) const override;
        InlineImageMaker(int i, const ImageString& s, const Size& sz)
        : id(i)
        , imgs(s)
        , fontsize(sz)
        {
        }
    };

    struct HyperlinkMaker : LRUCache<String>::Maker {
        dword   id;
        const   String& url;
        String  Key() const override;
        int     Make(String& link) const override;
        HyperlinkMaker(int i, const String& s)
        : id(i)
        , url(s)
        {
        }
    };

    void        Paint0(Draw& w, bool print = false);
    void        PaintImages(Draw& w, ImageParts& parts, const Size& csz);

    void        RenderImage(const ImageString& simg, bool scroll);
    const InlineImage& GetCachedImageData(dword id, const ImageString& simg, const Size& csz);

    void        RenderHyperlink(const String& uri);
    String      GetCachedHyperlink(dword id, const String& data = Null);

private:
    enum TextSelectionTypes : dword {
        SEL_NONE    = 0,
        SEL_TEXT    = 1,
        SEL_RECT    = 2,
        SEL_WORD    = 3,
        SEL_LINE    = 4
    };
    
    enum ModifierKeyFlags : dword {
        MKEY_NONE   = 0,
        MKEY_ESCAPE = 1,
        MKEY_SHIFT  = 2
    };

    enum ClipboardAccessFlags : dword {
        CLIP_NONE   = 0,
        CLIP_READ   = 1,
        CLIP_WRITE  = 2
    };

    const Display *imgdisplay;
    VScrollBar  sb;
    Scroller    scroller;
    Point       mousepos;
    Font        font            = Monospace();
    byte        charset;
    Rect        caretrect;
    Point       anchor          = Null;
    Point       selpos          = Null;
    dword       seltype         = SEL_NONE;
    bool        multiclick      = false;
    bool        ignorescroll    = false;
    bool        mousehidden     = false;
    bool        resizing        = false;
    bool        hinting         = false;
    bool        blinking        = false;
    int         blinkinterval   = 500;
    int         wheelstep       = GUI_WheelScrollLines();
    int         metakeyflags    = MKEY_ESCAPE;
    int         clipaccess      = CLIP_NONE;
    dword       activelink      = 0;
    dword       prevlink        = 0;
    Size        padding         = { 0, 0 };

    bool        eightbit;
    bool        reversewrap;
    bool        keynavigation;
    bool        legacycharsets;
    bool        alternatescroll;
    bool        pcstylefunctionkeys;
    bool        userdefinedkeys;
    bool        userdefinedkeyslocked;
    bool        windowactions;
    bool        windowreports;
    bool        sixelimages;
    bool        jexerimages;
    bool        iterm2images;
    bool        hyperlinks;
    bool        delayedrefresh;
    bool        lazyresize;
    bool        sizehint;
    bool        nobackground;
    bool        intensify;
    bool        blinkingtext;
    bool        dynamiccolors;
    bool        adjustcolors;
    bool        lightcolors;
    bool        hidemousecursor;

// Down beloe is the emulator stuff, formerley knonw as "Console"...

private:
    VTPage*     page;

private:
    Color       GetColorFromIndex(const VTCell& cell, int which) const;
    void        SetInkAndPaperColor(const VTCell& cell, Color& ink, Color& paper);
    void        ReportANSIColor(int opcode, int index, const Color& c);
    void        ReportDynamicColor(int opcode, const Color& c);
    void        SetProgrammableColors(const VTInStream::Sequence& seq, int opcode);
    void        ResetProgrammableColors(const VTInStream::Sequence& seq, int opcode);
    bool        SetSaveColor(int index, const Color& c);
    bool        ResetLoadColor(int index);
    void        ParseExtendedColors(VTCell& attrs, const Vector<String>& opcodes, int& index);

    VectorMap<int, Color> savedcolors;
    Color       colortable[MAX_COLOR_COUNT];

    struct ColorTableSerializer {
        Color   *table;
        void    Serialize(Stream& s);
        void    Jsonize(JsonIO& jio);
        void    Xmlize(XmlIO& xio);
        ColorTableSerializer(Color *ct) : table(ct) {}
    };

private:
    void        PutChar(int c);
    int         LookupChar(int c);

    void        ParseControlChars(byte c)                                               { DispatchCtl(c); }
    void        ParseEscapeSequences(const VTInStream::Sequence& seq);
    void        ParseCommandSequences(const VTInStream::Sequence& seq);
    void        ParseDeviceControlStrings(const VTInStream::Sequence& seq);
    void        ParseOperatingSystemCommands(const VTInStream::Sequence& seq);
    void        ParseApplicationProgrammingCommands(const VTInStream::Sequence& seq)    { WhenApplicationCommand(seq.payload); }

    bool        Convert7BitC1To8BitC1(const VTInStream::Sequence& seq);

    void        ClearPage(const VTInStream::Sequence& seq, dword flags);
    void        ClearLine(const VTInStream::Sequence& seq, dword flags);
    void        ClearTabs(const VTInStream::Sequence& seq);

    void        ReportMode(const VTInStream::Sequence& seq);
    void        ReportDeviceStatus(const VTInStream::Sequence& seq);
    void        ReportDeviceParameters(const VTInStream::Sequence& seq);
    void        ReportDeviceAttributes(const VTInStream::Sequence& seq);
    void        ReportControlFunctionSettings(const VTInStream::Sequence& seq);
    void        ReportRectAreaChecksum(const VTInStream::Sequence &seq);
    void        ReportPresentationState(const VTInStream::Sequence& seq);

    void        RestorePresentationState(const VTInStream::Sequence& seq);

    void        SelectGraphicsRendition(const VTInStream::Sequence& seq);
    void        SetGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes);
    void        InvertGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes);
    String      GetGraphicsRenditionOpcodes(const VTCell& attrs);

    void        ParseSixelGraphics(const VTInStream::Sequence& seq);
    void        ParseJexerGraphics(const VTInStream::Sequence& seq);
    void        ParseiTerm2Graphics(const VTInStream::Sequence& seq);

    void        ParseHyperlinks(const VTInStream::Sequence& seq);

    void        ParseClipboardRequests(const VTInStream::Sequence& seq);
    
    void        SetCaretStyle(const VTInStream::Sequence& seq);

    void        SetProgrammableLEDs(const VTInStream::Sequence& seq);

    void        SetDeviceConformanceLevel(const VTInStream::Sequence& seq);

    void        SetUserDefinedKeys(const VTInStream::Sequence& seq);

    void        CopyRectArea(const VTInStream::Sequence& seq);
    void        FillRectArea(const VTInStream::Sequence& seq);
    void        ClearRectArea(const VTInStream::Sequence& seq, bool selective = false);
    void        SelectRectAreaAttrsChangeExtent(const VTInStream::Sequence& seq);
    void        ChangeRectAreaAttrs(const VTInStream::Sequence& seq, bool invert);

    void        HandleWindowOpsRequests(const VTInStream::Sequence& seq);
    void        WindowMoveRequest(TopWindow *w, int x, int y);
    void        WindowResizeRequest(TopWindow *w, int cx, int cy);
    void        WindowPageResizeRequest(TopWindow *w, int cx, int cy);
    void        WindowMaximizeHorzRequest(TopWindow *w);
    void        WindowMaximizeVertRequest(TopWindow *w);

    void        SetHorizontalMargins(const VTInStream::Sequence& seq);
    void        SetVerticalMargins(const VTInStream::Sequence& seq);
    void        SetLinesPerPage(const VTInStream::Sequence& seq);

    void        SetColumns(int cols)                                { WhenSetSize(PageSizeToClient(Size(cols, page->GetSize().cy))); }
    void        SetRows(int rows)                                   { WhenSetSize(PageSizeToClient(Size(page->GetSize().cx, rows))); }

    void        SetDECStyleCellProtection(bool b)                   { page->Attributes(cellattrs.ProtectDEC(b)); }
    dword       GetDECStyleFillerFlags() const;
    void        SetISOStyleCellProtection(bool b)                   { page->Attributes(cellattrs.ProtectISO(b)); }
    dword       GetISOStyleFillerFlags() const;
    
    void        Backup(bool tpage = true, bool csets = true, bool attrs = true);
    void        Restore(bool tpage = true, bool csets = true, bool attrs = true);

    void        SetEmulation(int level, bool reset = true);

    void        Reset(bool full);

    void        AlternateScreenBuffer(bool b);
    
    void        VT52MoveCursor();   // VT52 direct cursor addressing.

private:
    VTInStream  parser;
    VTPage      dpage;
    VTPage      apage;
    VTCell      cellattrs;
    VTCell      cellattrs_backup;
    String      out;
    String      answerback;
    byte        clevel;
    bool        streamfill:1;

private:
    const VTCell&   GetAttrs() const                            { return cellattrs;  }

    void        SetPageSize(Size sz)                            { page->SetSize(sz); }
    VTPage&     GetDefaultPage()                                { return dpage; }
    bool        IsDefaultPage() const                           { return page == &dpage; }
    VTPage&     GetAlternatePage()                              { return apage; }
    bool        IsAlternatePage() const                         { return page == &apage; }

    TerminalCtrl&   Put0(const String& s, int cnt = 1);
    TerminalCtrl&   Put0(int c, int cnt = 1);
    TerminalCtrl&   Put(const WString& s, int cnt = 1);
    TerminalCtrl&   Put(int c, int cnt = 1);
    TerminalCtrl&   PutRaw(const String& s, int cnt = 1);
    TerminalCtrl&   PutESC(const String& s, int cnt = 1);
    TerminalCtrl&   PutESC(int c, int cnt = 1);
    TerminalCtrl&   PutCSI(const String& s, int cnt = 1);
    TerminalCtrl&   PutCSI(int c, int cnt = 1);
    TerminalCtrl&   PutOSC(const String& s, int cnt = 1);
    TerminalCtrl&   PutOSC(int c, int cnt = 1);
    TerminalCtrl&   PutDCS(const String& s, int cnt = 1);
    TerminalCtrl&   PutDCS(int c, int cnt = 1);
    TerminalCtrl&   PutSS2(const String& s, int cnt = 1);
    TerminalCtrl&   PutSS2(int c, int cnt = 1);
    TerminalCtrl&   PutSS3(const String& s, int cnt = 1);
    TerminalCtrl&   PutSS3(int c, int cnt = 1);
    TerminalCtrl&   PutEncoded(const WString& s, bool noctl = false);
    TerminalCtrl&   PutEol();

    void        Flush();
    void        CancelOut()                                     { out.Clear(); }

    void        DisplayAlignmentTest();

private:
    bool        GetUDKString(dword key, String& val);

private:
    VectorMap<dword, String> udk;

private:
    Bits        modes;

private:
     // ANSI modes.
    void        ANSIkam(bool b);
    void        ANSIcrm(bool b);
    void        ANSIirm(bool b);
    void        ANSIsrm(bool b);
    void        ANSIlnm(bool b);
    void        ANSIerm(bool b);

    // DEC private modes.
    void        DECanm(bool b);
    void        DECarm(bool b);
    void        DECawm(bool b);
    void        DECbkm(bool b);
    void        DECckm(bool b);
    void        DECcolm(bool b);
    void        DECkpam(bool b);
    void        DEClrmm(bool b);
    void        DECom(bool b);
    void        DECsclm(bool b);
    void        DECscnm(bool b);
    void        DECsdm(bool b);
    void        DECtcem(bool b);

    // Private mode extensions.
    void        XTasbm(int mode, bool b);
    void        XTanymm(bool b);
    void        XTascm(bool b);
    void        XTbrpm(bool b);
    void        XTdragm(bool b);
    void        XTfocusm(bool b);
    void        XTaltkeym(bool b);
    void        XTpcfkeym(bool b);
    void        XTrewrapm(bool b);
    void        XTsgrmm(bool b);
    void        XTsgrpxmm(bool b);
    void        XTsrcm(bool b);
    void        XTutf8mm(bool b);
    void        XTx10mm(bool b);
    void        XTx11mm(bool b);

    void        SetMode(const VTInStream::Sequence& seq, bool enable);

    using CbControl  = Tuple<byte, byte, Event<TerminalCtrl&, byte> >;
    using CbFunction = Tuple<byte, byte, Event<TerminalCtrl&, const VTInStream::Sequence&> >;
    using CbMode     = Tuple<word, byte, byte, Event<TerminalCtrl&, int, bool> >;

    const CbFunction* FindFunctionPtr(const VTInStream::Sequence& seq);
    const CbMode*     FindModePtr(word modenum, byte modetype);
    void              DispatchCtl(byte ctl);

private:
    // Key manipulation and VT and PC-style function keys support.
    struct FunctionKey : Moveable<FunctionKey> {
        enum Type : dword { Cursor, EditPad, NumPad, Programmable, Function };
        dword       type;
        dword       level;
        const char* code;
        const char* altcode;

        FunctionKey(dword type, dword level, const char *code, const char *altcode = nullptr)
        : type(type)
        , level(level)
        , code(code)
        , altcode(altcode)
        {
        }
    };

    bool ProcessKey(dword key, bool ctrlkey, bool altkey, int count);
    bool ProcessVTStyleFunctionKey(const FunctionKey& k, dword modkeys, int count);
    bool ProcessPCStyleFunctionKey(const FunctionKey& k, dword modkeys, int count);

public:
    // DEC and xterm style caret (cursor) support.
    class Caret {
        int       style;
        bool      blinking;
        bool      locked;
    public:
        enum : int
        {
            BLOCK = 0,
            BEAM,
            UNDERLINE
        };
        Event<> WhenAction;
        void    Set(int style_, bool blink);
        Caret&  Block(bool blink = true)                        { Set(BLOCK, blink); return *this; }
        Caret&  Beam(bool blink = true)                         { Set(BEAM, blink);  return *this; }
        Caret&  Underline(bool blink = true)                    { Set(UNDERLINE, blink); return *this; }
        Caret&  Blink(bool b = true)                            { if(!locked) { blinking = b; WhenAction(); }; return *this; }
        Caret&  Lock(bool b = true)                             { locked = b; return *this; }
        Caret&  Unlock()                                        { return Lock(false); }
        int     GetStyle() const                                { return style;    }
        bool    IsBlinking() const                              { return blinking; }
        bool    IsLocked() const                                { return locked;   }
        void    Serialize(Stream& s);
        void    Jsonize(JsonIO& jio);
        void    Xmlize(XmlIO& xio);
        Caret();
        Caret(int style, bool blink, bool lock);
    };

private:
    Caret       caret;

public:
    // Terminal legacy character sets ("G-set") support.
    class GSets {
        byte  g[4];
        byte  d[4];
        byte  ss;
        int   l, r;
    public:
        GSets&     G0toGL()                                     { l = 0; return *this; }
        GSets&     G1toGL()                                     { l = 1; return *this; }
        GSets&     G2toGL()                                     { l = 2; return *this; }
        GSets&     G3toGL()                                     { l = 3; return *this; }
        GSets&     G0toGR()                                     { r = 0; return *this; }
        GSets&     G1toGR()                                     { r = 1; return *this; }
        GSets&     G2toGR()                                     { r = 2; return *this; }
        GSets&     G3toGR()                                     { r = 3; return *this; }

        GSets&     G0(byte c)                                   { g[0] = c; return *this; }
        GSets&     G1(byte c)                                   { g[1] = c; return *this; }
        GSets&     G2(byte c)                                   { g[2] = c; return *this; }
        GSets&     G3(byte c)                                   { g[3] = c; return *this; }
        GSets&     SS(byte c)                                   { ss   = c; return *this; }
        GSets&     Broadcast(byte c)                            { g[0] = g[1] = g[2] = g[3] = c; return *this; }
        
        byte        Get(int c, bool allowgr = true) const       { return c < 0x80 || !allowgr ? g[l] : g[r]; }

        int         GetGLNum()                                  { return l; }
        int         GetGRNum()                                  { return r; }

        byte        GetGL() const                               { return g[l]; }
        byte        GetGR() const                               { return g[r]; }
        byte        GetG0() const                               { return g[0]; }
        byte        GetG1() const                               { return g[1]; }
        byte        GetG2() const                               { return g[2]; }
        byte        GetG3() const                               { return g[3]; }
        byte        GetSS() const                               { return ss;   }

        void        ConformtoANSILevel1();
        void        ConformtoANSILevel2();
        void        ConformtoANSILevel3();

        GSets&      ResetG0()                                   { g[0] = d[0]; return *this; }
        GSets&      ResetG1()                                   { g[1] = d[1]; return *this; }
        GSets&      ResetG2()                                   { g[2] = d[2]; return *this; }
        GSets&      ResetG3()                                   { g[3] = d[3]; return *this; }

        void        Reset();
        void        Serialize(Stream& s);
        void        Jsonize(JsonIO& jio);
        void        Xmlize(XmlIO& xio);

        GSets(byte defgset = CHARSET_ISO8859_1);
        GSets(byte g0, byte g1, byte g2, byte g3);
    };

    void            SetLegacyCharsets(GSets newgsets)           { gsets = newgsets;  }
    const GSets&    GetLegacyCharsets() const                   { return gsets;      }
    TerminalCtrl&   LegacyCharsets(bool b = true)               { legacycharsets = b; return *this; }
    TerminalCtrl&   NoLegacyCharsets()                          { return LegacyCharsets(false); }

private:
    byte            ResolveVTCharset(byte cs)                   { return ResolveCharset(legacycharsets ? cs : charset); }
    int             DecodeCodepoint(int c, byte gset);
    int             EncodeCodepoint(int c, byte gset);
    WString         DecodeDataString(const String& s);
    String          EncodeDataString(const WString& ws);

private:
    GSets           gsets;
    GSets           gsets_backup;


    // Currently supported ANSI and private terminal modes.

    enum TerminalModes : byte
    {
        GATM = 0,
        KAM,
        CRM,
        IRM,
        SRTM,
        ERM,
        VEM,
        HEM,
        PUM,
        SRM,
        FEAM,
        FETM,
        MATM,
        TTM,
        SATM,
        TSM,
        EBM,
        LNM,
        DECANM,
        DECARM,
        DECAWM,
        DECBKM,
        DECCKM,
        DECCOLM,
        DECKPAM,
        DECLRMM,
        DECOM,
        DECSCLM,
        DECSCNM,
        DECSDM,
        DECTCEM,
        XTASBM,
        XTASCM,
        XTBRPM,
        XTDRAGM,
        XTANYMM,
        XTFOCUSM,
        XTALTESCM,
        XTPCFKEYM,
        XTREWRAPM,
        XTSPREG,
        XTSRCM,
        XTSGRMM,
        XTSGRPXMM,
        XTUTF8MM,
        XTX10MM,
        XTX11MM,
        XTSHOWSB,
        VTMODECOUNT
    };
};

// Custom displays.

const Display& NormalImageCellDisplay();
const Display& ScaledImageCellDisplay();

// Color formatters and converters.

class ConvertHashColorSpec : public Convert {
public:
    ConvertHashColorSpec() {}
    int     Filter(int chr) const override;
    Value   Scan(const Value& text) const override;
    Value   Format(const Value& q) const override;
};

class ConvertRgbColorSpec : public Convert {
public:
    ConvertRgbColorSpec() {}
    int     Filter(int chr) const override;
    Value   Scan(const Value& text) const override;
    Value   Format(const Value& q) const override;
};

class ConvertCmykColorSpec : public Convert {
public:
    ConvertCmykColorSpec() {}
    int     Filter(int chr) const override;
    Value   Scan(const Value& text) const override;
    Value   Format(const Value& q) const override;
};

class ConvertColor : public Convert {
public:
    Value   Scan(const Value& text) const override;
    Value   Format(const Value& q) const override;
};

// Legacy charsets.

extern byte CHARSET_DEC_VT52;   // DEC VT52 graphics character set.
extern byte CHARSET_DEC_DCS;    // DEC VT100+ line drawing character set.
extern byte CHARSET_DEC_MCS;    // DEC VT200+ multinational character set.
extern byte CHARSET_DEC_TCS;    // DEC VT300+ technical character set.

INITIALIZE(DECGSets);
}
#endif
