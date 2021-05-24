#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void TerminalCtrl::ParseOperatingSystemCommands(const VTInStream::Sequence& seq)
{
	LLOG(seq);

	int opcode = seq.GetInt(1, 0);

	switch(opcode) {
	case 0:		// Set window titile
	case 2:		// Set window title (and icon name)
		WhenTitle(DecodeDataString(seq.GetStr(2)).ToString());
		break;
	case 4:		// Set ANSI colors
	case 10:	// Set dynamic color (ink)
	case 11:	// Set dynamic color (paper)
	case 17:	// Set dynamic color (selection ink)
	case 19:	// Set dynamic color (selection paper)
		SetProgrammableColors(seq, opcode);
		break;
	case 104:	// Reset ANSI colors
	case 110:	// Reset dynamic color (ink)
	case 111:	// Reset dynamic color (paper)
	case 117:	// Reset dynamic color (selection ink)
	case 119:	// Reset dynamic color (selection paper)
		ResetProgrammableColors(seq, opcode);
		break;
	case 8:		// Explicit hyperlinks protocol.
		ParseHyperlinks(seq);
		break;
	case 52:	// Clipboard manipulation protocol.
		ParseClipboardRequests(seq);
		break;
	case 444:	// Jexer inline images protocol.
		ParseJexerGraphics(seq);
		break;
	case 1337:	// iTerm2 inline images protocol.
		ParseiTerm2Graphics(seq);
		break;
	default:
		LLOG(Format("Unhandled OSC opcode: %d", opcode));
		break;
	}
}

void TerminalCtrl::ParseJexerGraphics(const VTInStream::Sequence& seq)
{
	// For more information on Jexer image protocol, see:
	// https://gitlab.com/klamonte/jexer/-/wikis/jexer-images

	if(!jexerimages)
		return;
	
	int type = seq.GetInt(2, Null);
	if(type > 2 || IsNull(type))	// V1 defines 3 types (0-based).
		return;

	ImageString simg;
	bool scroll = false;

	if(type == 0) {	// Bitmap
		simg.size.cx = min(seq.GetInt(3), 10000);
		simg.size.cy = min(seq.GetInt(4), 10000);
		scroll       = seq.GetInt(5, 0) > 0;
		simg.data    = pick(seq.GetStr(6));
	}
	else { // Other image formats (jpg, png, etc.)
		scroll       = seq.GetInt(3, 0) > 0;
		simg.data    = pick(seq.GetStr(4));
	}

	cellattrs.Hyperlink(false);

	RenderImage(simg, scroll);
}

void TerminalCtrl::ParseiTerm2Graphics(const VTInStream::Sequence& seq)
{
	// iTerm2's file and image download and display protocol,
	// Currently, we only support its inline images  portion.
	// See: https://iterm2.com/documentation-images.html
	
	if(!iterm2images)
		return;
	
	int pos = 0;
	String options, enc;
	if(!SplitTo(seq.payload, ':', false, options, enc)
	|| (pos = ToLower(options).FindAfter("file=")) < 0 || IsNull(enc))
		return;

	auto GetVal = [](const String& s, int p, int f) -> int
	{
		int n = max(StrInt(s), 0);
		if(!n)
			return n;
		if(s.IsEqual("auto"))
			return 0;
		if(s.EndsWith("px"))
			return min(n, 10000);
		if(s.EndsWith("%"))
			return (min(n, 1000) * p * f) / 100;
		return n * f;
	};

	ImageString simg(pick(enc));
	
	simg.size.Clear();
	bool show = false;
	
	for(const String& s : Split(options.Mid(pos), ';', false)) {
		String key, val;
		if(SplitTo(ToLower(s), '=', false, key, val)) {
			if(key.IsEqual("inline"))
				show = val == "1";
			else
			if(key.IsEqual("width"))
				simg.size.cx = GetVal(val, GetPageSize().cx, GetCellSize().cx);
			else
			if(key.IsEqual("height"))
				simg.size.cy = GetVal(val, GetPageSize().cy, GetCellSize().cy);
			else
			if(key.IsEqual("preserveaspectratio"))
				simg.keepratio = val == "1";
		}
	}
	
	if(!show)
		return;

	if(simg.size.cx == 0 && simg.size.cy == 0)
		simg.size.SetNull();

	RenderImage(simg, modes[DECSDM]);	// Rely on sixel scrolling mode.
}

void TerminalCtrl::ParseHyperlinks(const VTInStream::Sequence& seq)
{
	// For more information on explicit hyperlinks, see:
	// https://gist.github.com/egmontkob/eb114294efbcd5adb1944c9f3cb5feda

	if(!hyperlinks)
		return;

	constexpr const int MAX_URI_LENGTH = 2084;

	String uri = seq.GetStr(3);

	if(IsNull(uri) || uri.GetLength() > MAX_URI_LENGTH) {
		cellattrs.Hyperlink(false);
		cellattrs.data = 0;
	}
	else {
		uri = UrlDecode(uri);
		cellattrs.Image(false);
		cellattrs.Hyperlink(true);
		cellattrs.data = FoldHash(GetHashValue(uri));
		RenderHyperlink(uri);
	}
}

void TerminalCtrl::ParseClipboardRequests(const VTInStream::Sequence& seq)
{
	// For more information on application clipboard access, see:
	// https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
	
	if(!IsClipboardAccessPermitted() || !HasFocus())
		return;

	auto CheckInvalidBase64Chars = [](int c) -> bool
	{
		return !IsAlNum(c) && c != '=' && c != '+' && c != '/';
	};
	
	String params = seq.GetStr(2);	// We don't support multiple clipboard buffers...
	String data   = seq.GetStr(3);

	if(IsClipboardReadPermitted() && data.IsEqual("?")) {
		WString out = GetWString(Selection());
		if(IsNull(out))
			out = GetWString(Clipboard());
		PutOSC("52;s0;" + Base64Encode(EncodeDataString(out)));
	}
	else
	if(IsClipboardWritePermitted()) {
		if(FindMatch(data, CheckInvalidBase64Chars) < 0) {
			String in = Base64Decode(data);
			if(!IsNull(in))
				Copy(DecodeDataString(in));
		}
		else
			ClearClipboard();
	}
}
}