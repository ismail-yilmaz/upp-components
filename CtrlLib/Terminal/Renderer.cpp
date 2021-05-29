#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

class Renderer {
public:
	struct Attrs {
        const VTCell *cell = nullptr;
        int     x     = 0;
        int     y     = 0;
        Color   ink   = Null;
        Color   paper = Null;
        bool    is_link:1;
        bool    highlighted:1;
	};
	
private:
	Draw&		w;
	Rect		rect;
	WString		text;
	Point		textpos;
	Point		begin_link, end_link;
	Point		begin_line, end_line;
	Vector<int> dx;
	int			dxcount;
	Font		font;
	Size		fsz;
	word		sgr;
	Color		ink;
	Color		paper;
	Color		background;
	bool		transparent;
	bool		blinkingtext;

    using       ImagePart  = Tuple<dword, Point, Rect>;
    using       ImageParts = Vector<ImagePart>;
	ImageParts&	imageparts;
	
public:
	Renderer(Draw& w, ImageParts& im, Font f, Size sz, Color bkg, bool bt, bool tr)
		: w(w)
		, imageparts(im)
		, font(f)
		, fsz(sz)
		, ink(Null)
		, paper(Null)
		, background(bkg)
		, begin_link(Null)
		, end_link(Null)
		, begin_line(Null)
		, end_line(Null)
		, textpos(0, 0)
		, dxcount(0)
		, transparent(tr)
		, blinkingtext(bt)
		, sgr(VTCell::SGR_NORMAL) {}

	void DrawRect(const Attrs& a, int cx, int cy)
	{
		Rect r = RectC(a.x, a.y, cx, cy);
		if(rect.top == r.top && rect.bottom == r.bottom && rect.right == r.left && paper == a.paper) {
			rect.right = r.right;
			return;
		}
		FlushRect();
		paper = a.paper;
		rect  = r;
	}
	
	void FlushRect()
	{
		if(IsNull(rect) || (!transparent && paper == background))
			return;

		LTIMING("Renderer::FlushRect");

		w.DrawRect(rect, paper);
		rect = Null;
	}

	void DrawCell(const Attrs& a, Size csz, bool show)
	{
		const VTCell& cell = *a.cell;
		
		if(cell.IsImage())
			CollectImageParts(a, cell, csz);
		
		dxcount = dx.GetCount();
		bool overline = cell.IsOverlined() && show;
	
		if(textpos.y != a.y || textpos.x >= a.x || sgr != cell.sgr || ink != a.ink || !dxcount) {
			FlushCell();
			textpos = Point(a.x, a.y);
			ink = a.ink;
			sgr = cell.sgr;
			font.Bold(cell.IsBold());
			font.Italic(cell.IsItalic());
			font.Strikeout(cell.IsStrikeout());
			font.Underline(cell.IsUnderlined());

			if(a.is_link)
				begin_link = end_link = Point(a.x, a.y + fsz.cy - 1);

			if(overline)
				begin_line = end_line = Point(a.x, a.y);
		}

		if(cell.chr < 0x20 || cell.IsImage() || cell.IsConcealed() || (!show && (cell.IsBlinking() && blinkingtext))) {
			if(dxcount)
				dx.Top() += csz.cx;
		}
		else {
			text.Cat((int) cell.chr);
			dx.Add(csz.cx);
		}

		if(a.is_link)
			end_link.x += fsz.cx;

		if(overline)
			end_line.x += fsz.cx;
	}

	void FlushCell()
	{
		if(text.IsEmpty()) return;

		LTIMING("Renderer::FlushCell");

		w.DrawText(textpos.x, textpos.y, text, font, ink, dx);

		if(begin_link.x < end_link.x) // Hyperlink underline
			w.DrawLine(begin_link, end_link, PEN_DOT, ink);

		if(begin_line.x < end_line.x) // Text overline
			w.DrawLine(begin_line, end_line, PEN_SOLID, ink);

		dx.Clear();
		begin_link = end_link = Null;
		begin_line = end_line = Null;
		dxcount = 0;
		textpos.y = Null;
		text.Clear();
	}
	
	void CollectImageParts(const Attrs& a, const VTCell& cell, const Size& sz)
	{
		dword id = cell.chr;
		Point coords = Point(a.x, a.y);
		Rect  ir = RectC(cell.object.col * sz.cx, cell.object.row * sz.cy, sz.cx, sz.cy);
		if(!imageparts.IsEmpty()) {
			ImagePart& part = imageparts.Top();
			if(id == part.a && part.b.y == coords.y && part.c.right == ir.left) {
				part.c.right = ir.right;
				return;
			}
		}
		imageparts.Add(MakeTuple(id, coords, ir));
	}
};

