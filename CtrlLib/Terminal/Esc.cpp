#include "Terminal.h"

#define LLOG(x)	// RLOG("Console: " << x)

namespace Upp {

void Console::ParseEscapeSequences(const VTInStream::Sequence& seq)
{
	LLOG(Format("ESC: %c, %[N/A]~s",	seq.opcode, seq.intermediate));

	auto VT52MoveCursor = [=]() -> void	// VT52 direct cursor addressing.
	{
		if(parser.Peek() >= 32) {
			page->MoveToLine(parser.Get() - 31);
			if(parser.Peek() >= 32)
				page->MoveToColumn(parser.Get() - 31);
		}
	};

	if(Convert7BitsC1To8BitsC1(seq))	// Redirection
		return;

	bool refresh;
	switch(FindSequenceId(VTInStream::Sequence::ESC, clevel, seq, refresh)) {
	case SequenceId::DECKPAM:
		DECkpam(true);
		break;
	case SequenceId::DECKPNM:
		DECkpam(false);
		break;
	case SequenceId::DECALN:
		DisplayAlignmentTest();
		break;
	case SequenceId::DECSC:
		Backup();
		break;
	case SequenceId::DECRC:
		Restore();
		break;
	case SequenceId::DECBI:
		page->PrevColumn();
		break;
	case SequenceId::DECFI:
		page->NextColumn();
		break;
	case SequenceId::S7C1T:
		Set8BitsMode(false);
		break;
	case SequenceId::S8C1T:
		Set8BitsMode(true);
		break;
	case SequenceId::RIS:
		HardReset();
		break;
	case SequenceId::ANSICL1:
		gsets.ConformtoANSILevel1();
		break;
	case SequenceId::ANSICL2:
		gsets.ConformtoANSILevel2();
		break;
	case SequenceId::ANSICL3:
		gsets.ConformtoANSILevel3();
		break;
	case SequenceId::LS2:
		gsets.G2toGL();
		break;
	case SequenceId::LS3:
		gsets.G3toGL();
		break;
	case SequenceId::LS3R:
		gsets.G3toGR();
		break;
	case SequenceId::LS2R:
		gsets.G2toGR();
		break;
	case SequenceId::LS1R:
		gsets.G1toGR();
		break;
	case SequenceId::SCS_G0_ASCII:
		gsets.G0(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G1_ASCII:
		gsets.G1(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G2_ASCII:
		gsets.G2(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G3_ASCII:
		gsets.G3(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G1_LATIN1:
		gsets.G1(CHARSET_ISO8859_1);
		break;
	case SequenceId::SCS_G2_LATIN1:
		gsets.G3(CHARSET_ISO8859_1);
		break;
	case SequenceId::SCS_G3_LATIN1:
		gsets.G3(CHARSET_ISO8859_1);
		break;
	case SequenceId::SCS_G0_DEC_ACS:
		gsets.G0(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G1_DEC_ACS:
		gsets.G0(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G0_DEC_DCS:
		gsets.G0(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G1_DEC_DCS:
		gsets.G1(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G2_DEC_DCS:
		gsets.G2(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G3_DEC_DCS:
		gsets.G3(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G0_DEC_MCS:
		gsets.G0(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G1_DEC_MCS:
		gsets.G1(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G2_DEC_MCS:
		gsets.G2(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G3_DEC_MCS:
		gsets.G3(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G0_DEC_TCS:
		gsets.G0(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_G1_DEC_TCS:
		gsets.G1(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_G2_DEC_TCS:
		gsets.G2(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_G3_DEC_TCS:
		gsets.G3(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_UTF8:
		gsets.G0(CHARSET_UNICODE);
		gsets.G1(CHARSET_UNICODE);
		gsets.G2(CHARSET_UNICODE);
		gsets.G3(CHARSET_UNICODE);
		break;
	case SequenceId::SCS_DEFAULT:
		gsets.Reset();
		break;
	case SequenceId::VT52_DCS_ON:
		gsets.G0(CHARSET_DEC_VT52);
		break;
	case SequenceId::VT52_DCS_OFF:
		gsets.ResetG0();
		break;
	case SequenceId::VT52_CUU:
		page->MoveUp();
		break;
	case SequenceId::VT52_CUD:
		page->MoveDown();
		break;
	case SequenceId::VT52_CUF:
		page->MoveRight();
		break;
	case SequenceId::VT52_CUB:
		page->MoveLeft();
		break;
	case SequenceId::VT52_CUP:
		VT52MoveCursor();
		break;
	case SequenceId::VT52_HOME:
		page->MoveTopLeft();
		break;
	case SequenceId::VT52_RI:
		page->PrevLine();
		break;
	case SequenceId::VT52_ED:
		page->EraseAfter();
		break;
	case SequenceId::VT52_EL:
		page->EraseRight();
		break;
	case SequenceId::VT52_DA:
		ReportDeviceAttributes(seq);
		break;
	case SequenceId::VT52_DECANM:
		DECanm(true);
		break;
	case SequenceId::IGNORED:
		break;
	default:
		LLOG("Unhandled escape sequence.");
		break;
	}
	if(refresh)
		RefreshPage();
}

bool Console::Convert7BitsC1To8BitsC1(const VTInStream::Sequence& seq)
{
	bool b = IsLevel1() && seq.intermediate.IsEmpty();
	if(b) {
		byte c = seq.opcode + 64;	// Try to shift the final byte (opcode) into C1 region.
		b = 0x80 <= c && c <= 0x9F;
		if(b) ParseControlChars(c);
	}
	return b;
}
}