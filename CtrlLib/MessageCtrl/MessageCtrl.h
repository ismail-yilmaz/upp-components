#ifndef _Message_Message_h_
#define _Message_Message_h_

#include <CtrlLib/CtrlLib.h>

namespace Upp {

class MessageBox : public FrameCtrl<ParentCtrl> {
public:
    enum class Type  { INFORMATION, WARNING, QUESTION, SUCCESS, FAILURE, CUSTOM };
    enum class Place { TOP, BOTTOM };

    MessageBox()                                    { place = Place::TOP; }
    virtual ~MessageBox()                           { if(!IsDiscarded()) Discard(); }
    
    MessageBox& Placement(Place pl)                 { place = pl; return *this; }
    MessageBox& MessageType(Type t)                 { msgtype = t; return *this; }
    MessageBox& Icon(Image img)                     { icon  = img; return *this; }
    MessageBox& Background(Color c)                 { paper = c;   return *this; }
    MessageBox& ButtonR(int id, const String& s)    { id1 = id; bt1.SetLabel(s); return *this; }
    MessageBox& ButtonM(int id, const String& s)    { id2 = id; bt2.SetLabel(s); return *this; }
    MessageBox& ButtonL(int id, const String& s)    { id3 = id; bt3.SetLabel(s); return *this; }

    void        Set(Ctrl& c, const String& msg, bool animate = false, bool append = false, int secs = 0);

    bool        IsDiscarded() const                 { return discarded; }

    Event<int> WhenAction;
    Event<const String&> WhenLink;

    virtual void FrameLayout(Rect& r) override;
    virtual void FrameAddSize(Size& sz) override    { sz.cy += animated ? ctrl.GetSize().cy : GetHeight(); }
    virtual void FramePaint(Draw& w, const Rect& r) override;

private:
    int  GetHeight() const                          { return clamp(qtf.GetHeight() + 8, Ctrl::VertLayoutZoom(28), 1080); }
    void SetButtonLayout(Button& b, int id, int& rpos);
    void Discard();

    struct Dummy : public Ctrl { // Redirects layout synchronization.
        Ctrl* parent;
        void  Layout() final;
    };

    RichTextCtrl qtf;
    TimeCallback tcb;
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

class MessageCtrl {
public:
    MessageCtrl();

    MessageCtrl&    Animation(bool b = true)    { animate = b; return *this;}
    MessageCtrl&    Top()                       { place = MessageBox::Place::TOP; return *this; }
    MessageCtrl&    Bottom()                    { place = MessageBox::Place::BOTTOM; return *this; }
    MessageCtrl&    Append(bool b = true)       { append = b; return *this; }

    MessageBox&     Create();
    void            Clear(const Ctrl* c = nullptr);

    // Informative messages.
    MessageCtrl&    Information(Ctrl& c, const String& s, Event<const String&> link = Null, int secs = 0);
    MessageCtrl&    Warning(Ctrl& c, const String& s, Event<const String&> link = Null);
    MessageCtrl&    Success(Ctrl& c, const String& s, Event<const String&> link = Null);
    MessageCtrl&    Error(Ctrl& c, const String& s, Event<const String&> link = Null);

    // Interactive messages.
    MessageCtrl&    AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);

    MessageCtrl&    ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    MessageCtrl&    ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);

private:
    Array<MessageBox> messages;
    bool animate;
    bool append;
    MessageBox::Place place;
};
}
#endif
