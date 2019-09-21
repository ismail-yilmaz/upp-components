#include <CtrlLib/CtrlLib.h>
#include <Terminal/Sixel.h>

// This  example demonstrates the SixelRaster interface of theTerminal package.
// By default this viewer should be able to display bmp, png and sizel formats,
// through a common interface.

// Two sixel images are provided with this example:
// 1) scientia.sixel
// 2) van-gogh.sixel
// These sixels can be found in the upp-components/CtrlLib/Images directory.

using namespace Upp;

struct SixelViewer : TopWindow {
	Image img;
	void Paint(Draw& w) override
	{
		if(!IsNull(img)) w.DrawImage(0, 0, img);
	}

	SixelViewer()
	{
		Title(t_("Sixel viewer (Press CTRL + O to open a sixel file)"));
		Sizeable().Zoomable().CenterScreen().SetRect(0, 0, 640, 200);
	}
	
	bool Key(dword key, int count) override
	{
		if(key == K_CTRL_O) {
			img = StreamRaster::LoadFileAny(SelectFileOpen("*.png *.bmp *.sixel"));
			if(!IsNull(img))
				SetRect(Rect(GetRect().TopLeft(), img.GetSize()));
		}
		return true;
	}
};

GUI_APP_MAIN
{
	SixelViewer().Run();
}
