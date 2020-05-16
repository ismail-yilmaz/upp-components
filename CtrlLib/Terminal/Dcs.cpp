#include "Terminal.h"

#define LLOG(x)	// RLOG("Terminal: " << x)

namespace Upp {

void Terminal::ParseDeviceControlStrings(const VTInStream::Sequence& seq)
{
	LLOG("DCS " << seq);

	bool refresh;
	switch(FindSequenceId(VTInStream::Sequence::DCS, clevel, seq, refresh)) {
	case SequenceId::DECRQSS:
		ReportControlFunctionSettings(seq);
		break;
	case SequenceId::DECRSPS:
		RestorePresentationState(seq);
		break;
	case SequenceId::DECSIXEL:
		ParseSixelGraphics(seq);
		break;
	case SequenceId::DECUDK:
		SetUserDefinedKeys(seq);
		break;
	default:
		LLOG("Unhandled device control string.");
		break;
	}
	if(refresh)
		RefreshPage();
}

void Terminal::SetUserDefinedKeys(const VTInStream::Sequence& seq)
{
	if(!userdefinedkeys || userdefinedkeyslocked)
		return;

	bool clear = seq.GetInt(1) == 0;
	bool lock  = seq.GetInt(2) == 0;
	// int modifier = seq.GetInt(3, 0);

	Vector<String> vudk = Split(seq.payload, ';', false);

	if(clear)
		udk.Clear();

	String key, val;
	for(const String& pair : vudk)
		if(SplitTo(pair, '/', key, val))
			udk.Put(StrInt(key), ScanHexString(val));

	if(lock)
		LockUDK();
}

bool Terminal::GetUDKString(byte key, String& val)
{
	if(!IsLevel2() || udk.IsEmpty())
		return false;

	int i = udk.Find(key);
	if(i >= 0)
		val = udk[i];

	return i >= 0;
}

void Terminal::ReportControlFunctionSettings(const VTInStream::Sequence& seq)
{
	// TODO
	String reply;// = "0$r";	// Invalid request (unhandled sequence)

	if(seq.payload.IsEqual("r")) {					// DECSTBM
		Rect margins = page->GetMargins();
		reply = Format("%d`$r%d;%d", 1, margins.top, margins.bottom);
	}
	else
	if(IsLevel4() && seq.payload.IsEqual("s")) {	// DECSLRM
		Rect margins = page->GetMargins();
		reply = Format("%d`$r%d;%d", 1, margins.left, margins.right);
	}
	else
	if(seq.payload.IsEqual("m")) {					// SGR
		reply = Format("%d`$r0;%s", 1, GetGraphicsRenditionOpcodes(cellattrs));
	}
	else
	if(seq.payload.IsEqual("\"p")) {				// DECSCL
		int level = 62;
		switch(clevel) {
		case LEVEL_0:
		case LEVEL_1:
			level = 61;
			break;
		case LEVEL_2:
			level = 62;
			break;
		case LEVEL_3:
			level = 63;
			break;
		case LEVEL_4:
			level = 64;
			break;
		}
		reply = Format("%d`$r%d;%d", 1, level, Is8BitMode() ? 0 : 1);
	}
	else
	if(IsLevel4() && seq.payload.IsEqual(" q")) {	// DECSCUSR
		int style = 0;
		switch(caret.GetStyle()) {
		case Caret::BLOCK:
			style = caret.IsBlinking() ? 1 : 2;
			break;
		 case Caret::UNDERLINE:
			style = caret.IsBlinking() ? 3 : 4;
			break;
		case Caret::BEAM:
			style = caret.IsBlinking() ? 5 : 6;
			break;
		}
		reply = Format("%d`$r%d", 1, style);
	}
	else
	if(seq.payload.IsEqual("\"q")) {				// DECSCA
		reply = Format("%d`$r%d", 1, (int) cellattrs.IsProtected());
	}
	else
	if(IsLevel4() && seq.payload.IsEqual("*x")) {	// DECSACE
		reply = Format("%d`$r%d", 1, streamfill ? 1 : 2);
	}
	else
	if(seq.payload.IsEqual("t")) {					// DECSLPP
		reply = Format("%d`$r%d`t", 1, page->GetSize().cy);
	}
	else
	if(seq.payload.IsEqual("$|")) {					// DECSCPP
		reply = Format("%d`$r%d", 1, page->GetSize().cx);
	}
	else
	if(seq.payload.IsEqual("*|")) {					// DECSNLS
		reply = Format("%d`$r%d", 1, page->GetSize().cy);
	}

	if(!IsNull(reply)) {
		reply << seq.payload;
	}
	else {
		reply = "0$r";	// Invalid request (unhandled sequence)
		LLOG("Unhandled report request. Token: %s " << seq.payload);
	}
	
	PutDCS(reply);
}

void Terminal::RestorePresentationState(const VTInStream::Sequence& seq)
{
	int which = seq.GetInt(1, 0);
	
	if(which == 1) {	// DECCIR
		Vector<String> cr = Split(seq.payload, ';');

		if(cr.IsEmpty())
			return;

		auto GetInt = [&cr](int n) -> int
		{
			bool b = cr.GetCount() < max(1, n);
			if(b) return 0;
			String s = cr[--n];
			if(64 <= s[0] && s[0] <= 65536)
				return s[0] & 0xFFFF;
			else
				return Nvl(StrInt(s), 0);
		};

		auto GetStr = [&cr](int n) -> String
		{
			bool b = cr.GetCount() < max(1, n);
			return b ? Null : cr[--n];
		};

		auto GetChrset = [=](int i) -> byte
		{
			// TODO: This can be more precise...
			if(i == '0') return CHARSET_DEC_DCS;
			if(i == '>') return CHARSET_DEC_TCS;
			if(i == '<') return CHARSET_DEC_MCS;
			if(i == 'A') return CHARSET_ISO8859_1;
			return CHARSET_TOASCII;
		};
		
		Point pt;
		pt.y      = GetInt(1);
		pt.x      = GetInt(2);
		int sgr   = GetInt(4);
		int attrs = GetInt(5);
		int flags = GetInt(6);
		int gl    = GetInt(7);
		int gr    = GetInt(8);
		int sz	  = GetInt(9);
		String gs = GetStr(10);
		
		DECom(flags & 0x01);
		DECawm(flags & 0x08);
		
		cellattrs.Bold(sgr & 0x01);
		cellattrs.Blink(sgr & 0x04);
		cellattrs.Invert(sgr & 0x08);
		cellattrs.Underline(sgr & 0x02);
		cellattrs.Protect(attrs & 0x01);

		page->Attributes(cellattrs);

		page->MoveTo(pt);
		
		if(flags & 0x02)
			gsets.SS(0x8E);
		else
		if(flags & 0x04)
			gsets.SS(0x8F);
		
		if(IsNull(gs))
			return;
		
		for(int i = 0; i < gs.GetLength(); i++) {
			switch(i) {
			case 0:
				gsets.G0(GetChrset(gs[i]));
				break;
			case 1:
				gsets.G1(GetChrset(gs[i]));
				break;
			case 2:
				gsets.G2(GetChrset(gs[i]));
				break;
			case 3:
				gsets.G3(GetChrset(gs[i]));
				break;
			}
		}
		
		switch(gl) {
		case 0:
			gsets.G0toGL();
			break;
		case 1:
			gsets.G1toGL();
			break;
		case 2:
			gsets.G2toGL();
			break;
		case 3:
			gsets.G3toGL();
			break;
		}

		switch(gr) {
		case 1:
			gsets.G1toGR();
			break;
		case 2:
			gsets.G2toGR();
			break;
		case 3:
			gsets.G3toGR();
			break;
		}
	}
	else
	if(which == 2) {	// DECTABSR
		Vector<String> stab = Split(seq.payload, '/');

		page->ClearTabs();

		for(const auto& s : stab) {
			int pos = StrInt(s);
			if(pos > 0)
				page->SetTabAt(pos, true);
		}
	}
}

void Terminal::ParseSixelGraphics(const VTInStream::Sequence& seq)
{
	if(!sixelimages)
		return;

	int  nohole = seq.GetInt(2, 0);
	int  grid   = seq.GetInt(3, 0); // Omitted.
	int  ratio = 1;

	switch(seq.GetInt(1, 1)) {
	case 5 ... 6:
		ratio = 2;
		break;
	case 3 ... 4:
		ratio = 3;
		break;
	case 2:
		ratio = 5;
		break;
	default:
		ratio = 1;
		break;
	}
	
	cellattrs.Hyperlink(false);

	ImageString data(pick(Format("\033P%d;%d;q%s`\033\x5C", ratio, nohole, seq.payload)));
	data.encoded = false;
	
	RenderImage(data, modes[DECSDM]);
}
}