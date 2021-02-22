#include "Terminal.h"

#define LLOG(x)		// RLOG("TerminalCtrl: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

TerminalCtrl::TerminalCtrl()
: page(&dpage)
, legacycharsets(false)
, eightbit(false)
, windowactions(false)
, windowreports(false)
, sixelimages(false)
, jexerimages(false)
, iterm2images(false)
, hyperlinks(false)
, reversewrap(false)
, hidemousecursor(false)
, sizehint(true)
, delayedrefresh(true)
, lazyresize(false)
, blinkingtext(true)
, adjustcolors(false)
, lightcolors(false)
, dynamiccolors(false)
, intensify(false)
, nobackground(false)
, alternatescroll(false)
, keynavigation(true)
, userdefinedkeys(false)
, userdefinedkeyslocked(true)
, pcstylefunctionkeys(false)
, streamfill(false)
{
	Unicode();
	SetLevel(LEVEL_4);
	SetCharset(CHARSET_UNICODE);
	InitParser(parser);
	SetImageDisplay(NormalImageCellDisplay());
	SetFrame(FieldFrame());
	History();
	ResetColors();
	HideScrollBar();
	WhenBar = [=](Bar& menu) { StdBar(menu); };
	sb.WhenScroll = [=]() { Scroll(); };
	caret.WhenAction = [=]() { PlaceCaret(); };
	dpage.WhenUpdate = [=]() { ScheduleRefresh(); };
	apage.WhenUpdate = [=]() { ScheduleRefresh(); };
}

TerminalCtrl::~TerminalCtrl()
{
	// Make sure that no callback is left dangling...
	KillTimeCallback(TIMEID_REFRESH);
	KillTimeCallback(TIMEID_SIZEHINT);
	KillTimeCallback(TIMEID_BLINK);
}

Size TerminalCtrl::GetFontSize() const
{
	return Size(max(font.GetWidth('M'), font.GetWidth('W')), font.GetCy());
}

Size TerminalCtrl::GetPageSize() const
{
	Size wsz = GetSize();
	Size fsz = GetFontSize();
	return clamp(wsz / fsz, Size(1, 1), GetScreenSize() / fsz);
}

void TerminalCtrl::PlaceCaret(bool scroll)
{
	bool  b = modes[DECTCEM];

	if(!b || !caret.IsBlinking()) {
		KillCaret();
		if(!b) return;
	}
	if(caret.IsBlinking()) {
		SetCaret(GetCaretRect());
	}
	else {
		Refresh(caretrect);
		Refresh(GetCaretRect());
	}
	if(scroll && IsDefaultPage()) {
		sb.ScrollInto(GetCursorPos().y);
	}
}

Rect TerminalCtrl::GetCaretRect()
{
	Size fsz = GetFontSize();
	Point pt = GetCursorPos() * fsz;
	int   cw = page->GetCell().GetWidth();

	pt.y -= (fsz.cy * GetSbPos());

	switch(caret.GetStyle()) {
	case Caret::BEAM:
		fsz.cx = 1;
		break;
	case Caret::UNDERLINE:
		fsz.cx *= max(cw, 1); // Adjust the caret widt to cell size.
		pt.y += fsz.cy - 1;
		fsz.cy = 1;
		break;
	case Caret::BLOCK:
		fsz.cx *= max(cw, 1); // Adjust the caret width to cell size.
		break;
	}

	caretrect = Rect(pt, fsz);
	return Rect(GetSize()).Contains(caretrect) ? caretrect : Null;
}

void TerminalCtrl::Copy(const WString& s)
{
	if(!IsNull(s)) {
		ClearClipboard();
		AppendClipboardUnicodeText(s);
		AppendClipboardText(s.ToString());
	}
}

void TerminalCtrl::Paste(const WString& s, bool filter)
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

void TerminalCtrl::SelectAll(bool history)
{
	Size psz = GetPageSize();
	bool h = IsDefaultPage() && history;
	Rect r = RectC(0, h ? 0 : sb, psz.cx, (h ? sb + (sb.GetTotal() - sb) : psz.cy) - 1);
	anchor = r.TopLeft();
	selpos = r.BottomRight();
	Refresh();
}

