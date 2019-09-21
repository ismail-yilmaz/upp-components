#include "Terminal.h"

#define LLOG(x)	   // RLOG("Terminal: " << x)
#define LTIMING(x) // RTIMING(x)

namespace Upp {

// The optimizwd rendering classes below (VT[Rect/Text]Renderer)
// are adopted from Upp's own code. See: CtrlLib/LineEdit.cpp.
// Credits should go to Mirek Fidler and other members of the U++ team.

class sVTRectRenderer {
	Draw& w;
	Rect  cr;
	Color color;

public:
	void DrawRect(const Rect& r, Color c)
	{
		if(cr.top == r.top && cr.bottom == r.bottom && cr.right == r.left && c == color) {
			cr.right = r.right;
			return;
		}
		Flush();
		cr = r;
		color = c;
	}
	
	void DrawRect(int x, int y, int cx, int cy, Color c)
	{
		DrawRect(RectC(x, y, cx, cy), c);
	
	}
	void Flush()
	{
		if(!IsNull(cr)) {
			LTIMING("Terminal::VTRectRenderer");
			w.DrawRect(cr, color);
			cr = Null;
		}
	}
	
	sVTRectRenderer(Draw& w) : w(w)
	{
		cr = Null; color = Null;
	}
	
	~sVTRectRenderer()
	{
		Flush();
	}
};

class sVTTextRenderer {
	Draw&       w;
	int         x, y;
	int         xpos;
	Vector<int> dx;
	WString     text;
	Font        font;
	Color       color;

public:
	void DrawChar(int _x, int _y, int chr, Size fsz, Font _font, Color _color)
	{
		if(y == _y && chr == 0 && font == _font && color == _color && dx.GetCount() && _x >= xpos - dx.Top())
			dx.Top() += _x - xpos;
		else {
			Flush();
			x = _x;
			y = _y;
			font = _font;
			color = _color;
		}
		dx.Add(fsz.cx);
		if(chr)
			text.Cat(chr);
		xpos = _x + fsz.cx;
	}
	
	void Flush()
	{
		if(text.IsEmpty())
			return;
		LTIMING("Terminal::VTTextRenderer");
		w.DrawText(x, y, text, font, color, dx);
		y = Null;
		text.Clear();
		dx.Clear();
	}

	sVTTextRenderer(Draw& w) : w(w)
	{
		y = Null;
	}
	
