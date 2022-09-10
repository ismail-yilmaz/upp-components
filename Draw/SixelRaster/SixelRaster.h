#ifndef _SixelRaster_h_
#define _SixelRaster_h_

#include <Core/Core.h>
#include <Draw/Draw.h>

namespace Upp {

class SixelRaster : public StreamRaster {
    class Data;
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
