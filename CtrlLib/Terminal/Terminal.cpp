#include "Terminal.h"

#define LLOG(x)		// RLOG("Terminal: " << x)
#define LTIMING(x)	// RTIMING(x)

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
	NoLegacyCharsets();
	SetImageDisplay(NormalImageCellDisplay());
	SetFrame(FieldFrame());
	History();
	ResetColors();
	HideScrollBar();
	WhenBar = THISFN(StdBar);
	sb.WhenScroll = THISFN(Scroll);
	caret.WhenAction = [=] { PlaceCaret(); };
	GetDefaultPage().WhenScroll = [=] { SyncPage(); RefreshDisplay(); };
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

void Terminal::Copy(const WString& s)
{
	if(!IsNull(s)) {
		ClearClipboard();
		AppendClipboardUnicodeText(s);
		AppendClipboardText(s.ToString());
	}
}

void Terminal::Paste()
{
	DragAndDrop(Null, Clipboard());
}

void Terminal::Paste(const WString& s, bool filter)
{
	if(IsReadOnly())
		return;

	if(modes[XTBRPM]) {
		PutCSI("200~");
		PutEncoded(s, filter);
		PutCSI("201~");
	}
	else
		PutEncoded(s, filter);
}

void Terminal::SelectAll(bool history)
{
	Size psz = GetPageSize();
	bool h = IsDefaultPage() && history;
	Rect r = RectC(0, h ? 0 : sb, psz.cx, (h ? sb + (sb.GetTotal() - sb) : psz.cy) - 1);
	anchor = sGetPos(r.TopLeft(), psz);
	selpos = sGetPos(r.BottomRight(), psz);
	selclick = true;
	Refresh();
}

void Terminal::Layout()
{
	ReCalcSelection();
	SyncPage();
}

void Terminal::SyncSize(bool notify)
{
	// Apparently, the window minimize event on Windows "really" minimizes
	// the window. This results in a damaged terminal display. In order to
	// avoid this, we check the  new page size, and  do not attempt resize
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
	Refresh(GetSizeHint(GetView(), GetPageSize()).b.Inflated(8));
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
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	int  pos = GetSbPos();
	int blinking_cells = 0;
	bool hyperlinks = consoleflags & FLAG_HYPERLINKS;
	Vector<Rect> invalid;
	
	LTIMING("Terminal::RefreshDisplay");

	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); i++) {
		const VTLine& line = page->GetLine(i);
		int y = i * fsz.cy - (fsz.cy * pos);
		for(int j = 0; j < line.GetCount(); j++) {
			int x = j * fsz.cx;
			const VTCell& cell = line[j];
			if(hyperlinks && cell.IsHyperlink() &&
				(cell.data == activelink || cell.data == prevlink)) {
					if(!line.IsInvalid())
						AddRefreshRect(invalid, RectC(x, y, fsz.cx, fsz.cy));
			}
			else
			if(blinktext && cell.IsBlinking()) {
				if(!line.IsInvalid())
					AddRefreshRect(invalid, RectC(x, y, fsz.cx, fsz.cy));
				blinking_cells++;
			}
		}
		if(line.IsInvalid()) {
			line.Validate();
			AddRefreshRect(invalid, RectC(0, i * fsz.cy - (fsz.cy * pos), wsz.cx, fsz.cy));
		}
	}

	if(invalid.GetCount())
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
	if(IsReadOnly() || IsDragAndDropSource())
		return;
	
	WString s;

	if(AcceptFiles(d)) {
		for(const auto& f : GetFiles(d)) {
			s.Cat('\'');
			s.Cat(f.ToWString());
			s.Cat('\'');
		}
		s = TrimRight(s);
	}
	else
	if(AcceptText(d))
		s = GetWString(d);
	else
		return;

	d.SetAction(DND_COPY);
	
	bool noctl = WhenClip(d);

	if(d.IsAccepted())
		Paste(s, noctl);
}

void Terminal::LeftDown(Point p, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(p, LEFTDOWN, keyflags);
	else{
		int n = GetMouseSelPos(p);
		if(IsSelected(n)) {
			selclick = true;
			return;
		}
		else
			SetSelection(n, n);
	}
	SetCapture();
}

