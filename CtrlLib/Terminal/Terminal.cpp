#include "Terminal.h"

#define LLOG(x)		// RLOG("Terminal: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

Terminal::Terminal()
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
	WhenBar = THISFN(StdBar);
	sb.WhenScroll = THISFN(Scroll);
	caret.WhenAction = [=] { PlaceCaret(); };
	dpage.WhenScroll = THISFN(ScheduleRefresh);
	apage.WhenScroll = THISFN(ScheduleRefresh);
}

Terminal::~Terminal()
{
	// Make sure that no callback is left dangling...
	KillTimeCallback(TIMEID_REFRESH);
	KillTimeCallback(TIMEID_SIZEHINT);
	KillTimeCallback(TIMEID_BLINK);
}

Size Terminal::GetFontSize() const
{
	return Size(max(font.GetWidth('M'), font.GetWidth('W')), font.GetCy());
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

Rect Terminal::GetCaretRect()
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

void Terminal::Copy(const WString& s)
{
	if(!IsNull(s)) {
		ClearClipboard();
		AppendClipboardUnicodeText(s);
		AppendClipboardText(s.ToString());
	}
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
	anchor = r.TopLeft();
	selpos = r.BottomRight();
	Refresh();
}

String Terminal::GetSelectionData(const String& fmt) const
{
	return IsSelection() ? GetTextClip(GetSelectedText().ToString(), fmt) : Null;
}

void Terminal::SyncSize(bool notify)
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

void Terminal::ScheduleRefresh()
{
	if(delayedrefresh
	&& (!lazyresize || !resizing)
	&& !ExistsTimeCallback(TIMEID_REFRESH))  // Don't cancel a pending refresh.
		SetTimeCallback(16, [=] { SyncSb(); RefreshDisplay(); }, TIMEID_REFRESH);
}

Tuple<String, Rect> Terminal::GetSizeHint(Rect r, Size sz)
{
	Tuple<String, Rect> hint;
	hint.a << sz.cx << " x " << sz.cy;
	hint.b = r.CenterRect(GetTextSize(hint.a, StdFont()));
	return pick(hint);
}

void Terminal::SyncSb()
{
	if(IsAlternatePage())
		return;

	sb.SetTotal(page->GetLineCount());
	sb.SetPage(page->GetSize().cy);
	sb.SetLine(1);

	if(!ignorescroll)
		sb.End();
}

void Terminal::Scroll()
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

void Terminal::SwapPage()
{
	SyncSize(false);
	SyncSb();
	ClearSelection();
}

void Terminal::RefreshDisplay()
{
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	int  pos = GetSbPos();
	int blinking_cells = 0;
	
	LTIMING("Terminal::RefreshDisplay");

	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); i++) {
		const VTLine& line = page->FetchLine(i);
		int y = i * fsz.cy - (fsz.cy * pos);
		for(int j = 0; j < line.GetCount(); j++) {
			int x = j * fsz.cx;
			const VTCell& cell = line[j];
			if(hyperlinks && cell.IsHyperlink()
				&& (cell.data == activelink || cell.data == prevlink)) {
					if(!line.IsInvalid())
						Refresh(RectC(x, y, fsz.cx, fsz.cy));
			}
			else
			if(blinkingtext && cell.IsBlinking()) {
				if(!line.IsInvalid())
					Refresh(RectC(x, y, fsz.cx, fsz.cy));
				blinking_cells++;
			}
		}
		if(line.IsInvalid()) {
			line.Validate();
			Refresh(RectC(0, i * fsz.cy - (fsz.cy * pos), wsz.cx, fsz.cy));
		}
	}

	PlaceCaret();
	Blink(blinking_cells > 0);
}

void Terminal::Blink(bool b)
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

void Terminal::DragAndDrop(Point pt, PasteClip& d)
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

void Terminal::LeftDown(Point pt, dword keyflags)
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

void Terminal::LeftUp(Point pt, dword keyflags)
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

void Terminal::LeftDrag(Point pt, dword keyflags)
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

void Terminal::LeftDouble(Point pt, dword keyflags)
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

void Terminal::LeftTriple(Point pt, dword keyflags)
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

void Terminal::MiddleDown(Point pt, dword keyflags)
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

void Terminal::MiddleUp(Point pt, dword keyflags)
{
	if(IsTracking() && !modes[XTX10MM])
		VTMouseEvent(pt, MIDDLEUP, keyflags);
}

void Terminal::RightDown(Point pt, dword keyflags)
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

void Terminal::RightUp(Point pt, dword keyflags)
{
	if(IsTracking() && !modes[XTX10MM])
		VTMouseEvent(pt, RIGHTUP, keyflags);
}

void Terminal::MouseMove(Point pt, dword keyflags)
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

void Terminal::MouseWheel(Point pt, int zdelta, dword keyflags)
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

