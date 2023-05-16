#ifndef _EscPainter_h_
#define _EscPainter_h_

#include <Core/Core.h>
#include <Esc/Esc.h>
#include <Painter/Painter.h>

namespace Upp {

EscValue ToEsc(Font f);
EscValue ToEsc(Color c);
EscValue ToEsc(Rectf r);
EscValue ToEsc(Sizef sz);
EscValue ToEsc(Pointf pt);

Font     ToFont(EscValue v);
Color    ToColor(EscValue v);
Rectf    ToRectf(EscValue v);
Sizef    ToSizef(EscValue v);
Pointf   ToPointf(EscValue v);

class ESC_Painter : public EscHandle {
public:
	void GetSize(EscEscape& e);
	void GetRect(EscEscape& e);
	void GetCenterPos(EscEscape& e);
	void Begin(EscEscape& e);
	void End(EscEscape& e);
	void Clip(EscEscape& e);
	void Clear(EscEscape& e);
	void ColorStop(EscEscape& e);
	void ClearStops(EscEscape& e);
    void Opacity(EscEscape& e);
    void LineCap(EscEscape& e);
    void LineJoin(EscEscape& e);
    void MiterLimit(EscEscape& e);
    void EvenOdd(EscEscape& e);
    void Invert(EscEscape& e);
	void Translate(EscEscape& e);
	void Rotate(EscEscape& e);
	void Scale(EscEscape& e);
	void Move(EscEscape& e);
	void TopLeft(EscEscape& e);
	void TopRight(EscEscape& e);
	void TopCenter(EscEscape& e);
	void BottomCenter(EscEscape& e);
	void BottomLeft(EscEscape& e);
	void BottomRight(EscEscape& e);
	void Center(EscEscape& e);
	void Line(EscEscape& e);
	void Arc(EscEscape& e);
	void Path(EscEscape& e);
	void RectPath(EscEscape& e);
	void Cubic(EscEscape& e);
	void Quadratic(EscEscape& e);
	void BeginOnPath(EscEscape& e);
	void Stroke(EscEscape& e);
	void Fill(EscEscape& e);
	void Dash(EscEscape& e);
	void Rect(EscEscape& e);
	void RoundRect(EscEscape& e);
	void Circle(EscEscape& e);
	void Ellipse(EscEscape& e);
	void Text(EscEscape& e);
	void Character(EscEscape& e);
	void RenderSVG(EscEscape& e);

public:
	typedef ESC_Painter CLASSNAME;
	ESC_Painter(EscValue& v, Painter& w, Size sz);

private:
	Rectf GetBufferRect() const;
	Painter& w;
	Sizef size;
};

void  PainterLib(ArrayMap<String, EscValue>& global);

void  EscPaint(ArrayMap<String, EscValue>& global, Painter& w, Size sz);
void  EscPaint(ArrayMap<String, EscValue>& global, Painter& w, int cx, int cy);

Image EscPaintImage(ArrayMap<String, EscValue>& global, Size sz);
Image EscPaintImage(ArrayMap<String, EscValue>& global, int cx, int cy);
Image EscPaintImage(ArrayMap<String, EscValue>& global, const String& script, Size sz);
Image EscPaintImage(ArrayMap<String, EscValue>& global, const String& script, int cx, int cy);

}
#endif
