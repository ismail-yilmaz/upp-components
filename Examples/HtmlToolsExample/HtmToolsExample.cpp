#include <Core/Core.h>
#include <Core/SSL/SSL.h>
#include <HtmlTools/HtmlTools.h>

using namespace Upp;

void PrintHtml(const HtmlNode& node)
{
	for(const HtmlNode& q : node) {
		if(q.IsTag("title"))
			Cout() << q.GatherText();
		else
		if(q.IsTag("p"))
			Cout() << q.GatherText();
		else
		if(q.IsTag("a"))
			Cout() << "For more information, see: " << q.Attr(0) << EOL;
		PrintHtml(q);
	}
}

CONSOLE_APP_MAIN
{
	StdLogSetup(LOG_FILE);
	HtmlNode n = ParseHtml(
		HttpRequest("https://example.com/").Execute(),
		{ { "wrap", 96 } }); // libtidy options...
	PrintHtml(n);
}