String TerminalCtrl::GetSelectionData(const String& fmt) const
{
	return IsSelection() ? GetTextClip(GetSelectedText().ToString(), fmt) : Null;
}

void TerminalCtrl::SyncSize(bool notify)
{
	// Apparently, the window minimize event on Windows "really" minimizes
	// the window. This results in a damaged terminal display. In order to
	// avoid this, we check the  new page size, and  do not attempt resize
	// the page if the requested page size is < 2 x 2 cells.

	Size newsize = GetPageSize();
	resizing = page->GetSize() != newsize;

	auto OnSizeHint	= [=]
	{
		RefreshSizeHint();
		hinting = false;
	};
	
	auto OnResize = [=]
	{
		resizing = false;
		WhenResize();
	};
	
	if(resizing && newsize.cx > 1 && 1 < newsize.cy) {
		page->SetSize(newsize);
		if(notify) {
			if(sizehint) {
				hinting = true;
				KillSetTimeCallback(1000, OnSizeHint, TIMEID_SIZEHINT);
			}
			if(lazyresize)
				KillSetTimeCallback(100, OnResize, TIMEID_REFRESH);
			else
				OnResize();
		}
		else
			resizing = false;
	}
}

void TerminalCtrl::ScheduleRefresh()
{
	if(delayedrefresh
	&& (!lazyresize || !resizing)
	&& !ExistsTimeCallback(TIMEID_REFRESH))  // Don't cancel a pending refresh.
		SetTimeCallback(16, [=] { SyncSb(); RefreshDisplay(); }, TIMEID_REFRESH);
}

Tuple<String, Rect> TerminalCtrl::GetSizeHint(Rect r, Size sz)
{
	Tuple<String, Rect> hint;
	hint.a << sz.cx << " x " << sz.cy;
	hint.b = r.CenterRect(GetTextSize(hint.a, StdFont()));
	return pick(hint);
}

void TerminalCtrl::SyncSb()
{
	if(IsAlternatePage())
		return;

	sb.SetTotal(page->GetLineCount());
	sb.SetPage(page->GetSize().cy);
	sb.SetLine(1);

	if(!ignorescroll)
		sb.End();
}

void TerminalCtrl::Scroll()
{
	// It is possible to  have  an  alternate screen buffer with a history  buffer.
	// Some terminal  emulators already  come  with  this feature enabled. Terminal
	// ctrl can  also support this  feature out-of-the-boz, as it uses  the  VTPage
	// class for both its default and alternate screen buffers. Thus the difference
	// is only semantic and practical. At the  moment, however, this feature is n0t
	// enabled. This may change in the future.

	if(IsAlternatePage())
		return;

	if(hinting) // Prevents the size hint box from scrolling with the view.
		RefreshSizeHint();
	
	scroller.Scroll(*this, GetSize(), sb * GetFontSize().cy);
	PlaceCaret();
}

void TerminalCtrl::SwapPage()
{
	SyncSize(false);
	SyncSb();
	ClearSelection();
}

void TerminalCtrl::RefreshDisplay()
{
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	int  pos = GetSbPos();
	int blinking_cells = 0;
	
	LTIMING("TerminalCtrl::RefreshDisplay");

	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); i++) {
		const VTLine& line = page->FetchLine(i);
		int y = i * fsz.cy - (fsz.cy * pos);
		for(int j = 0; j < line.GetCount(); j++) {
			int x = j * fsz.cx;
			const VTCell& cell = line[j];
			if(hyperlinks && cell.IsHyperlink()
				&& (cell.data == activelink || cell.data == prevlink)) {
					if(!line.IsInvalid())
						Refresh(RectC(x, y, fsz.cx, fsz.cy).Inflated(4));
			}
			else
			if(blinkingtext && cell.IsBlinking()) {
				if(!line.IsInvalid())
					Refresh(RectC(x, y, fsz.cx, fsz.cy).Inflated(4));
				blinking_cells++;
			}
		}
		if(line.IsInvalid()) {
			line.Validate();
			Refresh(RectC(0, i * fsz.cy - (fsz.cy * pos), wsz.cx, fsz.cy).Inflated(4));
		}
	}

	PlaceCaret();
	Blink(blinking_cells > 0);
}

