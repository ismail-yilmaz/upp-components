#include "Terminal.h"

#define LLOG(x)	// RLOG("Terminal: " << x)

namespace Upp {

class NormalImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const auto& idata = q.To<Terminal::ImageData>();

		if(!IsNull(idata.image))
			w.DrawImage(r.left, r.top, idata.image, idata.paintrect);
	}
	virtual Size GetStdSize(const Value& q) const
	{
		const auto& idata = q.To<Terminal::ImageData>();
		return idata.image.GetSize();
	}
};

const Display& NormalImageCellDisplay() { return Single<NormalImageCellDisplayCls>(); }

class ScaledImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const auto& idata = q.To<Terminal::ImageData>();
		
		if(!IsNull(idata.image))
			w.DrawImage(r, CachedRescale(idata.image, idata.fitsize), idata.paintrect);
	}
	virtual Size GetStdSize(const Value& q) const
	{
		const auto& idata = q.To<Terminal::ImageData>();
		return idata.image.GetSize();
	}
};

const Display& ScaledImageCellDisplay() { return Single<ScaledImageCellDisplayCls>(); }
}