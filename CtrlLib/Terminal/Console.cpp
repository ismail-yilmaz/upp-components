#include "Console.h"

#define LLOG(x)		// RLOG("Console: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

Console::Console()
: page(&dpage),
  use_gsets(true),
  streamfill(false)
{
	SetLevel(LEVEL_4);
	Set8BitsMode(false);
	SetCharset(CHARSET_UNICODE);
	parser.WhenChr = THISFN(PutChar);
	parser.WhenCtl = THISFN(ParseControlChars);
	parser.WhenEsc = THISFN(ParseEscapeSequences);
	parser.WhenCsi = THISFN(ParseCommandSequences);
	parser.WhenDcs = THISFN(ParseDeviceControlStrings);
	parser.WhenOsc = THISFN(ParseOperatingSystemCommands);
	EnableUDK();
	UnlockUDK();
}

Console& Console::SetLevel(int level)
{
	SoftReset();

	clevel = level;

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
	return *this;
}

void Console::Reset(bool full)
{
	LLOG("Performing " << (full ? "full" : "soft") << " reset...");
	if(full) {
		Put(0x13);	// XOFF
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
	modes.Set(ERM);	// TODO: Check the documents. I'm not entirely sure about this one...
	modes.Set(SRM);
	modes.Set(DECTCEM);
	modes.Set(DECANM);
	modes.Set(DECAWM);

	dpage.SetTabs(8);
	dpage.OriginMode(false);
	dpage.WrapAround(true);

	apage.SetTabs(8);
	apage.OriginMode(false);
	apage.WrapAround(true);

	caret = Caret();
	
	CancelOut();

	if(full)
		Put(0x11);	// XON
}

void Console::Backup(bool tpage, bool csets, bool attrs)
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

void Console::Restore(bool tpage, bool csets, bool attrs)
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

void Console::PutChar(int c)
{
	VTCell cell = cellattrs;
	cell.chr = LookupChar(c);
	if(modes[IRM])
		page->InsertCell(cell);
	else
		page->AddCell(cell);
}

void Console::AlternateScreenBuffer(bool b)
{
	if(b && IsDefaultPage())
		page = &apage;
	else
	if(!b && IsAlternatePage()) {
		page = &dpage;
	}
	LLOG("Alternate screen buffer: " << (b ? "on" : "off"));
}

void Console::DisplayAlignmentTest()
{
	LLOG("Performing display alignment test...");
	for(auto& line : *page)
		for(auto& cell : line) {
			cell.Reset();
			cell = 'E';
		}
}

void Console::Write(const void *data, int size, bool utf8)
{
	if(size > 0) {
		PreParse();
		parser.Parse(data, size, utf8);
		PostParse();
	}
}

void Console::Flush()
{
	WhenOutput(out);
	if(!modes[SRM]) // Local echo on/off.
		WriteUtf8(out);
	out = Null;
}

void Console::PutC(int c)
{
	if(Is8BitsMode())
		out.Cat(c);
	else
	if(c >= 0x80 && 0x9F >= c) {
		out.Cat(0x1B);
		out.Cat(c - 0x40);
	}
	else
	if(!IsLevel2() && c >= 0xA0 && 0xFF >= c)
		out.Cat(c & 0x7F);
	else
		out.Cat(c);
	// Don't flush
}

void Console::PutC(const String& s)
{
	for(const auto& c: s)
		PutC(c);
}

void Console::Put(const char *s, int cnt)
{
	while(cnt-- > 0)
		PutC(s);
	Flush();
}

void Console::Put(int c, int cnt)
{
	while(cnt-- > 0)
		PutC(c);
	Flush();
}

void Console::PutUtf8(int c, int cnt)
{
	String s;
	
	word code = c;
	if(code < 0x80)
		s.Cat(code);
	else
	if(code < 0x800) {
		s.Cat(0xc0 | (code >> 6));
		s.Cat(0x80 | (code & 0x3f));
	}
	else
	if((code & 0xFF00) == 0xEE00)
		s.Cat(code);
	else {
		s.Cat(0xe0 | (code >> 12));
		s.Cat(0x80 | ((code >> 6) & 0x3f));
		s.Cat(0x80 | (code & 0x3f));
	}
	PutRaw(s, cnt);
}

void Console::PutRaw(const char *s, int cnt)
{
	while(cnt-- > 0)
		out.Cat(s);
	Flush();
}

void Console::PutESC(const String& s, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x1B);
		PutC(s);
	}
	Flush();
}

void Console::PutESC(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x1b);
		Put(c);
	}
	Flush();
}

void Console::PutCSI(const String& s, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x9B);
		PutC(s);
	}
	Flush();
}

void Console::PutCSI(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x9B);
		PutC(c);
	}
	Flush();
}

void Console::PutOSC(const String& s, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x9D);
		PutRaw(s);
		PutC(0x07);
	}
	Flush();
}

void Console::PutOSC(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x9D);
		PutC(c);
		PutC(0x07);
	}
	Flush();
}

void Console::PutDCS(const String& s, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x90);
		PutRaw(s);
		PutC(0x9C);
	}
	Flush();
}

void Console::PutDCS(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x90);
		PutC(c);
		PutC(0x9C);
	}
	Flush();
}

void Console::PutSS2(const String& s, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x8E);
		PutC(s);
	}
	Flush();
}

void Console::PutSS2(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x8E);
		PutC(c);
	}
	Flush();
}

void Console::PutSS3(const String& s, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x8F);
		PutC(s);
	}
	Flush();
}

void Console::PutSS3(int c, int cnt)
{
	while(cnt-- > 0) {
		PutC(0x8F);
		PutC(c);
	}
	Flush();
}

void Console::PutEncoded(const String& s, bool noctl)
{
	LTIMING("Console::PutEncoded");

	String txt, buf = s;

	if(!modes[LNM])
		buf.Replace("\n", "\r");

	for(int c : buf) {
		c = ConvertToCharset(c, gsets.Get(c, IsLevel2()));
		if(!noctl		||
			IsSpace(c)	||
				(c >= 0x20 && c <= 0xFFFF))
					txt.Cat(c == DEFAULTCHAR ? '?' : c);
	}
	PutRaw(txt);
}

void Console::PutEncoded(const WString& s, bool noctl)
{
	PutEncoded(ToUtf8(s), noctl);
}

void Console::PutEol()
{
	Put(modes[LNM] ? "\r\n" : "\r");
}

WString Console::AsWString(const VTPage::Line& line)
{
	WString s;
	if(!line.IsEmpty()) {
		int i = 0;
		for(const VTCell& cell : line) {
			if(cell.chr == 0) i++;
			else {
				if(i) {
					s.Cat(' ', i);
					i = 0;
				}
				s.Cat(cell);
			}
		}
	}
	return s;
}

void Console::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % clevel;
		s % eightbits;
		s % dpage;
		s % apage;
		s % charset;
		s % gsets;
		s % use_gsets;
		s % udkenabled;
		s % caret;
		for(int i = 0; i < 21; i++)
			s % colortable[i];
	}
}

Console::Caret::Caret()
{
	style = BLOCK;
	blinking = true;
	locked = false;
}

Console::Caret::Caret(byte style_, bool blink, bool lock)
{
	Set(style_, blink);
	locked = lock;
}

void Console::Caret::Set(byte style_, bool blink)
{
	if(!locked) {
		style = style_;
		blinking = blink;
		WhenAction();
	}
}

void Console::Caret::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % style;
		s % locked;
		s % blinking;
	}
}
}