void Terminal::LeftUp(Point p, dword keyflags)
{
	if(IsTracking()) {
		if(!modes[XTX10MM])
			VTMouseEvent(p, LEFTUP, keyflags);
	}
	else {
		int n = GetMouseSelPos(p);
		if(!HasCapture() && selclick && IsSelected(n))
			ClearSelection();
		selclick = false;
	}
	ReleaseCapture();
}

void Terminal::LeftDrag(Point p, dword keyflags)
{
	int n = GetMouseSelPos(p);
	if(!IsTracking() && !HasCapture() && selclick && IsSelected(n)) {
		WString sample = GetSelectedText();
		VectorMap<String, ClipData> data;
		Append(data, sample);
		Size ssz = StdSampleSize();
		ImageDraw iw(ssz);
		iw.DrawRect(ssz, Black());
		iw.Alpha().DrawRect(ssz, Black());
		DrawTLText(iw.Alpha(), 0, 0, ssz.cx, sample, font, White());
		DoDragAndDrop(data, iw, DND_COPY);
	}
}

void Terminal::LeftDouble(Point p, dword keyflags)
{
	// TODO: Word selection.
	
	if(IsTracking())
		Ctrl::LeftDouble(p, keyflags);
	else {
		ClearSelection();
		String uri = GetHyperlinkURI(GetMousePos(p), keyflags & K_CTRL);
		if(!IsNull(uri))
			WhenLink(uri);
	}
}

void Terminal::MiddleDown(Point p, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(p, MIDDLEDOWN, keyflags);
	else
		Paste();
}

void Terminal::MiddleUp(Point p, dword keyflags)
{
	if(IsTracking() && !modes[XTX10MM])
		VTMouseEvent(p, MIDDLEUP, keyflags);
}

void Terminal::RightDown(Point p, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(p, RIGHTDOWN, keyflags);
	else {
		int n = GetMouseSelPos(p);
		if(!(selclick = IsSelected(n)))
			ClearSelection();
		MenuBar::Execute(WhenBar);
	}
}

void Terminal::RightUp(Point p, dword keyflags)
{
	if(IsTracking() && !modes[XTX10MM])
		VTMouseEvent(p, RIGHTUP, keyflags);
}

void Terminal::MouseMove(Point p, dword keyflags)
{
	Rect r = GetView();
	p = clamp(p, r.TopLeft(), r.BottomRight());
	
	bool b = HasCapture();
	if(IsTracking()) {
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
	else
	if(consoleflags & FLAG_HYPERLINKS) {
		HighlightHyperlink(GetMousePos(p));
	}
}

void Terminal::MouseWheel(Point p, int zdelta, dword keyflags)
{
	bool b = IsTracking();
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
	
	p = GetMousePos(p) + 1;
	
	switch(event) {
	case LEFTUP:
	case LEFTDOWN:
		mouseevent = 0x00;
		break;
	case LEFTDRAG:
		if(p == mousepos)
			return;
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
		if(p == mousepos)
			return;
		mouseevent = 0x23;
		break;
	case MOUSEWHEEL:
		mouseevent = zdelta > 0 ? 0x40 : 0x41;
		break;
	default:
		return;
	}

	mousepos = p;
	
	if(keyflags & K_SHIFT) mouseevent |= 0x04;
	if(keyflags & K_ALT)   mouseevent |= 0x08;
	if(keyflags & K_CTRL)  mouseevent |= 0x10;
	

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
}

bool Terminal::IsTracking() const
{
	return modes[XTX10MM]  ||
	       modes[XTX11MM]  ||
	       modes[XTANYMM]  ||
	       modes[XTDRAGM];
}

Point Terminal::GetMousePos(Point p) const
{
	Size wsz = GetSize();
	Size psz = GetPageSize();
	p.y = p.y * psz.cy / (wsz.cy -= wsz.cy % psz.cy) + GetSbPos();
	p.x = p.x * psz.cx / wsz.cx;
	return p;
}

int Terminal::GetMouseSelPos(Point p) const
{
	return sGetPos(GetMousePos(p), GetPageSize());
}

void Terminal::SetSelection(int l, int h)
{
	anchor = min(l, h);
	selpos = max(l, h);
	Refresh();
}

bool Terminal::GetSelection(int& l, int& h) const
{
	if(anchor < 0 || anchor == selpos) {
		l = h = selpos;
		return false;
	}
	l = min(anchor, selpos);
	h = max(anchor, selpos);
	return true;
}

Rect Terminal::GetSelectionRect() const
{
	Rect r = Null;
	int l, h;
	if(GetSelection(l, h)) {
		Size psz = page->GetSize();
		Point a = sGetCoords(l, psz);
		Point b = sGetCoords(h, psz);
		r.Set(a, b);
	}
	return r;
}

void Terminal::ClearSelection()
{
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

bool Terminal::IsSelected(int pos) const
{
	int l, h;
	return GetSelection(l, h) && pos >= l && pos < h;
}

WString Terminal::GetSelectedText() const
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
			const VTLine& ln = page->GetLine(line);
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
			if(!ln.IsWrapped())
				PutCrLf();
		}
	}
	return txt;
}