void TerminalCtrl::Blink(bool b)
{
	bool bb = ExistsTimeCallback(TIMEID_BLINK);
	if(blinkingtext && b && !bb)
		SetTimeCallback(-blinkinterval, [=]{ blinking ^= 1; RefreshDisplay(); }, TIMEID_BLINK);
	else
	if(!blinkingtext || !b) {
		blinking = false;
		if(bb)
			KillTimeCallback(TIMEID_BLINK);
	}
}

void TerminalCtrl::DragAndDrop(Point pt, PasteClip& d)
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

void TerminalCtrl::LeftDown(Point pt, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(pt, LEFTDOWN, keyflags);
	else{
		if(IsSelected(ClientToPagePos(pt))) {
			return;
		}
		else {
			pt = SelectionToPagePos(pt);
			SetSelection(pt, pt, (keyflags & K_CTRL) ? SEL_RECT : SEL_TEXT);
		}
	}
	SetCapture();
}

void TerminalCtrl::LeftUp(Point pt, dword keyflags)
{
	if(IsTracking()) {
		if(!modes[XTX10MM])
			VTMouseEvent(pt, LEFTUP, keyflags);
	}
	else {
		pt = ClientToPagePos(pt);
		if(multiclick)
			multiclick = false;
		else
		if(!HasCapture() && IsSelected(pt))
			ClearSelection();
	}
	ReleaseCapture();
}

void TerminalCtrl::LeftDrag(Point pt, dword keyflags)
{
	pt = ClientToPagePos(pt);
	bool modifier = keyflags & K_CTRL;
	
	if(!IsTracking()) {
		VectorMap<String, ClipData> data;
		if(!HasCapture() && !modifier && IsSelected(pt)) {
			WString tsample = GetSelectedText();
			Append(data, tsample);
			Size tsz = StdSampleSize();
			ImageDraw iw(tsz);
			iw.DrawRect(tsz, Black());
			iw.Alpha().DrawRect(tsz, Black());
			DrawTLText(iw.Alpha(), 0, 0, tsz.cx, tsample, font, White());
			DoDragAndDrop(data, iw, DND_COPY);
		}
		else
		if(modifier && IsMouseOverHyperlink(pt)) {
			WString lsample = GetHyperlinkURI(pt, modifier).ToWString();
			Append(data, lsample);
			Size lsz = StdSampleSize();
			ImageDraw iw(lsz);
			iw.DrawRect(lsz, Black());
			iw.Alpha().DrawRect(lsz, Black());
			DrawTLText(iw.Alpha(), 0, 0, lsz.cx, lsample, font, White());
			DoDragAndDrop(data, iw, DND_COPY);
			ClearSelection();
		}
		else
		if(modifier && IsMouseOverImage(pt)) {
		// Unfortunately, Turtle and VirtualGui (e.g. linux framebuffer)
		// backends do  not support image  drag-and-drop, at the moment.
		#if !defined(TURTLE) && !defined(VIRTUALGUI)
			Image isample = GetInlineImage(pt, modifier);
			Append(data, isample);
			Size isz = GetFitSize(isample.GetSize(), StdSampleSize());
			ImageDraw iw(isz);
			iw.DrawImage(isz, isample);
			DoDragAndDrop(data, iw, DND_COPY);
			ClearSelection();
		#endif
		}
	}
}

void TerminalCtrl::LeftDouble(Point pt, dword keyflags)
{
	if(IsTracking())
		Ctrl::LeftDouble(pt, keyflags);
	else {
		ClearSelection();
		pt = ClientToPagePos(pt);
		if((keyflags & K_CTRL) == K_CTRL) {
			if(IsMouseOverImage(pt)) {
				Image img = GetInlineImage(pt, true);
				if(!IsNull(img))
					WhenImage(PNGEncoder().SaveString(img));
			}
			else
			if(IsMouseOverHyperlink(pt)) {
				String uri = GetHyperlinkURI(pt, true);
				if(!IsNull(uri))
					WhenLink(uri);
			}
		}
		else {
			Point pl, ph;
			if(GetWordSelection(pt, pl, ph)) {
				SetSelection(pl, ph, SEL_WORD);
				multiclick = true;
			}
		}
	}
}

