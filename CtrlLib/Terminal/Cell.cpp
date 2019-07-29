#include "Cell.h"

#define LLOG(x)	// RLOG("VTCell: " << x)

namespace Upp {

VTCell& VTCell::Ink(int n)
{
#ifdef flagTRUECOLOR
	ink = n;
#else
	ink = Nvl(n, 0xFFFF);
#endif
	return *this;
}

VTCell& VTCell::Paper(int n)
{
#ifdef flagTRUECOLOR
	paper = n;
#else
	paper = Nvl(n, 0xFFFF);
#endif
	return *this;
}

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
#ifdef flagTRUECOLOR
	ink   = Null;
	paper = Null;
#else
	ink   = 0xFFFF;
	paper = 0xFFFF;
#endif
	sgr   = SGR_NORMAL;
}

bool VTCell::IsNullInstance() const
{
	return  attrs == 0          &&
	        sgr   == SGR_NORMAL &&
#ifdef flagTRUECOLOR
			IsNull(ink)         &&
			IsNull(paper)       &&
#else
	        ink   == 0xFFFF     &&
	        paper == 0xFFFF     &&
#endif
	        chr == 0;
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