	~sVTTextRenderer()
	{
		Flush();
	}
};

void Terminal::Paint0(Draw& w, bool print)
{
	GuiLock __;
	int  pos = GetSbPos();
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	Font fnt = font;
	ImageParts imageparts;
	
	w.Clip(wsz);

	sVTRectRenderer rr(w);
	sVTTextRenderer tr(w);

	LTIMING("Terminal::Paint");

	if(!nobackground)
		w.DrawRect(wsz, colortable[COLOR_PAPER]);
	for(int i = pos; i < min(pos + psz.cy, page->GetLineCount()); i++) {
		int y = i * fsz.cy - (fsz.cy * pos);
		const VTLine& line = page->GetLine(i);
		int pass = 0, end  = 2;
		if(!line.IsEmpty() && w.IsPainting(0, y, wsz.cx, fsz.cy)) {
			while(pass < end) {
				for(int j = 0; j < psz.cx; j++) {
					const VTCell& cell = j < line.GetCount() ? line[j] : GetAttrs();
					Color ink, paper;
					SetInkAndPaperColor(cell, ink, paper);
					int x = j * fsz.cx;
					int n = i * psz.cx + j;
					bool highlight = IsSelected(n);
					if(pass == 0) {
					#ifdef flagTRUECOLOR
						bool defpcolor = IsNull(cell.paper);
					#else
						bool defpcolor = cell.paper == 0xFFFF;
					#endif
						if(!nobackground || print || highlight || cell.IsInverted() || !defpcolor) {
							int fcx = (j == psz.cx - 1) ? wsz.cx - x : fsz.cx;
							rr.DrawRect(x, y, fcx, fsz.cy, highlight ? colortable[COLOR_PAPER_SELECTED] : paper);
						}
					}
					else
					if(pass == 1) {
						if(cell.IsImage())
							AddImagePart(imageparts, x, y, cell, fsz);
						else {
							fnt.Bold(cell.IsBold());
							fnt.Italic(cell.IsItalic());
							fnt.Strikeout(cell.IsStrikeout());
							fnt.Underline(cell.IsUnderlined());
							if(!cell.IsConcealed()) {
								if(!cell.IsBlinking() || highlight || print || (cell.IsBlinking() && !blinking)) {
									tr.DrawChar(x, y, cell, fsz, fnt, highlight ? colortable[COLOR_INK_SELECTED] : ink);
								}
							}
						}
					}
				}
				if(pass == 0) {
					rr.Flush();
				}
				else
				if(pass == 1) {
					tr.Flush();
				}
				pass++;
			}
		}
	}

	// Paint inline images, if any
	PaintImages(w, imageparts, fsz);
	
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

void Terminal::SetInkAndPaperColor(const VTCell& cell, Color& ink, Color& paper)
{
	ink = GetColorFromIndex(cell, COLOR_INK);
	paper = GetColorFromIndex(cell, COLOR_PAPER);
	if(cell.IsInverted())
		Swap(ink, paper);
	if(modes[DECSCNM])
		Swap(ink, paper);
}

Color Terminal::GetColorFromIndex(const VTCell& cell, int which) const
{
	int index = which == COLOR_INK ? cell.ink : cell.paper;
	
	auto AdjustBrightness = [&which, &cell](Color c) -> Color
	{
		if(!cell.IsFaint() || which != COLOR_INK)
			return c;
		double h, s, v;
		RGBtoHSV(c.GetR() / 255.0, c.GetG() / 255.0, c.GetB() / 255.0, h, s, v);
		return HsvColorf(h, s, 0.70);
	};

#ifndef flagTRUECOLOR
	if(index == 0xFFFF) {
		index = which;
	}
	else
#else
	if(IsNull(index)) {
		index = which;
	}
	else
	if(index > 255 && index & 0x01000000) {	// True color support.
		byte r, g, b;
		r = (index & 0xFF0000) >> 16;
		g = (index & 0xFF00) >> 8;
		b =  index & 0xFF;
		return AdjustBrightness(Color(r, g, b));
	}
	else
#endif
	if(index >= 16) {	// 256 colors support.
		byte r, g, b;
		if(index < 232) {
			r =	(( index - 16) / 36) * 51,
			g =	(((index - 16) % 36) / 6) * 51,
			b =	(( index - 16) % 6)  * 51;
		}
		else // Grayscale
			r = g = b = (index - 232) * 10 + 8;
		return AdjustBrightness(Color(r, g, b));
	}

	if(lightcolors ||
		(intensify && which == COLOR_INK && cell.IsBold()))
			if(index < 8)
				index += 8;

	Color c = colortable[index];	// Adjust only the first 16 colors.
	return AdjustBrightness(adjustcolors ? AdjustIfDark(c) : c);
}

void Terminal::AddImagePart(ImageParts& parts, int x, int y, const VTCell& cell, Size fsz)
{
	dword id = cell.chr;
	Point coords = Point(x, y);
	Point pt(HIWORD(cell.data), LOWORD(cell.data));
	Rect  ir = RectC(pt.x * fsz.cx, pt.y * fsz.cy, fsz.cx, fsz.cy);
	if(!parts.IsEmpty()) {
		ImagePart& part = parts.Top();
		if(id == part.a && part.b.y == coords.y && part.c.right == ir.left) {
			part.c.right = ir.right;
			return;
		}
	}

	parts.Add(MakeTuple(id, coords, ir));
}

void Terminal::PaintImages(Draw& w, ImageParts& parts, const Size& fsz)
{
	if(parts.IsEmpty())
		return;

	LTIMING("Terminal::PaintImages");
	
	Color ink, paper;
	SetInkAndPaperColor(GetAttrs(), ink, paper);

	for(const ImagePart& part : parts) {
		const dword& id = part.a;
		const Point& pt = part.b;
		const Rect&  rr = part.c;
		Rect r = Rect(pt, rr.GetSize());
		if(w.IsPainting(r)) {
			ImageData imd = GetCachedImageData(id, Null, fsz);
			if(!IsNull(imd.image)) {
				imd.fitsize *= fsz;
				imd.paintrect = rr;
				imgdisplay->Paint(w, r, RawPickToValue(pick(imd)), ink, paper, 0);
			}
		}
	}
	
	parts.Clear();
}

void Terminal::RenderImage(const String& data)
{
	if(!sixelgraphics)
		return;

	if(WhenImage)
		WhenImage(data);
	else {
		LTIMING("Terminal::RenderImage");
		dword id = GetHashValue(data);
		ImageData imd = GetCachedImageData(id, data, GetFontSize());
		if(!IsNull(imd.image)) {
			page->AddImage(imd.fitsize, id, modes[DECSDM]);
			RefreshDisplay();
		}
	}
}

// Shared image data cache support.

static StaticMutex sCacheLock;
static LRUCache<Terminal::ImageData> sImageDataCache;
static int sCachedImageMaxSize =  4 * 1024 * 768 * 100;
static int sCachedImageMaxCount =  1000;

struct sVTImageMaker : ImageMaker {
	const dword&	id;
	const String&	data;
	
	String Key() const override
	{
		StringBuffer h;
		RawCat(h, id);
		return h;
	}
	
	Image Make() const override
	{
		return StreamRaster::LoadStringAny(data);
	}
	
	sVTImageMaker(const dword& id, const String& data)
		: id(id)
		, data(data)
	{
	}
};

struct sVTImageDataMaker : LRUCache<Terminal::ImageData>::Maker {
	dword	id;
	String	data;
	Size	fontsize;
	
	String Key() const override
	{
		StringBuffer h;
		RawCat(h, id);
		return h;
	}
	
	int Make(Terminal::ImageData& imagedata) const override
	{
		sVTImageMaker im(id, data);
		imagedata.image = MakeImagePaintOnly(im);
		if(!IsNull(imagedata.image))
			imagedata.fitsize = AsCellSize(imagedata.image.GetSize());
		return imagedata.image.GetLength() * 4;
	}
	
	Size AsCellSize(Sizef isz) const
	{
		isz = isz / Sizef(fontsize);
		return Size(fround(isz.cx), fround(isz.cy));
	}
};

Terminal::ImageData Terminal::GetCachedImageData(dword id, const String& data, const Size& fsz)
{
	Mutex::Lock __(sCacheLock);
	
	LTIMING("Terminal::GetCachedImageData");

	sVTImageDataMaker im;
	im.id = id;
	im.data = data;
	im.fontsize = fsz;
	sImageDataCache.Shrink(sCachedImageMaxSize, sCachedImageMaxCount);
	return sImageDataCache.Get(im);
}

void Terminal::ClearImageCache()
{
	Mutex::Lock __(sCacheLock);
	sImageDataCache.Clear();
}

void Terminal::SetImageCacheMaxSize(int maxsize, int maxcount)
{
	Mutex::Lock __(sCacheLock);
	sCachedImageMaxSize  = max(1, maxsize);
	sCachedImageMaxCount = max(1, maxcount);
}
}