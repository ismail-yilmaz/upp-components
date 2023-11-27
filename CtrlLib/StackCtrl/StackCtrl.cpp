#include "StackCtrl.h"

namespace Upp {

StackCtrl::StackCtrl()
: activectrl(nullptr)
, duration(0)
, vertical(false)
, wheel(false)
, animating(false)
{
}

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
	GuiLock __;
	
	if(!activectrl)
		activectrl = ctrl;
	
	if(ctrl != activectrl) {
		if(duration >= 100)
			Animate(ctrl);
		activectrl->Hide();
		activectrl = ctrl;
	}

	activectrl->Show();
	activectrl->SetFocus();
	WhenAction();
}

bool StackCtrl::ScrollCtrl(enum Direction d, Ctrl& ctrl, Rect r, Rect target, int time)
{
	switch(d) {
	case Direction::Left:
		r.left  -= ((r.left - target.left) * time) / duration;
		r.right -= ((r.right - target.right) * time) / duration;
		break;
	case Direction::Right:
		r.left  += ((target.left - r.left) * time) / duration;
		r.right += ((target.right - r.right) * time) / duration;
		break;
	case Direction::Up:
		r.top    -= ((r.top - target.top) * time) / duration;
		r.bottom -= ((r.bottom - target.bottom) * time) / duration;
		break;
	case Direction::Down:
		r.top    += ((target.top - r.top) * time) / duration;
		r.bottom += ((target.bottom - r.bottom) * time) / duration;
		break;
	default:
		return true;
	}
	ctrl.SetRect(r);
	return r == target;
}

void StackCtrl::Animate(Ctrl *nextctrl)
{
	if(animating)
		return;
	
	animating = true;
	
	int a = list.Find(activectrl);
	int b = list.Find(nextctrl);

	if(a == b) {
		animating = false;
		return;
	}
	
	Direction direction;
	Rect r = GetView(), rsrc1 = r, rdst1 = r, rsrc2 = r, rdst2 = r;
	Size sz = r.GetSize();
	
	auto SetUpLeft = [&, this]()
	{
		direction = vertical ? Direction::Up : Direction::Left;
		switch(direction) {
		case Direction::Up:
			rdst1.OffsetVert(sz.cx);
			rsrc2.OffsetVert(-sz.cx);
			break;
		case Direction::Left:
			rdst1.OffsetHorz(sz.cx);
			rsrc2.OffsetHorz(-sz.cx);
			break;
		default:
			NEVER();
		}
	};

	auto SetDownRight = [&, this]()
	{
		direction = vertical ? Direction::Down : Direction::Right;
		switch(direction) {
		case Direction::Down:
			rdst1.OffsetVert(-sz.cx);
			rsrc2.OffsetVert(sz.cx);
			break;
		case Direction::Right:
			rdst1.OffsetHorz(-sz.cx);
			rsrc2.OffsetHorz(sz.cx);
			break;
		default:
			NEVER();
		}
	};
	
	int n = GetCount();
	
	if(n > 2 && a == n - 1 && b == 0)
		SetDownRight();
	else
	if(n > 2 && a == 0 && b == n - 1)
		SetUpLeft();
	else
	if(a > b)
		SetUpLeft();
	else
	if(a < b)
		SetDownRight();

	nextctrl->SetRect(rsrc1);
	nextctrl->Show();

	dword start_time = msecs();
	for(;;) {
		int t = int(msecs() - start_time);
		if(t > duration)
			break;
		bool done1 = ScrollCtrl(direction, *activectrl, rsrc1, rdst1, t);
		bool done2 = ScrollCtrl(direction, *nextctrl,   rsrc2, rdst2, t);
		if(done1 || done2)
			break;
		#ifdef PLATFORM_WIN32
		// For some reason, Sync doesn't work as expected on Win32...
		nextctrl->Refresh();
		activectrl->Refresh();
		#else
		nextctrl->Sync();
		activectrl->Sync();
		#endif
		Ctrl::ProcessEvents();
		Sleep(0);
	}
	activectrl->SizePos();
	nextctrl->SizePos();
	
	animating = false;
}

void StackCtrl::Serialize(Stream& s)
{
	s % wheel % vertical % duration;
}

}