void TerminalCtrl::LeftTriple(Point pt, dword keyflags)
{
	if(IsTracking())
		Ctrl::LeftTriple(pt, keyflags);
	else {
		ClearSelection();
		Point pl, ph;
		GetLineSelection(ClientToPagePos(pt), pl, ph);
		SetSelection(pl, ph, SEL_LINE);
		multiclick = IsSelection();
	}
}

void TerminalCtrl::MiddleDown(Point pt, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(pt, MIDDLEDOWN, keyflags);
	else {
		WString w;
		if(IsSelection())
			w = GetSelectedText();
		else
		if(AcceptText(Selection()))
			w = GetWString(Selection());
		if(!IsNull(w))
			Paste(w);
	}
}

void TerminalCtrl::MiddleUp(Point pt, dword keyflags)
{
	if(IsTracking() && !modes[XTX10MM])
		VTMouseEvent(pt, MIDDLEUP, keyflags);
}

void TerminalCtrl::RightDown(Point pt, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(pt, RIGHTDOWN, keyflags);
	else {
		pt = ClientToPagePos(pt);
		if(!IsSelected(pt))
			ClearSelection();
		MenuBar::Execute(WhenBar);
	}
}

void TerminalCtrl::RightUp(Point pt, dword keyflags)
{
	if(IsTracking() && !modes[XTX10MM])
		VTMouseEvent(pt, RIGHTUP, keyflags);
}

void TerminalCtrl::MouseMove(Point pt, dword keyflags)
{
	auto sGetMouseMotionEvent = [](bool b) -> dword
	{
		if(!b) return Ctrl::MOUSEMOVE;
		if(GetMouseLeft()) return Ctrl::LEFTDRAG;
		if(GetMouseRight()) return Ctrl::RIGHTDRAG;
		if(GetMouseMiddle()) return Ctrl::MIDDLEDRAG;
		return 0;
	};

	pt = GetView().Bind(pt);
	bool captured = HasCapture();

	if(IsTracking()) {
		if((modes[XTDRAGM] && captured) || modes[XTANYMM])
			VTMouseEvent(pt, sGetMouseMotionEvent(captured), keyflags);
	}
	else
	if(captured) {
		selpos = SelectionToPagePos(pt);
		Refresh();
	}
	else
	if(hyperlinks) {
		HighlightHyperlink(ClientToPagePos(pt));
	}
}

void TerminalCtrl::MouseWheel(Point pt, int zdelta, dword keyflags)
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
			VTMouseEvent(pt, MOUSEWHEEL, keyflags, zdelta);
	}
}

Image TerminalCtrl::MouseEvent(int event, Point pt, int zdelta, dword keyflags)
{
	if(hidemousecursor) {
		if(mousehidden && event == Ctrl::CURSORIMAGE)
			return Null;
		else mousehidden = false;
	}
	return Ctrl::MouseEvent(event, pt, zdelta, keyflags);
}

