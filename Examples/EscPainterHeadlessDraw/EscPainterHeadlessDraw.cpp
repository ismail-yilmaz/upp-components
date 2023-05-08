// This example demonstrates a headless drawing example, using U++'s Esc scripting language.
#include <Core/Core.h>
#include <EscPainter/EscPainter.h>
#include <plugin/png/png.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_COUT | LOG_FILE);
	
	ArrayMap<String, EscValue> global;

	StdLib(global);		// Include Esc standard library.
	PainterLib(global);	// Include Esc painter library.

	try
	{
		PNGEncoder().SaveFile(
			GetHomeDirFile("EscPainterOutput.png"), 
			EscPaintImage(global, LoadFile(GetDataFile("script.usc")), 1024, 1024));
	}
	catch(CParser::Error e)
	{
		RDUMP(e);
	}
}
