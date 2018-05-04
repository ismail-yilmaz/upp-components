#include <Core/Core.h>
#include <AnsiParser/AnsiParser.h>

using namespace Upp;

CONSOLE_APP_MAIN
{
	StringStream s("\x1b\[32mHello World\n\x1b\x0d\x1b\[33mThis is a plain text.\n\x1b\]0;This is an xterm window title\x07");

	AnsiParser p;

	p.WhenEsc = [&p]
	{
		if(p.GetControl() == 0x0d) // Carriage return. (From the 7-bit C0 control characters.)
			Cout().PutEol();
	};
	p.WhenCsi = [&p]
	{
		DUMP(p.GetParameters());
	};
	p.WhenOsc = [&p]
	{
		DUMP(p.GetParameters()[1]);
	};
	try {
		p.Parse(s, [](int c){ Cout().Put(c); });
	}
	catch(const AnsiParser::Error& e) {
		Cout() << "Parser error: " << e;
	}
}
