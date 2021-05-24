#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

void TerminalCtrl::ParseEscapeSequences(const VTInStream::Sequence& seq)
{
	LLOG(seq);

	if(Convert7BitC1To8BitC1(seq))	// Redirection
		return;

	const CbFunction *p = FindFunctionPtr(seq);
	if(p) p->c(*this, seq);
}

bool TerminalCtrl::Convert7BitC1To8BitC1(const VTInStream::Sequence& seq)
{
	bool b = IsLevel1() && seq.intermediate.IsEmpty();
	if(b) {
		byte c = seq.opcode + 64;	// Try to shift the final byte (opcode) into C1 region.
		b = 0x80 <= c && c <= 0x9F;
		if(b) ParseControlChars(c);
	}
	return b;
}

void TerminalCtrl::VT52MoveCursor()
{
	if(parser.Peek() >= 32) {
		page->MoveToLine(parser.Get() - 31);
		if(parser.Peek() >= 32)
			page->MoveToColumn(parser.Get() - 31);
	}
}

void TerminalCtrl::DisplayAlignmentTest()
{
	LLOG("Performing display alignment test...");

	// According to DEC STD-070, DECALN:
	// 1) Resets the margins.
	// 2) Clears the EOL flag.
	// 3) Sets the page origins to 1, 1.
	// 3) Resets SGR.
	
	// The first two steps are covered in the third step.
	
	DECom(false);
	cellattrs.Normal();
	
	for(VTLine& line : *page)
		for(VTCell& cell : line) {
			cell.Reset();
			cell = 'E';
		}
}
}