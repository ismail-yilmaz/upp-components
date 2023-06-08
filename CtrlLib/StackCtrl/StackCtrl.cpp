#include "StackCtrl.h"

namespace Upp {

StackCtrl& StackCtrl::Add(Ctrl& c)
{
	return Insert(GetCount(), c);
}

StackCtrl& StackCtrl::Insert(int i, Ctrl& c)
{
	if(c.InFrame())
		Ctrl::Add(c);
	else {
		c.Hide();
		Ctrl::Add(c.SizePos());
		list.Insert(i, &c);
		Activate(&c);
	}
	return *this;
}

void StackCtrl::ChildRemoved(Ctrl *c)
{
	Ctrl::ChildRemoved(c);
	int i = list.Find(c);
	if(i < 0)
		return;
	if(i > 0)
		Prev();
	else
		Next();
	list.RemoveKey(c);
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

void StackCtrl::Activate(Ctrl *c)
{
	// TODO: Implement transition animation.
	
	if(!activectrl)
		activectrl = c;
	
	if(c != activectrl) {
		activectrl->Hide();
		activectrl = c;
	}

	activectrl->Show();
	activectrl->SetFocus();
	WhenAction();
}

}
