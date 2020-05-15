#include "Terminal.h"

#define LLOG(x)		// RLOG("Terminal: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

static inline int sGetPos(Point pt, Size sz) // 2D -> 1D
{
	return pt.y * sz.cx + pt.x;
}

static inline Point sGetCoords(int pos, Size sz) // 1D -> 2D
{
	int x = pos % sz.cx;
	return Point(x, (pos - x) / sz.cx);
}

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
	parser.WhenChr = THISFN(PutChar);
	parser.WhenCtl = THISFN(ParseControlChars);
	parser.WhenEsc = THISFN(ParseEscapeSequences);
	parser.WhenCsi = THISFN(ParseCommandSequences);
	parser.WhenDcs = THISFN(ParseDeviceControlStrings);
	parser.WhenOsc = THISFN(ParseOperatingSystemCommands);
	parser.WhenApc = THISFN(ParseApplicationProgrammingCommands);
	SetImageDisplay(NormalImageCellDisplay());
	SetFrame(FieldFrame());
	History();
	ResetColors();
	HideScrollBar();
	WhenBar = THISFN(StdBar);
	sb.WhenScroll = THISFN(Scroll);
	caret.WhenAction = [=] { PlaceCaret(); };
	dpage.WhenScroll = [=] { SyncPage(); ScheduleDelayedRefresh(); };
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
	if(caret.IsBlinking()) {
		SetCaret(GetCaretRect());
	}
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
	Point pt = GetCursorPos() * fsz;

	pt.y -= (fsz.cy * GetSbPos());
	
	switch(caret.GetStyle()) {
	case Caret::BEAM:
		fsz.cx = 1;
		break;
	case Caret::UNDERLINE:
		pt.y += fsz.cy - 1;
		fsz.cy = 1;
		break;
	case Caret::BLOCK:
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
	selclick = true;
	Refresh();
}

