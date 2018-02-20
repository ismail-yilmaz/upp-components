#include <CtrlLib/CtrlLib.h>

using namespace Upp;

struct Box : FrameCtrl<ParentCtrl> {
	bool a = false;
	RichTextCtrl qtf;
	void FrameAdd(Ctrl& c)      { c.Add(this->HSizePos()); Add(qtf.SizePos()); }
	void FrameLayout(Rect& r)   {  Upp::LayoutFrameTop(r, (Ctrl*) this, a ? GetSize().cy : qtf.GetHeight() + 18); }
	void FrameAddSize(Size& sz) {  sz.cy += a ? GetSize().cy : qtf.GetHeight() + 18; }
};
class RichTextTest : public TopWindow {
	Box box;
	Button bt;
	
public:
	RichTextTest()
	{
		SetRect(0, 0, 640, 480);
		Sizeable().Zoomable().CenterScreen();
		Add(bt.RightPos(4).BottomPos(4));
		
		bt << [=] {
			String z = "Hello World! asd assssssssssssssssssssssssssssss";
			box.a = true;
			box.qtf.NoSb();
			box.qtf.VCenter();                        // Comment out this line, and highligting works.
			box.qtf.SetQTF("[G1 " << z);
			                                     // As the height increses, highligting becomes impossible.
			                                      // This problem doesn't arise with wrapped (Line < 1) text
			box.qtf.SetFrame(FieldFrame());
			box.qtf.Background(LtGray());
			AddFrame(box);
			Animate(box, Rect(0,0, GetSize().cx, box.qtf.GetHeight() + 18), 1);
			box.a = false;
			};
	}
};

GUI_APP_MAIN
{
	RichTextTest().Run();
}
