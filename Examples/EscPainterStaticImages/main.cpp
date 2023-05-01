#include <CodeEditor/CodeEditor.h>
#include <EscPainter/EscPainter.h>

using namespace Upp;

struct ZoomableImageCtrl : Ctrl {
	Image img;
	void Paint(Draw& w) final { w.DrawImage(0, 0, img); }
};

struct EscPainter : TopWindow {
	ArrayMap<String, EscValue> global;
	Splitter   splitter;
	CodeEditor edit;
	ZoomableImageCtrl  ictl;
	
	EscPainter()
	{
		Title(t_("EscPainter libary demo. (static image generator)"));
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
	}
	
	void ExecuteScript()
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
	}
};



GUI_APP_MAIN
{
	StdLogSetup(LOG_FILE);
	EscPainter().Run();
}

