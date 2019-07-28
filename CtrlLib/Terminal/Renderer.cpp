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

void Terminal::Paint(Draw& w)
{
	GuiLock __;
	int  pos = GetSbPos();
	Size wsz = GetSize();
	Size psz = GetPageSize();
	Size fsz = GetFontSize();
	Font fnt = font;
	
	w.Clip(wsz);
	{
		sVTRectRenderer rr(w);
		sVTTextRenderer tr(w);
		LTIMING("Terminal::Paint");
		if(!nobackground)
			w.DrawRect(wsz, colortable[COLOR_PAPER]);
		for(int i = 0; i < pos + psz.cy; i++) {
			int y = i * fsz.cy - (fsz.cy * pos);
			if(w.IsPainting(0, y, wsz.cx, fsz.cy)) {
				int pass = 0;
				while(pass < 2) {
					const auto& line = page->GetLine(i);
					for(int j = 0; !line.IsEmpty() && j < psz.cx; j++) {
						const VTCell& cell = j < line.GetCount() ? line[j] : GetAttrs();
						Color ink = GetColorFromIndex(cell, COLOR_INK);
						Color paper = GetColorFromIndex(cell, COLOR_PAPER);
						if(cell.IsInverted())
							Swap(ink, paper);
						if(modes[DECSCNM])
							Swap(ink, paper);
						int x = j * fsz.cx;
						int n = i * psz.cx + j;
						bool highlight = IsSelected(n);
						if(pass == 0) {
							if(!nobackground ||
								(highlight || cell.IsInverted() || cell.paper != 0xFFFF)) {
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
								if(!cell.IsBlinking() || highlight || (cell.IsBlinking() && !blinking)) {
									tr.DrawChar(x, y, cell, fsz, fnt, highlight ? colortable[COLOR_INK_SELECTED] : ink);
								}
							}
						}
					}
					rr.Flush();
					tr.Flush();
					pass++;
				}
			}
		}
		// Paint a steady (non-blinking) caret, if enabled.
		if(modes[DECTCEM] && HasFocus() && !caret.IsBlinking())
			w.DrawRect(GetCaretRect(), InvertColor);

		// Hint new size.
		if(sizehint && hinting) {
			auto hint = GetSizeHint(GetView(), psz);
			DrawFrame(w ,hint.b.Inflated(8), LtGray);
			w.DrawRect(hint.b.Inflated(7), SColorText);
			w.DrawText(hint.b.left, hint.b.top, hint.a, StdFont(), SColorPaper);
		}
	}
	w.End();
}

static Color sAdjustBrightness(const Color& c, const VTCell& cell, int which, double brightness = 0.70f)
{
	if(!cell.IsFaint() || which != Console::COLOR_INK)
		return c;
	
	double h, s, v;
	RGBtoHSV(c.GetR() / 255.0, c.GetG() / 255.0, c.GetB() / 255.0, h, s, v);
	return HsvColorf(h, s, brightness);
}

Color Terminal::GetColorFromIndex(const VTCell& cell, int which) const
{
	int index = which == COLOR_INK ? cell.ink : cell.paper;

	Color c;

	if(index == 0xFFFF) {
		index = which;
	}
	else
	if(index >= 16) {	// 256 colors support.
		byte r, g, b;
		if(index < 232) {
			r =	(( index - 16) / 36) * 51,
			g =	(((index - 16) % 36) / 6) * 51,
			b =	(( index - 16) % 6)  * 51;
		}
		else // Grayscale
			r = g = b = (index - 232) * 10 + 8;
		return sAdjustBrightness(Color(r, g, b), cell, which);
	}

	if(lightcolors ||
		(intensify && which == COLOR_INK && cell.IsBold()))
			if(index < 8)
				index += 8;

	c = colortable[index];	// Adjust only the first 16 colors.
	return sAdjustBrightness(adjustcolors ? AdjustIfDark(c) : c, cell, which);
}
}