void TerminalCtrl::VTMouseEvent(Point pt, dword event, dword keyflags, int zdelta)
{
	int  mouseevent = 0;

	if(!modes[XTSGRPXMM])
		pt = ClientToPagePos(pt) + 1;

	switch(event) {
	case LEFTUP:
	case LEFTDOWN:
		mouseevent = 0x00;
		break;
	case LEFTDRAG:
		if(pt == mousepos)
			return;
		mouseevent = 0x20;
		break;
	case MIDDLEUP:
	case MIDDLEDOWN:
		mouseevent = 0x01;
		break;
	case MIDDLEDRAG:
		if(pt == mousepos)
			return;
		mouseevent = 0x21;
		break;
	case RIGHTUP:
	case RIGHTDOWN:
		mouseevent = 0x02;
		break;
	case RIGHTDRAG:
		if(pt == mousepos)
			return;
		mouseevent = 0x22;
		break;
	case MOUSEMOVE:
		if(pt == mousepos)
			return;
		mouseevent = 0x23;
		break;
	case MOUSEWHEEL:
		mouseevent = zdelta > 0 ? 0x40 : 0x41;
		break;
	default:
		ReleaseCapture();
		return;
	}

	mousepos = pt;

	if(keyflags & K_SHIFT) mouseevent |= 0x04;
	if(keyflags & K_ALT)   mouseevent |= 0x08;
	if(keyflags & K_CTRL)  mouseevent |= 0x10;

	bool buttondown = false;

	if((event & UP) == UP) {
		if(HasCapture())
			ReleaseCapture();
	}
	else {
		buttondown = true;	// Combines everything else with a button-down event
		if((event & DOWN) == DOWN)
			if(!HasCapture())
				SetCapture();
	}

	if(modes[XTSGRMM] || modes[XTSGRPXMM]) {
		PutCSI(Format("<%d;%d;%d%[1:M;m]s", mouseevent, pt.x, pt.y, buttondown));
	}
	else {
		if(!buttondown)
			mouseevent = 0x03;
		mouseevent += 0x20;
		pt += 0x20;
		// Note: We can't use PutCSI method to send X11 and UTF mouse coordinates here as
		// it won't pass values >= 128 unmodified, unless the terminal is in  8-bit mode.
		if(modes[XTUTF8MM]) {
			WString s;
			s.Cat(clamp(pt.x, 32, 2047));
			s.Cat(clamp(pt.y, 32, 2047));
			PutRaw(Format("\033[M%c%s", mouseevent, ToUtf8(s))).Flush();
		}
		else {
			pt.x = clamp(pt.x, 32, 255);
			pt.y = clamp(pt.y, 32, 255);
			PutRaw(Format("\033[M%c%c%c", mouseevent, pt.x, pt.y)).Flush();
		}
	}
}

bool TerminalCtrl::IsTracking() const
{
	return modes[XTX10MM]
		|| modes[XTX11MM]
		|| modes[XTANYMM]
		|| modes[XTDRAGM];
}

Point TerminalCtrl::ClientToPagePos(Point pt) const
{
	Size wsz = GetSize();
	Size psz = GetPageSize();
	pt.y = psz.cy * pt.y / (wsz.cy -= wsz.cy % psz.cy) + GetSbPos();
	pt.x = psz.cx * pt.x / (wsz.cx -= wsz.cx % psz.cx);
	return pt;
}

Point TerminalCtrl::SelectionToPagePos(Point pt) const
{
	// Aligns the anchor or selection point to cell boundaries.

	Size fsz = GetFontSize();
	int mx = pt.x % fsz.cx;
	pt.x += int(mx >= fsz.cx / 2) * fsz.cx - mx;
	return ClientToPagePos(pt);
}

void TerminalCtrl::SetSelection(Point pl, Point ph, dword type)
{
	anchor = pl;
	selpos = ph;
	seltype = type;
	SetSelectionSource(ClipFmtsText());
	Refresh();
}

bool TerminalCtrl::GetSelection(Point& pl, Point& ph) const
{
	if(IsNull(anchor) || anchor == selpos) {
		pl = ph = selpos;
		return false;
	}
	
	if(anchor.y == selpos.y || anchor.x == selpos.x || seltype == SEL_RECT) {
		pl = min(anchor, selpos);
		ph = max(anchor, selpos);
	}
	else
	if(anchor.y > selpos.y) {
		pl = selpos;
		ph = anchor;
	}
	else {
		pl = anchor;
		ph = selpos;
	}

	if(seltype == SEL_LINE) {
		// Updates the horizontal highlight on display resize.
		ph.x = GetPageSize().cx;
	}
	
	return true;
}

Rect TerminalCtrl::GetSelectionRect() const
{
	Point pl, ph;
	return GetSelection(pl, ph) ? Rect(pl, ph) : Null;
}

void TerminalCtrl::ClearSelection()
{
	ReleaseCapture();
	anchor = Null;
	selpos = Null;
	seltype = SEL_NONE;
	multiclick = false;
	Refresh();
}

