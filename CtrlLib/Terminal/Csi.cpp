#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void TerminalCtrl::ParseCommandSequences(const VTInStream::Sequence& seq)
{
	LLOG(seq);

	const CbFunction *p = FindFunctionPtr(seq);
	if(p) p->c(*this, seq);
}

void TerminalCtrl::ClearPage(const VTInStream::Sequence& seq, dword flags)
{
	switch(seq.GetInt(1, 0)) {
	case 0:
		page->EraseAfter(flags);
		break;
	case 1:
		page->EraseBefore(flags);
		break;
	case 2:
		page->ErasePage(flags);
		break;
	case 3:
		page->EraseHistory();
		break;
	}
}

void TerminalCtrl::ClearLine(const VTInStream::Sequence& seq, dword flags)
{
	switch(seq.GetInt(1, 0)) {
	case 0:
		page->EraseRight(flags);
		break;
	case 1:
		page->EraseLeft(flags);
		break;
	case 2:
		page->EraseLine(flags);
		break;
	default:
		break;
	}
}

void TerminalCtrl::ClearTabs(const VTInStream::Sequence& seq)
{
	switch(seq.GetInt(1, 0)) {
	case 0:
		page->SetTab(false);
		break;
	case 3:
		page->ClearTabs();
		break;
	default:
		break;
	}
}

void TerminalCtrl::ReportDeviceStatus(const VTInStream::Sequence& seq)
{
	int opcode = seq.GetInt(1, 0);
	Point p = page->GetRelPos();

	if(seq.mode == 0)
		switch(opcode) {
		case 5:
			// Operating status repost
			PutCSI("0n");
			break;
		case 6:
			// Cursor position report (normal))
			PutCSI(Format("%d;%d`R", p.y, p.x));
			break;
		default:
			break;
		}
	else
	if(seq.mode == '?')
		switch(opcode) {
		case 6:
			// Cursor position report (extended)
			PutCSI(Format("?%d;%d;1R", p.y, p.x));
			break;
		case 15:
			// Printer status report
			PutCSI("?13n");	// No printer
			break;
		case 25:
			// UDK status report
			PutCSI(Format("?%d`n", HasUDK() ? (IsUDKLocked() ? 21 : 20) : 23));
			break;
		case 26:
			// Keyboard status (dialect) report
			PutCSI("?27;1;0;0n"); // US-ASCII
			break;
		case 53:
		case 55:
			// DEC locator support
			PutCSI("?50n");		// No locator
			break;
		case 56:
			// DEC locator type
			PutCSI("?57;0n");  // Cannot identify
			break;
		case 62:
			// User-defined macros status report
			// TODO:
			break;
		case 63:
			// Memory checksum report
			// TODO:
			break;
		case 75:
			// Data integrity report
			PutCSI("?70n");	// No communication error
			break;
		case 85:
			// Multiple-session configuration status report
			PutCSI("?83n");	// Not supported
			break;
		default:
			break;
		}
}

void TerminalCtrl::ReportDeviceParameters(const VTInStream::Sequence& seq)
{
	if(IsLevel2()) // Reply only in VT1XX mode
		return;

	// We're sending a report identical to xterm's.
	// For more details on DECREQTPARM and its response, see:
	// 1) VT100 User Guide,
	// 2) https://invisible-island.net/xterm/ctlseqs/ctlseqs.html

	int opcode = seq.GetInt(1, 0);

	if(opcode == 0 || opcode == 1)
		PutCSI(Format("%d;1;1;1;1;1;0x", opcode + 2));
}

