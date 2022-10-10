#ifndef _SixelRenderer_h_
#define _SixelRenderer_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

namespace Upp{

class SixelStream : MemReadStream {
public:
    SixelStream(const void *data, int64 size);
    SixelStream(const String& data);
    
    SixelStream&    Background(bool b = true)       { background = b; return *this;  }
    operator        Image();
    
private:
    void            Clear();
    inline void     Return();
    inline void     LineFeed();
    void            SetPalette();
    void            GetRasterInfo();
    void            GetRepeatCount();
    int             ReadParams();
    void            CalcYOffests();
    void            AdjustBufferSize();
    void            PaintSixel(int c);

private:
    ImageBuffer     buffer;
    Vector<RGBA>    palette;
    RGBA            ink;
    RGBA            paper;
    int             repeat;
    int             params[8];
    int             coords[6];
    Size            size;
    Point           cursor;
    bool            background;
};
}
#endif