bool TerminalCtrl::IsSelected(Point pt) const
{
	Point pl, ph;
	if(!GetSelection(pl, ph))
		return false;

	if(seltype == SEL_RECT) {
		return pt.x >= pl.x
			&& pt.y >= pl.y
			&& pt.x <  ph.x
			&& pt.y <= ph.y;
	}
	else
	if(pl.y == ph.y) {
		return pt.y == pl.y
			&& pt.x >= pl.x
			&& pt.x <  ph.x;
	}
	else
	if(pt.y == pl.y) {
		Size psz = GetPageSize();
		return pt.x >= pl.x
			&& pt.x <  psz.cx;
	}
	else
	if(pt.y == ph.y) {
		return pt.x >= 0 && pt.x < ph.x;
	}

	return pl.y <= pt.y && pt.y <= ph.y;
}

WString TerminalCtrl::GetSelectedText() const
{
	return AsWString((const VTPage&)*page, GetSelectionRect(), seltype & SEL_RECT);
}

void TerminalCtrl::GetLineSelection(const Point& pt, Point& pl, Point& ph) const
{
	pl = ph = pt;
	pl.x = 0;
	ph.x = GetPageSize().cx;
	int cy = page->GetLineCount();
	
	while(pl.y > 0 && page->FetchLine(pl.y - 1).IsWrapped())
		pl.y--;
	while(ph.y < cy && page->FetchLine(ph.y).IsWrapped())
		ph.y++;
}

bool TerminalCtrl::GetWordSelection(const Point& pt, Point& pl, Point& ph) const
{
	pl = ph = pt;

	const VTLine& line = page->FetchLine(pt.y);
	if(!line.IsVoid()) {
		const VTCell& cell = line[pt.x];
		if(!cell.IsImage() && (cell.chr == 1 || cell.chr >= 32)) {
			ph.x++;
			if(IsLeNum(cell.chr) || cell.chr == '_') {
				GetWordPosL(line, pl);
				GetWordPosH(line, ph);
			}
			return true;
		}
	}
	return false;
}

bool IsWCh(const VTCell& cell, bool line_wrap)
{
	return !cell.IsImage()
		&& (IsLeNum(cell) || findarg(cell, 1, '_', '-') >= 0 || (cell == 0 && line_wrap));
}

void TerminalCtrl::GetWordPosL(const VTLine& line, Point& pl) const
{
	bool stopped = false;
	bool wrapped = line.IsWrapped();

	while(pl.x > 0 && !(stopped = !IsWCh(line[pl.x - 1], wrapped)))
		pl.x--;

	if(pl.x == 0 && !stopped) {
		const VTLine& prev = page->FetchLine(pl.y - 1);
		if(prev.IsWrapped()) {
			pl.x = prev.GetCount();
			pl.y--;
			GetWordPosL(prev, pl);
		}
	}
}

void TerminalCtrl::GetWordPosH(const VTLine& line, Point& ph) const
{
	bool stopped = false;
	bool wrapped = line.IsWrapped();

	while(ph.x < line.GetCount() && !(stopped = !IsWCh(line[ph.x], wrapped)))
		ph.x++;

	if(ph.x == line.GetCount() && !stopped) {
		const VTLine& next = page->FetchLine(ph.y + 1);
		if(line.IsWrapped()) {
			ph.x = 0;
			ph.y++;
			GetWordPosH(next, ph);
		}
	}
}

Image TerminalCtrl::GetInlineImage(Point pt, bool modifier)
{
	if(modifier) {
		const VTCell& cell = page->FetchCell(pt);
		if(cell.IsImage()) {
			Image img = GetCachedImageData(cell.chr, Null, GetFontSize()).image;
			if(!IsNull(img))
				return pick(img);
			LLOG("Unable to retrieve image from cache. Link id: " << cell.chr);
		}
	}
	return Null;
}

String TerminalCtrl::GetHyperlinkURI(Point pt, bool modifier)
{
	if(modifier) {
		const VTCell& cell = page->FetchCell(pt);
		if(cell.IsHyperlink()) {
			String uri = GetCachedHyperlink(cell.data);
			if(!IsNull(uri))
				return uri;
			LLOG("Unable to retrieve URI from link cache. Link id: " << cell.data);
		}
	}
	return Null;
}

