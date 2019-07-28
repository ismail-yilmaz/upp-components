#include "Terminal.h"

#define LLOG(x)	// RLOG("Console: " << x)

namespace Upp {

void Console::SelectGraphicsRendition(const VTInStream::Sequence& seq)
{
	SetGraphicsRendition(cellattrs, seq.parameters);
	page->Attributes(cellattrs);	// This update is required for BCE (background color erase).
}

void Console::SetGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes)
{
	auto HandleOpcodes = [&opcodes](int i) -> int	// This is to handle ISO color opcodes
	{
		const String& s = opcodes[i];
		int rc = Nvl(StrInt(s), 0);
		if(!rc && !s.IsEmpty()) {
			if(s.StartsWith("38:"))
				rc = 38;
			else
			if(s.StartsWith("48:"))
				rc = 48;
		}
		return rc;
	};

	for(int i = 0; i < opcodes.GetCount(); i++) {
		int opcode = HandleOpcodes(i);
		switch(opcode) {
		case 0:
			attrs.Reset();
			break;
		case 1:
			attrs.sgr |= VTCell::SGR_BOLD;
			break;
		case 2:
			attrs.sgr &= ~VTCell::SGR_BOLD;
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
			// ACS on
			break;
		case 15:
			// ACS off
			break;
		case 21:
			attrs.sgr &= ~VTCell::SGR_BOLD;
			break;
		case 22:
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
			attrs.ink = opcode - 30;
			break;
		case 38:
			SetISOColor(attrs, opcodes);
			return;
		case 39:
			attrs.Ink(Null);
			break;
		case 40 ... 47:
			attrs.paper = opcode - 40;
			break;
		case 48:
			SetISOColor(attrs, opcodes);
			return;
		case 49:
			attrs.Paper(Null);
			break;
		case 90 ... 97:
			attrs.ink = opcode - 82;
			break;
		case 100 ... 107:
			attrs.paper = opcode - 92;
			break;
		default:
			LOG("Unhandled SGR code: " << opcode);
			break;
		}
	}
}

void Console::InvertGraphicsRendition(VTCell& attrs, const Vector<String>& opcodes)
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
		default:
			break;
		}
	}
}

String Console::GetGraphicsRenditionOpcodes(const VTCell& attrs)
{
	Vector<String> v;
	
	if(attrs.sgr == VTCell::SGR_NORMAL)
		v.Add("0");
	else {
		if(attrs.sgr & VTCell::SGR_BOLD)
			v.Add("1");
		if(attrs.sgr & VTCell::SGR_ITALIC)
			v.Add("3");
		if(attrs.sgr & VTCell::SGR_UNDERLINE)
			v.Add("4");
		if(attrs.sgr & VTCell::SGR_BLINK)
			v.Add("5");
		if(attrs.sgr & VTCell::SGR_INVERTED)
			v.Add("7");
		if(attrs.sgr & VTCell::SGR_HIDDEN)
			v.Add("8");
		if(attrs.sgr & VTCell::SGR_STRIKEOUT)
			v.Add("9");
	}
	return Join(v, ";", true);

}
}
