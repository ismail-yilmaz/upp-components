#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void TerminalCtrl::InitParser(VTInStream& vts)
{
	vts.Reset();
	vts.WhenChr = [=](int c) { PutChar(c); };
	vts.WhenCtl = [=](byte c) { ParseControlChars(c); };
	vts.WhenEsc = [=](const VTInStream::Sequence& seq) { ParseEscapeSequences(seq); };
	vts.WhenCsi = [=](const VTInStream::Sequence& seq) { ParseCommandSequences(seq); };
	vts.WhenDcs = [=](const VTInStream::Sequence& seq) { ParseDeviceControlStrings(seq); };
	vts.WhenOsc = [=](const VTInStream::Sequence& seq) { ParseOperatingSystemCommands(seq); };
	vts.WhenApc = [=](const VTInStream::Sequence& seq) { ParseApplicationProgrammingCommands(seq); };
}

void TerminalCtrl::SetEmulation(int level, bool reset)
{
	if(reset)
		SoftReset();

	clevel = clamp(level, int(LEVEL_0), int(LEVEL_4));

	switch(level) {
	case LEVEL_0:
		DECanm(false);
		break;
	case LEVEL_1:
	case LEVEL_2:
	case LEVEL_3:
	case LEVEL_4:
	default:
		break;
	}
}

void TerminalCtrl::Reset(bool full)
{
	LLOG("Performing " << (full ? "full" : "soft") << " reset...");
	
	if(full) {
		AlternateScreenBuffer(false);
		DECcolm(false);
		dpage.Reset();
		apage.Reset();
		gsets_backup.Reset();
		cellattrs_backup = Null;
		dpage.WhenUpdate();
	}
	else {
		apage.Discard();
		dpage.Discard();
	}

	gsets.Reset();

	cellattrs.Clear();

	udk.Clear();
	
	modes.Clear();
	modes.Set(SRM);
	modes.Set(DECTCEM);
	modes.Set(DECANM);
	modes.Set(DECAWM);
	modes.Set(DECSDM);

	dpage.SetTabs(8);
	dpage.Displaced(false);
	dpage.AutoWrap(true);
	dpage.ReverseWrap(false);

	apage.SetTabs(8);
	apage.Displaced(false);
	apage.AutoWrap(true);
	apage.ReverseWrap(false);
	
	caret = Caret();
	caret.WhenAction = [=] { ScheduleRefresh(); };
	
	CancelOut();
}

void TerminalCtrl::Backup(bool tpage, bool csets, bool attrs)
{
	if(tpage)
		page->Backup();
	if(attrs)
		cellattrs_backup = cellattrs;
	if(csets) {
		gsets_backup  = gsets;
		gsets.Reset();
	}
}

void TerminalCtrl::Restore(bool tpage, bool csets, bool attrs)
{
	if(tpage)
		page->Restore();
	if(attrs) {
		cellattrs = cellattrs_backup;
		cellattrs_backup = Null;
	}
	if(csets) {
		gsets = gsets_backup;
		gsets_backup.Reset();
	}
}

void TerminalCtrl::PutChar(int c)
{
	VTCell cell = cellattrs;
	cell.chr = LookupChar(c);
	if(modes[IRM])
		page->InsertCell(cell);
	else
		page->AddCell(cell);
}

void TerminalCtrl::Write(const void *data, int size, bool utf8)
{
	if(size > 0) {
		PreParse();
		parser.Parse(data, size, utf8);
		PostParse();
	}
}

void TerminalCtrl::Flush()
{
	if(out.IsEmpty())
		return;
	
	LLOG("Flush() -> " << out.GetLength() << " bytes.");
	
	WhenOutput(out);
	if(!modes[SRM]) // Local echo on/off.
		Echo(out);
	out = Null;
}

TerminalCtrl& TerminalCtrl::Put0(int c, int cnt)
{
	bool bit8 = Is8BitMode();
	bool lvl2 = IsLevel2();

	c &= 0xFF;

	while(cnt-- > 0)
	{
		if(c >= 0x00 && c <= 0x7F)
		{
			out.Cat(c);
		}
		else
		if(c >= 0x80 && c <= 0x9F)
		{
			if(bit8)
			{
				 out.Cat(c);
			}
			else
			{
				out.Cat(0x1B);
				out.Cat(c - 0x40);
			}
		}
		else
		if(c >= 0xA0 && c <= 0xFF)
		{
			if(lvl2)
			{
				out.Cat(c);
			}
			else
			{
				out.Cat(c & 0x7F);
			}
		}
	}
	
	return *this;
}

TerminalCtrl& TerminalCtrl::Put0(const String& s, int cnt)
{
	while(cnt-- > 0)
		for(const byte& c : s) Put0(c);
	return *this;
}