void TerminalCtrl::HighlightHyperlink(Point pt)
{
	if(mousepos != pt) {
		mousepos = pt;
		const VTCell& cell = page->FetchCell(pt);
		if(cell.IsHyperlink() || activelink > 0) {
			if(cell.data != activelink) {
				prevlink = activelink;
				activelink = cell.data;
				RefreshDisplay();
			}
			String lnk = GetCachedHyperlink(activelink);
			Tip(UrlDecode(lnk));
		}
	}
}

void TerminalCtrl::StdBar(Bar& menu)
{
	menu.Sub(t_("Options"), [=](Bar& menu) { OptionsBar(menu); });
	menu.Separator();
	menu.Add(t_("Read only"), [=] { SetEditable(IsReadOnly()); })
		.Key(K_SHIFT_CTRL_L)
		.Check(IsReadOnly());
	if(IsMouseOverImage()) {
		menu.Separator();
		ImagesBar(menu);
	}
	else
	if(IsMouseOverHyperlink()) {
		menu.Separator();
		LinksBar(menu);
	}
	else {
		menu.Separator();
		EditBar(menu);
	}
}

void TerminalCtrl::EditBar(Bar& menu)
{
	menu.Add(IsSelection(), t_("Copy"), CtrlImg::copy(),  [=] { Copy();  })
		.Key(K_SHIFT_CTRL_C);
	menu.Add(IsEditable(), t_("Paste"), CtrlImg::paste(), [=] { Paste(); })
		.Key(K_SHIFT_CTRL_V);
	menu.Separator();
	menu.Add(t_("Select all"), CtrlImg::select_all(), [=] { SelectAll(); })
		.Key(K_SHIFT_CTRL_A);
}

void TerminalCtrl::LinksBar(Bar& menu)
{
	String uri = GetHyperlinkUri();
	if(IsNull(uri))
		return;

	menu.Add(t_("Copy link to clipboard"), CtrlImg::copy(), [=] { Copy(uri.ToWString()); })
		.Key(K_SHIFT_CTRL_H);
	menu.Add(t_("Open link..."), CtrlImg::open(), [=] { WhenLink(uri); })
		.Key(K_SHIFT_CTRL_O);
}

void TerminalCtrl::ImagesBar(Bar& menu)
{
	Point pt = mousepos;

	menu.Add(t_("Copy image to clipboard"), CtrlImg::copy(), [=]
		{
			Image img = GetInlineImage(pt, true);
			if(!IsNull(img))
				AppendClipboardImage(img);
		})
		.Key(K_SHIFT_CTRL_H);
	menu.Add(t_("Open image..."), CtrlImg::open(), [=]
		{
			Image img = GetInlineImage(pt, true);
			if(!IsNull(img))
				WhenImage(PNGEncoder().SaveString(img));
		})
		.Key(K_SHIFT_CTRL_O);
}

