#include "StackCtrl.h"

namespace Upp {

StackCtrl& StackCtrl::Add(Ctrl& ctrl)
{
	return Insert(GetCount(), ctrl);
}

StackCtrl& StackCtrl::Insert(int i, Ctrl& ctrl)
{
	if(ctrl.InFrame())
		Ctrl::Add(ctrl);
	else {
		ctrl.Hide();
		Ctrl::Add(ctrl.SizePos());
		list.Insert(i, &ctrl);
		Activate(&ctrl);
	}
	return *this;
}

void StackCtrl::Remove(Ctrl& ctrl)
{
	int i = list.Find(&ctrl);
	if(i < 0)
		return;
	if(i > 0)
		Prev();
	else
		Next();
	list.RemoveKey(&ctrl);
	ctrl.Remove();
}

void StackCtrl::Prev()
{
	int i = list.Find(activectrl);
	if(i < 0)
		return;
	if(i > 0)
		Goto(i - 1);
	else
	if(wheel)
		GoEnd();
}

void StackCtrl::Next()
{
	int i = list.Find(activectrl);
	if(i < 0)
		return;
	if(i < GetCount() - 1)
		Goto(i + 1);
	else
	if(wheel)
		GoBegin();
}

void StackCtrl::Activate(Ctrl *ctrl)
{
	// TODO: Implement transition animation.
	
	if(!activectrl)
		activectrl = ctrl;
	
	if(ctrl != activectrl) {
		activectrl->Hide();
		activectrl = ctrl;
	}

	activectrl->Show();
	activectrl->SetFocus();
	WhenAction();
}

}
