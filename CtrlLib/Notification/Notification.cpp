#include "Notification.h"

namespace Upp {


void NotifyFrame::Set(Ctrl& c, const char *txt, const char* button1, const char* button2, const char* button3,
                                     int id1, int id2, int id3, Image ico, Color color, bool anim, int ntype)
{
	// Note: Color scheme is taken and modified from KMessageWidget.
	// See: https://api.kde.org/frameworks/kwidgetsaddons/html/kmessagewidget_8cpp_source.html
	
	switch(ntype) {
	case NOTIFY_INFORMATION:
		paper = Blend(Color(128, 128, 255), Color(255, 255, 255));
		icon  = CtrlImg::information();
		break;
	case NOTIFY_QUESTION:
		paper = Blend(LtGray(), Color(239, 240, 241));
		icon  = CtrlImg::question();
		break;
	case NOTIFY_WARNING:
		paper = Blend(Color(246, 180, 0), Color(239, 240, 241));
		icon  = CtrlImg::exclamation();
		break;
	case NOTIFY_SUCCESS:
		paper = Blend(Color(39, 170, 96), Color(239, 240, 241));
		icon  = CtrlImg::information();
		break;
	case NOTIFY_ERROR:
		paper = Blend(Color(218, 68, 83), Color(239, 240, 241));
		icon  = CtrlImg::error();
		break;
	default:
		paper = color;
		icon  = ico;
		break;
	}
	
	ctrl.parent = &c;
	c.AddFrame(*this);
	SetFrame(FieldFrame());
	
	qtf.NoSb();
	qtf.VCenter();
	qtf.SetQTF(String("[G1 ") + txt);
	qtf.WhenLink = Proxy(WhenLink);
	
	int fcy  = Draw::GetStdFontCy();
	int gap  = fcy / 4;
	int rpos = 4;
	int bcx  = Ctrl::HorzLayoutZoom(24);
	int bcy  = Ctrl::VertLayoutZoom(fcy);

	if(button1) {
		bcx = max(2 * fcy + GetTextSize(button1, Draw::GetStdFont()).cx, bcx);
		Add(bt1.RightPosZ(rpos, bcx).VCenterPosZ(bcy));
		bt1.SetLabel(button1);
		bt1 << [=] { WhenAction(id1); Discard(); };
		rpos += bcx + gap;
	}
	if(button2) {
		bcx = max(2 * fcy + GetTextSize(button2, Draw::GetStdFont()).cx, bcx);
		Add(bt2.RightPosZ(rpos, bcx).VCenterPosZ(bcy));
		bt2.SetLabel(button2);
		bt2 << [=] { WhenAction(id2); Discard(); };
		rpos += bcx + gap;
	}
	if(button3) {
		bcx = max(2 * fcy + GetTextSize(button3, Draw::GetStdFont()).cx, bcx);
		Add(bt3.RightPosZ(rpos, bcx).VCenterPosZ(bcy));
		bt3.SetLabel(button3);
		bt3 << [=] { WhenAction(id3); Discard(); };
		rpos += bcx + gap;
	}
	Add(qtf.HSizePosZ(36, rpos).VSizePosZ());
	
	if((animated = anim)) {
		Animate(ctrl, Rect(0, 0, c.GetSize().cx, GetHeight()), GUIEFFECT_SLIDE);
		animated = false;
	}
	else
		ctrl.SetRect(0, 0, c.GetSize().cx, GetHeight());
}

void NotifyFrame::FrameLayout(Rect& r)
{
	LayoutFrameTop(r, this, animated ? ctrl.GetSize().cy : GetHeight());
}

void NotifyFrame::FrameAddSize(Size& sz)
{
	sz.cy += animated ? ctrl.GetSize().cy : GetHeight();
}

void NotifyFrame::FramePaint(Draw& w, const Rect& r)
{
	Size  sz = GetSize();
	w.DrawRect(r, paper);
	auto fcy = Ctrl::VertLayoutZoom(Draw::GetStdFontCy());
	w.DrawImage(4, r.top + (sz.cy / 2) - (fcy / 2), fcy, fcy, icon);
}

void NotifyFrame::Discard()
{
	if(GetParent())
		GetParent()->RemoveFrame(*this);
	ctrl.SetRect(Null);
	discarded = true;
}

NotifyFrame::NotifyFrame()
{
	discarded = false;
}

Notification& Notification::Information(Ctrl& c, const char *txt, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, NOTIFY_INFORMATION);
	n.WhenLink = link;
	return *this;
}

Notification& Notification::Warning(Ctrl& c, const char *txt, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, NOTIFY_WARNING);
	n.WhenLink = link;
	return *this;
}

Notification& Notification::Success(Ctrl& c, const char *txt, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, NOTIFY_SUCCESS);
	n.WhenLink = link;
	return *this;
}

Notification& Notification::AskYesNo(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("No"), t_("Yes"), NULL,  IDNO, IDYES, 0, Null, Null, animate, NOTIFY_QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::AskYesNoCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Cancel"), t_("No"), t_("Yes"), IDCANCEL, IDNO, IDYES, Null, Null, animate, NOTIFY_QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::AskRetryCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Cancel"), t_("Retry"), NULL,  IDCANCEL, IDRETRY, 0, Null, Null, animate, NOTIFY_QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::AskAbortRetry(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Retry"), t_("Abort"), NULL,  IDRETRY, IDABORT, 0, Null, Null, animate, NOTIFY_QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::AskAbortRetryIgnore(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Ignore"), t_("Retry"), t_("Abort"),  IDIGNORE, IDRETRY, IDABORT, Null, Null, animate, NOTIFY_QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;

}

Notification& Notification::Error(Ctrl& c, const char *txt, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	return *this;
}

Notification& Notification::ErrorOKCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Cancel"), t_("OK"), NULL,  IDCANCEL, IDOK, 0, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::ErrorYesNo(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("No"), t_("Yes"), NULL,  IDNO, IDYES, 0, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::ErrorYesNoCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Cancel"), t_("No"), t_("Yes"),  IDCANCEL, IDNO, IDYES, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::ErrorRetryCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Cancel"), t_("Retry"), NULL, IDCANCEL, IDRETRY, 0, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::ErrorAbortRetry(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Retry"), t_("Abort"), NULL,  IDRETRY, IDABORT, 0, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Notification& Notification::ErrorAbortRetryIgnore(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, txt, t_("Ignore"), t_("Retry"), t_("Abort"),  IDIGNORE, IDRETRY, IDABORT, Null, Null, animate, NOTIFY_ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

NotifyFrame& Notification::Create()
{
	for(int i = 0; i < notifications.GetCount(); i++) {
		auto& nf = notifications[i];
		if(!nf.IsDiscarded())
			continue;
		notifications.Remove(i);
		i--;
	}
	return notifications.Add();
}
}