TerminalCtrl& TerminalCtrl::Put(const WString& s, int cnt)
{
	if(IsUtf8Mode()) {
		String txt = ToUtf8(s);
		while(cnt-- > 0) out.Cat(txt);
	}
	else
		Put0(s.ToString(), cnt);
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::Put(int c, int cnt)
{
	if(IsUtf8Mode())
		while(cnt-- > 0) out.Cat(ToUtf8(c));
	else
		Put0(c, cnt);
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutRaw(const String& s, int cnt)
{
	LLOG("PutRaw() -> " << s);
	
	while(cnt-- > 0) out.Cat(s);
	return *this;
}

TerminalCtrl& TerminalCtrl::PutESC(const String& s, int cnt)
{
	LLOG("PutESC() -> " << s);
	
	while(cnt-- > 0) { Put0(0x1B).Put0(s); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutESC(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x1B).Put0(c); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutCSI(const String& s, int cnt)
{
	LLOG("PutOSC() -> " << s);

	while(cnt-- > 0) { Put0(0x9B).Put0(s); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutCSI(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x9B).Put0(c); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutOSC(const String& s, int cnt)
{
	LLOG("PutOSC() -> " << s);

	while(cnt-- > 0) { Put0(0x9D).PutRaw(s).Put0(0x9C); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutOSC(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x9D).Put0(c).Put0(0x9C); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutDCS(const String& s, int cnt)
{
	LLOG("PutDCS() -> " << s);

	while(cnt-- > 0) { Put0(0x90).PutRaw(s).Put0(0x9C); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutDCS(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x90).Put0(c).Put0(0x9C); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutSS2(const String& s, int cnt)
{
	LLOG("PutSS2() -> " << s);

	while(cnt-- > 0) { Put0(0x8E).Put0(s); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutSS2(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x8E).Put0(c); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutSS3(const String& s, int cnt)
{
	LLOG("PutSS3() -> " << s);
	
	while(cnt-- > 0) { Put0(0x8F).Put0(s); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutSS3(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x8F).Put0(c); }
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::PutEncoded(const WString& s, bool noctl)
{
	LTIMING("TerminalCtrl::PutEncoded");

	WString txt = s;

	if(!modes[LNM])
		txt.Replace("\r\n", "\r");

	auto sControlCharFilter = [](int c) -> int
	{
		return c * int(c > 0x20 || IsSpace(c));
	};

	return Put(noctl ? Filter(~txt, sControlCharFilter) : txt);
}

TerminalCtrl& TerminalCtrl::PutEol()
{
	Put0(modes[LNM] ? "\r\n" : "\r");
	Flush();
	return *this;
}

TerminalCtrl& TerminalCtrl::Echo(const String& s)
{
	VTInStream echoparser;
	InitParser(echoparser);
	PreParse();
	echoparser.Parse(s, IsUtf8Mode());
	PostParse();
	return *this;
}

void TerminalCtrl::Serialize(Stream& s)
{
	GuiLock __;

	ColorTableSerializer cts(colortable);
	String chrset = CharsetName(charset);
    
	int version = 1;
	s / version;

	if(version >= 1) {
		s % clevel;
		s % chrset;
		s % eightbit;
		s % font;
		s % caret;
		s % reversewrap;
		s % keynavigation;
		s % legacycharsets;
		s % alternatescroll;
		s % wheelstep;
		s % hidemousecursor;
		s % userdefinedkeys;
		s % userdefinedkeyslocked;
		s % pcstylefunctionkeys;
		s % metakeyflags;
		s % windowactions;
		s % windowreports;
		s % sixelimages;
		s % jexerimages;
		s % iterm2images;
		s % hyperlinks;
		s % clipaccess;
		s % delayedrefresh;
		s % lazyresize;
		s % sizehint;
		s % nobackground;
		s % intensify;
		s % blinkingtext;
		s % blinkinterval;
		s % dynamiccolors;
		s % adjustcolors;
		s % lightcolors;
		s % padding;
		s % gsets;
		s % dpage;
		s % cts;
	}

	if(s.IsLoading()) {
		SetCharset(CharsetByName(chrset));
		SetEmulation(clevel, false);
		Layout();
	}
}

void TerminalCtrl::Jsonize(JsonIO& jio)
{
	GuiLock __;

    ColorTableSerializer cts(colortable);
    String chrset = CharsetName(charset);
    
    jio ("ConformanceLevel",    clevel)
        ("EightBit",            eightbit)
        ("Charset",             chrset)
        ("LegacyCharsets",      legacycharsets)
        ("GSets",               gsets)
        ("Page",                dpage)
        ("Font",                font)
        ("CellPadding",         padding)
        ("Caret",               caret)
        ("ReverseWrap",         reversewrap)
        ("KeyNavigation",       keynavigation)
        ("MetaKeyFlags",        metakeyflags)
        ("PCStyleFunctionKeys", pcstylefunctionkeys)
        ("UDK",                 userdefinedkeys)
        ("LockUDK",             userdefinedkeyslocked)
        ("AlternateScroll",     alternatescroll)
        ("WheelStep",           wheelstep)
        ("AutoHideMouseCursor", hidemousecursor)
        ("WindowActions",       windowactions)
        ("WindowReports",       windowreports)
        ("SixelGraphics",       sixelimages)
        ("JexerGraphics",       jexerimages)
        ("iTerm2Graphics",      iterm2images)
        ("Hyperlinks",          hyperlinks)
        ("ClipboardAccess",     clipaccess)
        ("DelayedRefresh",      delayedrefresh)
        ("LazyResize",          lazyresize)
        ("SizeHint",            sizehint)
        ("BrightBoldText",      intensify)
        ("BlinkingText",        blinkingtext)
        ("TextBlinkInterval",   blinkinterval)
        ("DynamicColors",       dynamiccolors)
        ("LightColors",         lightcolors)
        ("AdjustColorstoTheme", adjustcolors)
        ("TransparentBackground", nobackground)
        ("ColorTable",          cts);
        
    if(jio.IsLoading()) {
        SetCharset(CharsetByName(chrset));
        SetEmulation(clevel, false);
        Layout();
    }
}

void TerminalCtrl::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

}