void TerminalCtrl::ReportDeviceAttributes(const VTInStream::Sequence& seq)
{
	static constexpr const char* VTID_52   = "/Z";
	static constexpr const char* VTID_1XX  = "?6c";
	static constexpr const char* VTID_2XX  = "?62;1;6;8;9;15;17;22";
	static constexpr const char* VTID_3XX  = "?63;1;6;8;9;15;17;22";
	static constexpr const char* VTID_4XX  = "?64;1;6;8;9;15;17;21;22;28";
	static constexpr const char* VTID_UNIT = "!|00000000";
	
	if(seq.mode == '\0') {
		String s;
		switch(clevel) {
		case LEVEL_0:
			PutESC(VTID_52);
			return;
		case LEVEL_1:
			PutCSI(VTID_1XX);
			return;
		case LEVEL_2:
			s = VTID_2XX;
			break;
		case LEVEL_3:
			s = VTID_3XX;
			break;
		case LEVEL_4:
			s = VTID_4XX;
			break;
		default:
			return;
		}
		
		// Advertise individually switchable capabilities.
		if(sixelimages)  s << ";4";
		if(jexerimages)  s << ";444";
		if(iterm2images) s << ";1337";
		
		PutCSI(s + "c");
	}
	else
	if(seq.mode == '>')
		PutCSI(">41;3;0c");
	else
	if(seq.mode == '=')
		PutDCS(VTID_UNIT);	// DECREPTUI
}

void TerminalCtrl::ReportPresentationState(const VTInStream::Sequence& seq)
{
	int report = seq.GetInt(1, 0);

	if(report == 1) {	// DECCIR
		byte sgr	= 0x40;
		byte attrs	= 0x40;
		byte flags	= 0x40;
		byte gsize	= 0x40;

		if(cellattrs.IsBold())
			sgr |= 0x01;
		if(cellattrs.IsUnderlined())
			sgr |= 0x02;
		if(cellattrs.IsBlinking())
			sgr |= 0x04;
		if(cellattrs.IsInverted())
			sgr |= 0x08;
		if(cellattrs.HasDECProtection())
			attrs |= 0x01;
		if(gsets.GetSS() == 0x8E)
			flags |= 0x02;
		if(gsets.GetSS() == 0x8F)
			flags |= 0x04;
		if(modes[DECOM])
			flags |= 0x01;
		if(page->IsEol())
			flags |= 0x08;

		auto Is96Chars = [=] (byte gx) -> bool
		{
			return CHARSET_ISO8859_1 <= gx && gx <= CHARSET_ISO8859_16;
		};

		auto GetCharsetId = [=] (byte chrset) -> const char*
		{
			// TODO: This can be more precise...
			return decode(chrset,
						CHARSET_DEC_DCS, "0",
						CHARSET_DEC_TCS, ">",
						CHARSET_DEC_MCS, "<",
						CHARSET_UNICODE, "G",
						CHARSET_ISO8859_1, "A", "B");
		};

		if(Is96Chars(gsets.GetG0()))
			gsize |= 0x01;
		if(Is96Chars(gsets.GetG1()))
			gsize |= 0x02;
		if(Is96Chars(gsets.GetG2()))
			gsize |= 0x04;
		if(Is96Chars(gsets.GetG3()))
			gsize |= 0x08;

		Point pt = page->GetRelPos();

		PutDCS(Format("1$u%d;%d;%d;%c;%c;%c;%d;%d;%c;%s%s%s%s",
					pt.y, pt.x, 1,
					sgr,
					attrs,
					flags,
					gsets.GetGLNum(),
					gsets.GetGRNum(),
					gsize,
					GetCharsetId(gsets.GetG0()),
					GetCharsetId(gsets.GetG1()),
					GetCharsetId(gsets.GetG2()),
					GetCharsetId(gsets.GetG3()))
			);
	}
	else
	if(report == 2) {	// DECTABSR
		Vector<int> tabstops;
		Vector<String> reply;

		page->GetTabs(tabstops);

		for(const auto& t : tabstops)
			reply.Add(AsString(t));

		PutDCS(Format("2$u%s", Join(reply, "/")));
	}
}

