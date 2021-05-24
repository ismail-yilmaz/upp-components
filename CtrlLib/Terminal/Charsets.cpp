#include "Terminal.h"

namespace Upp {

#ifndef CUNDEF
#define CUNDEF DEFAULTCHAR
#endif

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

byte CHARSET_DEC_VT52 = 0;
byte CHARSET_DEC_DCS  = 0;
byte CHARSET_DEC_MCS  = 0;
byte CHARSET_DEC_TCS  = 0;

static word CHRTAB_DEC_VT52_GRAPHICS[128] = {	// VT52 (this charset is adapted from xterm)
	0X0000, 0X0001, 0X0002, 0X0003, 0X0004, 0X0005, 0X0006, 0X0007,
	0X0008, 0X0009, 0X000A, 0X000B, 0X000C, 0X000D, 0X000E, 0X000F,
	0X0010, 0X0011, 0X0012, 0X0013, 0X0014, 0X0015, 0X0016, 0X0017,
	0X0018, 0X0019, 0X001A, 0X001B, 0X001C, 0X001D, 0X001E, 0X001F,
	0X0020, 0X0021, 0X0022, 0X0023, 0X0024, 0X0025, 0X0026, 0X0027,
	0X0028, 0X0029, 0X002A, 0X002B, 0X002C, 0X002D, 0X002E, 0X002F,
	0X0030, 0X0031, 0X0032, 0X0033, 0X0034, 0X0035, 0X0036, 0X0037,
	0X0038, 0X0039, 0X003A, 0X003B, 0X003C, 0X003D, 0X003E, 0X003F,
	0X0040, 0X0041, 0X0042, 0X0043, 0X0044, 0X0045, 0X0046, 0X0047,
	0X0048, 0X0049, 0X004A, 0X004B, 0X004C, 0X004D, 0X004E, 0X004F,
	0X0050, 0X0051, 0X0052, 0X0053, 0X0054, 0X0055, 0X0056, 0X0057,
	0X0058, 0X0059, 0X005A, 0X005B, 0X005C, 0X005D, 0X005E, 0X25AE,
	0X215F, 0x0020, 0x0020, 0x0020, 0X00B0, 0X00B1, 0X2192, 0X2026,
	0X00F7, 0X2193, 0X23BA, 0X23BA, 0X23BB, 0X23BB,	0X23BC, 0X23BC,
	0X23BD, 0X23BD, 0X2080, 0X2081, 0X2082, 0X2083, 0X2084, 0X2085,
	0X2086, 0X2087, 0X2088, 0X2089, 0X00B6, 0x0020, 0x0020, 0x007F
};

static word CHRTAB_DEC_LINEDRAWING[128] = {	// VT100+
	0X0000, 0X0001, 0X0002, 0X0003, 0X0004, 0X0005, 0X0006, 0X0007,
	0X0008, 0X0009, 0X000A, 0X000B, 0X000C, 0X000D, 0X000E, 0X000F,
	0X0010, 0X0011, 0X0012, 0X0013, 0X0014, 0X0015, 0X0016, 0X0017,
	0X0018, 0X0019, 0X001A, 0X001B, 0X001C, 0X001D, 0X001E, 0X001F,
	0X0020, 0X0021, 0X0022, 0X0023, 0X0024, 0X0025, 0X0026, 0X0027,
	0X0028, 0X0029, 0X002A, 0X002B, 0X002C, 0X002D, 0X002E, 0X002F,
	0X0030, 0X0031, 0X0032, 0X0033, 0X0034, 0X0035, 0X0036, 0X0037,
	0X0038, 0X0039, 0X003A, 0X003B, 0X003C, 0X003D, 0X003E, 0X003F,
	0X0040, 0X0041, 0X0042, 0X0043, 0X0044, 0X0045, 0X0046, 0X0047,
	0X0048, 0X0049, 0X004A, 0X004B, 0X004C, 0X004D, 0X004E, 0X004F,
	0X0050, 0X0051, 0X0052, 0X0053, 0X0054, 0X0055, 0X0056, 0X0057,
	0X0058, 0X0059, 0X005A, 0X005B, 0X005C, 0X005D, 0X005E, 0X00A0,
	0X25C6, 0X2592, 0X2409, 0X240C, 0X240D, 0X240A, 0X00B0, 0X00B1,
	0X2424, 0X240B, 0X2518, 0X2510, 0X250C, 0X2514, 0X253C, 0X23BA,
	0X23BB, 0X2500, 0X23BC, 0X23BD, 0X251C, 0X2524, 0X2534, 0X252C,
	0X2502, 0X2264, 0X2265, 0X03C0, 0X2260, 0X00A3, 0X00B7, 0X007F
};

static word CHRTAB_DEC_MULTINATIONAL[128] = {	// VT200+
	0X0080, 0X0081, 0X0082, 0X0083, 0X0084, 0X0085, 0X0086, 0X0087,
	0X0088, 0X0089, 0X008A, 0X008B, 0X008C, 0X008D, 0X008E, 0X008F,
	0X0090, 0X0091, 0X0092, 0X0093, 0X0094, 0X0095, 0X0096, 0X0097,
	0X0098, 0X0099, 0X009A, 0X009B, 0X009C, 0X009D, 0X009E, 0X009F,
	0x00A0, 0X00A1, 0X00A2, 0X00A3, CUNDEF, 0X00A5, CUNDEF, 0X00A7,
	0X00A8, 0X00A9, 0X00AA, 0X00AB, CUNDEF, CUNDEF, CUNDEF, CUNDEF,
	0X00B0, 0X00B1, 0X00B2, 0X00B3, CUNDEF, 0X00B5, 0X00B6, 0X00B7,
	CUNDEF, 0X00B9, 0X00BA, 0X00BB, 0X00BC, 0X00BD, CUNDEF, 0X00BF,
	0X00C0, 0X00C1, 0X00C2, 0X00C3, 0X00C4, 0X00C5, 0X00C6, 0X00C7,
	0X00C8, 0X00C9, 0X00CA, 0X00CB, 0X00CC, 0X00CD, 0X00CE, 0X00CF,
	CUNDEF, 0X00D1, 0X00D2, 0X00D3, 0X00D4, 0X00D5, 0X00D6, 0X0152,
	0X00D8, 0X00D9, 0X00DA, 0X00DB, 0X00DC, 0X0178, CUNDEF, 0X00DF,
	0X00E0, 0X00E1, 0X00E2, 0X00E3, 0X00E4, 0X00E5, 0X00E6, 0X00E7,
	0X00E8, 0X00E9, 0X00EA, 0X00EB, 0X00EC, 0X00ED, 0X00EE, 0X00EF,
	CUNDEF, 0X00F1, 0X00F2, 0X00F3, 0X00F4, 0X00F5, 0X00F6, 0X0153,
	0X00F8, 0X00F9, 0X00FA, 0X00FB, 0X00FC, 0X00FF, CUNDEF, CUNDEF
};

static word CHRTAB_DEC_TECHNICAL[128] = {	// VT300+
	0X0000, 0X0001, 0X0002, 0X0003, 0X0004, 0X0005, 0X0006, 0X0007,
	0X0008, 0X0009, 0X000A, 0X000B, 0X000C, 0X000D, 0X000E, 0X000F,
	0X0010, 0X0011, 0X0012, 0X0013, 0X0014, 0X0015, 0X0016, 0X0017,
	0X0018, 0X0019, 0X001A, 0X001B, 0X001C, 0X001D, 0X001E, 0X001F,
	0X0020,	0X221A, 0X250C, 0X2500, 0X2320, 0X2321, 0X2502, 0X23A1,
	0X23A3,	0X23A4, 0X23A6, 0X239B, 0X239D, 0X239E, 0X23A0, 0X23A8,
	0X23AC, CUNDEF, CUNDEF, CUNDEF, CUNDEF, CUNDEF, CUNDEF, CUNDEF,
	CUNDEF, CUNDEF, CUNDEF, CUNDEF, 0X2264, 0X2260, 0X2265, 0X222B,
	0X2234, 0X221D, 0X221E, 0X00F7, 0X0394, 0X2207, 0X03A6, 0X0397,
	0X223C, 0X2243, 0X0398, 0X00D7, 0X039B, 0X21D4, 0X21D2, 0X2261,
	0X03A0, 0X03A8, CUNDEF, 0X03A3, CUNDEF, CUNDEF, 0X221A, 0X03A9,
	0X029E, 0X03A5, 0X2282, 0X2283, 0X2229, 0X222A, 0X2227, 0X2228,
	0X00AC, 0X03B1, 0X03B2, 0X03C7, 0X03B4, 0X03B5, 0X03C6, 0X03B3,
	0X03B7, 0X03B9, 0X03B8, 0X03BA, 0X03BB, CUNDEF, 0X03BD, 0X2202,
	0X03C0, 0X03C8, 0X03C1, 0X03C3, 0X03C4, CUNDEF, 0X0192, 0X03C9,
	0X03BE, 0X03C5, 0X03B6, 0X2190, 0X2191, 0X2192, 0X2193, 0X007F
};

INITIALIZER(DECGSets)
{
	LLOG("Initializing DEC-specific gsets...");

	CHARSET_DEC_VT52 = AddCharSet("dec-vt52", CHRTAB_DEC_VT52_GRAPHICS);
	CHARSET_DEC_DCS  = AddCharSet("dec-dcs", CHRTAB_DEC_LINEDRAWING);
	CHARSET_DEC_MCS  = AddCharSet("dec-mcs", CHRTAB_DEC_MULTINATIONAL);
	CHARSET_DEC_TCS  = AddCharSet("dec-tcs", CHRTAB_DEC_TECHNICAL);
}

int TerminalCtrl::DecodeCodepoint(int c, byte gset)
{
	byte cs = ResolveVTCharset(gset);
	
	if(c < 0x80
	&&(gset == CHARSET_DEC_DCS		// Allow these charsets even when the g-sets are overridden.
	|| gset == CHARSET_DEC_TCS
	|| gset == CHARSET_DEC_VT52))
        c = ToUnicode(c | 0x80, gset);
	else
	if(cs == CHARSET_TOASCII)
		c = ToAscii(c);
	else
	if(cs != CHARSET_UNICODE)
		c = ToUnicode(c, cs);

	return c != DEFAULTCHAR ? c : 0xFFFD;
}

int TerminalCtrl::EncodeCodepoint(int c, byte gset)
{
	byte cs = ResolveVTCharset(gset);
	
	if(gset == CHARSET_DEC_DCS		// Allow these charsets even when the g-sets are overridden.
	|| gset == CHARSET_DEC_TCS
	|| gset == CHARSET_DEC_VT52)
		return FromUnicode(c, gset) & 0x7F;
	else
	if(cs == CHARSET_TOASCII)
		return ToAscii(c);
	else
	if(cs != CHARSET_UNICODE)
		return FromUnicode(c, cs);

	return c;
}

WString TerminalCtrl::DecodeDataString(const String& s)
{
	if(IsUtf8Mode() && CheckUtf8(s))
		return s.ToWString();
	
	WString txt;
	bool b = IsLevel2();
	const char *p = ~s;

	while(*p) {
		byte c = *p++;
		txt.Cat(DecodeCodepoint(c, gsets.Get(c, b)));
	}

	return txt;
}

String TerminalCtrl::EncodeDataString(const WString& ws)
{
	if(IsUtf8Mode())
		return ToUtf8(ws);

	String txt;
	bool b = IsLevel2();
	const wchar *s = ~ws;

	while(*s) {
		wchar c = *s++;
		c = (wchar) EncodeCodepoint(c, gsets.Get(c, b));
		txt.Cat(c == DEFAULTCHAR ? '?' : c);
	}

	return txt;
}

int TerminalCtrl::LookupChar(int c)
{
	// Perform single or locking shifts for GL and GR...
	// Single shifts are available on devices with level >= 1
	
	if(IsLevel1() && gsets.GetSS() != 0x00) {
		switch(gsets.GetSS()) {
		case 0x8E: // SS2
			c = DecodeCodepoint(c, gsets.GetG2());
			break;
		case 0x8F: // SS3
			c = DecodeCodepoint(c, gsets.GetG3());
			break;
		}
		gsets.SS(0x00);
		return c;
	}
	return DecodeCodepoint(c, gsets.Get(c, IsLevel2()));
}

TerminalCtrl::GSets::GSets(byte defgset)
: GSets(CHARSET_TOASCII, CHARSET_TOASCII, defgset, defgset)
{
}

TerminalCtrl::GSets::GSets(byte g0, byte g1, byte g2, byte g3)
{
	d[0] = g0;
	d[1] = g1;
	d[2] = g2;
	d[3] = g3;
	Reset();
}

void TerminalCtrl::GSets::ConformtoANSILevel1()
{
	ss = 0;
	g[0] = CHARSET_TOASCII;
	g[1] = CHARSET_ISO8859_1;
	G0toGL();
	G1toGR();
	
}

void TerminalCtrl::GSets::ConformtoANSILevel2()
{
	ss = 0;
	g[0] = CHARSET_TOASCII;
	g[1] = CHARSET_ISO8859_1;
	G0toGL();
	G1toGR();

}

void TerminalCtrl::GSets::ConformtoANSILevel3()
{
	ss = 0;
	g[0] = CHARSET_TOASCII;
	G0toGL();
}

void TerminalCtrl::GSets::Reset()
{
	ss = 0;
	ResetG0();
	ResetG1();
	ResetG2();
	ResetG3();
	G0toGL();
	G2toGR();
}

void TerminalCtrl::GSets::Serialize(Stream& s)
{
	int version = 1;
	s / version;

	Vector<String> v = {
		CharsetName(d[0]),
		CharsetName(d[1]),
		CharsetName(d[2]),
		CharsetName(d[3]),
		CharsetName(g[0]),
		CharsetName(g[1]),
		CharsetName(g[2]),
		CharsetName(g[3])
	};
	
	s % v;
	s % l;
	s % r;
	s % ss;

	if(s.IsLoading()) {
		d[0] = CharsetByName(v[0]);
		d[1] = CharsetByName(v[1]);
		d[2] = CharsetByName(v[2]);
		d[3] = CharsetByName(v[3]);
		g[0] = CharsetByName(v[4]);
		g[1] = CharsetByName(v[5]);
		g[2] = CharsetByName(v[6]);
		g[3] = CharsetByName(v[7]);
		clamp(l, 0, 3);
		clamp(r, 0, 3);
		if(ss != 0x00
		&& ss != 0x8E
		&& ss != 0x8F)
			ss = 0;
	}
}

void TerminalCtrl::GSets::Jsonize(JsonIO& jio)
{
	VectorMap<String, String> vm = {
		{ "Default_G0", CharsetName(d[0]) },
		{ "Default_G1", CharsetName(d[1]) },
		{ "Default_G2", CharsetName(d[2]) },
		{ "Default_G3", CharsetName(d[3]) },
		{ "G0",         CharsetName(g[0]) },
		{ "G1",         CharsetName(g[1]) },
		{ "G2",         CharsetName(g[2]) },
		{ "G3",         CharsetName(g[3]) }
	};
	
	for(int i = 0; i < vm.GetCount(); i++)
		jio(vm.GetKey(i), vm[i]);
		
	jio	("L",  l)
		("R",  r)
		("SingleShift", ss);

	if(jio.IsLoading()) {
		d[0] = CharsetByName(vm[0]);
		d[1] = CharsetByName(vm[1]);
		d[2] = CharsetByName(vm[2]);
		d[3] = CharsetByName(vm[3]);
		g[0] = CharsetByName(vm[4]);
		g[1] = CharsetByName(vm[5]);
		g[2] = CharsetByName(vm[6]);
		g[3] = CharsetByName(vm[7]);
		clamp(l, 0, 3);
		clamp(r, 0, 3);
		if(ss != 0x00
		&& ss != 0x8E
		&& ss != 0x8F)
			ss = 0;
	}
}

void TerminalCtrl::GSets::Xmlize(XmlIO& xio)
{
	XmlizeByJsonize(xio, *this);
}

}