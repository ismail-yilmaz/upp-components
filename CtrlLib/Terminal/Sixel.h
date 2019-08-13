#ifndef _SixelRenderer_h_
#define _SixelRenderer_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

#include "Parser.h"

// TODO: Needs optimixation.

namespace Upp{
    
class SixelRenderer : NoCopy {
public:
    struct Info {
        int    aspectratio = 1;
        bool   nohole      = true;
        Size   size        = Null;
    };
    
    SixelRenderer(const String& data);
    SixelRenderer(const String& data, Size sz);
    SixelRenderer(const String& data, Info info);
    
    Image           Get();
    operator        Image()                         { return Get(); }
    
    SixelRenderer&  SetSize(Size sz)                { info.size = sz; return *this; }
    Size            GetSize() const                 { return info.size; }

    SixelRenderer&  SetPaper(Color c)               { paper = c; return *this; }

    SixelRenderer&  SetAspectRatio(int r)           { info.aspectratio = r; return *this; }
    int             GetAspectRatio() const          { return info.aspectratio; }

    SixelRenderer&  NoColorHole(bool b = true)      { info.nohole = b; return *this; }
    
private:
    void            SetCanvas();
    void            SetColors();
    void            SetRasterInfo();
    void            SetRepeatCount();
    void            DrawSixel(int c);
    void            GetNumericParameters(Vector<int>& v, int delimiter = Null);
    void            SetColorRegister(int i, const Color& c);
    void            Clear();
    
    VectorMap<int, Color> colortable;
    StringStream    sixelstream;
    int             repeatcount;
    Point           cursor;
    Color           ink;
    Color           paper;
    ImageBuffer     buffer;
    Info            info;
    bool            init;
};

using SixelInfo = SixelRenderer::Info;

// Note that this helper function is meant to be used separately.
// It contains  a  VTInStream  instance of its  own, and  filters
// sequences when required.  And this is  completely  unnecessary
// for Terminal ctrl since it does its own parsing before passing
// the sixel data to client. Use the SixelRenderer class directly
// with the Terminal ctrl.

Image RenderSixelImage(const String& sixeldata, const Size& sizehint, Color paper, bool utf8 = true);
}
#endif
