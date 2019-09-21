#include "Console.h"

#define LLOG(x)	// RLOG("Console: " << x)

namespace Upp {

void Console::ParseCommandSequences(const VTInStream::Sequence& seq)
{
	
	LLOG(Format("CSI: %c, %[N/A]~s, %s %[0:;(Private)]s",
			seq.opcode, seq.intermediate, seq.parameters.ToString(), seq.mode));

	bool refresh;
	switch(FindSequenceId(VTInStream::Sequence::CSI, clevel, seq, refresh)) {
	case SequenceId::ICH:
		page->InsertCells(seq.GetInt(1));
		break;
	case SequenceId::SL:
		page->ScrollRight(seq.GetInt(1));
		break;
	case SequenceId::CUU:
		page->MoveUp(seq.GetInt(1));
		break;
	case SequenceId::SR:
		page->ScrollLeft(seq.GetInt(1));
		break;
	case SequenceId::CUD:
		page->MoveDown(seq.GetInt(1));
		break;
	case SequenceId::CUF:
		page->MoveRight(seq.GetInt(1));
		break;
	case SequenceId::CUB:
		page->MoveLeft(seq.GetInt(1));
		break;
	case SequenceId::CNL:
		page->MoveHome();
		page->MoveDown(seq.GetInt(1));
		break;
	case SequenceId::CPL:
		page->MoveHome();
		page->MoveUp(seq.GetInt(1));
		break;
	case SequenceId::CHA:
		page->MoveToColumn(seq.GetInt(1));
		break;
	case SequenceId::CUP:
		page->MoveTo(seq.GetInt(2), seq.GetInt(1));
		break;
	case SequenceId::CHT:
		page->NextTab(seq.GetInt(1));
		break;
	case SequenceId::ED:
	case SequenceId::DECSED:
		ClearPage(seq);
		break;
	case SequenceId::EL:
	case SequenceId::DECSEL:
		ClearLine(seq);
		break;
	case SequenceId::IL:
		page->InsertLines(seq.GetInt(1));
		break;
	case SequenceId::DL:
		page->RemoveLines(seq.GetInt(1));
		break;
	case SequenceId::DCH:
		page->RemoveCells(seq.GetInt(1));
		break;
	case SequenceId::SU:
		page->ScrollDown(seq.GetInt(1));
		break;
	case SequenceId::SD:
		page->ScrollUp(seq.GetInt(1));
		break;
	case SequenceId::ECH:
		page->EraseCells(seq.GetInt(1), GetFillerFlags(seq));
		break;
	case SequenceId::CBT:
		page->PrevTab(seq.GetInt(1));
		break;
	case SequenceId::HPA:
		page->MoveToColumn(seq.GetInt(1));
		break;
	case SequenceId::HPR:
		page->MoveRight(seq.GetInt(1));
		break;
	case SequenceId::REP:
		if(parser.WasChr())
			page->RepeatCell(seq.GetInt(1));
		break;
	case SequenceId::DA1:
	case SequenceId::DA2:
	case SequenceId::DA3:
		ReportDeviceAttributes(seq);
		break;
	case SequenceId::VPA:
		page->MoveToLine(seq.GetInt(1));
		break;
	case SequenceId::VPR:
		page->MoveDown(seq.GetInt(1));
		break;
	case SequenceId::HVP:
		page->MoveTo(seq.GetInt(2), seq.GetInt(1));
		break;
	case SequenceId::TBC:
		ClearTabs(seq);
		break;
	case SequenceId::SM:
		SetMode(seq, true);
		break;
	case SequenceId::RM:
		SetMode(seq, false);
		break;
	case SequenceId::DECRQM:
		ReportMode(seq);
		break;
	case SequenceId::SGR:
		SelectGraphicsRendition(seq);
		break;
	case SequenceId::DSR:
	case SequenceId::DECXCPR:
		ReportDeviceStatus(seq);
		break;
	case SequenceId::DECSCL:
		SetDeviceConformanceLevel(seq);
		break;
	case SequenceId::DECSTR:
		SoftReset();
		break;
	case SequenceId::DECLL:
		SetProgrammableLEDs(seq);
		break;
	case SequenceId::DECSCA:
		ProtectAttributes(seq.GetInt(1, 0) == 1);
		break;
	case SequenceId::DECSCUSR:
		SetCaretStyle(seq);
		break;
	case SequenceId::DECSTBM:
		page->SetVertMargins(seq.GetInt(1, 0), seq.GetInt(2, 0));
		break;
	case SequenceId::DECSLRM:
		if(modes[DECLRMM])
			page->SetHorzMargins(seq.GetInt(1, 0), seq.GetInt(2, 0));
		else // SCOSC
		if(IsNull(seq.GetStr(1)))
			Backup();
		break;
	case SequenceId::SCORC:
		Restore();
		break;
	case SequenceId::DECRQPSR:
		ReportPresentationState(seq);
		break;
	case SequenceId::DECREQTPARM:
		ReportDeviceParameters(seq);
		break;
	case SequenceId::DECTST:
		// Device confidence tests.
		break;
	case SequenceId::DECIC:
		page->ScrollLeft(seq.GetInt(1));
		break;
	case SequenceId::DECDC:
		page->ScrollRight(seq.GetInt(1));
		break;
	case SequenceId::DECSACE:
		SelectRectAreaAttrsChangeExtent(seq);
		break;
	case SequenceId::DECCARA:
		ChangeRectAreaAttrs(seq, false);
		break;
	case SequenceId::DECRARA:
		ChangeRectAreaAttrs(seq, true);
		break;
	case SequenceId::DECCRA:
		CopyRectArea(seq);
		break;
	case SequenceId::DECFRA:
		FillRectArea(seq);
		break;
	case SequenceId::DECERA:
		ClearRectArea(seq);
		break;
	case SequenceId::DECSERA:
		ClearRectArea(seq, true);
		break;
	case SequenceId::DECRQCRA:
		ReportRectAreaChecksum(seq);
		break;
	case SequenceId::DECSCPP:
		WhenSetColumns(seq.GetInt(1) != 132 ? 80 : 132);
		break;
	case SequenceId::DECSLPP:
		ReportWindowProperties(seq);
		break;
	case SequenceId::DECSNLS:
		WhenSetLines(max(seq.GetInt(1), 1));
		break;
	case SequenceId::DECST8C:
		if(seq.GetInt(1) == 5)
			page->SetTabs(8);
		break;
	case SequenceId::IGNORED:
		break;
	default:
		LLOG("Unhandled command sequence.");
		break;
	}
	if(refresh)
		RefreshPage();
}

void Console::ProtectAttributes(bool protect)
{
	cellattrs.Protect(protect);
	page->Attributes(cellattrs);
}

void Console::ClearPage(const VTInStream::Sequence& seq)
{
	dword flags = GetFillerFlags(seq);

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
		page->ErasePage(flags);
		page->EraseHistory();
		break;
	}
}

