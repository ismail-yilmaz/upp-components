#include "Console.h"

#define LLOG(x)	 // RLOG("Console: " << x);
#define LDUMP(x) // RLOG("Console: Mode: " << #x << " = " << modes[x])

namespace Upp {

void Console::SetMode(const VTInStream::Sequence& seq, bool enable)
{
	for(const String& s : seq.parameters) {		// Multiple terminal modes can be set/reset at once.
		bool refresh;
		int modenum = StrInt(s);
		switch(FindModeId(modenum, seq.mode, clevel, refresh)) {
		case KAM:
			ANSIkam(enable);
			break;
		case CRM:
			ANSIcrm(enable);
			break;
		case IRM:
			ANSIirm(enable);
			break;
		case ERM:
			ANSIerm(enable);
			break;
		case SRM:
			ANSIsrm(enable);
			break;
		case LNM:
			ANSIlnm(enable);
			break;
		case DECCKM:
			DECckm(enable);
			break;
		case DECANM:
			DECanm(enable);
			break;
		case DECCOLM:
			DECcolm(enable);
			break;
		case DECSCLM:
			DECsclm(enable);
			break;
		case DECSCNM:
			DECscnm(enable);
			break;
		case DECOM:
			DECom(enable);
			break;
		case DECAWM:
			DECawm(enable);
			break;
		case DECARM:
			DECarm(enable);
			break;
		case DECTCEM:
			DECtcem(enable);
			break;
		case DECLRMM:
			DEClrmm(enable);
			break;
		case DECBKM:
			DECbkm(enable);
			break;
		case XTASCM:
			XTascm(enable);
			break;
		case XTBRPM:
			XTbrpm(enable);
			break;
		case XTX10MM:
			XTx10mm(enable);
			break;
		case XTX11MM:
			XTx11mm(enable);
			break;
		case XTSGRMM:
			XTsgrmm(enable);
			break;
		case XTUTF8MM:
			XTutf8mm(enable);
			break;
		case XTSRCM:
			XTsrcm(enable);
			break;
		case XTDRAGM:
			XTdragm(enable);
			break;
		case XTANYMM:
			XTanymm(enable);
			break;
		case XTFOCUSM:
			XTfocusm(enable);
			break;
		case XTASBM:
			XTasbm(modenum, enable);
			break;
		default:
			LLOG(Format("Unhandled %[0:ANSI;private]s mode: %d", seq.mode, modenum));
			break;
		}
	}
}

void Console::ReportMode(const VTInStream::Sequence& seq)
{
	bool refresh;
	int modenum = seq.GetInt(1, 0);
	int mid = FindModeId(modenum, seq.mode, clevel, refresh);  // Only one mode at a time can be reported

	// Possible reply codes:
	// ====================
	// 0: Mode not recognized
	// 1: Set
	// 2: Reset
	// 3: Permanently set
	// 4: Permanently reset

	int reply = 0;

	if(mid >= 0)
		reply = modes[mid] ? 1 : 2;

	PutCSI(Format("%[1:?;]s%d;%d`$y", seq.mode == '?', modenum, reply));
}

void Console::ANSIkam(bool b)
{
	modes.Set(KAM, b);
	LDUMP(KAM);
}

void Console::ANSIcrm(bool b)
{
	modes.Set(CRM, b);
	LDUMP(CRM);
}

void Console::ANSIirm(bool b)
{
	modes.Set(IRM, b);
	LDUMP(IRM);
}

void Console::ANSIsrm(bool b)
{
	modes.Set(SRM, b);
	LDUMP(SRM);
}

void Console::ANSIerm(bool b)
{
	modes.Set(ERM, b);
	LDUMP(ERM);
}

void Console::ANSIlnm(bool b)
{
	modes.Set(LNM, b);
	LDUMP(LNM);
}

void Console::DECom(bool b)
{
	modes.Set(DECOM, b);
	page->OriginMode(b);
	page->MoveTopLeft();
	LDUMP(DECOM);
}

void Console::DECckm(bool b)
{
	modes.Set(DECCKM, b);
	LDUMP(DECCKM);
}

void Console::DECanm(bool b)
{
	// According to DEC'S internal VT52 emulation document
	// (EL-00070-0A, p. A-45), exiting the VT52 mode always
	// sets the device conformance level to 1 (VT1xx mode).
	// This is also consistent with vttest/xterm's behavior.

	modes.Set(DECANM, b);
	if(b) {
		clevel = LEVEL_1;
		Restore(false);
	}
	else {
		clevel = LEVEL_0;
		Backup(false);
		page->SetTabs(8);
		charsets = Charsets(CHARSET_TOASCII);
	}
	LDUMP(DECANM);
}

void Console::DECawm(bool b)
{
	modes.Set(DECAWM, b);
	page->WrapAround(b);
	LDUMP(DECAWM);
}

void Console::DECarm(bool b)
{
	modes.Set(DECARM, b);
	LDUMP(DECARM);
}

void Console::DECkpam(bool b)
{
	modes.Set(DECKPAM, b);
	LDUMP(DECKPAM);
}

void Console::DECcolm(bool b)
{
	modes.Set(DECCOLM, b);
	page->ErasePage();
	DECom(false);
	When132Column(b);
	LDUMP(DECCOLM);
}

void Console::DECsclm(bool b)
{
	modes.Set(DECSCLM, b);
	LDUMP(DECSCLM);
}

void Console::DECscnm(bool b)
{
	modes.Set(DECSCNM, b);
	page->Invalidate();
	RefreshPage(true);
	LDUMP(DECSCNM);
}

void Console::DECtcem(bool b)
{
	modes.Set(DECTCEM, b);
	caret.WhenAction();
	LDUMP(DECTCEM);
}

void Console::DECst8c(bool b)
{
	modes.Set(DECST8C, b);
	if(b) page->SetTabs(8);
	LDUMP(DECST8C);
}

void Console::DEClrmm(bool b)
{
	modes.Set(DECLRMM, b);
	LDUMP(DECLRMM);
}

void Console::DECbkm(bool b)
{
	modes.Set(DECBKM, b);
	LDUMP(DECBKM);
}

void Console::XTascm(bool b)
{
	modes.Set(XTASCM, b);
	LDUMP(XTASCM);
}

void Console::XTbrpm(bool b)
{
	modes.Set(XTBRPM, b);
	LDUMP(XTBRPM);
}

void Console::XTsrcm(bool b)
{
	modes.Set(XTSRCM, b);
	b ? Backup() : Restore(); // This mode is used in conjunction with the private mode 1047.
	LDUMP(XTSRCM);
}

void Console::XTasbm(int mode, bool b)
{
	modes.Set(XTASBM, b);
	if(mode == 47) {
		AlternateScreenBuffer(b);
	}
	else
	if(mode == 1047) {
		if(!b) page->ErasePage();
		AlternateScreenBuffer(b);
	}
	else
	if(mode == 1049) {
		if(b) Backup();
		AlternateScreenBuffer(b);
		if(b) page->ErasePage();
		else Restore();
	}
	SwapPage();
	LDUMP(XTASBM);
}

void Console::XTx10mm(bool b)
{
	modes.Set(XTX10MM, b);
	LDUMP(XTX10MM);
}

void Console::XTx11mm(bool b)
{
	modes.Set(XTX11MM, b);
	LDUMP(XTX11MM);
}

void Console::XTsgrmm(bool b)
{
	modes.Set(XTSGRMM, b);
	LDUMP(XTSGRMM);
}

void Console::XTutf8mm(bool b)
{
	modes.Set(XTUTF8MM, b);
	LDUMP(XTUTF8MM);
}

void Console::XTdragm(bool b)
{
	modes.Set(XTDRAGM, b);
	LDUMP(XTDRAGM);
}

void Console::XTfocusm(bool b)
{
	modes.Set(XTFOCUSM, b);
	LDUMP(XTFOCUSM);
}

void Console::XTanymm(bool b)
{
	modes.Set(XTANYMM, b);
	LDUMP(XTANYMM);
}

}