void TerminalCtrl::Paint0(Draw& w, bool print)
{
	GuiLock __;

	int  pos = GetSbPos();
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size csz = GetCellSize();
	ImageParts imageparts;

	w.Clip(wsz);

	Renderer rr(
		w,
		imageparts,
		font,
		GetFontSize(),
		colortable[COLOR_PAPER],
		blinkingtext,
		nobackground
	);

	LTIMING("TerminalCtrl::Paint");

	Buffer<Renderer::Attrs> lineattrs(psz.cx);

	if(!nobackground)
		w.DrawRect(wsz, colortable[COLOR_PAPER]);
	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); ++i) {
		int y = i * csz.cy - (csz.cy * pos);
		const VTLine& line = page->FetchLine(i);
		if(!line.IsVoid() && w.IsPainting(0, y, wsz.cx, csz.cy)) {
			Renderer::Attrs *la = lineattrs;
			for(int j = 0; j < psz.cx; ++j, ++la) {
				la->cell = &line.Get(j, GetAttrs());
				la->x = j * csz.cx;
				la->y = y;
				la->is_link = hyperlinks && la->cell->IsHyperlink();
				if((la->highlighted = IsSelected(Point(j, i)))) {
					la->ink   = colortable[COLOR_INK_SELECTED];
					la->paper = colortable[COLOR_PAPER_SELECTED];
				}
				else {
					SetInkAndPaperColor(*la->cell, la->ink, la->paper);
				}
				if(!nobackground
				|| !IsNull(la->cell->paper)
				|| la->highlighted
				|| la->cell->IsInverted()
				|| (la->is_link && la->cell->data == activelink)
				|| print) {
					rr.DrawRect(*la, (j == psz.cx - 1) ? wsz.cx - la->x : csz.cx, csz.cy);
				}
			}
			rr.FlushRect();
			la = lineattrs;
			for(int j = 0; j < psz.cx; ++j, ++la) {
				la->x += padding.cx;
				la->y += padding.cy;
				rr.DrawCell(*la, csz, la->highlighted || !blinking || print);
			}
			rr.FlushCell();
		}
	}

	// Paint inline images, if any
	if(imageparts.GetCount())
		PaintImages(w, imageparts, csz);

	// Paint a steady (non-blinking) caret, if enabled.
	if(modes[DECTCEM] && HasFocus() && (print || !caret.IsBlinking()))
		w.DrawRect(GetCaretRect(), InvertColor);

	// Hint new size.
	if(sizehint && hinting) {
		auto hint = GetSizeHint(GetView(), psz);
		DrawFrame(w ,hint.b.Inflated(8), LtGray);
		w.DrawRect(hint.b.Inflated(7), SColorText);
		w.DrawText(hint.b.left, hint.b.top, hint.a, StdFont(), SColorPaper);
	}
	w.End();
}

void TerminalCtrl::PaintImages(Draw& w, ImageParts& parts, const Size& csz)
{
	LTIMING("TerminalCtrl::PaintImages");

	for(const ImagePart& part : parts) {
		const dword& id = part.a;
		const Point& pt = part.b;
		const Rect&  rr = part.c;
		Rect r(pt, rr.GetSize());
		if(w.IsPainting(r)) {
			InlineImage im = GetCachedImageData(id, Null, csz);
			if(!IsNull(im.image)) {
				im.paintrect = rr;	// Keep it updated.
				im.fontsize  = csz;	// Keep it updated.
				imgdisplay->Paint(w, r, im, colortable[COLOR_INK], colortable[COLOR_PAPER], 0);
			}
		}
	}

	if(parts.GetCount())
		parts.Clear();
}

void TerminalCtrl::RenderImage(const ImageString& imgs, bool scroll)
{
	bool encoded = imgs.encoded; // Sixel images are not base64 encoded.

	if(WhenImage) {
		WhenImage(encoded ? Base64Decode(imgs.data) : imgs.data);
		return;
	}

	LTIMING("TerminalCtrl::RenderImage");

	Size fsz = GetCellSize();
	dword id = FoldHash(CombineHash(imgs, fsz));
	const InlineImage& imd = GetCachedImageData(id, imgs, fsz);
	if(!IsNull(imd.image)) {
		page->AddImage(imd.cellsize, id, scroll, encoded);
		RefreshDisplay();
	}
}

// Shared image data cache support.

static StaticMutex sImageCacheLock;
static LRUCache<TerminalCtrl::InlineImage> sInlineImagesCache;
static int sCachedImageMaxSize =  1024 * 1024 * 4 * 128;
static int sCachedImageMaxCount =  256000;

String TerminalCtrl::InlineImageMaker::Key() const
{
	StringBuffer h;
	RawCat(h, id);
	return String(h); // Make MSVC happy...
}

