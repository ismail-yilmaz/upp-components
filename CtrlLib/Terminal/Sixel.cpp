#include "Sixel.h"

#define LLOG(x)		// RLOG("SixelRenderer: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

SixelRenderer::SixelRenderer(Stream& sixelstream)
: sstream(sixelstream)
, nohole(true)
, aspectratio(1)
{
	Clear();
}

void SixelRenderer::Clear()
{
		palette = {
			{ Black()    },
			{ Blue()     },
			{ Red()      },
			{ Green()    },
			{ Magenta()  },
			{ Cyan()     },
			{ Yellow()   },
			{ White()    },
			{ Gray()     },
			{ LtBlue()   },
			{ LtRed()    },
			{ LtGreen()  },
			{ LtMagenta()},
			{ LtCyan()   },
			{ LtYellow() },
			{ White()    },
		};

	init = false;
	rect   = Null;
	size   = Size(0, 0);
	cursor = Point(0, 0);
	datapos = 0;
	repeatcount = 1;
	ink = SColorText;
}

void SixelRenderer::GetNumericParams(Vector<int>& v, int delim)
{
	String number;

	while(!sstream.IsEof()) {
		int c = sstream.Peek();
		if(c >= '0' && '9' >= c) {
			number.Cat(sstream.Get());
		}
		else {
			v.Add(Nvl(StrInt(number), 0));
			if(c != delim || IsNull(delim))
				break;
			number.Clear();
			sstream.Skip(1);
		}
	}
}

void SixelRenderer::Return()
{
	cursor.x =  0;
}

void SixelRenderer::LineFeed()
{
	cursor.x =  0;
	cursor.y += 6;
	if(pass == 0)
		size.cy = cursor.y + 6;
}

static Color sHSLColor(int h, int s, int l)
{
	// See: https://en.wikipedia.org/wiki/HSL_and_HSV?oldformat=true#HSL_to_RGB

	if(s == 0) return Color(l, l, l);
	
	double h1 = fmod((h + 240) / 60, 6);
	double l1 = l / 100.0;
	double s1 = s / 100.0;
	
	double c = (1.0 - abs((2.0 * s1) - 1.0)) * l1;
	double x = c * (1.0 - abs(fmod(h1, 2.0) - 1.0));
	
	double r, g, b;
	switch(ffloor(h1)) {
	case 1: r = c; g = x; b = 0; break;
	case 2: r = x; g = c; b = 0; break;
	case 3: r = 0; g = c; b = x; break;
	case 4: r = 0; g = x; b = c; break;
	case 5: r = x; g = 0; b = c; break;
	case 6: r = c; g = 0; b = x; break;
	default: return Color(100, 100, 100);
	}
	
	double m = s1 - (c * 0.5);
	
	return Color(
		(int) clamp((r + m) * 100.0 + 0.5, 0.0, 100.0),
		(int) clamp((g + m) * 100.0 + 0.5, 0.0, 100.0),
		(int) clamp((b + m) * 100.0 + 0.5, 0.0, 100.0));
}

void SixelRenderer::SetPalette()
{
	Vector<int> params;

	GetNumericParams(params, ';');

	if(pass == 0 && params.GetCount() == 5) {
		if(params[1] == 1) { //	 HSL
			int h = params[2];
			int l = params[3];
			int s = params[4];
			palette.At(params[0]) = sHSLColor(h, s, l);
		}
		else
		if(params[1] == 2) { //	 RGB
			byte r = params[2] * 255 / 100;
			byte g = params[3] * 255 / 100;
			byte b = params[4] * 255 / 100;
			palette.At(params[0]) = Color(r, g, b);
		}
	}
	else
	if(pass == 1 && params.GetCount() == 1)
		ink = palette.At(params[0]);
}

void SixelRenderer::GetRasterInfo()
{
	if(pass != 0)
		return;

	Vector<int> params;

	GetNumericParams(params, ';');

	if(params.GetCount() == 4) {
		aspectratio = max(params[0] / params[1], 1);
		rect = Size(params[2], params[3]);
	}
}

void SixelRenderer::GetRepeatCount()
{
	Vector<int> params;

	GetNumericParams(params);

	if(params.GetCount() == 1)
		repeatcount = params[0];
}

