#ifndef _SixelRenderer_h_
#define _SixelRenderer_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

// TODO: Needs optimixation.

namespace Upp{

class SixelRenderer : NoCopy {
public:
    SixelRenderer(Stream& sdata);

    Image           Get();
    operator        Image()                         { return Get(); }
    Size            GetSize() const                 { return size;  }
    int             GettRatio() const               { return aspectratio; }
    
private:
    void            Clear();
    inline void     Return();
    inline void     LineFeed();
    void            SetPalette();
    void            GetRasterInfo();
    void            GetRepeatCount();
    void            ParseEsc(int c);
    void            SetOptions();
    bool            InitBuffer(ImageBuffer& ib);
    void            DrawSixel(ImageBuffer& ib, int c);
    void            GetNumericParams(Vector<int>& v, int delim = Null);

private:
    Stream&         sstream;
    Vector<Color>   palette;
    int             datapos;
    int             repeatcount;
    int             aspectratio;
    int             pass;
    Rect            rect;
    Size            size;
    Point           cursor;
    bool            nohole;
    bool            init;
    Color           ink;
};

class SixelRaster : public StreamRaster {
public:
    SixelRaster()                       {}

    virtual bool    Create();
    virtual Size    GetSize();
    virtual Info    GetInfo();
    virtual Line    GetLine(int line);
    
private:
    One<ImageRaster> imgraster;
};

INITIALIZE(SixelRaster);
}
#endif
