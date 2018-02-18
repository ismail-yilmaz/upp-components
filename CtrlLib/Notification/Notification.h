#ifndef _Nofitifcation_Nofitifcation_h
#define _Nofitifcation_Nofitifcation_h

#include <CtrlLib/CtrlLib.h>

namespace Upp {

enum NotificationType {
    NOTIFY_INFORMATION,
    NOTIFY_QUESTION,
    NOTIFY_WARNING,
    NOTIFY_SUCCESS,
    NOTIFY_ERROR,
    NOTIFY_CUSTOM
};

class NotifyFrame : FrameCtrl<ParentCtrl> {
public:
    void Set(Ctrl& c, const char *txt, const char* button1, const char* button2, const char* button3,
                            int id1, int id2, int id3, Image ico, Color color, bool anim, int ntype);
    
    Event<int>           WhenAction;
    Event<const String&> WhenLink;

    inline bool  IsDiscarded() const { return discarded; }
    
    virtual void FrameLayout(Rect& r);
    virtual void FrameAddSize(Size& sz);
    virtual void FramePaint(Draw& w, const Rect& r);
    
private:
    struct Refresher : Ctrl {
        Ctrl *parent;
        virtual void Layout() final { parent->RefreshLayout(); parent->Sync(); }
    };
    
    RichTextCtrl qtf;
    Refresher    ctrl;
    Button       bt1, bt2, bt3;
    Image        icon;
    Color        paper;
    bool         animated;
    bool         discarded;
    int          type;

    int  GetHeight() const { return clamp(qtf.GetHeight() + 8, Ctrl::VertLayoutZoom(28), 1080); }
    void Discard();
    
public:
    NotifyFrame();
    virtual ~NotifyFrame() {}
};

class Notification {
public:
    Notification&   Information(Ctrl& c, const char *txt, Event<const String&> link = Null);
    Notification&   Warning(Ctrl& c, const char *txt, Event<const String&> link = Null);
    Notification&   Success(Ctrl& c, const char *txt, Event<const String&> link = Null);
    
    Notification&   AskYesNo(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   AskYesNoCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   AskRetryCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   AskAbortRetry(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   AskAbortRetryIgnore(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);

    Notification&   Error(Ctrl& c, const char *txt, Event<const String&> link = Null);
    Notification&   ErrorOKCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   ErrorYesNo(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   ErrorYesNoCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   ErrorRetryCancel(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   ErrorAbortRetry(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    Notification&   ErrorAbortRetryIgnore(Ctrl& c, const char *txt, Event<int> action, Event<const String&> link = Null);
    
    Notification&   Animation(bool b = true) { animate = b; return *this;}

    NotifyFrame&    Create();
    
    void            Clear()     { notifications.Clear(); }
        
    Notification() { animate = false; }

private:
    Array<NotifyFrame> notifications;
    bool animate;

};
}
#endif