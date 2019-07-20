#include "Cell.h"

#define LLOG(x)	// RLOG("VTCell: " << x)

namespace Upp {

void VTCell::Fill(const VTCell& filler, dword flags)
{
	if(flags == FILL_NORMAL) {
		*this = filler;
		return;
	}
	else
	if(flags & FILL_SELECTIVE)
		if(IsProtected())
			return;
	if(flags & FILL_CHAR)
		chr = filler.chr;
	if(flags & FILL_ATTRS)
		attrs = filler.attrs;
	if(flags & FILL_INK)
		ink = filler.ink;
	if(flags & FILL_PAPER)
	    paper = filler.paper;
	if(flags & FILL_SGR)
		sgr = filler.sgr;
	else
	if(flags & XOR_SGR)
		sgr ^= filler.sgr;
}

void VTCell::Reset()
{
	ink   = 0xFFFF;
	paper = 0xFFFF;
	sgr   = SGR_NORMAL;
}

void VTCell::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % chr;
		s % attrs;
		s % sgr;
		s % ink;
		s % paper;
	}
}

VTCell::VTCell()
{
	chr = 0;
	Reset();
}


}
