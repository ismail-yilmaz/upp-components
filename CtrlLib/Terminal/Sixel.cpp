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
		{ (RGBA) Black()    },
		{ (RGBA) Blue()     },
		{ (RGBA) Red()      },
		{ (RGBA) Green()    },
		{ (RGBA) Magenta()  },
		{ (RGBA) Cyan()     },
		{ (RGBA) Yellow()   },
		{ (RGBA) White()    },
		{ (RGBA) Gray()     },
		{ (RGBA) LtBlue()   },
		{ (RGBA) LtRed()    },
		{ (RGBA) LtGreen()  },
		{ (RGBA) LtMagenta()},
		{ (RGBA) LtCyan()   },
		{ (RGBA) LtYellow() },
		{ (RGBA) White()    },
	};

	rect    = Null;
	size    = Size(0, 0);
	cursor  = Point(0, 0);
	datapos = 0;
	repeat  = 1;
	ink     = palette[0];
	ink.a   = 0xff;
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
			v.Add() = Nvl(StrInt(number), 0);
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
	size.cx = max(size.cx, cursor.x);
	cursor.x =  0;
	cursor.y += 6;
	size.cy = max(size.cy, cursor.y);
}

static Color sHSLColor(int h, int s, int l)
{
	// See: https://en.wikipedia.org/wiki/HSL_and_HSV?oldformat=true#HSL_to_RGB

	if(s == 0) return Color(l, l, l);
	
	double h1 = fmod((h + 240) / 60, 6);
	double l1 = l * 0.01;
	double s1 = s * 0.01;
	
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

	if(params.GetCount() == 5) {
		if(params[1] == 1) { //	 HSL
			int h = params[2];
			int l = params[3];
			int s = params[4];
			palette.At(params[0]) = sHSLColor(h, s, l);
		}
		else
		if(params[1] == 2) { //	 RGB
			RGBA& rgba = palette.At(params[0]);
			rgba.r = params[2] * 255 * 0.01;
			rgba.g = params[3] * 255 * 0.01;
			rgba.b = params[4] * 255 * 0.01;
		}
	}
	else
	if(params.GetCount() == 1) {
		ink = palette.At(params[0]);
		ink.a = 0xFF;
	}
}

void SixelRenderer::GetRasterInfo()
{
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
	if(params.GetCount() == 1) {
		repeat = max(1, params[0]) * max(aspectratio, 1);
	}
}

void SixelRenderer::InitBuffer(ImageBuffer& ib)
{
	Size sz = ib.GetSize();
	if((sz.cx <= cursor.x + repeat) || (sz.cy <= cursor.y + 6)) {
		if((cursor.x + repeat >= 4096) || (cursor.y + 6 >= 4098))
			throw Exc("Sixel canvas size is too big > (4096 x 4096)");
		ImageBuffer ibb(ib.GetSize() << 1);
		Copy(ibb, Point(0, 0), ib, ib.GetSize());
		ib = ibb;
		if(nohole && !IsNull(rect))
			Fill(ib, rect, palette.At(0));
		else
			Fill(ib, ib.GetSize(), RGBAZero());
	}
}

void SixelRenderer::PaintSixel(ImageBuffer& ib, int c)
{
	InitBuffer(ib);
	for(int n = 0, r = repeat; n < 6; ++n, r = repeat) {
		if(c & (1 << n)) {
			RGBA *rgba = ib[cursor.y + n] + cursor.x; // this seems to be a bit faster than ib[y][x++] = ink ...
			while(r--)
				*rgba++ = ink;
		}
	}
	cursor.x += repeat;
	size.cx = max(size.cx, cursor.x);
	repeat = 1 * max(1, aspectratio);
}

void SixelRenderer::SetOptions()
{
	Vector<int> params;
	GetNumericParams(params, ';');

	if(sstream.Get() != 'q') {
		sstream.SetError();
		return;
	}

	if(params.GetCount() >= 1) {
		aspectratio = decode(params[0], 5, 2, 6, 2, 3, 3, 4, 3, 2, 5, 1);
	}
	else
	if(params.GetCount() >= 2) {
		nohole = params[1] != 1;
	}
	else
	if(params.IsEmpty()) {
		aspectratio = 1;
		nohole = true;
	}

	datapos = sstream.GetPos();
	repeat = 1 * min(1, aspectratio);
}

void SixelRenderer::Validate()
{
	bool valid = false;
	
	int c = sstream.Get();
	valid = c == 0x90 || (c == 0x1B && sstream.Get() == 0x50);
	if(valid)
		SetOptions();
	else
		sstream.SetError();
}

Image SixelRenderer::Get()
{
	Clear();

	LTIMING("SixelRenderer::Get");

	ImageBuffer buffer(1024, 1024);
	Fill(buffer, buffer.GetSize(), palette[0]);
	
	try {
		if(!sstream.IsError()) {
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
						PaintSixel(buffer, c - 0x3F);
					break;
				}
			}
		}
	}
	catch(const Exc& e) {
		LLOG(e);
	}

	return !sstream.IsError() ? Crop(buffer, size) : Image();
}

bool SixelRaster::Create()
{
//	RTIMING("SixelRaster::Create");

	Stream& stream = GetStream();
	if(!stream.IsOpen()) {
		SetError();
		return false;
	}
	ASSERT(stream.IsLoading());
	img = SixelRenderer(stream).Get();
	return !img.IsEmpty();
}

Raster::Info SixelRaster::GetInfo()
{
	Raster::Info f = Raster::GetInfo();
	f.dots = img.GetDots();
	f.hotspot = img.GetHotSpot();
	f.kind = img.GetKind();
	return f;
}

Size SixelRaster::GetSize()
{
	return img.GetSize();
}

Raster::Line SixelRaster::GetLine(int line)
{
	return Line(img[line], false);
}

INITIALIZER(SixelRaster)
{
	StreamRaster::Register<SixelRaster>();
}
}