#include "Sixel.h"

#define LLOG(x)		// RLOG("SixelRenderer: " << x)
#define LTIMING(x)	 RTIMING(x)

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

Color HslColorf(double h, double s, double l)
{
	// This function is based on an HSL->RGB conversion formula which can be found on Wikipedia:
	// https:en.wikipedia.org/wiki/HSL_and_HSV?oldformat=true#HSL_to_RGB_alternative

	auto fn = [=] (double n) -> double
	{
		double a = s * min(l, 1.0 - l);
		double k = modulo(h / (30.0 / 360.0) + n, 12.0);
		return l - a * max(min(k - 3.0, 9.0 - k * 1.0), -1.0);
	};

	return Color(fn(0) * 255, fn(8) * 255, fn(4) * 255);
}

void SixelRenderer::SetPalette()
{
	Vector<int> params;

	GetNumericParams(params, ';');

	if(pass == 0 && params.GetCount() == 5) {
		if(params[1] == 1) { //	 HSL
			double h = params[2] * 1.0;
			double l = params[3] / 100.0;
			double s = params[4] / 100.0;
			palette.At(params[0]) = HslColorf(h, s, l);
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
					if( c & (1 << n))
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

	if(sstream.Get() != 'q')
		return;

	if(params.IsEmpty()) {
		aspectratio = 1;
		nohole = true;
	}
	else
	for(int i = 0; i < params.GetCount(); i++) {
		if(i == 0) {
			switch(params[0]) {
			case 5 ... 6:
				aspectratio = 2;
				break;
			case 3 ... 4:
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

void SixelRenderer::ParseEsc(int c)
{
	if(pass != 0 || datapos != 0)
		return;

	int cc = sstream.Peek();

	if((c == 0x1B && (cc == 0x50 || cc == 0x5C)) || 0x90) {
		if(c  != 0x90)
			sstream.Skip(1);
		if(cc != 0x5c)
			SetOptions();
	}
}

Image SixelRenderer::Get()
{
	Clear();

	LTIMING("SixelRenderer::Get");

	ImageBuffer buffer;

	for(pass = 0; pass < 2 && !sstream.IsError(); pass++) {
		if(pass == 1)
		{
			cursor = Point(0, 0);
			sstream.Seek(datapos);
		}
		while(!sstream.IsEof()) {
			int c = sstream.Get();
			switch(c) {
			case 0x1B:
			case 0x90:
			case 0x9C:
				ParseEsc(c);
				break;
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
			case 0x3F ... 0x7E:
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