#include "Terminal.h"

#define LLOG(x)	// RLOG("Terminal: " << x)

namespace Upp {

class LeftAlignedImageDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		if(!IsNull(paper))
			w.DrawRect(r, paper);
		Image m = q;
		if(!IsNull(m)) {
			Size sz = GetFitSize(m.GetSize(), r.Size());
			w.DrawImage(r.left, r.top, Rescale(m, sz));
		}
	}
	virtual Size GetStdSize(const Value& q) const
	{
		return Image(q).GetSize();
	}
};

const Display& LeftAlignedImageDisplay() { return Single<LeftAlignedImageDisplayCls>(); }

class RightAlignedImageDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		if(!IsNull(paper))
			w.DrawRect(r, paper);
		Image m = q;
		if(!IsNull(m)) {
			Size sz = GetFitSize(m.GetSize(), r.Size());
			w.DrawImage(r.right - sz.cx, r.top, Rescale(m, sz));
		}
	}
	virtual Size GetStdSize(const Value& q) const
	{
		return Image(q).GetSize();
	}
};

const Display& RightAlignedImageDisplay() { return Single<RightAlignedImageDisplayCls>(); }
}