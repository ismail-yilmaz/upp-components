#ifndef _Message_Message_h_
#define _Message_Message_h_

#include <CtrlLib/CtrlLib.h>

namespace Upp {

class MessageBox : public FrameCtrl<ParentCtrl> {
public:
    enum class Type  { INFORMATION, WARNING, QUESTION, SUCCESS, ERROR, CUSTON };
    enum class Place { TOP, BOTTOM };

    MessageBox()                                    { place = Place::TOP; }
    MessageBox& Placement(Place pl)                 { place = pl; return *this; }
    MessageBox& MessageType(Type t)                 { msgtype = t; return *this; }
    MessageBox& Icon(Image img)                     { icon  = img; return *this; }
    MessageBox& Background(Color c)                 { paper = c;   return *this; }
    MessageBox& ButtonR(int id, const String& s)    { id1 = id; bt1.SetLabel(s); return *this; }
    MessageBox& ButtonM(int id, const String& s)    { id2 = id; bt2.SetLabel(s); return *this; }
    MessageBox& ButtonL(int id, const String& s)    { id3 = id; bt3.SetLabel(s); return *this; }
    
    void        Set(Ctrl& c, const String& msg, bool animate = false);

    bool        IsDiscarded() const                 { return discarded; }
    
    Event<int> WhenAction;
    Event<const String&> WhenLink;

    virtual void FrameLayout(Rect& r) override;
    virtual void FrameAddSize(Size& sz) override    { sz.cy += animated ? ctrl.GetSize().cy : GetHeight(); }
    virtual void FramePaint(Draw& w, const Rect& r) override;
    
private:
    int  GetHeight() const                          { return clamp(qtf.GetHeight() + 8, Ctrl::VertLayoutZoom(28), 1080); }
    void SetButtonLayout(Button& b, int id, int& rpos, int& cx);
    void Discard();

    struct Dummy : public Ctrl { // Redirects layout synchronization.
        Ctrl* parent;
        void  Layout() final;
    };
    
    RichTextCtrl qtf;
    Button  bt1, bt2, bt3;
    int     id1, id2, id3;
    Dummy   ctrl;
    Image   icon;
    Color   paper;
    bool    animated;
    bool    discarded;
    Type    msgtype;
    Place   place;
};

class Message {
public:
    Message() { animate = false; place = MessageBox::Place::TOP; }
    
    Message&    Animation(bool b = true)    { animate = b; return *this;}
    Message&    Top()                       { place = MessageBox::Place::TOP; return *this; }
    Message&    Bottom()                    { place = MessageBox::Place::BOTTOM; return *this; }
    
    MessageBox& Create();
    void        Clear()                     { messages.Clear(); }

    Message&    Information(Ctrl& c, const String& s, Event<const String&> link = Null);
    Message&    Warning(Ctrl& c, const String& s, Event<const String&> link = Null);
    Message&    Success(Ctrl& c, const String& s, Event<const String&> link = Null);
    
    Message&    AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);

    Message&    Error(Ctrl& c, const String& s, Event<const String&> link = Null);
    Message&    ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&    ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);

private:
    Array<MessageBox> messages;
    bool animate;
    MessageBox::Place place;
};
}
#endif