void Console::ClearLine(const VTInStream::Sequence& seq)
{
	dword flags = GetFillerFlags(seq);

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

void Console::ClearTabs(const VTInStream::Sequence& seq)
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

void Console::ReportDeviceStatus(const VTInStream::Sequence& seq)
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
			PutCSI(Format("?%d`n", IsUDKEnabled() ? (IsUDKLocked() ? 21 : 20) : 23));
			break;
		case 26:
			// Keyboard status (dialect) report
			PutCSI("?27;1;0;0n"); // US-ASCII
			break;
		case 53:
			// DEC locator support
			PutCSI("?53n");		// No locator (yet)
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

void Console::ReportDeviceParameters(const VTInStream::Sequence& seq)
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

void Console::ReportDeviceAttributes(const VTInStream::Sequence& seq)
{
	static constexpr const char* VTID_52   = "/Z";
	static constexpr const char* VTID_1XX  = "?6c";
	static constexpr const char* VTID_2XX  = "?62;1;6;8;9;15;22c";
	static constexpr const char* VTID_3XX  = "?63;1;4;6;8;9;15;22c";
	static constexpr const char* VTID_4XX  = "?64;1;4;6;8;9;15;17;21;22c";
	static constexpr const char* VTID_UNIT = "!|00000000";
		
	if(seq.mode == '\0') {
		switch(clevel) {
		case LEVEL_0:
			PutESC(VTID_52);
			break;
		case LEVEL_1:
			PutCSI(VTID_1XX);
			break;
		case LEVEL_2:
			PutCSI(VTID_2XX);
			break;
		case LEVEL_3:
			PutCSI(VTID_3XX);
			break;
		case LEVEL_4:
			PutCSI(VTID_4XX);
			break;
		default:
			break;
		}
	}
	else
	if(seq.mode == '>')
		PutCSI(">41;2;0c");
	else
	if(seq.mode == '=')
		PutDCS(VTID_UNIT);	// DECREPTUI
}

void Console::ReportPresentationState(const VTInStream::Sequence& seq)
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
		if(cellattrs.IsProtected())
			attrs |= 0x01;
		if(gsets.GetSS() == 0x8E)
			flags |= 0x02;
		if(gsets.GetSS() == 0x8F)
			flags |= 0x04;
		if(modes[DECOM])
			flags |= 0x01;
		if(modes[DECAWM])
			flags |= 0x08;

		auto Is96Chars = [=] (byte gx) -> bool
		{
			return CHARSET_ISO8859_1 <= gx && gx <= CHARSET_ISO8859_16;
		};
		
		auto GetCharsetId = [=] (byte chrset) -> const char*
		{
			// TODO: This can be more precise...
			if(chrset == CHARSET_DEC_DCS)   return "0";
			if(chrset == CHARSET_DEC_TCS)   return ">";
			if(chrset == CHARSET_DEC_MCS)   return "<";
			if(chrset == CHARSET_ISO8859_1)	return "A";
			return "B";		// ASCII
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

void Console::ReportWindowProperties(const VTInStream::Sequence& seq)
{
    int opcode = seq.GetInt(1, 0);
    if(opcode < 24) {
		opcode  = (opcode << 8) | seq.GetInt(2,  0);
	    ReportWindowProperties(opcode & 0xFFFF);
    }
    else WhenSetLines(max(opcode, 1));
}

void Console::SetDeviceConformanceLevel(const VTInStream::Sequence& seq)
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
	Set8BitsMode(mode == 0 || mode == 2);

	LLOG(Format("Device conformance level is set to: %d (%d-bits mode)",
				(int) clevel, Is8BitsMode() ? 8 : 7 ));
}

void Console::SetProgrammableLEDs(const VTInStream::Sequence& seq)
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

void Console::SetCaretStyle(const VTInStream::Sequence& seq)
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

void Console::CopyRectArea(const VTInStream::Sequence& seq)
{
	int srcpage  = seq.GetInt(5);
	int destpage = seq.GetInt(8);
	
	if(srcpage != destpage || srcpage != 1 || destpage != 1) // Currently we don't support multiple pages.
			return;
	
	Rect r;
	r.left   = seq.GetInt(2, Null);
	r.top    = seq.GetInt(1, Null);
	r.right  = seq.GetInt(4, Null);
	r.bottom = seq.GetInt(3, Null);
	
	Point pt;
	pt.x = seq.GetInt(7, Null);
	pt.y = seq.GetInt(6, Null);
	
	page->CopyRect(r, pt);
}

void Console::FillRectArea(const VTInStream::Sequence& seq)
{
	Rect r;
	r.left   = seq.GetInt(3, Null);
	r.top    = seq.GetInt(2, Null);
	r.right  = seq.GetInt(5, Null);
	r.bottom = seq.GetInt(4, Null);

	int chr = seq.GetInt(1, 0);
	
	page->FillRect(r, LookupChar(chr));
}

void Console::ClearRectArea(const VTInStream::Sequence& seq, bool selective)
{
	Rect r;
	r.left   = seq.GetInt(2, Null);
	r.top    = seq.GetInt(1, Null);
	r.right  = seq.GetInt(4, Null);
	r.bottom = seq.GetInt(3, Null);
	
	dword flags = selective
		? VTCell::FILL_SELECTIVE | VTCell::FILL_CHAR
		: VTCell::FILL_NORMAL;

	page->EraseRect(r, flags);
}

void Console::SelectRectAreaAttrsChangeExtent(const VTInStream::Sequence& seq)
{
	streamfill = seq.GetInt(1) != 2;
}

void Console::ChangeRectAreaAttrs(const VTInStream::Sequence& seq, bool invert)
{
	Rect r;
	r.left   = seq.GetInt(2, Null);
	r.top    = seq.GetInt(1, Null);
	r.right  = seq.GetInt(4, Null);
	r.bottom = seq.GetInt(3, Null);
	
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

void Console::ReportRectAreaChecksum(const VTInStream::Sequence& seq)
{
	int id = seq.GetInt(1);
	int pn = seq.GetInt(2);
	
	Rect r;
	r.left   = seq.GetInt(4, Null);
	r.top    = seq.GetInt(3, Null);
	r.right  = seq.GetInt(6, Null);
	r.bottom = seq.GetInt(5, Null);
	
	int checksum = 0;
	
	auto CalcRectAreaChecksum = [=, &checksum](const VTCell& cell, const Point& pt) -> void
	{
		// I couldn't find any official documentation on how to calculate
		// a rectangular area's checksum. I was able to figure out that I
		// have to negate the cells character codes, shich shoud be either
		// GL or GR, but I wasn't able to figure out how to take the SGR
		// values into account. Luckily, I found the anwser in xterm's code.
		// See: xterm/screen.c, ln 2719-2817 (xtermCheckRect() function)
		// Credits should go to Thomas E. Dickey (xterm's maintainer) et al.
		
		int chr = cell.chr;
		checksum -= ConvertToCharset(chr, gsets.Get(chr, IsLevel2()));
		if(cell.IsUnderlined())
			checksum -= 0x10;
		if(cell.IsInverted())
			checksum -= 0x20;
		if(cell.IsBlinking())
			checksum -= 0x40;
		if(cell.IsBold())
			checksum -= 0x80;
	};
	
	// According to DEC EK-VT420-RM002  (p. 242)
	// and DEC EK 00070-05 (p. 5-179)  documents,
	// DECRQCRA and its response DECCKSR is also
	// affected by the relative positioning mode.

	page->GetRelRectArea(r, CalcRectAreaChecksum);
	
	PutDCS(Format("%d!~%2X", id, (word) checksum));
}

dword Console::GetFillerFlags(const VTInStream::Sequence& seq) const
{
	dword flags = VTCell::FILL_NORMAL;
	if(!modes[ERM] || (IsLevel2() && seq.mode == '?'))
		flags = VTCell::FILL_SELECTIVE |
		        VTCell::FILL_CHAR      |
		        VTCell::FILL_ATTRS     |
		        VTCell::FILL_INK       |
		        VTCell::FILL_PAPER     |
		        VTCell::FILL_SGR;
	return flags;
}
}
