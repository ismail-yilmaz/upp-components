#include "Terminal.h"

#define LLOG(x)	    // RLOG("Terminal: " << x)
#define LTIMING(x)  // RTIMING(x)

namespace Upp {

// The optimizwd rendering classes below (VT[Rect/Text]Renderer)
// are adopted from Upp's own code. See: CtrlLib/LineEdit.cpp.
// Credits should go to Mirek Fidler and other members of the U++ team.

class sVTRectRenderer {
	Draw& w;
	Rect  cr;
	Color color;

public:
	void DrawRect(const Rect& r, Color color);
	void DrawRect(int x, int y, int cx, int cy, Color color) { DrawRect(RectC(x, y, cx, cy), color); }
	void Flush();

	sVTRectRenderer(Draw& w) : w(w) { cr = Null; color = Null; }
	~sVTRectRenderer()              { Flush(); }
};

void sVTRectRenderer::Flush()
{
	if(!IsNull(cr)) {
		LTIMING("Terminal::VTRectRenderer");
		w.DrawRect(cr, color);
		cr = Null;
	}
}

void sVTRectRenderer::DrawRect(const Rect& r, Color c)
{
	if(cr.top == r.top && cr.bottom == r.bottom && cr.right == r.left && c == color) {
		cr.right = r.right;
		return;
	}
	Flush();
	cr = r;
	color = c;
}

class sVTTextRenderer {
	Draw&       w;
	int         x, y;
	int         xpos;
	Vector<int> dx;
	WString     text;
	Font        font;
	Color       color;

public:
	void DrawChar(int x, int y, int chr, Size fsz, Font afont, Color acolor);
	void Flush();

	sVTTextRenderer(Draw& w) : w(w) { y = Null; }
	~sVTTextRenderer()              { Flush(); }
};

void sVTTextRenderer::Flush()
{
	if(text.GetCount() == 0)
		return;
	LTIMING("Terminal::VTTextRenderer");
	w.DrawText(x, y, text, font, color, dx);
	y = Null;
	text.Clear();
	dx.Clear();
}

void sVTTextRenderer::DrawChar(int _x, int _y, int chr, Size fsz, Font _font, Color _color)
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

static void sAddCanvasRect(Vector<Tuple<dword, Rect>>& ca, const VTLine& line, Size fsz, Rect r)
{
	int id  = line.GetSpecialId();
	int row = line.GetRowNumber();
	
	if(ca.IsEmpty()) {
		if(row != 0)
			r.top -= (row * fsz.cy);
		ca.Add(MakeTuple(id, r));
	}
	else {
		Rect& rr = ca.Top().b;
		if(rr.bottom == r.top)
			rr.bottom = r.bottom;
		else {
			if(row != 0)
				r.top -= (row * fsz.cy);
			ca.Add(MakeTuple(id, r));
		}
	}
}

void Terminal::Paint0(Draw& w, bool print)
{
	GuiLock __;
	int  pos = GetSbPos();
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	Font fnt = font;
	Vector<Tuple<dword, Rect>> canvases;

	w.Clip(wsz);
	sVTRectRenderer rr(w);
	sVTTextRenderer tr(w);

	LTIMING("Terminal::Paint");

	if(!nobackground)
		w.DrawRect(wsz, colortable[COLOR_PAPER]);
	int b = pos;
	int e = pos + psz.cy;
	if(!imagecache.IsEmpty()) {
		b = 0;
		e = page->GetLineCount();
	}
	for(int i = b; i < e; i++) {
		int y = i * fsz.cy - (fsz.cy * pos);
		const VTLine& line = page->GetLine(i);
		int pass = 0, end  = 2;
		if(line.IsSpecial()) {
			sAddCanvasRect(canvases, line, fsz, RectC(0, i * fsz.cy - (pos * fsz.cy), wsz.cx, fsz.cy));
			end = 1;
		}
		if(!line.IsEmpty() && w.IsPainting(0, y, wsz.cx, fsz.cy)) {
			while(i >= pos && i < pos + psz.cy && pass < end) {
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

	// Paint images.
	if(sixelgraphics && !canvases.IsEmpty() && !imagecache.IsEmpty())
		PaintImage(w, canvases);

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

void Terminal::PaintImage(Draw& w, Vector<Tuple<dword, Rect>>& canvases)
{
	for(const Tuple<dword, Rect>& canvas : canvases) {
		Image *img = imagecache.FindPtr(canvas.a);
		if(img) {
			Rect r  = canvas.b;
			Size isz = img->GetSize();
			if(w.IsPainting(r)) {
				const VTCell& cell = GetAttrs();
				Color ink, paper;
				SetInkAndPaperColor(cell, ink, paper);
				imgdisplay->Paint(w, r.Deflated(font.GetCy() / 2), *img, ink, paper, 0);
			}
		}
	}
}

void Terminal::UpdateImageCache(const Index<dword>& imageids)
{
	if(IsAlternatePage() || imagecache.IsEmpty())
		return;

	if(imageids.IsEmpty()) {
		LLOG("Clearing image cache. Cached image count: " << imagecache.GetCount());
		imagecache.Clear();
	}
	else {
		// FIXME: or at least fix this. This is uglier than even me!..
		Vector<dword> idkeys = clone(imageids.GetKeys());
		Vector<dword> cached = clone(imagecache.GetKeys());
		Sort(idkeys);
		Sort(cached);
		for(const dword& id : RemoveSorted(cached, idkeys)) {
			int i = imagecache.Find(id);
			if(i >= 0)
				imagecache.Unlink(i);
		}
		if(imagecache.HasUnlinked())
			imagecache.Sweep();
	}
}

void Terminal::RenderSixel(const String& data, int ratio, bool nohole)
{
	if(!sixelgraphics)
		return;
	
	if(WhenSixel) {
		SixelInfo si;
		si.size = GetSize();	// Sixe hint.
		si.nohole = nohole;
		si.aspectratio = ratio;
		WhenSixel(si, data);
	}
	else {	// TODO: Lazy rendering (using worker threads) on MT environment.
		dword   id = data.GetHashValue();
		Image *img = imagecache.FindPtr(id);
		if(!img) {
			img = &imagecache.Add(
					id,
					SixelRenderer(data)
						.SetAspectRatio(ratio)
							.NoColorHole(nohole)
								.SetPaper(Black()));
		}
		int cy = img->GetSize().cy / GetFontSize().cy;
		page->SpecialLines(cy + 1, id);
	}
}
}