int TerminalCtrl::InlineImageMaker::Make(InlineImage& imagedata) const
{
	LTIMING("TerminalCtrl::ImageDataMaker::Make");

	auto ToCellSize = [=](Sizef sz) -> Size
	{
		sz = sz / Sizef(fontsize);
		return Size(fround(sz.cx), fround(sz.cy));
	};

	auto AdjustSize = [=](Size sr, Size sz) -> Size
	{
		if(imgs.keepratio) {
			if(sr.cx == 0 && sr.cy > 0)
				sr.cx = sr.cy * sz.cx / sz.cy;
			else
			if(sr.cy == 0 && sr.cx > 0)
				sr.cy = sr.cx * sz.cy / sz.cx;
		}
		else {
			if(sr.cx <= 0)
				sr.cx = sz.cx;
			if(sr.cy <= 0)
				sr.cy = sz.cy;
		}
		return sr != sz ? sr : Null;
	};

	Image img;
	if(!imgs.encoded) {
		MemReadStream ms(imgs.data, imgs.data.GetLength());
		img = SixelRenderer(ms).Get();
	}
	else {
		img = StreamRaster::LoadStringAny(Base64Decode(imgs.data));
	}

	if(IsNull(img))
		return 0;
	if(IsNull(imgs.size))
		imagedata.image = img;
	else {
		Size sz = AdjustSize(imgs.size, img.GetSize());
		imagedata.image = IsNull(sz) ? img : Rescale(img, sz);
	}
	imagedata.fontsize = fontsize;
	imagedata.cellsize = ToCellSize(imagedata.image.GetSize());
	return imagedata.image.GetLength() * 4;
}

const TerminalCtrl::InlineImage& TerminalCtrl::GetCachedImageData(dword id, const ImageString& imgs, const Size& csz)
{
	Mutex::Lock __(sImageCacheLock);

	LTIMING("TerminalCtrl::GetCachedImageData");

	InlineImageMaker im(id, imgs, csz);
	sInlineImagesCache.Shrink(sCachedImageMaxSize, sCachedImageMaxCount);
	return sInlineImagesCache.Get(im);
}

void TerminalCtrl::ClearImageCache()
{
	Mutex::Lock __(sImageCacheLock);
	sInlineImagesCache.Clear();
}

void TerminalCtrl::SetImageCacheMaxSize(int maxsize, int maxcount)
{
	Mutex::Lock __(sImageCacheLock);
	sCachedImageMaxSize  = max(1, maxsize);
	sCachedImageMaxCount = max(1, maxcount);
}

void TerminalCtrl::RenderHyperlink(const String& uri)
{
	GetCachedHyperlink(FoldHash(GetHashValue(uri)), uri);
}

// Shared hyperlink cache support.

static StaticMutex sLinkCacheLock;
static LRUCache<String> sLinkCache;
static int sCachedLinkMaxSize = 2084 * 100000;
static int sCachedLinkMaxCount = 100000;

String TerminalCtrl::HyperlinkMaker::Key() const
{
	StringBuffer h;
	RawCat(h, id);
	return String(h); // Make MSVC happy...
}

int TerminalCtrl::HyperlinkMaker::Make(String& link) const
{
	LTIMING("TerminalCtrl::HyperlinkMaker::Make");

	link = url;
	return link.GetLength();
}

String TerminalCtrl::GetCachedHyperlink(dword id, const String& data)
{
	Mutex::Lock __(sLinkCacheLock);

	LTIMING("TerminalCtrl::GetCachedHyperlink");

	HyperlinkMaker hm(id, data);
	sLinkCache.Shrink(sCachedLinkMaxSize, sCachedLinkMaxCount);
	return sLinkCache.Get(hm);
}

void TerminalCtrl::ClearHyperlinkCache()
{
	Mutex::Lock __(sLinkCacheLock);
	sLinkCache.Clear();
}

void TerminalCtrl::SetHyperlinkCacheMaxSize(int maxcount)
{
	Mutex::Lock __(sLinkCacheLock);
	sCachedLinkMaxSize  = max(2084, maxcount * 2084);
	sCachedLinkMaxCount = max(1, maxcount);
}

// Image display support.

class NormalImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const auto& im = q.To<TerminalCtrl::InlineImage>();
		if(!IsNull(im.image))
			w.DrawImage(r.left, r.top, im.image, im.paintrect);
	}
	virtual Size GetStdSize(const Value& q) const
	{
		const auto& im = q.To<TerminalCtrl::InlineImage>();
		return im.image.GetSize();
	}
};

const Display& NormalImageCellDisplay() { return Single<NormalImageCellDisplayCls>(); }

class ScaledImageCellDisplayCls : public Display {
public:
	virtual void Paint(Draw& w, const Rect& r, const Value& q, Color ink, Color paper, dword style) const
	{
		const auto& im = q.To<TerminalCtrl::InlineImage>();
		if(!IsNull(im.image)) {
			Size csz = im.cellsize;
			Size fsz = im.fontsize;
			w.DrawImage(r, CachedRescale(im.image, csz * fsz), im.paintrect);
		}
	}
	virtual Size GetStdSize(const Value& q) const
	{
		const auto& im = q.To<TerminalCtrl::InlineImage>();
		return im.image.GetSize();
	}
};

const Display& ScaledImageCellDisplay() { return Single<ScaledImageCellDisplayCls>(); }
}