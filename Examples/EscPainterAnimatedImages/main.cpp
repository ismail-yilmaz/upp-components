#include <CodeEditor/CodeEditor.h>
#include <EscPainter/EscPainter.h>

using namespace Upp;

struct ZoomableImageCtrl : Ctrl {
	Image img;
	void Paint(Draw& w) final { w.DrawImage(0, 0, img); }
};

struct EscAnimator : TopWindow {
	ArrayMap<String, EscValue> global;
	Splitter   splitter;
	CodeEditor edit;
	ZoomableImageCtrl  ictl;

    enum TimerIds { TIMEID_REFRESH = Ctrl::TIMEID_COUNT, TIMEID_COUNT };
    
	EscAnimator()
	{
		Title(t_("EscPainter libary demo. (Animated images)"));
		Sizeable().Zoomable().CenterScreen().SetRect(0, 0, 1024, 800);
		Add(splitter.SizePos());
		edit.LineNumbers(true);
		edit.Set(LoadFile(GetDataFile("script.usc")));
		edit.WhenAction = [=] {	if(edit.IsDirty()) ExecuteScript(); };
		splitter.Set(edit.SizePos(), ictl.SizePos());
		InitEscLibs();
		ExecuteScript();
	}
	
	void InitEscLibs()
	{
		global.Clear();
		StdLib(global);
		PainterLib(global);
		
		// Animation helper function
		Escape(global, "msecs()", [](EscValue& e) { e = msecs(); });
	}
	
	void ExecuteScript()
	{
		KillSetTimeCallback(-16, [this]
		{
			try
			{
				ictl.img = EscPaintImage(global, edit.Get(), ictl.GetSize());
				ictl.Refresh();
			}
			catch(CParser::Error e)
			{
				RLOG(e);
			}
			catch(...)
			{
			}
		},
		TIMEID_REFRESH);
	}
};



GUI_APP_MAIN
{
	StdLogSetup(LOG_FILE);
	EscAnimator().Run();
}

