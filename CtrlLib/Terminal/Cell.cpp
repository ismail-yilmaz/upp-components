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
	if(flags & FILL_DATA)
		data = filler.data;
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
	ink   = Null;
	paper = Null;
	sgr   = SGR_NORMAL;
	data  = 0;
}

bool VTCell::IsDoubleWidth() const
{
	// This function is taken from Markus Kuhn's wcwidth implementation.
	// For license and implementation details, see:
	// https://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
	
	return !IsImage()
		&& (chr >= 0x1100
		&& (chr <= 0x115F
		||  chr == 0x2329
		||  chr == 0x232A
		|| (chr >= 0x2E80 && chr <= 0xA4CF && chr != 0x303F)
		|| (chr >= 0xAC00 && chr <= 0xD7A3)
		|| (chr >= 0xF900 && chr <= 0xFAFF)
		|| (chr >= 0xFE10 && chr <= 0xFE19)
		|| (chr >= 0xFE30 && chr <= 0xFE6F)
		|| (chr >= 0xFF00 && chr <= 0xFF60)
		|| (chr >= 0xFFE0 && chr <= 0xFFE6)
		|| (chr >= 0x20000 && chr <= 0x02FFFD)
		|| (chr >= 0x30000 && chr <= 0x03FFFD)));
}

bool VTCell::IsNullInstance() const
{
	return IsVoid()
		|| (chr  == 0
		&& data  == 0
		&& attrs == 0
		&& sgr   == SGR_NORMAL
		&& IsNull(ink)
		&& IsNull(paper));
}

const VTCell& VTCell::Void()
{
	static VTCell sCell;
	return sCell;
}

void VTCell::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % chr;
		s % data;
		s % attrs;
		s % sgr;
		s % ink;
		s % paper;
	}
}
}
