#include "Console.h"


#define LLOG(x)	// RLOG("Console: " << x);

namespace Upp {

void Console::ParseOperatingSystemCommands(const VTInStream::Sequence& seq)
{
	LLOG(Format("OSC: %s", seq.payload));

	int opcode = seq.GetInt(1, 0);

	switch(opcode) {
	case 0:		// Set window titile
	case 2:		// Set window title (and icon name)
		WhenTitle(seq.GetStr(2));
		break;
	case 4:		// Set ANSI colors
	case 10:	// Set dynamic color (ink)
	case 11:	// Set dynamic color (paper)
	case 17:	// Set dynamic color (selection ink)
	case 19:	// Set dynamic color (selection paper)
	case 104:	// Reset ANSI colors
	case 110:	// Reset dynamic color (ink)
	case 111:	// Reset dynamic color (paper)
	case 117:	// Reset dynamic color (selection ink)
	case 119:	// Reset dynamic color (selection paper)
		ChangeColors(opcode, seq.payload, opcode > 100);
		break;
	default:
		LLOG(Format("Unhandled OSC opcode: %d", opcode));
		break;
	}
}
}