#include <CtrlLib/CtrlLib.h>
#include <Terminal/Sixel.h>

// This example demonstrates the SixelRenderer class of Terminal package.
// Two sixel images are provided with this example:
// 1) scientia.sixel
// 2) van-gogh.sixel
// These sixels can be found in the upp-components/CtrlLib/Images directory.

using namespace Upp;

struct SixelViewer : TopWindow {

	Image img;
	void Paint(Draw& w)
	{
		if(!IsNull(img))
			w.DrawImage(GetSize(), img); // Scaled.
	}

	SixelViewer(const String& data)
	{
		Title(t_("Sixel Viewer")).Sizeable().Zoomable().CenterScreen();
		img = RenderSixelImage(data, Size(800, 600), SBlack(),  CheckUtf8(data));
		SetRect(img.GetSize());
	}
};

GUI_APP_MAIN
{
	while(1) {
		String sixel = SelectLoadFile("*.sixel");
		if(sixel.IsEmpty()) break;
		SixelViewer(sixel).Run();
	}
}