void TerminalCtrl::HandleWindowOpsRequests(const VTInStream::Sequence& seq)
{
	// This method implements most of the xterm's WindowOps feature.
	// See: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
	
	enum WindowActions : int {
        ACTION_UNMINIMIZE        = 10,
        ACTION_MINIMIZE          = 20,
        ACTION_MOVE              = 30,
        ACTION_RESIZE_VIEW       = 40,
    //  ACTION_RAISE             = 50,
    //  ACTION_LOWER             = 60,
        ACTION_REFRESH           = 70,
        ACTION_RESIZE_PAGE       = 80,
        ACTION_UNMAXIMIZE        = 90,
        ACTION_MAXIMIZE          = 91,
        ACTION_MAXIMIZE_VERT     = 92,
        ACTION_MAXIMIZE_HORZ     = 93,
        ACTION_NOFULLSCREEN      = 100,
        ACTION_FULLSCREEN        = 101,
        ACTION_FULLSCREEN_TOGGLE = 102
	};

	enum WindowReports : int { // P = pixels, C = cells
        REPORT_WINDOW_STATE             = 110,
        REPORT_WINDOW_POSITION          = 130,
        REPORT_PAGE_POSITION            = 132,
        REPORT_PAGE_SIZE_IN_PIXELS      = 140,
        REPORT_WINDOW_SIZE_IN_PIXELS    = 142,
        REPORT_SCREEN_SIZE_IN_PIXELS    = 150,
        REPORT_CELL_SIZE                = 160,
        REPORT_PAGE_SIZE_IN_CELLS       = 180,
        REPORT_SCREEN_SIZE_IN_CELLS     = 190,
        REPORT_WINDOW_ICOON_LABEL       = 120,
        REPORT_WINDOW_TITLE             = 210
	};

	int opcode = seq.GetInt(1);
	
	TopWindow *w = GetTopWindow();

	if(!w)
		return;
		
	if(windowactions && 0 < opcode && opcode < 11) {
		opcode *= 10;
		if(opcode > 90) opcode += seq.GetInt(2, 0);
		switch(opcode) {
		case ACTION_UNMINIMIZE:
			WhenWindowMinimize(false);
			break;
		case ACTION_MINIMIZE:
			WhenWindowMinimize(true);
			break;
		case ACTION_MOVE: {
			int x = seq.GetInt(3, 0);
			int y = seq.GetInt(2, 0);
			WindowMoveRequest(w, x, y);
			break; }
		case ACTION_RESIZE_VIEW: {
			int cx = StrInt(seq.GetStr(3));
			int cy = StrInt(seq.GetStr(2));
			WindowResizeRequest(w, cx, cy);
			break; }
		case ACTION_RESIZE_PAGE: {
			int cx = StrInt(seq.GetStr(3));
			int cy = StrInt(seq.GetStr(2));
			WindowPageResizeRequest(w, cx, cy);
			break; }
		case ACTION_UNMAXIMIZE:
			WhenWindowMaximize(false);
			break;
		case ACTION_MAXIMIZE:
			WhenWindowMaximize(true);
			break;
		case ACTION_MAXIMIZE_VERT:
			WindowMaximizeVertRequest(w);
			break;
		case ACTION_MAXIMIZE_HORZ:
			WindowMaximizeHorzRequest(w);
			break;
		case ACTION_NOFULLSCREEN:
			WhenWindowFullScreen(-1);
			break;
		case ACTION_FULLSCREEN:
			WhenWindowFullScreen(1);
			break;
		case ACTION_FULLSCREEN_TOGGLE:
			WhenWindowFullScreen(0);
			break;
		default:
			LLOG("Unhandled window action request: " << opcode);
			return;
		}
	}
	else
	if(windowreports && 10 < opcode && opcode < 24) {
		Rect r;
		Size sz;
		Rect wr = GetWorkArea();
		switch(opcode * 10 + seq.GetInt(2, 0)) {
		case REPORT_WINDOW_POSITION:
			r = w->GetRect();
			PutCSI(Format("3;%d;%d`t", r.left, r.top));
			break;
		case REPORT_PAGE_POSITION:
			r = GetRect();
			PutCSI(Format("3;%d;%d`t", r.left, r.top));
			break;
		case REPORT_PAGE_SIZE_IN_PIXELS:
			sz = GetPageSize() * GetCellSize();
			PutCSI(Format("4;%d;%d`t", sz.cy, sz.cx));
			break;
		case REPORT_WINDOW_SIZE_IN_PIXELS:
			sz = w->GetSize();
			PutCSI(Format("4;%d;%d`t", sz.cy, sz.cx));
			break;
		case REPORT_SCREEN_SIZE_IN_PIXELS:
			sz = GetWorkArea().GetSize();
			PutCSI(Format("5;%d;%d`t", sz.cy, sz.cx));
			break;
		case REPORT_PAGE_SIZE_IN_CELLS:
			sz = GetPageSize();
			PutCSI(Format("8;%d;%d`t", sz.cy, sz.cx));
			break;
		case REPORT_CELL_SIZE:
			sz = GetCellSize();
			PutCSI(Format("6;%d;%d`t", sz.cy, sz.cx));
			break;
		case REPORT_SCREEN_SIZE_IN_CELLS: {
			sz = GetWorkArea().GetSize() / GetCellSize();
			PutCSI(Format("9;%d;%d`t", sz.cy, sz.cx));
			break; }
		case REPORT_WINDOW_STATE:
			PutCSI(Format("%d`t", w->IsMinimized() ? 2 : 1));
			break;
		case REPORT_WINDOW_ICOON_LABEL:
			break;
		case REPORT_WINDOW_TITLE:
			PutOSC(Format("l%s", EncodeDataString(w->GetTitle())));
			break;
		default:
			LLOG("Unhandled window report request: " << opcode);
			return;
		}
	}
}

