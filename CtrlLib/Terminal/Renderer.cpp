#include "Terminal.h"

#define LLOG(x)     // RLOG("TerminalCtrl (#" << this << "]: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

struct CellPaintData {
	Point pos   = {0, 0};
	Size  size  = {0, 0};
	Color ink   = Null;
	Color paper = Null;
	bool show:1;
	bool highlight:1;
};

class sRectRenderer {
	Draw& w;
	Rect  cr;
	Color color;
	Color background;
	bool  transparent:1;

public:
	void DrawRect(const VTCell& cell, const CellPaintData& data);
	void Flush();

	sRectRenderer(Draw& w, Color bkg, bool tr)
		: w(w), background(bkg), transparent(tr), cr(Null), color(Null) {}
	~sRectRenderer()              { Flush(); }
};

void sRectRenderer::Flush()
{
	if(IsNull(cr) || (!transparent && color == background))
		return;
	LTIMING("sRectRenderer::Flush");
	w.DrawRect(cr, color);
	cr = Null;
}

void sRectRenderer::DrawRect(const VTCell& cell, const CellPaintData& data)
{
	Rect r(data.pos, data.size);
	if(cr.top == r.top && cr.bottom == r.bottom && cr.right == r.left && data.paper == color) {
		cr.right = r.right;
		return;
	}
	Flush();
	cr = r;
	color = data.paper;
}

class sTextRenderer {
	Draw&       w;
	Font        font;
	bool        canlink;
	int         y, icount = 0, lcount = 0;
	struct Chrs : Moveable<Chrs> {
		Vector<int> x;
		Vector<int> width;
		WString     text;
	};
	VectorMap< Tuple<dword, Color>, Chrs > cache;

public:
	int  GetImageCount() const                     { return icount; }
	int  GetLinkCount()  const                     { return lcount; }
	void DrawChar(const VTCell& cell, const CellPaintData& data);
	void Flush();

	sTextRenderer(Draw& w, Font f, bool l) : w(w), font(f), canlink(l) { y = Null; }
	~sTextRenderer()                               { Flush();  }
};

void sTextRenderer::Flush()
{
	if(cache.GetCount() == 0)
		return;
	
	LTIMING("sTextRenderer::Flush");

	for(int i = 0; i < cache.GetCount(); i++) {
		Chrs& c = cache[i];
		if(c.x.GetCount()) {
			const Tuple<dword, Color>& fc = cache.GetKey(i);
			int x = c.x[0], cx = c.x.Top();
			for(int i = 0; i < c.x.GetCount() - 1; i++)
				c.x[i] = c.x[i + 1] - c.x[i];
			c.x.Top() = c.width.Top();
			font.Bold(fc.a & VTCell::SGR_BOLD)
			    .Italic(fc.a & VTCell::SGR_ITALIC)
				.Strikeout(fc.a & VTCell::SGR_STRIKEOUT)
				.Underline(fc.a & VTCell::SGR_UNDERLINE);
			if(fc.a & VTCell::SGR_OVERLINE) {
				int h = font.GetDescent() - 2;
				w.DrawLine(x, y + h, cx + font.GetMonoWidth(), y + h, PEN_SOLID, fc.b);
			}
			if(canlink && fc.a & VTCell::SGR_HYPERLINK) {
				int h = font.GetAscent() + 2;
				w.DrawLine(x, y + h, cx + font.GetMonoWidth(), y + h, PEN_DOT, fc.b);
				font.NoUnderline();
			}
			w.DrawText(x, y, c.text, font, fc.b, c.x);
		}
	}
	cache.Clear();
}

void sTextRenderer::DrawChar(const VTCell& cell, const CellPaintData& data)
{
	Point p = data.pos;
	if(y != p.y) {
		Flush();
		y = p.y;
	}

	Chrs *c = &cache.GetAdd(MakeTuple(cell.sgr, data.ink));
	
	if(c->x.GetCount() && c->x.Top() > p.x || (cell.IsUnderlined() || cell.IsHyperlink()) && cache.GetCount() > 1) {
		Flush();
		c = &cache.GetAdd(MakeTuple(cell.sgr, data.ink));
	}

	icount += (int) cell.sgr & VTCell::SGR_IMAGE;
	lcount += (int) cell.sgr & VTCell::SGR_HYPERLINK;

	bool hide = false;
	hide |= cell.chr < 0x20;
	hide |= cell.IsImage();
	hide |= cell.IsConcealed();
	hide |= (!data.show && !data.highlight && cell.IsBlinking());
	
	if(hide) {
		if(c->width.GetCount())
			c->width.Top() += data.size.cx;
	}
	else {
		c->text.Cat((int) cell.chr);
		c->x.Add(p.x);
		c->width.Add(data.size.cx);
	}
}

