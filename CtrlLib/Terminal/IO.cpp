#include "Terminal.h"

#define LLOG(x)		// RLOG("Terminal: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void Terminal::InitParser(VTInStream& vts)
{
	vts.Reset();
	vts.WhenChr = THISFN(PutChar);
	vts.WhenCtl = THISFN(ParseControlChars);
	vts.WhenEsc = THISFN(ParseEscapeSequences);
	vts.WhenCsi = THISFN(ParseCommandSequences);
	vts.WhenDcs = THISFN(ParseDeviceControlStrings);
	vts.WhenOsc = THISFN(ParseOperatingSystemCommands);
	vts.WhenApc = THISFN(ParseApplicationProgrammingCommands);
}

void Terminal::SetEmulation(int level, bool reset)
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

void Terminal::Reset(bool full)
{
	LLOG("Performing " << (full ? "full" : "soft") << " reset...");
	
	if(full) {
		AlternateScreenBuffer(false);
		DECcolm(false);
		dpage.Reset();
		apage.Reset();
		gsets_backup.Reset();
		cellattrs_backup = Null;
		dpage.WhenScroll();
	}

	gsets.Reset();

	cellattrs.Reset();

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
	
	CancelOut();
}

void Terminal::Backup(bool tpage, bool csets, bool attrs)
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

void Terminal::Restore(bool tpage, bool csets, bool attrs)
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

void Terminal::PutChar(int c)
{
	VTCell cell = cellattrs;
	cell.chr = LookupChar(c);
	if(modes[IRM])
		page->InsertCell(cell);
	else
		page->AddCell(cell);
}

void Terminal::Write(const void *data, int size, bool utf8)
{
	if(size > 0) {
		PreParse();
		parser.Parse(data, size, utf8);
		PostParse();
	}
}

void Terminal::Flush()
{
	if(out.IsEmpty())
		return;
	
	LLOG("Flush() -> " << out.GetLength() << " bytes.");
	
	WhenOutput(out);
	if(!modes[SRM]) // Local echo on/off.
		Echo(out);
	out = Null;
}

Terminal& Terminal::Put0(int c, int cnt)
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

Terminal& Terminal::Put0(const String& s, int cnt)
{
	while(cnt-- > 0)
		for(const byte& c : s) Put0(c);
	return *this;
}

Terminal& Terminal::Put(const WString& s, int cnt)
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

Terminal& Terminal::Put(int c, int cnt)
{
	if(IsUtf8Mode())
		while(cnt-- > 0) out.Cat(ToUtf8(c));
	else
		Put0(c, cnt);
	Flush();
	return *this;
}

Terminal& Terminal::PutRaw(const String& s, int cnt)
{
	LLOG("PutRaw() -> " << s);
	
	while(cnt-- > 0) out.Cat(s);
	return *this;
}

Terminal& Terminal::PutESC(const String& s, int cnt)
{
	LLOG("PutESC() -> " << s);
	
	while(cnt-- > 0) { Put0(0x1B).Put0(s); }
	Flush();
	return *this;
}

Terminal& Terminal::PutESC(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x1B).Put0(c); }
	Flush();
	return *this;
}

Terminal& Terminal::PutCSI(const String& s, int cnt)
{
	LLOG("PutOSC() -> " << s);

	while(cnt-- > 0) { Put0(0x9B).Put0(s); }
	Flush();
	return *this;
}

Terminal& Terminal::PutCSI(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x9B).Put0(c); }
	Flush();
	return *this;
}

Terminal& Terminal::PutOSC(const String& s, int cnt)
{
	LLOG("PutOSC() -> " << s);

	while(cnt-- > 0) { Put0(0x9D).PutRaw(s).Put0(0x9C); }
	Flush();
	return *this;
}

Terminal& Terminal::PutOSC(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x9D).Put0(c).Put0(0x9C); }
	Flush();
	return *this;
}

Terminal& Terminal::PutDCS(const String& s, int cnt)
{
	LLOG("PutDCS() -> " << s);

	while(cnt-- > 0) { Put0(0x90).PutRaw(s).Put0(0x9C); }
	Flush();
	return *this;
}

Terminal& Terminal::PutDCS(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x90).Put0(c).Put0(0x9C); }
	Flush();
	return *this;
}

Terminal& Terminal::PutSS2(const String& s, int cnt)
{
	LLOG("PutSS2() -> " << s);

	while(cnt-- > 0) { Put0(0x8E).Put0(s); }
	Flush();
	return *this;
}

Terminal& Terminal::PutSS2(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x8E).Put0(c); }
	Flush();
	return *this;
}

Terminal& Terminal::PutSS3(const String& s, int cnt)
{
	LLOG("PutSS3() -> " << s);
	
	while(cnt-- > 0) { Put0(0x8F).Put0(s); }
	Flush();
	return *this;
}

Terminal& Terminal::PutSS3(int c, int cnt)
{
	while(cnt-- > 0) { Put0(0x8F).Put0(c); }
	Flush();
	return *this;
}

Terminal& Terminal::PutEncoded(const WString& s, bool noctl)
{
	LTIMING("Terminal::PutEncoded");

	WString txt = s;

	if(!modes[LNM])
		txt.Replace("\r\n", "\r");

	auto sControlCharFilter = [](int c) -> int
	{
		return c * int(c > 0x20 || IsSpace(c));
	};

	return Put(noctl ? Filter(~txt, sControlCharFilter) : txt);
}

Terminal& Terminal::PutEol()
{
	Put0(modes[LNM] ? "\r\n" : "\r");
	Flush();
	return *this;
}

Terminal& Terminal::Echo(const String& s)
{
	VTInStream echoparser;
	InitParser(echoparser);
	echoparser.Parse(out, IsUtf8Mode());
	return *this;
}

void Terminal::Serialize(Stream& s)
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

void Terminal::Jsonize(JsonIO& jio)
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
        ("Caret",               caret)
        ("ReverseWrap",         reversewrap)
        ("KeyNavigation",       keynavigation)
        ("MetaKeyFlags",        metakeyflags)
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

void Terminal::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

}