void Terminal::SyncSize(bool notify)
{
	// Apparently, the window minimize event on Windows "really" minimizes
	// the window. This results in a damaged terminal display. In order to
	// avoid this, we check the  new page size, and  do not attempt resize
	// the page if the requested page size is < 2 x 2 cells.

	Size newsize = GetPageSize();
	resizing = page->GetSize() != newsize;
	
	if(resizing && newsize.cx > 1 && 1 < newsize.cy) {
		page->SetSize(newsize);
		if(notify) {
			if(sizehint) {
				hinting = true;
				KillSetTimeCallback(1000, THISFN(HintNewSize), TIMEID_SIZEHINT);
			}
			if(lazyresize)
				KillSetTimeCallback(100, THISFN(DoLazyResize), TIMEID_REFRESH);
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

void Terminal::ScheduleDelayedRefresh()
{
	if(delayedrefresh
	&& (!lazyresize || !resizing)
	&& !ExistsTimeCallback(TIMEID_REFRESH))  // Don't cancel a pending refresh.
		SetTimeCallback(16, THISFN(DoDelayedRefresh), TIMEID_REFRESH);
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
	// It is possible to  have  an  alternate screen buffer with a history  buffer.
	// Some terminal  emulators already  come  with  this feautre enabled. Terminal
	// ctrl can  also support this feature out-of-the-boz, as  it uses  the  VTPage
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
	if(!delayedrefresh)
		RefreshDisplay();
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
						Refresh(RectC(x, y, fsz.cx, fsz.cy).InflatedVert(1));
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
		SetTimeCallback(-blinkinterval, THISFN(RefreshBlinkingText), TIMEID_BLINK);
	else
	if(!blinkingtext || !b) {
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
		pt = ClientToPagePos(pt);
		if(IsSelected(pt)) {
			selclick = true;
			return;
		}
		else {
			SetSelection(pt, pt, keyflags & K_CTRL);
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
		if(!HasCapture() && selclick && IsSelected(pt))
			ClearSelection();
		selclick = false;
	}
	ReleaseCapture();
}

void Terminal::LeftDrag(Point pt, dword keyflags)
{
	pt = ClientToPagePos(pt);
	bool modifier = keyflags & K_CTRL;
	
	if(!IsTracking()) {
		VectorMap<String, ClipData> data;
		if(!HasCapture() && !modifier && IsSelection() && IsSelected(pt)) {
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
		}
		else
		if(modifier && IsMouseOverImage(pt)) {
			Image isample = GetInlineImage(pt, modifier);
			Append(data, isample);
			Size isz = GetFitSize(isample.GetSize(), StdSampleSize());
			ImageDraw iw(isz);
			iw.DrawImage(isz, isample);
			DoDragAndDrop(data, iw, DND_COPY);
		}
	}
}

void Terminal::LeftDouble(Point pt, dword keyflags)
{
	// TODO: Word selection.
	
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
	}
}

void Terminal::MiddleDown(Point pt, dword keyflags)
{
	SetFocus();
	if(IsTracking())
		VTMouseEvent(pt, MIDDLEDOWN, keyflags);
	else
		Paste();
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
		if(!(selclick = IsSelected(pt)))
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
	pt = GetView().Bind(pt);
	bool b = HasCapture();
	if(IsTracking()) {
		if(b && modes[XTDRAGM])
			VTMouseEvent(pt, LEFTDRAG, keyflags);
		else
		if(modes[XTANYMM])
			VTMouseEvent(pt, b ? LEFTDRAG : MOUSEMOVE, keyflags);
	}
	else
	if(HasCapture()) {
		selpos = ClientToPagePos(pt);
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

void Terminal::VTMouseEvent(Point pt, dword event, dword keyflags, int zdelta)
{
	bool buttondown = (event & UP) != UP; // Combines everything else with a button-down event
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
	case RIGHTUP:
	case RIGHTDOWN:
		mouseevent = 0x02;
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
		return;
	}

	mousepos = pt;
	
	if(keyflags & K_SHIFT) mouseevent |= 0x04;
	if(keyflags & K_ALT)   mouseevent |= 0x08;
	if(keyflags & K_CTRL)  mouseevent |= 0x10;
	

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
	pt.y = pt.y * psz.cy / (wsz.cy -= wsz.cy % psz.cy) + GetSbPos();
	pt.x = pt.x * psz.cx / wsz.cx;
	return pt;
}

void Terminal::SetSelection(Point pl, Point ph, bool rsel)
{
	anchor = min(pl, ph);
	selpos = max(pl, ph);
	rectsel = rsel;
	Refresh();
}

bool Terminal::GetSelection(Point& pl, Point& ph) const
{
	if(IsNull(anchor) || anchor == selpos) {
		pl = ph = selpos;
		return false;
	}
	
	Size psz = GetPageSize();
	if(sGetPos(selpos, psz) < sGetPos(anchor, psz)) {
		pl = selpos;
		ph = anchor;
	}
	else {
		pl = anchor;
		ph = selpos;
	}
	
	return true;
}

Rect Terminal::GetSelectionRect() const
{
	Rect r = Null;
	Point pl, ph;
	if(GetSelection(pl, ph))
		r.Set(min(pl, ph), max(pl, ph));
	return r;
}

void Terminal::ClearSelection()
{
	anchor = Null;
	selpos = Null;
	rectsel = false;
	Refresh();
}

bool Terminal::IsSelected(Point pt) const
{
	if(rectsel) {
		Rect r = GetSelectionRect();
		return !IsNull(r)
			&& pt.x >= r.left
			&& pt.y >= r.top
			&& pt.x <  r.right
			&& pt.y <= r.bottom;
	}
	
	Point pl, ph;
	if(GetSelection(pl, ph)) {
		Size psz = page->GetSize();
		int x = sGetPos(pt, psz);
		int b = sGetPos(pl, psz);
		int e = sGetPos(ph, psz);
		return b <= x && x < e;
	}
	return false;
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
			const VTLine& ln = page->FetchLine(line);
			WString s = rectsel ? AsWString(ln, r.left, r.right) : ln.ToWString();
			if(IsNull(s)) {
				i++;
				continue;
			}
			if(i) {
				PutCrLf(i);
				i = 0;
			}
			if(!rectsel && line == r.top)
					txt.Cat(s.Mid(r.left, (r.top == r.bottom ? r.right : s.GetLength()) - r.left));
			else
			if(rectsel && line == r.bottom)
					 txt.Cat(s.Left(r.right));
			else
				txt.Cat(s);
			if(rectsel || !ln.IsWrapped())
				PutCrLf();
		}
	}
	return txt;
}

Image Terminal::GetInlineImage(Point pt, bool modifier)
{
	if(!modifier)
		return Null;
	
	Image img;
	const VTCell& cell = page->FetchCell(pt);
	if(cell.IsImage()) {
		img = GetCachedImageData(cell.chr, Null, GetFontSize()).image;
		if(IsNull(img))
			LLOG("Unable to retrieve image from cache. Link id: " << cell.chr);
	}
	return pick(img);
}

String Terminal::GetHyperlinkURI(Point pt, bool modifier)
{
	if(!modifier)
		return Null;
	
	String uri;
	const VTCell& cell = page->FetchCell(pt);
	if(cell.IsHyperlink()) {
		uri = GetCachedHyperlink(cell.data);
		if(IsNull(uri))
			LLOG("Unable to retrieve URI from link cache. Link id: " << cell.data);
	}
	return uri;
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
			Tip(GetCachedHyperlink(activelink));
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

}