void TerminalCtrl::OptionsBar(Bar& menu)
{
	bool inlineimages = jexerimages || sixelimages || iterm2images;

	menu.Sub(t_("Cursor style"), [=](Bar& menu)
		{
			byte cstyle   = caret.GetStyle();
			bool unlocked = !caret.IsLocked();
			menu.Add(unlocked,
				t_("Block"),
				[=] { caret.Block(caret.IsBlinking()); })
				.Radio(cstyle == Caret::BLOCK);
			menu.Add(unlocked,
				t_("Beam"),
				[=] { caret.Beam(caret.IsBlinking()); })
				.Radio(cstyle == Caret::BEAM);
			menu.Add(unlocked,
				t_("Underline"),
				[=] { caret.Underline(caret.IsBlinking()); })
				.Radio(cstyle == Caret::UNDERLINE);
			menu.Separator();
			menu.Add(unlocked,
				t_("Blinking"),
				[=] { caret.Blink(!caret.IsBlinking());	 })
				.Check(caret.IsBlinking());
			menu.Separator();
			menu.Add(t_("Locked"),
				[=] { caret.Lock(!caret.IsLocked()); })
				.Check(!unlocked);
		});
	menu.Separator();
	menu.Add(t_("Scrollbar"),
		[=] { ShowScrollBar(!sb.IsChild()); })
		.Key(K_SHIFT_CTRL_S)
		.Check(sb.IsChild());
	menu.Add(t_("Auto-hide mouse cursor"),
		[=] { AutoHideMouseCursor(!hidemousecursor); })
		.Key(K_SHIFT_CTRL_M)
		.Check((hidemousecursor));
	menu.Add(t_("Alternate scroll"),
		[=] { AlternateScroll(!alternatescroll); })
		.Key(K_SHIFT|K_ALT_S)
		.Check(alternatescroll);
	menu.Add(t_("Key navigation"),
		[=] { KeyNavigation(!keynavigation); })
		.Key(K_SHIFT_CTRL_K)
		.Check(keynavigation);
	menu.Add(t_("PC-style function keys"),
		[=] { PCStyleFunctionKeys(!pcstylefunctionkeys); })
		.Key(K_SHIFT_CTRL_P)
		.Check(pcstylefunctionkeys);
	menu.Add(t_("Dynamic colors"),
		[=] { DynamicColors(!dynamiccolors); })
		.Key(K_SHIFT_CTRL_D)
		.Check(dynamiccolors);
	menu.Add(t_("Light colors"),
		[=] { LightColors(!lightcolors); })
		.Key(K_SHIFT|K_ALT_L)
		.Check(lightcolors);
	menu.Add(t_("Adjust to dark themes"),
		[=] { AdjustColors(!adjustcolors); })
		.Key(K_SHIFT|K_ALT_D)
		.Check(adjustcolors);
	menu.Add(t_("Blinking text"),
		[=] { BlinkingText(!blinkingtext); })
		.Key(K_SHIFT_CTRL_B)
		.Check(blinkingtext);
	menu.Add(t_("Hyperlinks"),
		[=] { Hyperlinks(!hyperlinks); })
		.Key(K_SHIFT|K_ALT_H)
		.Check(hyperlinks);
	menu.Add(t_("Inline images"),
		[=] { InlineImages(!inlineimages); })
		.Key(K_SHIFT_CTRL_I)
		.Check(inlineimages);
	menu.Add(t_("Size hint"),
		[=] { ShowSizeHint(!sizehint); })
		.Key(K_SHIFT_CTRL_W)
		.Check(sizehint);
	menu.Add(t_("Buffered refresh"),
		[=] { DelayedRefresh(!delayedrefresh); })
		.Key(K_SHIFT_CTRL_Z)
		.Check(delayedrefresh);
	menu.Add(t_("Lazy resize"),
		[=] { LazyResize(!lazyresize); })
		.Key(K_SHIFT_CTRL_Z)
		.Check(lazyresize);
}

TerminalCtrl& TerminalCtrl::ShowScrollBar(bool b)
{
	GuiLock __;

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

Image TerminalCtrl::CursorImage(Point p, dword keyflags)
{
	if(IsTracking())
		return Image::Arrow();
	else
	if(IsMouseOverHyperlink())
		return Image::Hand();
	else
		return Image::IBeam();
}

void TerminalCtrl::State(int reason)
{
	if(reason == Ctrl::OPEN)
		WhenResize();
}

TerminalCtrl::Caret::Caret()
: style(BLOCK)
, blinking(true)
, locked(false)
{
}

TerminalCtrl::Caret::Caret(int style_, bool blink, bool lock)
{
	Set(style_, blink);
	locked = lock;
}

void TerminalCtrl::Caret::Set(int style_, bool blink)
{
	if(!locked) {
		style = clamp(style_, int(BLOCK), int(UNDERLINE));
		blinking = blink;
		WhenAction();
	}
}

void TerminalCtrl::Caret::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % style;
		s % locked;
		s % blinking;
	}
}

void TerminalCtrl::Caret::Jsonize(JsonIO& jio)
{
	jio ("Style", style)
		("Locked", locked)
		("Blinking", blinking);
}

void TerminalCtrl::Caret::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

INITBLOCK
{
	Value::Register<TerminalCtrl::InlineImage>();
}

}