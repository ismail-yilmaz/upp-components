#include <Terminal/Terminal.h>
#include <Terminal/PtyProcess.h>

// This example demonstrates a virtual terminal with external sixel image  viewer.
// Two sixel images are provided with this example:
// 1) scientia.sixel
// 2) van-gogh.sixel
// These sixels can be found in the upp-components/CtrlLib/Images directory.
// You can use the "cat" command to view them.

using namespace Upp;

const char *nixshell = "/bin/bash";

struct SixelTerminalExample : TopWindow {
	Terminal	term;
	PtyProcess	pty;				// This class is completely optional
	TopWindow	sviewer;
	ImageCtrl	imgctrl;

	typedef SixelTerminalExample CLASSNAME;

	SixelTerminalExample()
	{
		SetRect(term.GetStdSize());	// 80 x 24 cells (scaled)
		Sizeable().Zoomable().CenterScreen().Add(term.SizePos());
		sviewer.Title(t_("Sixel Viewer")).Sizeable().Zoomable().Add(imgctrl.SizePos());

		term.WhenBell   = [=]()			{ BeepExclamation(); };
		term.WhenTitle  = [=](String s)	{ Title(s);	};
		term.WhenResize = [=]()			{ pty.SetSize(term.GetPageSize()); };
		term.WhenOutput = [=](String s)	{ PutGet(s); };
		term.WhenSixel  = THISFN(ShowSixelImage);
		term.SixelGraphics();

		SetTimeCallback(-1, [=] { PutGet(); });
		pty.Start(nixshell, Environment(), GetHomeDirectory()); // Defaults to TERM=xterm
	}

	void PutGet(String out = Null)
	{
		term.CheckWriteUtf8(pty.Get());
		pty.Write(out);
		if(!pty.IsRunning())
			Break();
	}

	void ShowSixelImage(const SixelInfo& sinfo, const String& sdata)
	{
		Image img = SixelRenderer(sdata, sinfo).SetPaper(Black());
		imgctrl.SetImage(img);
		if(!sviewer.IsOpen()) {
			sviewer.SetRect(img.GetSize());
			sviewer.CenterOwner().Open(this);
		}
	}
};

GUI_APP_MAIN
{
	SixelTerminalExample().Run();
}