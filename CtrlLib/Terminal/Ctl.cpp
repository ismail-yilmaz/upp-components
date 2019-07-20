#include "Console.h"

#define LLOG(x)	// RLOG("Console: " << x);

namespace Upp {
	
void Console::ParseControlChars(byte c)
{
	LLOG(Format("CTL: 0x%02X (C%[1:0;1]s`)", c, c < 0x80));
	
	if(modes[CRM]) {
		PutChar(c);
		return;
	}

	bool refresh;
	switch(FindControlId(c, clevel, refresh)) {
	case ControlId::NUL:
		break;
	case ControlId::ENQ:
		Put(answerback);
		break;
	case ControlId::BEL:
		WhenBell();
		break;
	case ControlId::BS:
		page->MoveLeft();
		break;
	case ControlId::HT:
		page->NextTab();
		break;
	case ControlId::LF:
	case ControlId::VT:
	case ControlId::FF:
		modes[LNM] ? page->NewLine() : page->NextLine();
		break;
	case ControlId::CR:
		modes[LNM] ? page->NewLine() : page->MoveHome();
		break;
	case ControlId::LSI:
		charsets.G1toGL();
		break;
	case ControlId::LSO:
		charsets.G0toGL();
		break;
	case ControlId::XON:
		break;
	case ControlId::XOFF:
		break;
	case ControlId::DEL:
		break;
	case ControlId::IND:
		page->NextLine();
		break;
	case ControlId::NEL:
		page->NewLine();
		break;
	case ControlId::HTS:
		page->SetTab();
		break;
	case ControlId::RI:
		page->PrevLine();
		break;
	case ControlId::SS2:
	case ControlId::SS3:
		charsets.SS(c);
		break;
	case ControlId::SPA:
		ProtectAttributes(true);
		break;
	case ControlId::EPA:
		ProtectAttributes(false);
		break;
	case ControlId::ST:
		break;
	default:
		LLOG(Format("Unhandled control byte: 0X%02X", c));
		break;
	}
	if(refresh)
		RefreshPage();
}
}