#include "Console.h"

// ANSI, dynamic, and ISO colors support.
// See: https://invisible-island.net/xterm/ctlseqs/ctlseqs.html#h2-Operating-System-Commands

#define LLOG(x)	// RLOG("Console: " << x)

namespace Upp {
	
void Console::ReportANSIColor(int opcode, int index, const Color& c)
{
	PutOSC(Format("%d;%d;rgb:%02x/%02x/%02x", opcode, index, c.GetR(), c.GetG(), c.GetB()));
}

void Console::ReportDynamicColor(int opcode, const Color& c)
{
	PutOSC(Format("%d;rgb:%02x/%02x/%02x", opcode, c.GetR(), c.GetG(), c.GetB()));
}

void Console::ChangeColors(int opcode, const String& oscs, bool reset)
{
	Vector<String> params = Split(oscs, ';', false);
	VectorMap<int, String> colormap;

	bool ansicolors = opcode == 4 || opcode == 104;
	int pos = (ansicolors && !reset) ? 1 : 0;
	
	for(int i = pos; i < params.GetCount() - pos; i += 2)
		if(i + 1 < params.GetCount())
			colormap.Add(StrInt(params[i]), params[i + 1]);

	if(colormap.IsEmpty())
		return;
	
	int changed_colors = 0;
	
	for(int i = 0; i < colormap.GetCount(); i++) {
		int index = colormap.GetKey(i);
		String colorspec = ToLower(colormap[i]);
		colorspec.Replace("rgb:", "");	// TODO: Handle HSV too (Apparently it is rarely used.)
		if(SetColorTable(opcode, index, colorspec, ansicolors, reset))
			changed_colors++;
	}

	if(changed_colors > 0)
		RefreshPage(true);
}

bool Console::SetColorTable(int opcode, int index, String colorspec, bool ansicolor, bool reset)
{
	if(ansicolor && index >= ANSI_COLOR_COUNT)
		return false;
	else
	if(!ansicolor) {
		switch(opcode) {
		case 10:
		case 110:
			index = COLOR_INK;
			break;
		case 11:
		case 111:
			index = COLOR_PAPER;
			break;
		case 17:
		case 117:
			index = COLOR_INK_SELECTED;
			break;
		case 19:
		case 119:
			index = COLOR_PAPER_SELECTED;
			break;
		default:
			LLOG("Unhandled dynamic color opcode: " << opcode);
			return false;
		}
	}
	Color c;
	if(colorspec.IsEqual("?")) { // Report color value
		c = colortable[index];
		if(ansicolor)
			ReportANSIColor(opcode, index, c);
		else
			ReportDynamicColor(opcode, c);
		return false;
	}
	return reset
			? ResetLoadColor(index)
			: SetSaveColor(index, ColorFromText(colorspec));
}

bool Console::SetSaveColor(int index, const Color& c)
{
	if(IsNull(c))
		return false;
	if(savedcolors.Find(index) < 0)
		savedcolors.Add(index, colortable[index]);
	colortable[index] = c;
	return true;
}

bool Console::ResetLoadColor(int index)
{
	int i = savedcolors.Find(index);
	if(i < 0)
		return false;
	colortable[index] = savedcolors[i];
	savedcolors.Remove(i);
	return true;
}

void Console::SetISOColor(VTCell& attrs, const Vector<String>& opcodes)
{
	Vector<int> v;

	auto ParseISOColorOpcodes = [&opcodes, &v] () -> void
	{
		String s = Join(opcodes, ";", true);
		s.Replace(":", ";");
		for(const String& opcode : Split(s, ';', true))
			v.Add(Nvl(StrInt(opcode), 0));
	};
	
	auto GetParam = [&v](int n) -> int
	{
		bool b = v.GetCount() < max(1, n);
		return b ? 0 : v[--n];
	};

	ParseISOColorOpcodes();
	
	int which = GetParam(1);
	int mode  = GetParam(2);
	
	if(mode == 5) {	// Indexed colors mode (256 colors)
		if(which == 38)
			attrs.Ink(GetParam(3));
		else
		if(which == 48)
			attrs.Paper(GetParam(3));
	}
	else
	if(mode == 2) {	// Direct color mode (True color)
		// TODO:
	}
}
}