bool Terminal::GetCellAtMousePos(VTCell& cell, Point p) const
{
	if(p.y >= 0 && p.y < page->GetLineCount()) {
		const VTLine& line = page->GetLine(p.y);
		if(p.x >= 0 && p.x < line.GetCount()) {
			cell = line[p.x];
			return true;
		}
	}
	return false;
}

bool Terminal::IsMouseOverImage(Point p) const
{
	VTCell cell;
	return GetCellAtMousePos(cell, p) && cell.IsImage();
}

bool Terminal::IsMouseOverHyperlink(Point p) const
{
	VTCell cell;
	return GetCellAtMousePos(cell, p) && cell.IsHyperlink();
}

String Terminal::GetHyperlinkURI(Point p, bool modifier)
{
	String uri;
	VTCell cell;
	if(GetCellAtMousePos(cell, p) && cell.Hyperlink() && modifier) {
		uri = GetCachedHyperlink(cell.data);
		if(IsNull(uri))
			LLOG("Unable to retrieve URI from link cache. Link id: " << cell.data);
	}
	return uri;
}

void Terminal::HighlightHyperlink(Point p)
{
	if(mousepos != p) {
		mousepos = p;
		VTCell cell;
		if(!GetCellAtMousePos(cell, p))
			return;
		if(cell.IsHyperlink() || activelink > 0) {
			if(cell.data != activelink) {
				prevlink = activelink;
				activelink = cell.data;
				RefreshDisplay();
			}
			Tip(GetCachedHyperlink(activelink));
		}
	}
}

void Terminal::StdBar(Bar& menu)
{
	OptionsBar(menu);
	if(IsMouseOverHyperlink()) {
		menu.Separator();
		LinksBar(menu);
	}
	menu.Separator();
	EditBar(menu);
}

void Terminal::EditBar(Bar& menu)
{
	menu.Add(IsSelection(), t_("Copy"), CtrlImg::copy(), [=] { Copy(); })
		.Key(K_SHIFT_CTRL_C);
	menu.Add(IsEditable(), t_("Paste"), CtrlImg::paste(), [=] { Paste(); })
		.Key(K_SHIFT_CTRL_V);
	menu.Separator();
	menu.Add(t_("Select all"), CtrlImg::select_all(), [=] { SelectAll(); })
		.Key(K_SHIFT_CTRL_A);
}

void Terminal::LinksBar(Bar& menu)
{
	String uri = GetHyperlinkUri();
	if(IsNull(uri))
		return;

	menu.Add(t_("Copy link to clipboard"), CtrlImg::copy(), [=] { Copy(uri.ToWString()); })
		.Key(K_SHIFT_CTRL_H);
	menu.Add(t_("Open link..."), CtrlImg::open(), [=] { WhenLink(uri); })
		.Key(K_SHIFT_CTRL_O);
}

void Terminal::ImagesBar(Bar& menu)
{
	// TODO
}

