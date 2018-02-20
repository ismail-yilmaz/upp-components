#include "Message.h"

namespace Upp {

void MessageBox::Set(Ctrl& c, const String& msg, bool animate)
{
	// Note: Color scheme is taken and modified from KMessageWidget.
	// See: https://api.kde.org/frameworks/kwidgetsaddons/html/kmessagewidget_8cpp_source.html

	switch(msgtype) {
	case MessageBox::Type::INFORMATION:
		paper = Blend(Color(128, 128, 255), Color(255, 255, 255));
		icon  = CtrlImg::information();
		break;
	case MessageBox::Type::QUESTION:
		paper = Blend(LtGray(), Color(239, 240, 241));
		icon  = CtrlImg::question();
		break;
	case MessageBox::Type::WARNING:
		paper = Blend(Color(246, 180, 0), Color(239, 240, 241));
		icon  = CtrlImg::exclamation();
		break;
	case MessageBox::Type::SUCCESS:
		paper = Blend(Color(39, 170, 96), Color(239, 240, 241));
		icon  = CtrlImg::information();
		break;
	case MessageBox::Type::ERROR:
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
	int bcx  = Ctrl::HorzLayoutZoom(24);

	SetButtonLayout(bt1, id1, rpos, bcx);
	SetButtonLayout(bt2, id2, rpos, bcx);
	SetButtonLayout(bt3, id3, rpos, bcx);

	Add(qtf.HSizePosZ(36, rpos).VSizePosZ());

	if((animate = animate)) {
		Animate(ctrl, Rect(0, 0, c.GetSize().cx, GetHeight()), GUIEFFECT_SLIDE);
		animated = false;
	}
	else
		ctrl.SetRect(0, 0, c.GetSize().cx, GetHeight());
}

void MessageBox::SetButtonLayout(Button& b, int id, int& rpos, int& cx)
{
	if(IsNull(b.GetLabel()))
		return;

	int fcy  = Draw::GetStdFontCy();
	int gap  = fcy / 4;
	int bcy  = Ctrl::VertLayoutZoom(fcy);

	cx = max(2 * fcy + GetTextSize(b.GetLabel(), Draw::GetStdFont()).cx, cx);
	Add(b.RightPosZ(rpos, cx).VCenterPosZ(bcy));
	b << [=] { WhenAction(id); Discard(); };
	rpos += cx + gap;
}

void MessageBox::Discard()
{
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
	auto fcy = Ctrl::VertLayoutZoom(Draw::GetStdFontCy());
	w.DrawImage(4, r.top + (sz.cy / 2) - (fcy / 2), fcy, fcy, icon);
}

void MessageBox::Dummy::Layout()
{
	if(parent) {
		parent->RefreshLayout();
		parent->Sync();
	}
}

Message& Message::Information(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::INFORMATION);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

Message& Message::Warning(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::WARNING);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

Message& Message::Success(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::SUCCESS);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

Message& Message::AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
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

Message& Message::AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
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

Message& Message::AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
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

Message& Message::AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
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

Message& Message::AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
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

Message& Message::Error(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	return *this;
}

Message& Message::ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDOK, t_("OK"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

Message& Message::ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDNO, t_("No"));
	msg.ButtonL(IDYES, t_("Yes"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

Message& Message::ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonM(IDNO, t_("No"));
	msg.ButtonL(IDYES,t_("Yes"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

Message& Message::ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDCANCEL, t_("Cancel"));
	msg.ButtonL(IDRETRY,t_("Retry"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

Message& Message::ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT, t_("Abort"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

Message& Message::ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& msg = Create();
	msg.MessageType(MessageBox::Type::ERROR);
	msg.Placement(place);
	msg.ButtonR(IDIGNORE, t_("Ignore"));
	msg.ButtonM(IDRETRY, t_("Retry"));
	msg.ButtonL(IDABORT,t_("Abort"));
	msg.Set(c, s, animate);
	msg.WhenLink = link;
	msg.WhenAction = action;
	return *this;
}

MessageBox& Message::Create()
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