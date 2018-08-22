#ifndef _AnsiParser_AnsiParser_h
#define _AnsiParser_AnsiParser_h

#include <Core/Core.h>

namespace Upp {

class AnsiParser : Moveable<AnsiParser> {
public:
    AnsiParser()                                    { Reset(); }
    
    virtual int     Parse(int c);
    void            Parse(Stream& in, Event<int> out)       { Parse0(in, pick(out), false); }
    void            ParseUtf8(Stream& in, Event<int> out)   { Parse0(in, pick(out), true);  }
    void            Reset();
    
    int             GetControl() const              { return ctl; }
    Vector<String>  GetParameters() const           { return Split(parameters, ';', false); }
    String          GetIntermediate() const         { return intermediate; }
    int             GetTerminator() const           { return fin; }
    
    bool            IsEsc(int c) const              { return c == 0x1b; }
    bool            IsCsi(int c) const              { return c == 0x5b || c == 0x9b; }
    bool            IsOsc(int c) const              { return c == 0x5d || c == 0x9d; }
    bool            IsCmd(int c) const              { return IsCsi(c) || IsOsc(c);   }
    bool            IsC0(int c) const               { return c >= 0x00 && c <= 0x1f; }
    bool            IsC1(int c) const               { return c >= 0x80 && c <= 0x9f; }
    bool            IsControl(int c) const          { return IsC0(c) || IsC1(c);     }
    bool            IsIntermediate(int c) const     { return c >= 0x20 && c <= 0x2f; }
    bool            IsParameter(int c) const        { return c >= 0x30 && c <= 0x3f; }
    bool            IsAlphabetic(int c) const       { return c >= 0x40 && c <= 0x7e; }
    
    Event<>         WhenEsc;
    Event<>         WhenCsi;
    Event<>         WhenOsc;
        
    struct Error : Exc
    {
        Error(const String& reason) : Exc(reason) {}
    };
    
    AnsiParser(AnsiParser&&) = default;
    AnsiParser& operator=(AnsiParser&&) = default;
    
protected:
    enum class      Mode    { PLAIN, ESCAPE };
    enum class      Type    { CONTROL, PARAMETER, INTERMEDIATE, ALPHABETIC, INVALID };
    
    int             ctl;
    int             fin;
    Type            type;
    Mode            mode;
    String          parameters;
    String          intermediate;
    
    Type            GetType(int c);
    virtual void    ParseCsi(int c);
    virtual void    ParseEsc(int c);
    virtual void    ParseOsc(int c);
    
private:
    void            Parse0(Stream& in, Event<int>&& out, bool utf8 = false);
};
}
#endif
