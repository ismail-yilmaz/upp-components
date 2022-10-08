#include <CtrlLib/CtrlLib.h>
#include <SixelRaster/SixelRaster.h>

// This  example demonstrates the usage of SixelRaster class as a a part of U++ image decoding factory.

// Two sixel images are provided with this example:
// 1) scientia.sixel
// 2) van-gogh.sixel
// These sixels can be found in the upp-components/CtrlLib/Images directory.

using namespace Upp;

GUI_APP_MAIN
{
	TopWindow win;
	ImageCtrl ctl;
	win.Add(ctl.SizePos());

	for(;;) {
		SelectFileIn fi("*.sixel *.six");
		if(!fi)	return;
		Image img = StreamRaster::LoadAny(fi); // Use common StreamRaster interface to decode the image.
		ctl.SetImage(img);
		win.SetRect(img.GetSize());
		win.Sizeable().Zoomable().Run();
	}
}
