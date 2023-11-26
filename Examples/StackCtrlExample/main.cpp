#include <StackCtrl/StackCtrl.h>

using namespace Upp;

#define LAYOUTFILE <StackCtrlExample/StackCtrl.lay>
#include <CtrlCore/lay.h>

struct MyApp : WithMainLayout<TopWindow> {
	Clock     clock;
	Calendar  calendar;
	DocEdit   docedit;
	ArrayCtrl list;
	MyApp()
	{
		CtrlLayout(*this, "StackCtrl test");
		Sizeable().Zoomable().CenterScreen();

		list.AddColumn("Numbers");
		for(int i = 0; i < 100; i++)
			list.Add(AsString(i));

		prev  << [this] { stack.Prev(); };
		next  << [this] { stack.Next(); };
		home  << [this] { stack.GoBegin(); };
		end   << [this] { stack.GoEnd(); };
		wheel << [this] { stack.Wheel(~wheel); };
		
		animmode.Add(0, "No animation");
		animmode.Add(1, "Vertical animation");
		animmode.Add(2, "Horizontal animation");
		animmode.GoBegin();
		
		animmode << [this]
		{
			int i = ~animmode;
			switch(i) {
			case 1:  stack.Vert().Animation(); break;
			case 2:  stack.Horz().Animation(); break;
			default: stack.Animation(0);
			};
		};

        // Stack children.      // Indices
        stack.Add(clock);       // 0
        stack.Add(calendar);    // 1
        stack.Add(docedit);     // 2
        stack.Add(list);        // 3

		// Get a child ctrl from index and set its data.
		stack.Get(2) <<=  "Hello world.";
	}
	
	bool Key(dword key, int count) final
	{
		switch(key) {
		case K_SHIFT|K_ALT|K_LEFT:  stack.Prev(); break;
		case K_SHIFT|K_ALT|K_RIGHT: stack.Next(); break;
		case K_SHIFT|K_ALT|K_HOME:  stack.GoBegin(); break;
		case K_SHIFT|K_ALT|K_END:   stack.GoEnd(); break;
		default: return Ctrl::Key(key, count);
		}
		return true;
	}
};

GUI_APP_MAIN
{
	MyApp().Run();
}