void TerminalCtrl::WindowMoveRequest(TopWindow *w, int x, int y)
{
	Rect r(Point(x, y), w->GetRect().GetSize());
	
	LLOG("WindowMoveRequest(" << x << ", " << y << ") -> " << r);
	
	WhenWindowGeometryChange(r);
}

void TerminalCtrl::WindowResizeRequest(TopWindow *w, int cx, int cy)
{
	Rect r = w->GetRect();
	Rect wr = GetWorkArea();

	Size sz;
	sz.cx = IsNull(cx) ? r.Width() : !cx ? wr.Width() : cx;
	sz.cy = IsNull(cy) ? r.Height() : !cy ? wr.Height() : cy;

	LLOG("WindowResizeRequest(" << cx << ", " << cy << ") -> " << sz);
		
	WhenWindowGeometryChange(Rect(r.TopLeft(), sz));
}

void TerminalCtrl::WindowPageResizeRequest(TopWindow *w, int cx, int cy)
{
	LLOG("WindowPageResizeRequest(" << cx << ", " << cy << ")");
	
	Rect r = w->GetRect();
	Rect wr = GetWorkArea();
	Size csz = GetCellSize();
	
	Size sz;
	sz.cx = IsNull(cx) ? r.Width() : !cx ? wr.Width() : AddFrameSize(Size(cx, 1) * csz).cx;
	sz.cy = IsNull(cy) ? r.Height() : !cy ? wr.Height() : AddFrameSize(Size(1, cy) * csz).cy;
	
	WhenWindowGeometryChange(Rect(r.TopLeft(), sz));
}

void TerminalCtrl::WindowMaximizeHorzRequest(TopWindow *w)
{
	Rect r = GetWorkArea();
	r.bottom = r.top + w->GetView().Height();
	WhenWindowGeometryChange(r);
}

void TerminalCtrl::WindowMaximizeVertRequest(TopWindow *w)
{
	Rect r = GetWorkArea();
	r.right = r.left + w->GetView().Width();
	WhenWindowGeometryChange(r);
}

void TerminalCtrl::SetDeviceConformanceLevel(const VTInStream::Sequence& seq)
{
	int level = seq.GetInt(1, 0);
	int mode  = seq.GetInt(2, -1);

	switch(level) {
	case 61:
		clevel = LEVEL_1;
		break;
	case 62:
		clevel = LEVEL_2;
		break;
	case 63:
		clevel = LEVEL_3;
		break;
	case 64:
		clevel = LEVEL_4;
		break;
	default:
		LLOG("Unknown device conformance level: " << level - 60);
		return;
	}

	SoftReset();
	Set8BitMode(mode == 0 || mode == 2);

	LLOG(Format("Device conformance level is set to: %d (%d-bits mode)",
				(int) clevel, Is8BitMode() ? 8 : 7 ));
}