Image Terminal::MouseEvent(int event, Point pt, int zdelta, dword keyflags)
{
	if(hidemousecursor) {
		if(mousehidden && event == Ctrl::CURSORIMAGE)
			return Null;
		else mousehidden = false;
	}
	return Ctrl::MouseEvent(event, pt, zdelta, keyflags);
}

void Terminal::VTMouseEvent(Point pt, dword event, dword keyflags, int zdelta)
{
	int  mouseevent = 0;

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

	if(modes[XTSGRMM])
		PutCSI(Format("<%d;%d;%d%[1:M;m]s", mouseevent, pt.x, pt.y, buttondown));
	else {
		if(!buttondown)
			mouseevent = 0x03;
		mouseevent += 0x20;
		pt += 0x20;
		if(modes[XTUTF8MM]) {
			WString s;
			s.Cat(pt.x);
			s.Cat(pt.y);
			PutCSI(Format("M%c%s", mouseevent, ToUtf8(s)));
		}
		else
			PutCSI(Format("M%c%c%c", mouseevent, pt.x, pt.y));
	}
}

bool Terminal::IsTracking() const
{
	return modes[XTX10MM]
		|| modes[XTX11MM]
		|| modes[XTANYMM]
		|| modes[XTDRAGM];
}

Point Terminal::ClientToPagePos(Point pt) const
{
	Size wsz = GetSize();
	Size psz = GetPageSize();
	pt.y = psz.cy * pt.y / (wsz.cy -= wsz.cy % psz.cy) + GetSbPos();
	pt.x = psz.cx * pt.x / (wsz.cx -= wsz.cx % psz.cx);
	return pt;
}

Point Terminal::SelectionToPagePos(Point pt) const
{
	// Aligns the anchor or selection point to cell boundaries.

	Size fsz = GetFontSize();
	int mx = pt.x % fsz.cx;
	pt.x += int(mx >= fsz.cx / 2) * fsz.cx - mx;
	return ClientToPagePos(pt);
}

void Terminal::SetSelection(Point pl, Point ph, dword type)
{
	anchor = pl;
	selpos = ph;
	seltype = type;
	SetSelectionSource(ClipFmtsText());
	Refresh();
}

bool Terminal::GetSelection(Point& pl, Point& ph) const
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

Rect Terminal::GetSelectionRect() const
{
	Point pl, ph;
	return GetSelection(pl, ph) ? Rect(pl, ph) : Null;
}

void Terminal::ClearSelection()
{
	ReleaseCapture();
	anchor = Null;
	selpos = Null;
	seltype = SEL_NONE;
	multiclick = false;
	Refresh();
}

bool Terminal::IsSelected(Point pt) const
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

WString Terminal::GetSelectedText() const
{
	return AsWString((const VTPage&)*page, GetSelectionRect(), seltype & SEL_RECT);
}

void Terminal::GetLineSelection(const Point& pt, Point& pl, Point& ph) const
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

bool Terminal::GetWordSelection(const Point& pt, Point& pl, Point& ph) const
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

void Terminal::GetWordPosL(const VTLine& line, Point& pl) const
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

void Terminal::GetWordPosH(const VTLine& line, Point& ph) const
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

Image Terminal::GetInlineImage(Point pt, bool modifier)
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

String Terminal::GetHyperlinkURI(Point pt, bool modifier)
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

void Terminal::HighlightHyperlink(Point pt)
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

void Terminal::StdBar(Bar& menu)
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

void Terminal::EditBar(Bar& menu)
{
	menu.Add(IsSelection(), t_("Copy"), CtrlImg::copy(),  [=] { Copy();  })
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

void Terminal::OptionsBar(Bar& menu)
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

Terminal& Terminal::ShowScrollBar(bool b)
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

Image Terminal::CursorImage(Point p, dword keyflags)
{
	if(IsTracking())
		return Image::Arrow();
	else
	if(IsMouseOverHyperlink())
		return Image::Hand();
	else
		return Image::IBeam();
}

void Terminal::State(int reason)
{
	if(reason == Ctrl::OPEN)
		WhenResize();
}

Terminal::Caret::Caret()
: style(BLOCK)
, blinking(true)
, locked(false)
{
}

Terminal::Caret::Caret(int style_, bool blink, bool lock)
{
	Set(style_, blink);
	locked = lock;
}

void Terminal::Caret::Set(int style_, bool blink)
{
	if(!locked) {
		style = clamp(style_, int(BLOCK), int(UNDERLINE));
		blinking = blink;
		WhenAction();
	}
}

void Terminal::Caret::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % style;
		s % locked;
		s % blinking;
	}
}

void Terminal::Caret::Jsonize(JsonIO& jio)
{
	jio ("Style", style)
		("Locked", locked)
		("Blinking", blinking);
}

void Terminal::Caret::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

INITBLOCK
{
	Value::Register<Terminal::InlineImage>();
}

}