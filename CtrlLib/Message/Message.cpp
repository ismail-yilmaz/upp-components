#include "Message.h"

namespace Upp {

void Message::Frame::Set(Ctrl& c, const String& s, const String& button1,const String& button2,
	const String& button3, int id1, int id2, int id3, Image ico, Color color, bool anim, Message::Type ntype)
{
	// Note: Color scheme is taken and modified from KMessageWidget.
	// See: https://api.kde.org/frameworks/kwidgetsaddons/html/kmessagewidget_8cpp_source.html
	
	switch(ntype) {
	case Message::Type::INFORMATION:
		paper = Blend(Color(128, 128, 255), Color(255, 255, 255));
		icon  = CtrlImg::information();
		break;
	case Message::Type::QUESTION:
		paper = Blend(LtGray(), Color(239, 240, 241));
		icon  = CtrlImg::question();
		break;
	case Message::Type::WARNING:
		paper = Blend(Color(246, 180, 0), Color(239, 240, 241));
		icon  = CtrlImg::exclamation();
		break;
	case Message::Type::SUCCESS:
		paper = Blend(Color(39, 170, 96), Color(239, 240, 241));
		icon  = CtrlImg::information();
		break;
	case Message::Type::ERROR:
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
	qtf.SetQTF(String("[G1 ") + s);
	qtf.WhenLink = Proxy(WhenLink);
	
	int rpos = 4;
	int bcx  = Ctrl::HorzLayoutZoom(24);

	SetButtonLayout(bt1, button1, rpos, bcx, id1);
	SetButtonLayout(bt2, button2, rpos, bcx, id2);
	SetButtonLayout(bt3, button3, rpos, bcx, id3);

	Add(qtf.HSizePosZ(36, rpos).VSizePosZ());
	
	if((animated = anim)) {
		Animate(ctrl, Rect(0, 0, c.GetSize().cx, GetHeight()), GUIEFFECT_SLIDE);
		animated = false;
	}
	else
		ctrl.SetRect(0, 0, c.GetSize().cx, GetHeight());
}

void Message::Frame::FramePaint(Draw& w, const Rect& r)
{
	Size  sz = GetSize();
	w.DrawRect(r, paper);
	auto fcy = Ctrl::VertLayoutZoom(Draw::GetStdFontCy());
	w.DrawImage(4, r.top + (sz.cy / 2) - (fcy / 2), fcy, fcy, icon);
}

void Message::Frame::SetButtonLayout(Button& c, const String& s, int& pos, int& cx, int id)
{
	if(IsNull(s))
		return;

	int fcy  = Draw::GetStdFontCy();
	int gap  = fcy / 4;
	int bcy  = Ctrl::VertLayoutZoom(fcy);
	
	cx = max(2 * fcy + GetTextSize(s, Draw::GetStdFont()).cx, cx);
	Add(c.RightPosZ(pos, cx).VCenterPosZ(bcy));
	c.SetLabel(s);
	c << [=] { WhenAction(id); Discard(); };
	pos += cx + gap;
}

void Message::Frame::Discard()
{
	if(GetParent())
		GetParent()->RemoveFrame(*this);
	ctrl.SetRect(Null);
	ctrl.parent = NULL;
	discarded = true;
}

Message& Message::Information(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& fr = Create();
	fr.Set(c, s, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, Message::Type::INFORMATION);
	fr.WhenLink = link;
	return *this;
}

Message& Message::Warning(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& fr = Create();
	fr.Set(c, s, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, Message::Type::WARNING);
	fr.WhenLink = link;
	return *this;
}

Message& Message::Success(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, Message::Type::SUCCESS);
	n.WhenLink = link;
	return *this;
}

Message& Message::AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("No"), t_("Yes"), NULL,  IDNO, IDYES, 0, Null, Null, animate, Message::Type::QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Cancel"), t_("No"), t_("Yes"), IDCANCEL, IDNO, IDYES, Null, Null, animate, Message::Type::QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Cancel"), t_("Retry"), NULL,  IDCANCEL, IDRETRY, 0, Null, Null, animate, Message::Type::QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Retry"), t_("Abort"), NULL,  IDRETRY, IDABORT, 0, Null, Null, animate, Message::Type::QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Ignore"), t_("Retry"), t_("Abort"),  IDIGNORE, IDRETRY, IDABORT, Null, Null, animate, Message::Type::QUESTION);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::Error(Ctrl& c, const String& s, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("OK"), NULL, NULL,  IDOK, 0, 0, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	return *this;
}

Message& Message::ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Cancel"), t_("OK"), NULL,  IDCANCEL, IDOK, 0, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("No"), t_("Yes"), NULL,  IDNO, IDYES, 0, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Cancel"), t_("No"), t_("Yes"),  IDCANCEL, IDNO, IDYES, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Cancel"), t_("Retry"), NULL, IDCANCEL, IDRETRY, 0, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Retry"), t_("Abort"), NULL,  IDRETRY, IDABORT, 0, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message& Message::ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link)
{
	auto& n = Create();
	n.Set(c, s, t_("Ignore"), t_("Retry"), t_("Abort"),  IDIGNORE, IDRETRY, IDABORT, Null, Null, animate, Message::Type::ERROR);
	n.WhenLink = link;
	n.WhenAction = action;
	return *this;
}

Message::Frame& Message::Create()
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