void TerminalCtrl::SetProgrammableLEDs(const VTInStream::Sequence& seq)
{
	int  led = seq.GetInt(1, 0);
	bool set = led >= 1 && led < 21;

	switch(led) {
	case 0:
		WhenLED(LED_ALL, set);
		break;
	case 1:
	case 21:
		WhenLED(LED_NUMLOCK, set);
		break;
	case 2:
	case 22:
		WhenLED(LED_CAPSLOCK, set);
		break;
	case 3:
	case 23:
		WhenLED(LED_SCRLOCK, set);
		break;
	default:
		break;
	}
}

void TerminalCtrl::SetCaretStyle(const VTInStream::Sequence& seq)
{
	if(caret.IsLocked())
		return;

	int n = seq.GetInt(1, 0);
	bool blink = n == 0 || n % 2;

	switch(n) {
	case 0:
	case 1:
	case 2:
		caret.Block(blink);
		break;
	case 3:
	case 4:
		caret.Underline(blink);
		break;
	case 5:
	case 6:
		caret.Beam(blink);
		break;
	}
}

void TerminalCtrl::SetHorizontalMargins(const VTInStream::Sequence& seq)
{
	if(modes[DECLRMM])
		page->SetHorzMargins(seq.GetInt(1), seq.GetInt(2));
	else // SCOSC (uses the same CSI, but different mode.
	if(IsNull(seq.GetStr(1)))
		Backup();
}

void TerminalCtrl::SetVerticalMargins(const VTInStream::Sequence& seq)
{
	page->SetVertMargins(seq.GetInt(1), seq.GetInt(2));
}

void TerminalCtrl::SetLinesPerPage(const VTInStream::Sequence& seq)
{
	if(seq.GetInt(1) < 24)
		HandleWindowOpsRequests(seq);
	else
		SetRows(seq.GetInt(1));
}

void TerminalCtrl::CopyRectArea(const VTInStream::Sequence& seq)
{
	int srcpage  = seq.GetInt(5);
	int destpage = seq.GetInt(8);

	if(srcpage != destpage || srcpage != 1 || destpage != 1) // Currently we don't support multiple pages.
			return;

	Rect view = page->GetView();

	Rect r;
	r.top    = seq.GetInt(1, view.top);
	r.left   = seq.GetInt(2, view.left);
	r.bottom = seq.GetInt(3, view.bottom);
	r.right  = seq.GetInt(4, view.right);

	Point pt;
	pt.x = seq.GetInt(7);
	pt.y = seq.GetInt(6);

	page->CopyRect(pt, r);
}

void TerminalCtrl::FillRectArea(const VTInStream::Sequence& seq)
{
	int chr = seq.GetInt(1, 0x20);

	if(chr <= 0x1F
	|| chr == 0x7F
	|| chr >  0xFF
	||(chr >= 0x80 && chr < 0xA0))
		return;

	Rect view = page->GetView();

	Rect r;
	r.top    = seq.GetInt(2, view.top);
	r.left   = seq.GetInt(3, view.left);
	r.bottom = seq.GetInt(4, view.bottom);
	r.right  = seq.GetInt(5, view.right);

	page->FillRect(r, LookupChar(chr));
}

void TerminalCtrl::ClearRectArea(const VTInStream::Sequence& seq, bool selective)
{
	Rect view = page->GetView();

	Rect r;
	r.top    = seq.GetInt(1, view.top);
	r.left   = seq.GetInt(2, view.left);
	r.bottom = seq.GetInt(3, view.bottom);
	r.right  = seq.GetInt(4, view.right);

	dword flags = selective
		? VTCell::FILL_DEC_SELECTIVE | VTCell::FILL_CHAR
		: VTCell::FILL_NORMAL;

	page->EraseRect(r, flags);
}

