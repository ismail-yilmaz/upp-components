#include "Terminal.h"

#define LLOG(x)		// RLOG("Terminal: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void Terminal::SelectGraphicsRendition(const VTInStream::Sequence& seq)
{
	SetGraphicsRendition(cellattrs, seq.parameters);
	page->Attributes(cellattrs);	// This update is required for BCE (background color erase).
}

void Terminal::SetGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes)
{
	LTIMING("Terminal::SetGraphicsRendition");

	for(int i = 0; i < opcodes.GetCount(); i++) {
		int opcode = Nvl(StrInt(opcodes[i]), 0);
		switch(opcode) {
		case 0:
			attrs.Reset();
			break;
		case 1:
			attrs.sgr |= VTCell::SGR_BOLD;
			attrs.sgr &= ~VTCell::SGR_FAINT;
			break;
		case 2:
			attrs.sgr |= VTCell::SGR_FAINT;
			break;
		case 3:
			attrs.sgr |= VTCell::SGR_ITALIC;
			break;
		case 4:
			attrs.sgr |= VTCell::SGR_UNDERLINE;
			break;
		case 5:
		case 6:
			attrs.sgr |= VTCell::SGR_BLINK;
			break;
		case 7:
			attrs.sgr |= VTCell::SGR_INVERTED;
			break;
		case 8:
			attrs.sgr |= VTCell::SGR_HIDDEN;
			break;
		case 9:
			attrs.sgr |= VTCell::SGR_STRIKEOUT;
			break;
		case 14:
			 //ACS on
			break;
		case 15:
			 //ACS off
			break;
		case 22:
			attrs.sgr &= ~VTCell::SGR_FAINT;
			attrs.sgr &= ~VTCell::SGR_BOLD;
			break;
		case 23:
			attrs.sgr &= ~VTCell::SGR_ITALIC;
			break;
		case 24:
			attrs.sgr &= ~VTCell::SGR_UNDERLINE;
			break;
		case 25:
		case 26:
			attrs.sgr &= ~VTCell::SGR_BLINK;
			break;
		case 27:
			attrs.sgr &= ~VTCell::SGR_INVERTED;
			break;
		case 28:
			attrs.sgr &= ~VTCell::SGR_HIDDEN;
			break;
		case 29:
			attrs.sgr &= ~VTCell::SGR_STRIKEOUT;
			break;
		case 30 ... 37:
			attrs.ink = Color::Special(opcode - 30);
			break;
		case 38:
			ParseExtendedColors(attrs, opcodes, i);
			break;
		case 39:
			attrs.ink = Null;
			break;
		case 40 ... 47:
			attrs.paper = Color::Special(opcode - 40);
			break;
		case 48:
			ParseExtendedColors(attrs, opcodes, i);
			break;
		case 49:
			attrs.paper = Null;
			break;
		case 53:
			attrs.sgr |= VTCell::SGR_OVERLINE;
			break;
		case 55:
			attrs.sgr &= ~VTCell::SGR_OVERLINE;
			break;
		case 90 ... 97:
			attrs.ink = Color::Special(opcode - 82);
			break;
		case 100 ... 107:
			attrs.paper = Color::Special(opcode - 92);
			break;
		default:
			LOG("Unhandled SGR code: " << opcode);
			break;
		}
	}
}

void Terminal::InvertGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes)
{
	for(const auto& opcode : opcodes) {
		switch(Nvl(StrInt(opcode), 0)) {
		case 0:
			attrs.Reset();
			break;
		case 1:
			attrs.sgr ^= VTCell::SGR_BOLD;
			break;
		case 3:
			attrs.sgr ^= VTCell::SGR_ITALIC;
			break;
		case 4:
			attrs.sgr ^= VTCell::SGR_UNDERLINE;
			break;
		case 5:
			attrs.sgr ^= VTCell::SGR_BLINK;
			break;
		case 7:
			attrs.sgr ^= VTCell::SGR_INVERTED;
			break;
		case 8:
			attrs.sgr ^= VTCell::SGR_HIDDEN;
			break;
		case 9:
			attrs.sgr ^= VTCell::SGR_STRIKEOUT;
			break;
		case 53:
			attrs.sgr ^= VTCell::SGR_OVERLINE;
		default:
			break;
		}
	}
}

String Terminal::GetGraphicsRenditionOpcodes(const VTCell& attrs)
{
	Vector<String> v;
	
	if(attrs.IsBold())
		v.Add("1");
	if(attrs.IsItalic())
		v.Add("3");
	if(attrs.IsUnderlined())
		v.Add("4");
	if(attrs.IsBlinking())
		v.Add("5");
	if(attrs.IsInverted())
		v.Add("7");
	if(attrs.IsConcealed())
		v.Add("8");
	if(attrs.IsStrikeout())
		v.Add("9");
	if(attrs.IsOverlined())
		v.Add("53");
	
	return Join(v, ";", true);

}
}
