#ifndef _VTCell_h_
#define _VTCell_h_

#include <Core/Core.h>

namespace Upp {
    
struct VTCell : Moveable<VTCell> {
    int     chr;
    word    attrs;
    word    sgr;
#ifdef flagTRUECOLOR
	int     ink;
	int     paper;
#else
    word    ink;
    word    paper;
#endif

    enum Attrs : byte {
        ATTR_PROTECTED  = 0x0001,
    };
    
    enum Sgr : byte {
        SGR_NORMAL      = 0x0000,
        SGR_BOLD        = 0x0001,
        SGR_ITALIC      = 0x0002,
        SGR_UNDERLINE   = 0x0004,
        SGR_STRIKEOUT   = 0x0008,
        SGR_BLINK       = 0x0010,
        SGR_INVERTED    = 0x0020,
        SGR_HIDDEN      = 0x0040,
        SGR_FAINT       = 0x0080
    };

    enum FillerFlags : dword {
        FILL_NORMAL     = 0x0000,
        FILL_SELECTIVE  = 0x0001,
        FILL_CHAR       = 0x0002,
        FILL_ATTRS      = 0x0004,
        FILL_SGR        = 0x0008,
        FILL_INK        = 0x0010,
        FILL_PAPER      = 0x0020,
        XOR_SGR         = 0x0040
    };
    
    VTCell& Normal()                            { sgr = SGR_NORMAL; return *this; }
    VTCell& Bold(bool b = true)                 { if(b) sgr |= SGR_BOLD; else sgr &= ~SGR_BOLD; return *this;                 }
    VTCell& Faint(bool b = true)                { if(b) sgr |= SGR_FAINT; else sgr &= ~SGR_FAINT; return *this;               }
    VTCell& Italic(bool b = true)               { if(b) sgr |= SGR_ITALIC; else sgr &= ~SGR_ITALIC; return *this;             }
    VTCell& Underline(bool b = true)            { if(b) sgr |= SGR_UNDERLINE; else sgr &= ~SGR_UNDERLINE; return *this;       }
    VTCell& Blink(bool b = true)                { if(b) sgr |= SGR_BLINK; else sgr &= ~SGR_BLINK; return *this;               }
    VTCell& Strikeout(bool b = true)            { if(b) sgr |= SGR_STRIKEOUT; else sgr &= ~SGR_STRIKEOUT; return *this;       }
    VTCell& Invert(bool b = true)               { if(b) sgr |= SGR_INVERTED; else sgr &= ~SGR_INVERTED; return *this;         }
    VTCell& Conceal(bool b = true)              { if(b) sgr |= SGR_HIDDEN; else sgr &= ~SGR_HIDDEN; return *this;             }
    VTCell& Protect(bool b = true)              { if(b) attrs |= ATTR_PROTECTED; else attrs &= ~ATTR_PROTECTED; return *this; }

    VTCell& Ink(int n);
    VTCell& Paper(int n);

    bool    IsNormal() const                    { return sgr == SGR_NORMAL; }
    bool    IsBold() const                      { return sgr & SGR_BOLD;    }
    bool    IsFaint() const                     { return sgr & SGR_FAINT;   }
    bool    IsItalic() const                    { return sgr & SGR_ITALIC;  }
    bool    IsUnderlined() const                { return sgr & SGR_UNDERLINE;}
    bool    IsBlinking() const                  { return sgr & SGR_BLINK;   }
    bool    IsInverted() const                  { return sgr & SGR_INVERTED;}
    bool    IsStrikeout() const                 { return sgr & SGR_STRIKEOUT;}
    bool    IsConcealed() const                 { return sgr & SGR_HIDDEN;  }
    bool    IsProtected() const                 { return attrs & ATTR_PROTECTED;}
    bool    IsNullInstance() const;

    void    Fill(const VTCell& filler, dword flags);

    void    Reset();
    
    void    Clear()                             { Reset(); chr = 0; attrs = 0; }
    void    operator=(const Nuller&)            { Clear(); }
    void    operator=(int c)                    { chr = c; }
    operator int() const                        { return chr; }
    
    void    Serialize(Stream& s);
    
    VTCell();
};
}
#endif