void TerminalCtrl::SelectRectAreaAttrsChangeExtent(const VTInStream::Sequence& seq)
{
	streamfill = seq.GetInt(1) != 2;
}

void TerminalCtrl::ChangeRectAreaAttrs(const VTInStream::Sequence& seq, bool invert)
{
	Rect view = page->GetView();

	Rect r;
	r.top    = seq.GetInt(1, view.top);
	r.left   = seq.GetInt(2, view.left);
	r.bottom = seq.GetInt(3, view.bottom);
	r.right  = seq.GetInt(4, view.right);

	VTCell filler = cellattrs;

	Vector<String> sgrcodes;
	for(int i = 4; i < seq.parameters.GetCount(); i++)
		sgrcodes.Add(seq.parameters[i]);

	invert
		? InvertGraphicsRendition(filler, sgrcodes)
		: SetGraphicsRendition(filler, sgrcodes);

	dword flags = invert
					? VTCell::XOR_SGR
					: VTCell::FILL_SGR;

	streamfill
		? page->FillStream(r, filler, flags)
		: page->FillRect(r,  filler, flags);
}

void TerminalCtrl::ReportRectAreaChecksum(const VTInStream::Sequence& seq)
{
	int id = seq.GetInt(1);
	int pn = seq.GetInt(2, 0);

	Rect view = page->GetView();

	Rect r;
	r.top    = seq.GetInt(3, view.top);
	r.left   = seq.GetInt(4, view.left);
	r.bottom = seq.GetInt(5, view.bottom);
	r.right  = seq.GetInt(6, view.right);

	dword checksum = 0;

	auto CalcRectAreaChecksum = [=, &checksum](Point pt) -> void
	{
		// I couldn't find any official documentation on how to calculate
		// a rectangular area's checksum. I was able to figure out that I
		// have to negate the cells character codes, which shoud be either
		// GL or GR, but I wasn't able to figure out how to take the SGR
		// values into account. Luckily, I found the answer in xterm's code.
		// See: xterm/screen.c, ln 2719-2817 (xtermCheckRect() function)
		// Credits should go to Thomas E. Dickey (xterm's maintainer) et al.

		const VTCell& cell = page->GetCell(pt);
		if(!cell.IsVoid()) {
			checksum -= cell.chr == 0 ? 0x20 : EncodeCodepoint(cell, gsets.Get(cell, IsLevel2()));
			if(cell.IsUnderlined())
				checksum -= 0x10;
			if(cell.IsInverted())
				checksum -= 0x20;
			if(cell.IsBlinking())
				checksum -= 0x40;
			if(cell.IsBold())
				checksum -= 0x80;
		}
	};

	// According to DEC EK-VT420-RM002  (p. 242)
	// and DEC EK 00070-05 (p. 5-179)  documents,
	// DECRQCRA and its response DECCKSR is also
	// affected by the relative positioning mode.

	page->GetRectArea(r, CalcRectAreaChecksum, page->IsDisplaced());

	PutDCS(Format("%d!~%2X", id, (word) checksum));
}

void TerminalCtrl::AlternateScreenBuffer(bool b)
{
	page = b ? &apage : &dpage;
	LLOG("Alternate screen buffer (#" << &apage << "): " << (b ? "on" : "off"));
}

dword TerminalCtrl::GetDECStyleFillerFlags() const
{
	return VTCell::FILL_DEC_SELECTIVE
			| VTCell::FILL_CHAR
			| VTCell::FILL_ATTRS
			| VTCell::FILL_INK
			| VTCell::FILL_PAPER
			| VTCell::FILL_SGR
			| VTCell::FILL_DATA;
}

dword TerminalCtrl::GetISOStyleFillerFlags() const
{
	if(modes[ERM])
		return VTCell::FILL_NORMAL;

	return VTCell::FILL_ISO_SELECTIVE
			| VTCell::FILL_CHAR
			| VTCell::FILL_ATTRS
			| VTCell::FILL_INK
			| VTCell::FILL_PAPER
			| VTCell::FILL_SGR
			| VTCell::FILL_DATA;
}

}
