#include "EscPainter.h"
#include "Macros.brc"

namespace Upp {

static struct { const char  *name; Color color; } sColors[] = {
	{ "Black", Black },
	{ "Red",   Red },
	{ "Green", Green },
	{ "Brown", Brown },
	{ "Blue",  Blue },
	{ "Magenta", Magenta },
	{ "Cyan", Cyan },
	{ "Gray", Gray },
	{ "LtGray", LtGray },
	{ "LtRed", LtRed },
	{ "LtGreen", LtGreen },
	{ "LtYellow", LtYellow },
	{ "LtBlue", LtBlue },
	{ "LtMagenta", LtMagenta },
	{ "LtCyan", LtCyan },
	{ "Yellow", Yellow },
	{ "WhiteGray", WhiteGray },
	{ "White", White },
};

double GetNumber(const EscValue& e, const String& s)
{
	if(e.IsMap()) {
		const VectorMap<EscValue, EscValue>& map = e.GetMap();
		int n = map.Find(s);
		if(n >= 0) return map[n].GetNumber();
	}
	return 0;
}

bool GetOptFlag(EscEscape& e)
{
	if(!e.GetCount())
		return true;
	return IsTrue(e[0]);
}

EscValue ToEsc(Color c)
{
	EscValue v;
	int i = c.GetSpecial();
	if(i >= 0 && i < 18)
		c = sColors[i].color;
	if(!IsNull(c)) {
		v.MapSet("r", c.GetR());
		v.MapSet("g", c.GetG());
		v.MapSet("b", c.GetB());
	}
	return v;
}

Color ToColor(EscValue v)
{
	return v.IsVoid()
		? Color(Null)
		: Color(
			v.GetFieldInt("r"),
			v.GetFieldInt("g"),
			v.GetFieldInt("b"));
}

EscValue ToEsc(Rectf r)
{
	EscValue v;
	v.MapSet("left", r.left);
	v.MapSet("right", r.right);
	v.MapSet("top", r.top);
	v.MapSet("bottom", r.bottom);
	return v;
}

Rectf ToRectf(EscValue v)
{
	return Rectf(
		GetNumber(v, "left"),
		GetNumber(v, "top"),
	    GetNumber(v, "right"),
	    GetNumber(v, "bottom"));
}

EscValue ToEsc(Sizef sz)
{
	EscValue v;
	v.MapSet("cx", sz.cx);
	v.MapSet("cy", sz.cy);
	return v;
}

Sizef ToSizef(EscValue v)
{
	return Sizef(
		GetNumber(v, "cx"),
		GetNumber(v, "cy"));
}

EscValue ToEsc(Pointf pt)
{
	EscValue v;
	v.MapSet("x", pt.x);
	v.MapSet("y", pt.y);
	return v;
}

Pointf ToPointf(EscValue v)
{
	return Pointf(
		GetNumber(v, "x"),
		GetNumber(v, "y"));
}

void SIC_GetTextSize(EscEscape& e)
{
	if(e.GetCount() < 1 || e.GetCount() > 2)
		e.ThrowError("wrong number of arguments in call to 'GetTextSize'");
	e.CheckArray(0);
	WString text = e[0];
	Font font = StdFont();
	if(e.GetCount() > 1)
		font = ToFont(e[1]);
	e = ToEsc(GetTextSize(text, font));
}

struct SIC_Font : public EscHandle {
	Font font;

	void Height(EscEscape& e)
	{
		e = ToEsc(ToFont(e.self).Height(e.Int(0)));
	}
	void Bold(EscEscape& e)
	{
		e = ToEsc(ToFont(e.self).Bold(GetOptFlag(e)));
	}
	void Italic(EscEscape& e)
	{
		e = ToEsc(ToFont(e.self).Italic(GetOptFlag(e)));
	}
	void Underline(EscEscape& e)
	{
		e = ToEsc(ToFont(e.self).Underline(GetOptFlag(e)));
	}

	void GetWidth(EscEscape& e) const
	{
		e = ToFont(e.self).GetWidth(e[0].GetInt());
	}
	
	void GetAscent(EscEscape& e) const
	{
		e = (double) ToFont(e.self).GetAscent();
	}

	void GetDescent(EscEscape& e) const
	{
		e = (double) ToFont(e.self).GetDescent();
	}
	
	typedef SIC_Font CLASSNAME;

