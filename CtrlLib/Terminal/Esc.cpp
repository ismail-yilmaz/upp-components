#include "Terminal.h"

#define LLOG(x)	// RLOG("Console: " << x);

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
		charsets.ConformtoANSILevel1();
		break;
	case SequenceId::ANSICL2:
		charsets.ConformtoANSILevel2();
		break;
	case SequenceId::ANSICL3:
		charsets.ConformtoANSILevel3();
		break;
	case SequenceId::LS2:
		charsets.G2toGL();
		break;
	case SequenceId::LS3:
		charsets.G3toGL();
		break;
	case SequenceId::LS3R:
		charsets.G3toGR();
		break;
	case SequenceId::LS2R:
		charsets.G2toGR();
		break;
	case SequenceId::LS1R:
		charsets.G1toGR();
		break;
	case SequenceId::SCS_G0_ASCII:
		charsets.ResetG0();
		break;
	case SequenceId::SCS_G1_ASCII:
		charsets.ResetG1();
		break;
	case SequenceId::SCS_G2_ASCII:
		charsets.ResetG2();
		break;
	case SequenceId::SCS_G3_ASCII:
		charsets.ResetG3();
		break;
	case SequenceId::SCS_G1_LATIN1:
		charsets.G1(CHARSET_ISO8859_1);
		break;
	case SequenceId::SCS_G2_LATIN1:
		charsets.G3(CHARSET_ISO8859_1);
		break;
	case SequenceId::SCS_G3_LATIN1:
		charsets.G3(CHARSET_ISO8859_1);
		break;
	case SequenceId::SCS_G0_DEC_ACS:
		charsets.G0(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G1_DEC_ACS:
		charsets.G0(CHARSET_TOASCII);
		break;
	case SequenceId::SCS_G0_DEC_DCS:
		charsets.G0(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G1_DEC_DCS:
		charsets.G1(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G2_DEC_DCS:
		charsets.G2(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G3_DEC_DCS:
		charsets.G3(CHARSET_DEC_DCS);
		break;
	case SequenceId::SCS_G0_DEC_MCS:
		charsets.G0(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G1_DEC_MCS:
		charsets.G1(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G2_DEC_MCS:
		charsets.G2(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G3_DEC_MCS:
		charsets.G3(CHARSET_DEC_MCS);
		break;
	case SequenceId::SCS_G0_DEC_TCS:
		charsets.G0(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_G1_DEC_TCS:
		charsets.G1(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_G2_DEC_TCS:
		charsets.G2(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_G3_DEC_TCS:
		charsets.G3(CHARSET_DEC_TCS);
		break;
	case SequenceId::SCS_UTF8:
		charsets.G0(CHARSET_UNICODE);
		charsets.G1(CHARSET_UNICODE);
		charsets.G2(CHARSET_UNICODE);
		charsets.G3(CHARSET_UNICODE);
		break;
	case SequenceId::SCS_DEFAULT:
		charsets.Reset();
		break;
	case SequenceId::VT52_DCS_ON:
		charsets.G0(CHARSET_DEC_VT52);
		break;
	case SequenceId::VT52_DCS_OFF:
		charsets.ResetG0();
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