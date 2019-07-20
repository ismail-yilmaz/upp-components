#include "Console.h"


#define LLOG(x)	// RLOG("Console: " << x);

namespace Upp {

void Console::ParseDeviceControlStrings(const VTInStream::Sequence& seq)
{
	LLOG(Format("DCS: %c, %[N/A]~s, %s, [%s]",
			seq.opcode, seq.intermediate, seq.parameters.ToString(), seq.payload));

	bool refresh;
	switch(FindSequenceId(VTInStream::Sequence::DCS, clevel, seq, refresh)) {
	case SequenceId::DECRQSS:
		ReportControlFunctionSettings(seq);
		break;
	case SequenceId::DECUDK:
		SetUserDefinedKeys(seq);
		break;
	default:
		LLOG("Unhandled device control string.");
		break;
	}
	if(refresh)
		RefreshPage();
}

void Console::SetUserDefinedKeys(const VTInStream::Sequence& seq)
{
	if(!IsUDKEnabled() || IsUDKLocked())
		return;

	bool clear = seq.GetInt(1) == 0;
	bool lock  = seq.GetInt(2) == 0;
	int modifier = seq.GetInt(3, 0);

	Vector<String> vudk = Split(seq.payload, ';', false);

	if(clear)
		udk.Clear();

	String key, val;
	for(const String& pair : vudk)
		if(SplitTo(pair, '/', key, val))
			udk.Put(StrInt(key), ScanHexString(val));

	if(lock)
		LockUDK();
}

bool Console::GetUDKString(byte key, String& val)
{
	if(!IsLevel2() || udk.IsEmpty())
		return false;

	int i = udk.Find(key);
	if(i >= 0)
		val = udk[i];

	return i >= 0;
}

void Console::ReportControlFunctionSettings(const VTInStream::Sequence& seq)
{
	// TODO

	String reply = "0$r";				// Invalid request (unhandled sequence)

	if(seq.payload.IsEqual("r")) {		// DECSTBM
		Rect margins = page->GetMargins();
		reply = Format("%d`$r%d`;%d`r", 1, margins.top, margins.bottom);
	}
	else
	if(seq.payload.IsEqual("s")) {		// DECSLRM
		Rect margins = page->GetMargins();
		reply = Format("%d`$r%d`;%d`s", 1, margins.left, margins.right);
	}
	else
	if(seq.payload.IsEqual("m")) {		// SGR
		reply = Format("%d`$r%s`m", 1, GetGraphicsRenditionOpcodes(cellattrs));
	}
	else
	if(seq.payload.IsEqual("\"p")) {	// DECSCL
		int level = 62;
		switch(clevel) {
		case LEVEL_0:
		case LEVEL_1:
			level = 61;
			break;
		case LEVEL_2:
			level = 62;
			break;
		case LEVEL_3:
			level = 63;
			break;
		case LEVEL_4:
			level = 64;
			break;
		}
		reply = Format("%d`$r%d`;%[1:2;1]s`\"p", 1, level, Is8BitsMode());
	}
	else
	if(seq.payload.IsEqual(" q")) {		// DECSCUSR
		int style = 0;
		switch(caret.GetStyle()) {
		case Caret::BLOCK:
			style = caret.IsBlinking() ? 1 : 2;
			break;
		 case Caret::UNDERLINE:
			style = caret.IsBlinking() ? 3 : 4;
			break;
		case Caret::BEAM:
			style = caret.IsBlinking() ? 5 : 6;
			break;
		}
		reply = Format("%d`$r%d q", 1, style);
	}
	else
	if(seq.payload.IsEqual("\"q")) {	// DECSCA
		reply = Format("%d`$r%[1:1;0]s`\"q", 1, cellattrs.IsProtected());
	}
	else
	if(seq.payload.IsEqual("*x")) {		// DECSACE
		reply = Format("%d`$r%[1:1;2]s`*x", 1, streamfill);
	}
	PutDCS(reply);
}
}