void TerminalCtrl::Paint0(Draw& w, bool print)
{
	GuiLock __;

	int  pos = GetSbPos();
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size csz = GetCellSize();

	w.Clip(wsz);

	LTIMING("TerminalCtrl::Paint");

	Color bkg = colortable[COLOR_PAPER];

	CellPaintData cpd;
	cpd.size = csz;
	Buffer<CellPaintData> linepaintdata(psz.cx, cpd);

	if(!nobackground)
		w.DrawRect(wsz, bkg);
	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); ++i) {
		int y = i * csz.cy - (csz.cy * pos);
		const VTLine& line = page->FetchLine(i);
		if(!line.IsVoid() && w.IsPainting(0, y, wsz.cx, csz.cy)) {
			auto PaintLine = [this, &w, &linepaintdata, &csz, &wsz, &psz, &pos, &bkg, &print](const VTLine& line, int i) {
				LTIMING("TerminalCtrl::PaintLine");
				int y = i * csz.cy - (csz.cy * pos);
				{
					// Render the background rectangles.
					sRectRenderer rr(w, bkg, nobackground);
					for(int j = 0, x = 0; j < psz.cx; j++, x += csz.cx) {
						const VTCell& cell = line.Get(j, GetAttrs());
						CellPaintData& data = linepaintdata[j];
						data.highlight = IsSelected(Point(j,i));
						data.show |= cell.IsHyperlink() && cell.data == activelink;
						data.show |= cell.IsInverted();
						data.show |= print;
						if(data.highlight) {
							data.ink = colortable[COLOR_INK_SELECTED];
							data.paper = colortable[COLOR_PAPER_SELECTED];
						}
						else {
							SetInkAndPaperColor(cell, data.ink, data.paper);
						}
						data.pos = {x, y};
						if(j == psz.cx - 1)
							data.size.cx = wsz.cx - x;
						data.show |= data.highlight;
						rr.DrawRect(cell, data);
					}
				}
				int icount = 0, lcount = 0;
				{
					// Render the text by combining non-contiguous chunks of chars.
					sTextRenderer tr(w, GetFont(), hyperlinks);
					for(int j = 0, x = 0; j < psz.cx; j++, x += csz.cx) {
						CellPaintData& data = linepaintdata[j];
						data.pos = { x + padding.cx, y + padding.cy };
						data.show = !blinking;
						tr.DrawChar(line.Get(j, GetAttrs()), data);
					}
					icount = tr.GetImageCount();
					lcount = tr.GetLinkCount();
				}
				{
					if(icount) {
						// Render inline images, if any.
						ImageParts ip;
						for(int j = 0, x = 0; j < psz.cx; j++, x += csz.cx) {
							if(line[j].IsImage())
								CollectImage(ip, x, y, line.Get(j, GetAttrs()), csz);
						}
						PaintImages(w, ip, csz);
					}
				}
			};
			if(highlight) {
					LTIMING("TerminalCtrl::WhenHighlight");
					VectorMap<int, VTLine> hl;
					page->FetchLine(i, hl);
					WhenHighlight(hl);
					for(const auto& h : ~hl)
						PaintLine(h.value, h.key);
			}
			else
				PaintLine(line, i);
		}
	}

	// Paint a steady (non-blinking) caret, if enabled.
	if(modes[DECTCEM] && HasFocus() && (print || !caret.IsBlinking()))
		w.DrawRect(caretrect, InvertColor);

	// Hint new size.
	if(sizehint && hinting && IsVisible())
		PaintSizeHint(w);

	w.End();
}

void TerminalCtrl::PaintSizeHint(Draw& w)
{
	Tuple<String, Size> hint = GetSizeHint();
	Rect rr = GetViewRect().CenterRect(hint.b).Inflated(8);
	Rect rx = Rect(rr.GetSize()).CenterRect(hint.b);
	ImagePainter ip(rr.GetSize());
	ip.Begin();
	ip.Clear(RGBAZero());
	ip.RoundedRectangle(0, 0, rr.Width(), rr.Height(), 10.0)
	  .Stroke(1, LtGray())
	  .Fill(SColorText());
	ip.DrawText(rx.left, rx.top, hint.a, StdFont(), SColorPaper);
	ip.End();
	w.DrawImage(rr.left, rr.top, ip.GetResult());
}

void TerminalCtrl::PaintImages(Draw& w, ImageParts& parts, const Size& csz)
{
	LTIMING("TerminalCtrl::PaintImages");

	for(const ImagePart& part : parts) {
		const dword& id = part.a;
		const Point& pt = part.b;
		const Rect&  rr = part.c;
		Rect r(pt, rr.GetSize());
		InlineImage im = GetCachedImageData(id, Null, csz);
		if(!IsNull(im.image)) {
			im.paintrect = rr;	// Keep it updated.
			im.fontsize  = csz;	// Keep it updated.
			imgdisplay->Paint(w, r, im, colortable[COLOR_INK], colortable[COLOR_PAPER], 0);
		}
	}
}

void TerminalCtrl::CollectImage(ImageParts& ip, int x, int y, const VTCell& cell, const Size& sz)
{
	LTIMING("TerminalCtrl::CollectImage");
	
	dword id = cell.chr;
	Point coords = Point(x, y);
	Rect  ir = RectC(cell.object.col * sz.cx, cell.object.row * sz.cy, sz.cx, sz.cy);
	if(!ip.IsEmpty()) {
		ImagePart& part = ip.Top();
		if(id == part.a && part.b.y == coords.y && part.c.right == ir.left) {
			part.c.right = ir.right;
			return;
		}
	}
	ip.Add(MakeTuple(id, coords, ir));
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
		img = (Image) SixelStream(imgs.data).Background(!imgs.transparent);
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