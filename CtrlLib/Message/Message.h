#ifndef _Message_Message_h_
#define _Message_Message_h_

#include <CtrlLib/CtrlLib.h>

namespace Upp {

class Message {
public:
	enum class Type { INFORMATION, WARNING, QUESTION, SUCCESS, ERROR, CUSTON };

    class Frame : FrameCtrl<ParentCtrl> {
    public:
        Frame()														{ discarded = false; ctrl.parent = NULL; }
        
        void Set(Ctrl& c, const String& s, const String& button1, const String& button2, const String& button3,
                                int id1, int id2, int id3, Image ico, Color color, bool anim, Message::Type ntype);
                            
        virtual void FrameLayout(Rect& r) override                  { LayoutFrameTop(r, this, animated ? ctrl.GetSize().cy : GetHeight()); }
        virtual void FrameAddSize(Size& sz) override                { sz.cy += animated ? ctrl.GetSize().cy : GetHeight(); }
        virtual void FramePaint(Draw& w, const Rect& r) override;

        inline bool  IsDiscarded() const { return discarded; }

        Event<int>           WhenAction;
        Event<const String&> WhenLink;
        
    private:
        struct Refresher : Ctrl {
            Ctrl *parent;
            virtual void Layout() final { if(!parent) return; parent->RefreshLayout(); parent->Sync(); }
        };
        
        RichTextCtrl qtf;
        Refresher    ctrl;
        Button       bt1, bt2, bt3;
        Image        icon;
        Color        paper;
        bool         animated;
        bool         discarded;
        int          type;
    
        int  GetHeight() const { return  clamp(qtf.GetHeight() + 8, Ctrl::VertLayoutZoom(28), 1080); }
        void SetButtonLayout(Button& c, const String& s, int& pos, int& cx, int id);
        void Discard();
    };

public:
    Message() { animate = false; }

    Message&   Information(Ctrl& c, const String& s, Event<const String&> link = Null);
    Message&   Warning(Ctrl& c, const String& s, Event<const String&> link = Null);
    Message&   Success(Ctrl& c, const String& s, Event<const String&> link = Null);
    
    Message&   AskYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   AskYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   AskRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   AskAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   AskAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);

    Message&   Error(Ctrl& c, const String& s, Event<const String&> link = Null);
    Message&   ErrorOKCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   ErrorYesNo(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   ErrorYesNoCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   ErrorRetryCancel(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   ErrorAbortRetry(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    Message&   ErrorAbortRetryIgnore(Ctrl& c, const String& s, Event<int> action, Event<const String&> link = Null);
    
    Message&   Animation(bool b = true) { animate = b; return *this;}

    Frame& Create();
    
    void       Clear()     { messages.Clear(); }

private:
    Array<Frame> messages;
    bool animate;
};
}
#endif
