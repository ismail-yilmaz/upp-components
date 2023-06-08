#include <StackCtrl/StackCtrl.h>

using namespace Upp;

struct MyApp : TopWindow {
	StackCtrl stack;
	ArrayCtrl list;
	Array<Ctrl> children;
	
	MyApp()
	{
		Sizeable().Zoomable().CenterScreen().SetRect(0, 0, 800, 600);

		list.AddColumn("Numbers");

		// Stack children.				// Indices
		stack.Add(children.Create<Clock>());		// 0
		stack.Add(children.Create<Calendar>());		// 1
		stack.Add(children.Create<DocEdit>());		// 2
		stack.Add(list);				// 3
		
		// Get a child ctrl from index and set its data.
		stack.Get(2) <<=  "Hello world.";

		// Find a child ctrl in the stack and modify it.
		int i = stack.Find(list);
		if(i >= 0) {
			ArrayCtrl *l = dynamic_cast<ArrayCtrl*>(&stack.Get(i));
			for(int j = 0; l && (j < 100); j++)
				l->Add(AsString(j));
		}
		// Add a stackctrl and enable the wheel (circular navigation) mode.
		Add(stack.Wheel().SizePos());
	}
	
	bool Key(dword key, int count) final
	{
		switch(key) {
		case K_SHIFT|K_ALT|K_LEFT:  stack.Prev(); break;
		case K_SHIFT|K_ALT|K_RIGHT: stack.Next(); break;
		case K_SHIFT|K_ALT|K_HOME:  stack.GoBegin(); break;
		case K_SHIFT|K_ALT|K_END:   stack.GoEnd();
		default: return Ctrl::Key(key, count);
		}
		return true;
	}
};

GUI_APP_MAIN
{
	MyApp().Run();
}

