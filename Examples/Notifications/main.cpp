#include <CtrlLib/CtrlLib.h>
#include <Notification/Notification.h>

using namespace Upp;

class Notifications : public TopWindow {
	Notification nf;
	DocEdit editor;
	Button  button;
	
public:
	Notifications()
	{
		Title("U++ Passive Notifications");
		SetRect(0,0, 640, 480);
		Sizeable().Zoomable().CenterScreen();
		SetMinSize({100, 100});
		
		Add(editor.HSizePosZ().VSizePos(0, 24));
		Add(button.RightPos(4).BottomPos(4));
		button.SetLabel("Test");
		button <<  [=] {
			nf.Animation()
			  .Information(*this, "This is an information notification")
			  .Success(*this, "This is a success notification.")
			  .Warning(*this, "This is a warning notification")
			  .Error(*this, "This is an error notification");
			
			auto action = [=](int id) {
				switch(id) {
				case IDYES: PromptOK("You've chosen 'yes'"); break;
				case IDNO:  PromptOK("You've chosen 'no'"); break;
				}
			};
			nf.AskYesNo(editor, "This is a question notification  'in' the text editor with "
			                   "[^https://www`.ultimatepp`.org^ l`i`n`k]"
			                   " support. Would you like to continue?",
			                   action,
			                   callback(LaunchWebBrowser)
			);
			
		};
	}
};

GUI_APP_MAIN
{
	Notifications().Run();
}
