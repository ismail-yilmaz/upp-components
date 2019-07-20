#include "Terminal.h"

#define LLOG(x)	// RLOG("Terminal: " << x);

namespace Upp {

static int sGetPos(Point pt, Size sz) // 2D -> 1D
{
	return pt.y * sz.cx + pt.x;
}

static Point sGetCoords(int pos, Size sz) // 1D -> 2D
{
	int x = pos % sz.cx;
	return Point(x, (pos - x) / sz.cx);
}

Terminal::Terminal() : Console()
{
	Unicode();
	SetCharset(CHARSET_UNICODE);
	SetFrame(FieldFrame());
	History();
	ResetColors();
	HideScrollBar();
	WhenBar = THISFN(StdBar);
	sb.WhenScroll = THISFN(Scroll);
	caret.WhenAction = [=] { PlaceCaret(); };
	GetDefaultPage().WhenScroll = [=] { SyncPage(); RefreshDisplay();  };
}

void Terminal::PreParse()
{
	if(delayed &&
		(!lazyresize || !resizing))
			if(!ExistsTimeCallback(TIMEID_REFRESH)) // Don't cancel a pending refresh.
					SetTimeCallback(-16, THISFN(DoDelayedRefresh), TIMEID_REFRESH);
}

void Terminal::PostParse()
{
	if(!delayed)
		RefreshDisplay();
}

void Terminal::SetCharset(byte chrset)
{
	SetCharsets((charset = ResolveCharset(chrset)));
}

Size Terminal::GetFontSize() const
{
	FontInfo fi = font.Info();
	return Size(max(fi['M'], fi['W']), fi.GetHeight());
}

Size Terminal::GetPageSize() const
{
	Size wsz = GetSize();
	Size fsz = GetFontSize();
	return clamp(wsz / fsz, Size(1, 1), GetScreenSize() / fsz);
}

void Terminal::PlaceCaret(bool scroll)
{
	bool  b = modes[DECTCEM];

	if(!b || !caret.IsBlinking()) {
		KillCaret();
		if(!b) return;
	}
	if(caret.IsBlinking())
		SetCaret(GetCaretRect());
	else {
		Refresh(caretrect);
		Refresh(GetCaretRect());
	}
	if(scroll) {
		sb.ScrollInto(GetCursorPos().y);
	}
}

Rect Terminal::GetCaretRect()
{
	Size fsz = GetFontSize();
	Point p  = GetCursorPos();
	
	p.x = p.x * fsz.cx;
	p.y = p.y * fsz.cy - (GetSbPos() * fsz.cy);

	switch(caret.GetStyle()) {
	case Caret::BEAM:
		fsz.cx = 1;
		break;
	case Caret::UNDERLINE:
		p.y += fsz.cy - 1;
		fsz.cy = 1;
		break;
	case Caret::BLOCK:
		break;
	}

	return caretrect = RectC(p.x, p.y, fsz.cx, fsz.cy);
}

void Terminal::Copy()
{
	String txt = GetSelectedText();
	if(!IsNull(txt)) {
		ClearClipboard();
		AppendClipboardText(txt);
	}
}

void Terminal::Paste()
{
	Paste(Clipboard());
}

void Terminal::Paste(PasteClip& d)
{
	if(IsReadOnly())
		return;

	String s;
	bool accepted;
	if((accepted = AcceptFiles(d))) {
		for(auto f : GetFiles(d))
			s << "'" << f << "' ";
		s = TrimRight(s);
	}
	else
	if((accepted = AcceptText(d))) {
		s = GetWString(d).ToString();
	}
	if(accepted && !IsNull(s)) {
		if(modes[XTBRPM]) {		// Bracketed paste mode.
			PutCSI("200~");
			PutRaw(s);
			PutCSI("201~");
		}
		else PutRaw(s);
	}
	ClearSelection();
}

void Terminal::SelectAll(bool history)
{
	Size psz = GetPageSize();
	bool h = IsDefaultPage() && history;
	Rect r = RectC(0, h ? 0 : sb, psz.cx, (h ? sb + (sb.GetTotal() - sb) : psz.cy) - 1);
	anchor = sGetPos(r.TopLeft(), psz);
	selpos = sGetPos(r.BottomRight(), psz);
	selection = true;
	Refresh();
}

void Terminal::Layout()
{
	ReCalcSelection();
	SyncPage();
}

void Terminal::SyncSize(bool notify)
{
	if(resizing && !IsShown())
		return;
	
	// Apparently, the window minimize event on Windows really minimizes
	// the window. (I mean, really!?!?) This results in a damaged display.
	// We check the new page size to prevent this and don't attemp resize
	// the page if the requested page size is < 2 x 2 cells.

	Size psz = GetPageSize();
	if((resizing = page->GetSize() != psz) && psz.cx > 1 && 1 < psz.cy) {
		page->SetSize(psz);
		if(notify) {
			if(sizehint) {
				hinting = true;
				KillSetTimeCallback(-1000, THISFN(HintNewSize), TIMEID_SIZEHINT);
			}
			if(lazyresize)
				KillSetTimeCallback(-100, THISFN(DoLazyResize), TIMEID_REFRESH);
			else {
				resizing = false;
				WhenResize();
			}
		}
		else
			resizing = false;
	}
}

void Terminal::DoLazyResize()
{
	KillTimeCallback(TIMEID_REFRESH);
	resizing = false;
	WhenResize();
}

void Terminal::DoDelayedRefresh()
{
	KillTimeCallback(TIMEID_REFRESH);
	RefreshDisplay();
}

void Terminal::HintNewSize()
{
	KillTimeCallback(TIMEID_SIZEHINT);
	RefreshSizeHint();
	hinting = false;
}

Tuple<String, Rect> Terminal::GetSizeHint(Rect r, Size sz)
{
	Tuple<String, Rect> hint;
	hint.a << sz.cx << " x " << sz.cy;
	hint.b = r.CenterRect(GetTextSize(hint.a, StdFont()));
	return pick(hint);
}

void Terminal::RefreshSizeHint()
{
	Refresh(GetSizeHint(GetRect(), GetPageSize()).b.Inflated(8));
}

void Terminal::SyncSb()
{
	sb.SetTotal(page->GetLineCount());
	sb.SetPage(page->GetSize().cy);
	sb.SetLine(1);
}

void Terminal::Scroll()
{
	if(IsDefaultPage()) {
		scroller.Scroll(*this, GetSize(), sb * GetFontSize().cy);
		PlaceCaret();
	}
}

void Terminal::SyncPage(bool notify)
{
	SyncSize(notify);
	SyncSb();
	if(!ignorescroll)
		sb.End();
}

void Terminal::SwapPage()
{
	SyncPage(false);
	ClearSelection();
}

void Terminal::RefreshPage(bool full)
{
	if(full)
		Refresh();
	else
	if(!delayed)
		RefreshDisplay();
}

void Terminal::RefreshDisplay()
{
	Size wsz = GetSize();
	Size psz = page->GetSize();
	Size fsz = GetFontSize();
	int	 pos = GetSbPos();
	int blinking_cells = 0;
	Vector<Rect> invalid;
	
	for(int i = pos; i < pos + psz.cy; i++) {
		const auto& line = page->GetLine(i);
		if(blinktext)
			for(const VTCell& cell : line)
				if(cell.IsBlinking()) {
					blinking_cells++;
					line.Invalidate();
					break;
				}
		if(line.IsInvalid()) {
			line.Validate();
			AddRefreshRect(invalid, RectC(0, i * fsz.cy - (fsz.cy * pos), wsz.cx, fsz.cy));
		}
	}

	for(const Rect& r : invalid)
		Refresh(r);
	PlaceCaret();
	Blink(blinking_cells > 0);
}

void Terminal::Blink(bool b)
{
	bool bb = ExistsTimeCallback(TIMEID_BLINK);
	if(blinktext && b && !bb)
		SetTimeCallback(-blinkinterval, THISFN(RefreshBlinkingText), TIMEID_BLINK);
	else
	if(!blinktext || !b) {
		blinking = false;
		if(bb)
			KillTimeCallback(TIMEID_BLINK);
	}
}

void Terminal::RefreshBlinkingText()
{
	blinking ^= 1;
	RefreshDisplay();
}

void Terminal::GotFocus()
{
	if(modes[XTFOCUSM])
		PutCSI('I');
	Refresh();
}

void Terminal::LostFocus()
{
	if(modes[XTFOCUSM])
		PutCSI('O');
	Refresh();
}

void Terminal::DragAndDrop(Point p, PasteClip& d)
{
	if(!IsDragAndDropSource())
		Paste(d);
}

void Terminal::LeftDown(Point p, dword keyflags)
{
	SetFocus();
	if(IsTrackingEnabled())
		VTMouseEvent(p, LEFTDOWN, keyflags);
	else {
		int l, h, n = GetMouseSelPos(p);
		if(selection || GetSelection(l, h) && n >= l && n < h) {
			ClearSelection();
			return;
		}
		SetSelection(n, n);
	}
	SetCapture();
}

void Terminal::LeftUp(Point p, dword keyflags)
{
	if(IsTrackingEnabled() && !modes[XTX10MM])
		VTMouseEvent(p, LEFTUP, keyflags);
	else {
		int l, h;
		if(selection && !GetSelection(l, h))
			ClearSelection();
	}
	ReleaseCapture();
}

void Terminal::MiddleDown(Point p, dword keyflags)
{
	SetFocus();
	if(IsTrackingEnabled())
		VTMouseEvent(p, MIDDLEDOWN, keyflags);
	else
	if(!IsReadOnly())
		Paste();
}

void Terminal::MiddleUp(Point p, dword keyflags)
{
	if(IsTrackingEnabled() && !modes[XTX10MM])
		VTMouseEvent(p, MIDDLEUP, keyflags);
}

void Terminal::RightDown(Point p, dword keyflags)
{
	SetFocus();
	if(IsTrackingEnabled())
		VTMouseEvent(p, RIGHTDOWN, keyflags);
	else {
		if(!IsSelected(GetMouseSelPos(p)))
			ClearSelection();
		MenuBar::Execute(WhenBar);
	}
}

void Terminal::RightUp(Point p, dword keyflags)
{
	if(IsTrackingEnabled() && !modes[XTX10MM])
		VTMouseEvent(p, RIGHTUP, keyflags);
}

void Terminal::MouseMove(Point p, dword keyflags)
{
	Rect r = GetRect();
	p = clamp(p, r.TopLeft(), r.BottomRight());
	
	bool b = HasCapture();
	if(IsTrackingEnabled()) {
		if(b && modes[XTDRAGM])
			VTMouseEvent(p, LEFTDRAG, keyflags);
		else
		if(modes[XTANYMM])
			VTMouseEvent(p, b ? LEFTDRAG : MOUSEMOVE, keyflags);
	}
	else
	if(HasCapture()) {
		selpos = GetMouseSelPos(p);
		Refresh();
	}
}

void Terminal::MouseWheel(Point p, int zdelta, dword keyflags)
{
	bool b = IsTrackingEnabled();
	if(!b && page->HasHistory())
		sb.Wheel(zdelta, wheelstep);
	else
	if(zdelta != 0) {
		if(IsAlternatePage() &&
			(alternatescroll || (alternatescroll && modes[XTASCM])))
				VTKey(zdelta > 0 ? K_UP : K_DOWN, wheelstep);
		else
		if(b && !modes[XTX10MM])
			VTMouseEvent(p, MOUSEWHEEL, keyflags, zdelta);
	}
}

void Terminal::VTMouseEvent(Point p, dword event, dword keyflags, int zdelta)
{
	bool buttondown = (event & UP) != UP; // Combines everything else with a button-down event
	int  mouseevent = 0;
	
	switch(event) {
	case LEFTUP:
	case LEFTDOWN:
		mouseevent = 0x00;
		break;
	case LEFTDRAG:
		mouseevent = 0x20;
		break;
	case MIDDLEUP:
	case MIDDLEDOWN:
		mouseevent = 0x01;
		break;
	case RIGHTUP:
	case RIGHTDOWN:
		mouseevent = 0x02;
		break;
	case MOUSEMOVE:
		mouseevent = 0x23;
		break;
	case MOUSEWHEEL:
		mouseevent = zdelta > 0 ? 0x40 : 0x41;
		break;
	default:
		return;
	}

	if(keyflags & K_SHIFT) mouseevent |= 0x04;
	if(keyflags & K_ALT)   mouseevent |= 0x08;
	if(keyflags & K_CTRL)  mouseevent |= 0x10;
	
	p = GetMousePos(p) + 1;

	if(modes[XTSGRMM])
		PutCSI(Format("<%d;%d;%d%[1:M;m]s", mouseevent, p.x, p.y, buttondown));
	else {
		if(!buttondown)
			mouseevent = 0x03;
		mouseevent += 0x20;
		p += 0x20;
		if(modes[XTUTF8MM]) {
			WString s;
			s.Cat(p.x);
			s.Cat(p.y);
			PutCSI(Format("M%c%s", mouseevent, ToUtf8(s)));
		}
		else
			PutCSI(Format("M%c%c%c", mouseevent, p.x, p.y));
	}
	RefreshDisplay();
}

bool Terminal::IsTrackingEnabled() const
{
	return modes[XTX10MM]  ||
	       modes[XTX11MM]  ||
	       modes[XTANYMM]  ||
	       modes[XTDRAGM];
}

Point Terminal::GetMousePos(Point p)
{
	Size wsz = GetSize();
	Size psz = GetPageSize();
	p.y = p.y * psz.cy / (wsz.cy -= wsz.cy % psz.cy) + GetSbPos();
	p.x = p.x * psz.cx / wsz.cx;
	return p;
}

int Terminal::GetMouseSelPos(Point p)
{
	return sGetPos(GetMousePos(p), GetPageSize());
}

void Terminal::SetSelection(int l, int h)
{
	anchor = min(l, h);
	selpos = max(l, h);
	selection = true;
}

bool Terminal::GetSelection(int& l, int& h)
{
	if(anchor < 0 || anchor == selpos) {
		l = h = selpos;
		return false;
	}
	l = min(anchor, selpos);
	h = max(anchor, selpos);
	return true;
}

Rect Terminal::GetSelectionRect()
{
	Rect r;
	int l, h;
	if(selection && GetSelection(l, h)) {
		Size psz = page->GetSize();
		Point a = sGetCoords(l, psz);
		Point b = sGetCoords(h, psz);
		r.Set(a, b);
	}
	return r;
}

void Terminal::ClearSelection()
{
	selection = false;
	anchor = selpos = -1;
	Refresh();
}

void Terminal::ReCalcSelection()
{
	Rect r = GetSelectionRect();
	if(!IsNull(r)) {
		Size psz = GetPageSize();
		anchor = sGetPos(r.TopLeft(), psz);
		selpos = sGetPos(r.BottomRight(), psz);
	}
}

bool Terminal::IsSelected(int pos)
{
	int l, h;
	return selection && GetSelection(l, h) && pos >= l && pos < h;
}

String Terminal::GetSelectedText()
{
	WString txt;
	Rect r = GetSelectionRect();

	auto PutCrLf = [&txt](int cnt = 1) -> void
	{
		while(cnt-- > 0) {
		#ifdef PLATFORM_WIN32
			txt.Cat('\r');
		#endif
			txt.Cat('\n');
		}
	};
	
	for(int line = r.top, i = 0; !IsNull(r) && line <= r.bottom; line++) {
		if(line < page->GetLineCount()) {
			const auto& ln = page->GetLine(line);
			bool iswrapped = ln.wrapped;
			WString s = AsWString(ln);
			if(IsNull(s)) {
				i++;
				continue;
			}
			if(i) {
				PutCrLf(i);
				i = 0;
			}
			if(line == r.top)
				txt.Cat(s.Mid(r.left, (r.top == r.bottom ? r.right : s.GetLength()) - r.left));
			else
			if(line == r.bottom)
				 txt.Cat(s.Left(r.right));
			else
				txt.Cat(s);
			if(!iswrapped)
				PutCrLf();
		}
	}
	return ToUtf8(txt);
}

void Terminal::StdBar(Bar& menu)
{
	menu.Add(t_("Read only"), [=] { SetEditable(IsReadOnly()); })
		.Key(K_SHIFT_CTRL_L)
		.Check(IsReadOnly());
	menu.Add(t_("Scrollbar"), [=] { ShowScrollBar(!sb.IsChild()); })
		.Key(K_SHIFT_CTRL_S)
		.Check(sb.IsChild());
	menu.Separator();
	menu.Add(selection, t_("Copy"), CtrlImg::copy(), [=] { Copy(); })
		.Key(K_SHIFT_CTRL_C);
	menu.Add(IsEditable(), t_("Paste"), CtrlImg::paste(), [=] { Paste(); })
		.Key(K_SHIFT_CTRL_V);
	menu.Separator();
	menu.Add(t_("Select all"), CtrlImg::select_all(), [=] { SelectAll(); })
		.Key(K_SHIFT_CTRL_A);
}

Terminal& Terminal::ShowScrollBar(bool b)
{
	if(!sb.IsChild() && b) {
		ignorescroll = true;
		AddFrame(sb.AutoDisable());
	}
	else
	if(sb.IsChild() && !b) {
		ignorescroll = true;
		RemoveFrame(sb);
	}
	ignorescroll = false;
	return *this;
}

Terminal& Terminal::ResetColors()
{
	colortable[COLOR_BLACK] = SBlack;
	colortable[COLOR_RED] = SRed;
	colortable[COLOR_GREEN] = SGreen;
	colortable[COLOR_YELLOW] = SYellow;
	colortable[COLOR_BLUE] = SBlue;
	colortable[COLOR_MAGENTA] = SMagenta;
	colortable[COLOR_CYAN] = SCyan;
	colortable[COLOR_WHITE] = SWhite;
	colortable[COLOR_LTBLACK] = SGray;
	colortable[COLOR_LTRED] = SLtRed;
	colortable[COLOR_LTGREEN] = SLtGreen;
	colortable[COLOR_LTYELLOW] = SLtYellow;
	colortable[COLOR_LTBLUE] = SLtBlue;
	colortable[COLOR_LTMAGENTA] = SLtMagenta;
	colortable[COLOR_LTCYAN] = SLtCyan;
	colortable[COLOR_LTWHITE] = SWhite;
	colortable[COLOR_INK] = SColorText;
	colortable[COLOR_INK_SELECTED] = SColorHighlightText;
	colortable[COLOR_PAPER] = SColorPaper;
	colortable[COLOR_PAPER_SELECTED] = SColorHighlight;
	return *this;
}

Image Terminal::CursorImage(Point p, dword keyflags)
{
	return IsTrackingEnabled() ? Image::Arrow() : Image::IBeam();
}

void Terminal::State(int reason)
{
	if(reason == Ctrl::OPEN)
		WhenResize();
}

void Terminal::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % font;
		s % charset;
		s % delayed;
		s % lazyresize;
		s % sizehint;
		s % blinktext;
		s % blinkinterval;
		s % adjustcolors;
		s % lightcolors;
		s % intensify;
		s % wheelstep;
		s % keynavigation;
		s % nobackground;
		s % alternatescroll;
		for(int i = 0; i < 20; i++)
			s % colortable[i];
		Console::Serialize(s);
	}
	if(s.IsLoading())
		Layout();
}
}