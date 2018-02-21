#include <CtrlLib/CtrlLib.h>

using namespace Upp;

class RichTextTest : public TopWindow {
	Array<Button> bt;
	
public:
	RichTextTest()
	{
		SetRect(0, 0, 640, 480);
		Sizeable().Zoomable().CenterScreen();
		
		for(int i = 0; i < 10;)
			;

	}
};

GUI_APP_MAIN
{
	RichTextTest().Run();
}
