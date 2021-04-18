#ifndef _SixelRenderer_h_
#define _SixelRenderer_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

namespace Upp{

class SixelRenderer : NoCopy {
public:
    SixelRenderer(Stream& sdata);

    Image           Get();
    operator        Image()                         { return Get(); }
    Size            GetSize() const                 { return size;  }
    int             GetRatio() const                { return aspectratio; }
    
private:
    void            Clear();
    inline void     Return();
    inline void     LineFeed();
    void            SetPalette();
    void            GetRasterInfo();
    void            GetRepeatCount();
    void            SetOptions();
    void            Validate();
    void            InitBuffer(ImageBuffer& ib);
    void            PaintSixel(ImageBuffer& ib, int c);
    void            GetNumericParams(Vector<int>& v, int delim = Null);

private:
    Stream&         sstream;
    Vector<RGBA>    palette;
    RGBA            ink;
    int64           datapos;
    int             repeat;
    int             aspectratio;
    Rect            rect;
    Size            size;
    Point           cursor;
    bool            nohole;
};

class SixelRaster : public StreamRaster {
public:
    SixelRaster()                       {}

    virtual bool    Create();
    virtual Size    GetSize();
    virtual Info    GetInfo();
    virtual Line    GetLine(int line);
    
private:
    Image img;
};

INITIALIZE(SixelRaster);
}
#endif
