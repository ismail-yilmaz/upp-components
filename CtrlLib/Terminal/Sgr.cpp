#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void TerminalCtrl::SelectGraphicsRendition(const VTInStream::Sequence& seq)
{
	SetGraphicsRendition(cellattrs, seq.parameters);
	page->Attributes(cellattrs);	// This update is required for BCE (background color erase).
}

void TerminalCtrl::SetGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes)
{
	LTIMING("TerminalCtrl::SetGraphicsRendition");

	for(int i = 0; i < opcodes.GetCount(); i++) {
		int opcode = ReadInt(opcodes[i], 0);
		switch(opcode) {
		case 0:
			attrs.Reset();
			break;
		case 1:
			attrs.Bold().Faint(false);
			break;
		case 2:
			attrs.Faint();
			break;
		case 3:
			attrs.Italic();
			break;
		case 4:
			attrs.Underline();
			break;
		case 5:
		case 6:
			attrs.Blink();
			break;
		case 7:
			attrs.Invert();
			break;
		case 8:
			attrs.Conceal();
			break;
		case 9:
			attrs.Strikeout();
			break;
		case 14:
			 //ACS on
			break;
		case 15:
			 //ACS off
			break;
		case 22:
			attrs.Faint(false).Bold(false);
			break;
		case 23:
			attrs.Italic(false);
			break;
		case 24:
			attrs.Underline(false);
			break;
		case 25:
		case 26:
			attrs.Blink(false);
			break;
		case 27:
			attrs.Invert(false);
			break;
		case 28:
			attrs.Conceal(false);
			break;
		case 29:
			attrs.Strikeout(false);
			break;
		case 38:
			ParseExtendedColors(attrs, opcodes, i);
			break;
		case 39:
			attrs.ink = Null;
			break;
		case 48:
			ParseExtendedColors(attrs, opcodes, i);
			break;
		case 49:
			attrs.paper = Null;
			break;
		case 53:
			attrs.Overline();
			break;
		case 55:
			attrs.Overline(false);
			break;
		default:
			if(opcode >= 30 && opcode <= 37)
				attrs.ink = Color::Special(opcode - 30);
			else
			if(opcode >= 40 && opcode <= 47)
				attrs.paper = Color::Special(opcode - 40);
			else
			if(opcode >= 90 && opcode <= 97)
				attrs.ink = Color::Special(opcode - 82);
			else
			if(opcode >= 100 && opcode <= 107)
				attrs.paper = Color::Special(opcode - 92);
			else
				LLOG("Unhandled SGR code: " << opcode);
			break;
		}
	}
}

void TerminalCtrl::InvertGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes)
{
	for(const auto& opcode : opcodes) {
		switch(ReadInt(opcode, 0)) {
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

String TerminalCtrl::GetGraphicsRenditionOpcodes(const VTCell& attrs)
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