	SIC_Font(EscValue& v)
	{
		v.Escape("Height(h)", this, THISBACK(Height));
		v.Escape("Bold(...)", this, THISBACK(Bold));
		v.Escape("Italic(...)", this, THISBACK(Italic));
		v.Escape("Underline(...)", this, THISBACK(Underline));
		v.Escape("GetWidth(w)", this, THISBACK(GetWidth));
		v.Escape("GetAscent()", this, THISBACK(GetAscent));
		v.Escape("GetDescent()", this, THISBACK(GetDescent));
	}
};

EscValue ToEsc(Font f)
{
	EscValue v;
	(new SIC_Font(v))->font = f;
	return v;
}

Font ToFont(EscValue v)
{
	if(!v.IsMap())
		return Null;
	const VectorMap<EscValue, EscValue>& m = v.GetMap();
	int q = m.Find("Height");
	if(q < 0)
		return Null;
	const EscLambda& l = m[q].GetLambda();
	if(!dynamic_cast<SIC_Font *>(l.handle))
		return Null;
	Font f = ((SIC_Font *)l.handle)->font;
	if(f.GetHeight() == 0)
		f.Height(StdFont().GetHeight());
	return f;
}

void SIC_StdFont(EscEscape& e)
{
	if(e.GetCount() == 1)
		e = ToEsc(StdFont()(e.Int(0)));
	else
		e = ToEsc(StdFont());
}

void SIC_Arial(EscEscape& e)
{
	e = ToEsc(Arial(e.Int(0)));
}

void SIC_Roman(EscEscape& e)
{
	e = ToEsc(Roman(e.Int(0)));
}

void SIC_Courier(EscEscape& e)
{
	e = ToEsc(Courier(e.Int(0)));
}

ESC_Painter::ESC_Painter(EscValue& v, Painter& w_, Size sz)
: w(w_)
, size(sz)
{
		v.Escape("Begin()",	this, THISFN(Begin));
		v.Escape("End()"  ,	this, THISFN(End));
		v.Escape("Background(color)", this, THISFN(SetBackground));
		v.Escape("Stroke(...)", this, THISFN(Stroke));
		v.Escape("Fill(...)",	this, THISFN(Fill));
		v.Escape("Dash(...)", this, THISFN(Dash));
		v.Escape("Translate(...)", this, THISFN(Translate));
		v.Escape("Rotate(r)", this, THISFN(Rotate));
		v.Escape("Scale(...)", this, THISFN(Scale));
		v.Escape("Move(...)", this, THISFN(Move));
		v.Escape("TopLeft()", this, THISFN(TopLeft));
		v.Escape("TopRight()", this, THISFN(TopRight));
		v.Escape("TopCenter()", this, THISFN(TopCenter));
		v.Escape("BottomCenter()", this, THISFN(BottomCenter));
		v.Escape("BottomLeft()", this, THISFN(BottomLeft));
		v.Escape("BottomRight()", this, THISFN(BottomRight));
		v.Escape("Center()", this, THISFN(Center));
		v.Escape("Line(...)", this, THISFN(Line));
		v.Escape("Circle(...)",	this, THISFN(Circle));
		v.Escape("Ellipse(...)", this, THISFN(Ellipse));
		v.Escape("Arc(...)", this, THISFN(Arc));
		v.Escape("Path(x)",	this, THISFN(Path));
		v.Escape("Cubic(...)", this, THISFN(Cubic));
		v.Escape("Quadratic(...)", this, THISFN(Quadratic));
		v.Escape("BeginOnPath(q, b)", this, THISFN(BeginOnPath));
		v.Escape("Rectangle(...)", this, THISFN(Rect));
		v.Escape("RoundedRectangle(...)", this, THISFN(RoundRect));
		v.Escape("GetSize()", this, THISFN(GetSize));
		v.Escape("GetRect()", this, THISFN(GetRect));
		v.Escape("GetCenterPoint()", this, THISFN(GetCenterPos));
		v.Escape("Text(...)", this, THISFN(Text));
		v.Escape("Character(...)", this, THISFN(Character));
}

Rectf ESC_Painter::GetBufferRect() const
{
	return size;
}

void ESC_Painter::GetSize(EscEscape& e)
{
	e = ToEsc(size);
}

void ESC_Painter::GetRect(EscEscape& e)
{
	e = ToEsc(GetBufferRect());
}

void ESC_Painter::GetCenterPos(EscEscape& e)
{
	e = ToEsc(GetBufferRect().CenterPoint());
}

void ESC_Painter::Begin(EscEscape& e)
{
	w.Begin();
	e = e.self;
}

void ESC_Painter::End(EscEscape& e)
{
	w.End();
	e = e.self;
}

void ESC_Painter::Translate(EscEscape& e)
{
	if(e.GetCount() == 2)
		w.Translate(
			e[0].GetNumber(),
			e[1].GetNumber());
	else
	if(e.GetCount() == 1) {
		w.Translate(
			ToPointf(e[0]));
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Translate'");
	e = e.self;
}

void ESC_Painter::Rotate(EscEscape& e)
{
	w.Rotate(e[0].GetNumber());
	e = e.self;
}

void ESC_Painter::Scale(EscEscape& e)
{
	if(e.GetCount() == 1)
		w.Scale(
			e[0].GetNumber());
	else
	if(e.GetCount() == 2)
		w.Scale(
			e[0].GetNumber(),
			e[1].GetNumber());
	else
		e.ThrowError("wrong number of arguments in call to 'Scale'");
	e = e.self;
}

void ESC_Painter::Move(EscEscape& e)
{
	if(e.GetCount() == 2)
		w.Move(
			e[0].GetNumber(),
			e[1].GetNumber());
	else
	if(e.GetCount() == 1)
		w.Move(
			ToPointf(e[0]));
	else
		e.ThrowError("wrong number of arguments in call to 'Move'");
	e = e.self;
}

void ESC_Painter::TopLeft(EscEscape& e)
{
	w.Translate(GetBufferRect().TopLeft());
	e = e.self;
}

void ESC_Painter::TopRight(EscEscape& e)
{
	w.Translate(GetBufferRect().TopRight());
	e = e.self;
}

void ESC_Painter::TopCenter(EscEscape& e)
{
	w.Translate(GetBufferRect().TopCenter());
	e = e.self;
}

void ESC_Painter::BottomCenter(EscEscape& e)
{
	w.Translate(GetBufferRect().BottomCenter());
	e = e.self;
}

void ESC_Painter::BottomLeft(EscEscape& e)
{
	w.Translate(GetBufferRect().BottomLeft());
	e = e.self;
}

void ESC_Painter::BottomRight(EscEscape& e)
{
	w.Translate(GetBufferRect().BottomRight());
	e = e.self;
}

void ESC_Painter::Center(EscEscape& e)
{
	w.Translate(GetBufferRect().CenterPoint());
	e = e.self;
}

void ESC_Painter::Line(EscEscape& e)
{
	if(e.GetCount() == 2)
		w.Line(
			e[0].GetNumber(),
			e[1].GetNumber());
	else
	if(e.GetCount() == 1)
		w.Line(
			ToPointf(e[0]));
	else
		e.ThrowError("wrong number of arguments in call to 'Line'");
	e = e.self;
}

void ESC_Painter::Arc(EscEscape& e)
{
	if(e.GetCount() == 4) {
		w.Arc(
			ToPointf(e[0]),
			ToPointf(e[1]),
			e[2].GetNumber(),
			e[3].GetNumber());
	}
	else
	if(e.GetCount() == 5) {
		w.Arc(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			e[3].GetNumber(),
			e[4].GetNumber());
	}
	else
	if(e.GetCount() == 6) {
		w.Arc(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			e[3].GetNumber(),
			e[4].GetNumber(),
			e[5].GetNumber());
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Arc'");
	e = e.self;
}

void ESC_Painter::Path(EscEscape& e)
{
	w.Path((const String&) e[0]);
	e = e.self;
}

void ESC_Painter::Cubic(EscEscape& e)
{
	int n = e.GetCount();
	
	if(n == 4 || n == 5) {
		w.Cubic(
			e[0].GetNumber(),
		    e[1].GetNumber(),
		    e[2].GetNumber(),
		    e[3].GetNumber(),
		    n == 5 ? IsTrue(e[4]) : false);
	}
	else
	if(n == 6 || n == 7) {
		w.Cubic(
			e[0].GetNumber(),
		    e[1].GetNumber(),
		    e[2].GetNumber(),
		    e[3].GetNumber(),
		    e[4].GetNumber(),
		    e[5].GetNumber(),
		    n == 7 ? IsTrue(e[6]) : false);
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Cubic'");
	e = e.self;

}

void ESC_Painter::Quadratic(EscEscape& e)
{
	int n = e.GetCount();
	
	if(n == 2 || n == 3) {
		w.Quadratic(
			e[0].GetNumber(),
		    e[1].GetNumber(),
			n == 3 ? IsTrue(e[2]) : false);
	}
	else
	if(n == 4 || n == 5) {
		w.Quadratic(
			e[0].GetNumber(),
		    e[1].GetNumber(),
		    e[2].GetNumber(),
		    e[3].GetNumber(),
		    n == 5 ? IsTrue(e[4]) : false);
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Quadratic'");
	e = e.self;

}

void ESC_Painter::BeginOnPath(EscEscape& e)
{
	if(e.GetCount() == 2) {
		w.BeginOnPath(
			e[0].GetNumber(),
			IsTrue(e[1]));
	}
	else
		e.ThrowError("wrong number of arguments in call to 'BeginOnPath'");
	e = e.self;

}

void ESC_Painter::Stroke(EscEscape& e)
{
	int n =  e.GetCount();

	if(n == 2) {
		w.Stroke(
			e[0].GetNumber(),
			ToColor(e[1]));
	}
	else
	if(n == 5 || n == 6) {
		w.Stroke(
			e[0].GetNumber(),
			ToPointf(e[1]),
			ToColor(e[2]),
			ToPointf(e[3]),
			ToColor(e[4]),
			n == 6 ? e[5].GetInt() : GRADIENT_PAD);
	}
	else
	if(n == 7 || n == 8) {
		w.Stroke(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			ToColor(e[3]),
			e[4].GetNumber(),
			e[5].GetNumber(),
			ToColor(e[6]),
			n == 8 ? e[7].GetInt() : GRADIENT_PAD);
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Stroke'");
	
	e = e.self;
}

void ESC_Painter::Fill(EscEscape& e)
{
	int n =  e.GetCount();

	if(n == 1) {
		w.Fill(
			ToColor(e[0]));
	}
	else
	if(n == 4 || n == 5) {
		w.Fill(
			ToPointf(e[0]),
			ToColor(e[1]),
			ToPointf(e[2]),
			ToColor(e[3]),
			n == 5 ? e[4].GetInt() : GRADIENT_PAD);
	}
	else
	if(n == 6 || n == 7) {
		w.Fill(
			e[0].GetNumber(),
			e[1].GetNumber(),
			ToColor(e[2]),
			e[3].GetNumber(),
			e[4].GetNumber(),
			ToColor(e[5]),
			n == 7 ? e[6].GetInt() : GRADIENT_PAD);
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Fill'");
	e = e.self;
}

void ESC_Painter::Dash(EscEscape& e)
{
	int n = e.GetCount();
	
	if(n == 1 || n == 2)
		w.Dash(
			(const String&) e[0],
			n == 2 ? e[1].GetInt() : 0);
	else
		e.ThrowError("wrong number of arguments in call to 'Dash'");
	e = e.self;
}

void ESC_Painter::SetBackground(EscEscape& e)
{
	w.RectPath(GetBufferRect())
		.Fill(ToColor(e[0]));
	e = e.self;
}

void ESC_Painter::Rect(EscEscape& e)
{
	if(e.GetCount() == 1) {
		Rectf r = ToRectf(e[0]);
		w.Rectangle(
			r.left,
			r.top,
			r.right,
			r.bottom);
	}
	else
	if(e.GetCount() == 4)
		w.Rectangle(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			e[3].GetNumber());
	else
		e.ThrowError("wrong number of arguments in call to 'Rectangle'");
	e = e.self;
}

void ESC_Painter::RoundRect(EscEscape& e)
{
	if(e.GetCount() == 2) {
		Rectf r = ToRectf(e[0]);
		w.RoundedRectangle(
			r.left,
			r.top,
			r.Width(),
			r.Height(),
			e[1].GetNumber());
	}
	else
	if(e.GetCount() == 3) {
		Rectf r = ToRectf(e[0]);
		w.RoundedRectangle(
			r.left,
			r.top,
			r.Width(),
			r.Height(),
			e[1].GetNumber(),
			e[2].GetNumber());
	}
	else
	if(e.GetCount() == 5) {
		w.RoundedRectangle(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			e[3].GetNumber(),
			e[4].GetNumber());
	}
	else
	if(e.GetCount() == 6) {
		w.RoundedRectangle(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			e[3].GetNumber(),
			e[4].GetNumber(),
			e[5].GetNumber());
	}
	else
		e.ThrowError("wrong number of arguments in call to 'RoundedRectangle'");
	e = e.self;

}

void ESC_Painter::Circle(EscEscape& e)
{
	if(e.GetCount() == 2) {
		Pointf pt = ToPointf(e[0]);
		w.Circle(
			pt.x,
			pt.y,
			e[1].GetNumber());
	}
	else
	if(e.GetCount() == 3)
		w.Circle(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber());
	else
		e.ThrowError("wrong number of arguments in call to 'Circle'");
	e = e.self;
}

void ESC_Painter::Ellipse(EscEscape& e)
{
	if(e.GetCount() == 2) {
		Pointf pt = ToPointf(e[0]);
		w.Ellipse(
			pt.x,
			pt.y,
			e[1].GetNumber(),
			e[2].GetNumber());
	}
	else
	if(e.GetCount() == 4)
		w.Ellipse(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetNumber(),
			e[3].GetNumber());
	else
		e.ThrowError("wrong number of arguments in call to 'Circle'");
	e = e.self;
}

void ESC_Painter::Text(EscEscape& e)
{
	if(e.GetCount() == 2) {
		w.Text(
			0, 0,
			String(e[0]).ToWString(),
			ToFont(e[1]),
			nullptr);
	}
	else
	if(e.GetCount() == 3) {
		w.Text(
			ToPointf(e[0]),
			String(e[1]).ToWString(),
			ToFont(e[2]),
			nullptr);
	}
	else
	if(e.GetCount() == 4) {
		w.Text(
			e[0].GetNumber(),
			e[1].GetNumber(),
			String(e[2]).ToWString(),
			ToFont(e[3]),
			nullptr);
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Text'");
	e = e.self;
}

void ESC_Painter::Character(EscEscape& e)
{
	if(e.GetCount() == 3) {
		w.Character(
			ToPointf(e[0]),
			e[1].GetInt(),
			ToFont(e[2]));
	}
	else
	if(e.GetCount() == 4) {
		w.Character(
			e[0].GetNumber(),
			e[1].GetNumber(),
			e[2].GetInt(),
			ToFont(e[3]));
	}
	else
		e.ThrowError("wrong number of arguments in call to 'Character'");
	e = e.self;
}

void PainterLib(ArrayMap<String, EscValue>& global)
{
	global.Add("Black",		ToEsc(Black));
	global.Add("Gray",		ToEsc(Gray));
	global.Add("LtGray",	ToEsc(LtGray));
	global.Add("WhiteGray", ToEsc(WhiteGray));
	global.Add("White",		ToEsc(White));
	global.Add("Red",		ToEsc(Red));
	global.Add("Green",		ToEsc(Green));
	global.Add("Brown",		ToEsc(Brown));
	global.Add("Blue",		ToEsc(Blue));
	global.Add("Magenta",	ToEsc(Magenta));
	global.Add("Cyan",		ToEsc(Cyan));
	global.Add("Yellow",	ToEsc(Yellow));
	global.Add("LtRed",		ToEsc(LtRed));
	global.Add("LtGreen",	ToEsc(LtGreen));
	global.Add("LtYellow",	ToEsc(LtYellow));
	global.Add("LtBlue",	ToEsc(LtBlue));
	global.Add("LtMagenta", ToEsc(LtMagenta));
	global.Add("LtCyan",	ToEsc(LtCyan));

	Escape(global, "StdFont(...)",  SIC_StdFont);
	Escape(global, "Arial(h)",      SIC_Arial);
	Escape(global, "Roman(h)",      SIC_Roman);
	Escape(global, "Courier(h)",    SIC_Courier);
	Escape(global, "GetTextSize(...)", SIC_GetTextSize);

	Scan(global, String(macros_plain, macros_plain_length));
}

void EscPaint(ArrayMap<String, EscValue>& global, Painter& w, Size sz)
{
	int i = global.Find("Paint");
	if(i < 0 || !global[i].IsLambda())
		return;
	Vector<EscValue> arg;
	EscValue draw;
	new ESC_Painter(draw, w, sz);
	arg.Add(draw);
	Execute(global, nullptr, global[i], arg, INT_MAX);
}

void EscPaint(ArrayMap<String, EscValue>& global, Painter& w, int cx, int cy)
{
	EscPaint(global, w, Size(cx, cy));
}

Image EscPaintImage(ArrayMap<String, EscValue>& global, Size sz)
{
	ImagePainter im(sz);
	EscPaint(global, im, sz);
	return im.GetResult();
}

Image EscPaintImage(ArrayMap<String, EscValue>& global, int cx, int cy)
{
	return EscPaintImage(global, Size(cx, cy));
}

Image EscPaintImage(ArrayMap<String, EscValue>& global, const String& script, Size sz)
{
	Scan(global, script);
	return EscPaintImage(global, sz);
}

Image EscPaintImage(ArrayMap<String, EscValue>& global, const String& script, int cx, int cy)
{
	return EscPaintImage(global, script, Size(cx, cy));
}


}
