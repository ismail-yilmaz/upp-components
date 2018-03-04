#include "MessageCtrl.h"

namespace Upp {

static bool operator||(MessageBox::Type a, MessageBox::Type b) 
{
	return (int)a || int(b);
}

void MessageBox::Set(Ctrl& c, const String& msg, bool animate, int secs)
{
	// Note: Color scheme is taken and modified from KMessageWidget.
	// See: https://api.kde.org/frameworks/kwidgetsaddons/html/kmessagewidget_8cpp_source.html
	
	int duration = clamp(secs, 0, 24 * 60) * 1000;
	
	switch(msgtype) {
	case Type::INFORMATION:
		paper = Blend(Color(128, 128, 255), Color(255, 255, 255));
		icon  = CtrlImg::information();
		break;
	case Type::QUESTION:
		paper = Blend(LtGray(), Color(239, 240, 241));
		icon  = CtrlImg::question();
		break;
	case Type::WARNING:
		paper = Blend(Color(246, 180, 0), Color(239, 240, 241));
		icon  = CtrlImg::exclamation();
		break;
	case Type::SUCCESS:
		paper = Blend(Color(39, 170, 96), Color(239, 240, 241));
		icon  = CtrlImg::information();
		break;
	case Type::FAILURE:
		paper = Blend(Color(218, 68, 83), Color(239, 240, 241));
		icon  = CtrlImg::error();
		break;
	default:
		break;
	}

	SetFrame(FieldFrame());
	
	discarded   = false;
	ctrl.parent = &c;

	c.AddFrame(*this);

	qtf.NoSb();
	qtf.VCenter();
	qtf.SetQTF(String("[G1 ") + msg);
	qtf.WhenLink = Proxy(WhenLink);

	int rpos = 4;

	SetButtonLayout(bt1, id1, rpos);
	SetButtonLayout(bt2, id2, rpos);
	SetButtonLayout(bt3, id3, rpos);

	Add(qtf.HSizePosZ(IsNull(icon) ? 4 : 24, rpos).VSizePosZ());

	if((animated = animate)) {
		Animate(ctrl, Rect(0, 0, c.GetSize().cx, GetHeight()), GUIEFFECT_SLIDE);
		animated = false;
	}
	else
		ctrl.SetRect(0, 0, c.GetSize().cx, GetHeight());
	
	if((msgtype == Type::INFORMATION || msgtype == Type::CUSTOM) && duration)
		tcb.Set(duration, [=] { Discard(); });
}

void MessageBox::SetButtonLayout(Button& b, int id, int& rpos)
{
	if(IsNull(b.GetLabel()))
		return;

	int fcy  = Draw::GetStdFontCy();
	int gap  = fcy / 4;
	int cx   = 28;
	
	cx = max(2 * fcy + GetTextSize(b.GetLabel(), Draw::GetStdFont()).cx, cx);
	Add(b.RightPosZ(rpos, cx).VCenterPosZ(24));
	b << [=] { WhenAction(id); Discard(); };
	rpos += cx + gap;
}

void MessageBox::Discard()
{
	tcb.Kill();
	if(GetParent())
		GetParent()->RemoveFrame(*this);
	ctrl.SetRect(Null);
	ctrl.parent = nullptr;
	discarded   = true;
}

void MessageBox::FrameLayout(Rect& r)
{
	switch(place) {
	case Place::TOP:
		LayoutFrameTop(r, this, animated ? ctrl.GetSize().cy : GetHeight());
		break;
	case Place::BOTTOM:
		LayoutFrameBottom(r, this, animated ? ctrl.GetSize().cy : GetHeight());
		break;
	}
}

void MessageBox::FramePaint(Draw& w, const Rect& r)
{
	Size  sz = GetSize();
	w.DrawRect(r, paper);
	
	auto cy = Ctrl::VertLayoutZoom(16);
	w.DrawImage(
		4,
		(place == Place::TOP ? (r.top + (sz.cy / 2)) : r.bottom - (sz.cy /2)) - (cy / 2),
		cy, cy,
		icon
	);
}

void MessageBox::Dummy::Layout()
{
	if(parent) {
		parent->RefreshLayout();
		parent->Sync();
	}
}

MessageCtrl& MessageCtrl::Information(Ctrl& c, const String& s, Event<const String&> link, int sec)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::INFORMATION);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate, sec);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::Warning(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::WARNING);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::Success(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::SUCCESS);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.ButtonR(IDNO, t_("No"));
	msg.ButtonL(IDYES, t_("Yes"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonM(IDNO, t_("No"));
	msg.ButtonL(IDYES,t_("Yes"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDRETRY,t_("Retry"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.ButtonR(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT, t_("Abort"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::QUESTION);
	msg.Placement(place);
	msg.ButtonR(IDIGNORE, t_("Ignore"));
	msg.ButtonM(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT,t_("Abort"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::Error(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDNO, t_("No"));
	msg.ButtonL(IDYES, t_("Yes"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonM(IDNO, t_("No"));
	msg.ButtonL(IDYES,t_("Yes"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDRETRY,t_("Retry"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT, t_("Abort"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageCtrl& MessageCtrl::ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::FAILURE);
	msg.Placement(place);
	msg.ButtonR(IDIGNORE, t_("Ignore"));
	msg.ButtonM(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT,t_("Abort"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageBox& MessageCtrl::Create()
{
	for(int i = 0; i < messages.GetCount(); i++) {
		auto& msg = messages[i];
		if(!msg.IsDiscarded())
			continue;
		messages.Remove(i);
		i--;
	}
	return messages.Add();
}
}