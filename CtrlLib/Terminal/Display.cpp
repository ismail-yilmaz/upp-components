#include "Terminal.h"

#define LLOG(x)	// RLOG("Terminal: " << x)

namespace Upp {

class NormalImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const Image& img = q[0];	// Image.
		const Rect&  irr = q[3];	// The section of the image to paint.
	
		if(!IsNull(img)) {
			w.DrawImage(r.left, r.top, img, irr);
		}
	}
	virtual Size GetStdSize(const Value& q) const
	{
		return Image(q[0]).GetSize();
	}
	
};

const Display& NormalImageCellDisplay() { return Single<NormalImageCellDisplayCls>(); }

class ScaledImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const Image& img = q[0];	// Image.
		const Size&  isz = q[1];	// Image size (in cells).
		const Size&  csz = q[2];	// Current cell size (in pixels).
		const Rect&  irr = q[3];	// The section of the image to paint.
		
		if(!IsNull(img)) {
			Size sz = GetFitSize(img.GetSize(), isz * csz);
			w.DrawImage(r, CachedRescale(img, sz), irr);
		}
	}
	virtual Size GetStdSize(const Value& q) const
	{
		return Image(q[0]).GetSize();
	}
};

const Display& ScaledImageCellDisplay() { return Single<ScaledImageCellDisplayCls>(); }
}