bool SixelRenderer::InitBuffer(ImageBuffer& ib)
{
	if(size.cx < 1 || size.cy < 1)
		sstream.SetError();
	else {
		ib.Create(size);
		Fill(ib, ib.GetSize(), RGBAZero());
		if(nohole && !IsNull(rect))
			Fill(ib, rect, palette.At(0));
		init = true;
	}
	return !sstream.IsError();
}

void SixelRenderer::DrawSixel(ImageBuffer& ib, int c)
{
	repeatcount = max(1, repeatcount) * max(aspectratio, 1);

	if(pass == 0) { // Calculate canvas sixe.
		cursor.x += repeatcount;
		size.cx   = max(cursor.x, size.cx);
		repeatcount = 0;
	}
	else { // Paint sixel.
		if(!init && !InitBuffer(ib))
			return;
		while(repeatcount-- > 0) {
			Point pt = cursor;
			for(int n = 0; pt.y < cursor.y + 6; pt.y++, n++) {
				if(pt.x < size.cx && pt.y < size.cy)
					if(c & (1 << n))
						 ib[pt.y][pt.x] = ink;
			}
			cursor.x++;
		}
	}
}

void SixelRenderer::SetOptions()
{
	Vector<int> params;

	GetNumericParams(params, ';');

	if(sstream.Get() != 'q') {
		sstream.SetError();
		return;
	}

	if(params.IsEmpty()) {
		aspectratio = 1;
		nohole = true;
	}
	else
	for(int i = 0; i < params.GetCount(); i++) {
		if(i == 0) {
			switch(params[0]) {
			case 5:
			case 6:
				aspectratio = 2;
				break;
			case 3:
			case 4:
				aspectratio = 3;
				break;
			case 2:
				aspectratio = 5;
				break;
			default:
				aspectratio = 1;
				break;
			}
		}
		else
		if(i == 1) {
			nohole = params[1] != 1;
		}
	}

	datapos = sstream.GetPos();
}

void SixelRenderer::Validate()
{
	bool valid = false;
	
	if(pass == 0) {
		int c = sstream.Get();
		valid = c == 0x90 || (c == 0x1B && sstream.Get() == 0x50);
		if(valid)
			SetOptions();
	}
	else
	if(pass == 1) {
		valid = datapos > 0 && datapos < sstream.GetSize();
		if(valid) {
			cursor = Point(0, 0);
			sstream.Seek(datapos);
		}
	}
	if(!valid)
		sstream.SetError();
}

Image SixelRenderer::Get()
{
	Clear();

	LTIMING("SixelRenderer::Get");

	ImageBuffer buffer;

	for(pass = 0; pass < 2 && !sstream.IsError(); pass++) {
		Validate();
		while(!sstream.IsEof()) {
			int c = sstream.Get();
			switch(c) {
			case 0x21:
				GetRepeatCount();
				break;
			case 0x22:
				GetRasterInfo();
				break;
			case 0x23:
				SetPalette();
				break;
			case 0x24:
				Return();
				break;
			case 0x2D:
				LineFeed();
				break;
			default:
				if(0x3F <= c && c <= 0x7E)
					DrawSixel(buffer, c - 0x3F);
				break;
			}
		}
	}

	if(sstream.IsError())
		return Image();

	Premultiply(buffer);
	return buffer;
}

bool SixelRaster::Create()
{
	LTIMING("SixelRaster::Create");

	Stream& stream = GetStream();
	if(!stream.IsOpen()) {
		SetError();
		return false;
	}

	ASSERT(stream.IsLoading());

	imgraster.Create(SixelRenderer(stream).Get());
	return imgraster;
}

Raster::Info SixelRaster::GetInfo()
{
	return imgraster ? imgraster->GetInfo() : Raster::GetInfo();
}

Size SixelRaster::GetSize()
{
	return imgraster ? imgraster->GetSize() : Size(1, 1);
}

Raster::Line SixelRaster::GetLine(int line)
{
	if(imgraster)
		return imgraster->GetLine(line);
	return Line(new byte[sizeof(RGBA)], this, true);
}

INITIALIZER(SixelRaster)
{
	StreamRaster::Register<SixelRaster>();
}
}