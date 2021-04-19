#ifndef _VTCell_h_
#define _VTCell_h_

#include <Core/Core.h>

namespace Upp {

struct VTCell : Moveable<VTCell> {
    dword   chr; // TODO: Grapheme support.
    union {
        dword data;
        struct {
            word col; // Max. 65536
            word row; // Max. 65536
        } object;
    };
    word    attrs;
    word    sgr;
    Color   ink;
    Color   paper;

    enum Attrs : word { // It is possible to have both DEC and ISO selective erases...
        ATTR_PROTECTION_DEC = 0x0001,
        ATTR_PROTECTION_ISO = 0x0002,
        ATTR_PROTECTION_ALL = ATTR_PROTECTION_DEC | ATTR_PROTECTION_ISO
    };
    
    enum Sgr : word {
        SGR_NORMAL      = 0x0000,
        SGR_BOLD        = 0x0001,
        SGR_ITALIC      = 0x0002,
        SGR_UNDERLINE   = 0x0004,
        SGR_OVERLINE    = 0x0008,
        SGR_STRIKEOUT   = 0x0010,
        SGR_BLINK       = 0x0020,
        SGR_INVERTED    = 0x0040,
        SGR_HIDDEN      = 0x0080,
        SGR_FAINT       = 0x0100,
        SGR_IMAGE       = 0x0200,
        SGR_HYPERLINK   = 0x0400
    };

    enum FillerFlags : dword {
        FILL_NORMAL        = 0x0000,
        FILL_DEC_SELECTIVE = 0x0001,
        FILL_ISO_SELECTIVE = 0x0002,
        FILL_CHAR          = 0x0004,
        FILL_ATTRS         = 0x0008,
        FILL_SGR           = 0x0010,
        FILL_INK           = 0x0020,
        FILL_PAPER         = 0x0040,
        FILL_DATA          = 0x0080,
        XOR_SGR            = 0x0100,
    };
    
    VTCell& Normal()                         { sgr = SGR_NORMAL; return *this; }
    VTCell& Bold(bool b = true)              { sgr = (sgr & ~SGR_BOLD)      | (-word(b) & SGR_BOLD);  return *this;     }
    VTCell& Faint(bool b = true)             { sgr = (sgr & ~SGR_FAINT)     | (-word(b) & SGR_FAINT); return *this;     }
    VTCell& Italic(bool b = true)            { sgr = (sgr & ~SGR_ITALIC)    | (-word(b) & SGR_ITALIC); return *this;    }
    VTCell& Underline(bool b = true)         { sgr = (sgr & ~SGR_UNDERLINE) | (-word(b) & SGR_UNDERLINE); return *this; }
    VTCell& Overline(bool b = true)          { sgr = (sgr & ~SGR_OVERLINE)  | (-word(b) & SGR_OVERLINE); return *this;  }
    VTCell& Blink(bool b = true)             { sgr = (sgr & ~SGR_BLINK)     | (-word(b) & SGR_BLINK); return *this;     }
    VTCell& Strikeout(bool b = true)         { sgr = (sgr & ~SGR_STRIKEOUT) | (-word(b) & SGR_STRIKEOUT); return *this; }
    VTCell& Invert(bool b = true)            { sgr = (sgr & ~SGR_INVERTED)  | (-word(b) & SGR_INVERTED); return *this;  }
    VTCell& Conceal(bool b = true)           { sgr = (sgr & ~SGR_HIDDEN)    | (-word(b) & SGR_HIDDEN); return *this;    }
    VTCell& Image(bool b = true)             { sgr = (sgr & ~SGR_IMAGE)     | (-word(b) & SGR_IMAGE); return *this;     }
    VTCell& Hyperlink(bool b = true)         { sgr = (sgr & ~SGR_HYPERLINK) | (-word(b) & SGR_HYPERLINK); return *this; }

    VTCell& ProtectDEC(bool b = true)        { attrs = (attrs & ~ATTR_PROTECTION_DEC) | (-word(b) & ATTR_PROTECTION_DEC); return *this; }
    VTCell& ProtectISO(bool b = true)        { attrs = (attrs & ~ATTR_PROTECTION_ISO) | (-word(b) & ATTR_PROTECTION_ISO); return *this; }
    VTCell& Protect(bool b = true)           { attrs = (attrs & ~ATTR_PROTECTION_ALL) | (-word(b) & ATTR_PROTECTION_ALL); return *this; }

    static const VTCell& Void();
    
    VTCell& Ink(Color c)                     { ink = c; return *this;   }
    VTCell& Paper(Color c)                   { paper = c; return *this; }

    int  GetWidth() const;
    
    bool IsVoid() const                      { return this == &Void();       }
    bool IsNormal() const                    { return sgr == SGR_NORMAL;     }
    bool IsBold() const                      { return sgr & SGR_BOLD;        }
    bool IsFaint() const                     { return sgr & SGR_FAINT;       }
    bool IsItalic() const                    { return sgr & SGR_ITALIC;      }
    bool IsUnderlined() const                { return sgr & SGR_UNDERLINE;   }
    bool IsOverlined() const                 { return sgr & SGR_OVERLINE;    }
    bool IsBlinking() const                  { return sgr & SGR_BLINK;       }
    bool IsInverted() const                  { return sgr & SGR_INVERTED;    }
    bool IsStrikeout() const                 { return sgr & SGR_STRIKEOUT;   }
    bool IsConcealed() const                 { return sgr & SGR_HIDDEN;      }
    bool IsImage() const                     { return sgr & SGR_IMAGE;       }
    bool IsHyperlink() const                 { return sgr & SGR_HYPERLINK;   }
    bool IsProtected() const                 { return attrs & ATTR_PROTECTION_ALL; }
    bool HasDECProtection() const            { return attrs & ATTR_PROTECTION_DEC; }
    bool HasISOProtection() const            { return attrs & ATTR_PROTECTION_ISO; }
    bool IsNullInstance() const;

    void    Fill(const VTCell& filler, dword flags);

    void    Reset();
    
    void    Clear()                             { Reset(); chr = 0; attrs = 0; }
    void    operator=(const Nuller&)            { Clear(); }
    void    operator=(dword c)                  { chr = c; }
    operator dword() const                      { return chr; }

    void    Serialize(Stream& s);
    
    VTCell()                                    { Clear(); }
};
}
#endif
