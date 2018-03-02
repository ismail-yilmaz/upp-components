#include <CtrlLib/CtrlLib.h>
#include <MessageCtrl/MessageCtrl.h>

using namespace Upp;

class Messages : public TopWindow {
	MessageCtrl msg;
	DocEdit editor;
	Button  button;
	
public:
	Messages()
	{
		Title("U++ Message Boxes (Passive Notifications)");
		SetRect(0,0, 640, 480);
		Sizeable().Zoomable().CenterScreen();
		SetMinSize({100, 100});

		auto action = [=](int id) {
			switch(id) {
			case IDYES: PromptOK("You've chosen 'yes'"); break;
			case IDNO:  PromptOK("You've chosen 'no'"); break;
			}
		};
		
		Add(editor.HSizePosZ().VSizePos(0, 24));
		Add(button.RightPos(4).BottomPos(4));
		button.SetLabel("Test");
		button <<  [=] {
			msg.Animation()
			  .Top()
			  .Information(*this, "This is a time-constrained information message. It will disappear in 5 seconds.", Null, 5)
			  .Success(*this, "This is a success message.")
			  .Warning(*this, "This is a warning message.")
			  .Error(*this, "This is an error message.")
			  .Bottom()
			  .AskYesNo(editor, "This is a question box 'in' the text editor with "
			                   "[^https:www`.ultimatepp`.org^ l`i`n`k]"
			                   " support. Would you like to continue?",
			                   action,
			                   callback(LaunchWebBrowser)
			);
			
		};
	}
};

GUI_APP_MAIN
{
	Messages().Run();
}