void Terminal::OptionsBar(Bar& menu)
{
	menu.Add(t_("Read only"), [=] { SetEditable(IsReadOnly()); })
		.Key(K_SHIFT_CTRL_L)
		.Check(IsReadOnly());
	menu.Add(t_("Scrollbar"), [=] { ShowScrollBar(!sb.IsChild()); })
		.Key(K_SHIFT_CTRL_S)
		.Check(sb.IsChild());
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
	// Note: The U++ color constants with 'S' prefix are automatically adjusted
	//       to the color theme of OS. On the other hand, the 8 ANSI colors and
	//       their brighter  counterparts are assumed to be constant. Therefore
	//       it would be better if we avoid using the auto-adjusted versions by
	//       default, and leave it up to client code to change them  on demand.
	//       Note that this rule does not apply to the  default ink, paper, and
	//       selection colors.
	
	colortable[COLOR_BLACK] = Black();
	colortable[COLOR_RED] = Red();
	colortable[COLOR_GREEN] = Green();
	colortable[COLOR_YELLOW] = Yellow();
	colortable[COLOR_BLUE] = Blue();
	colortable[COLOR_MAGENTA] = Magenta();
	colortable[COLOR_CYAN] = Cyan();
	colortable[COLOR_WHITE] = White();
	colortable[COLOR_LTBLACK] = Gray();
	colortable[COLOR_LTRED] = LtRed();
	colortable[COLOR_LTGREEN] = LtGreen();
	colortable[COLOR_LTYELLOW] = LtYellow();
	colortable[COLOR_LTBLUE] = LtBlue();
	colortable[COLOR_LTMAGENTA] = LtMagenta();
	colortable[COLOR_LTCYAN] = LtCyan();
	colortable[COLOR_LTWHITE] = White();
	colortable[COLOR_INK] = SColorText;
	colortable[COLOR_INK_SELECTED] = SColorHighlightText;
	colortable[COLOR_PAPER] = SColorPaper;
	colortable[COLOR_PAPER_SELECTED] = SColorHighlight;
	return *this;
}

void Terminal::ReportWindowProperties(int opcode)
{
	Rect r;
	Size sz;
	TopWindow *win = GetTopWindow();
	
	switch(opcode) {
	case WINDOW_REPORT_POSITION:
		if(!win) return;
		r = win->GetRect();
		PutCSI(Format("3;%d;%d`t", r.left, r.top));
		break;
	case WINDOW_REPORT_VIEW_POSITION:
		r = GetView();
		PutCSI(Format("3;%d;%d`t", r.left, r.top));
		break;
	case WINDOW_REPORT_SIZE:
		sz = GetSize();
		PutCSI(Format("4;%d;%d`t", sz.cy, sz.cx));
		break;
	case WINDOW_REPORT_VIEW_SIZE:
		sz = GetView().GetSize();
		PutCSI(Format("4;%d;%d`t", sz.cy, sz.cx));
		break;
	case WINDOW_REPORT_PAGE_SIZE:
		sz = GetPageSize();
		PutCSI(Format("8;%d;%d`t", sz.cy, sz.cx));
		break;
	case WINDOW_REPORT_CELL_SIZE:
		sz = GetFontSize();
		PutCSI(Format("6;%d;%d`t", sz.cy, sz.cx));
		break;
	case WINDOW_REPORT_STATE:
		if(!win) return;
		PutCSI(Format("%d`t", win->IsMinimized() ? 2 : 1));
		break;
	case WINDOW_REPORT_TITLE:
		PutOSC("1;");	// Always empty for security reasons.
		break;
	default:
		LLOG("Unhandled window op: " << opcode);
		return;
	}
}

Image Terminal::CursorImage(Point p, dword keyflags)
{
	if(IsTracking())
		return Image::Arrow();
	else
	if(IsMouseOverHyperlink())
		return CtrlImg::HandCursor();
	else
		return Image::IBeam();
}

void Terminal::State(int reason)
{
	if(reason == Ctrl::OPEN)
		WhenResize();
}

void Terminal::Serialize(Stream& s)
{
	GuiLock __;
	
	int version = 1;
	s / version;
	if(version >= 1) {
		s % font;
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
		s % metakeyflags;
		Console::Serialize(s);
	}
	if(s.IsLoading())
		Layout();
}
}