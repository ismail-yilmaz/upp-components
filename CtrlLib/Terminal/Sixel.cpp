#include "Sixel.h"

#define LLOG(x)		 // RLOG("SixelStream: " << x)
#define LTIMING(x)	 // RTIMING(x)

namespace Upp {

SixelStream::SixelStream(const void *data, int64 size)
: MemReadStream(data, size)
, background(true)
{
}

SixelStream::SixelStream(const String& data)
: MemReadStream(~data, data.GetLength())
, background(true)
{
}

void SixelStream::Clear()
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

	size    = Size(0, 0);
	cursor  = Point(0, 1);
	repeat  = 0;
	ink     = palette[0];
	ink.a   = 0xFF;
	paper   = ink;

	Zero(params);
	
	buffer.Create(1024, 1024);
	Fill(buffer, buffer.GetSize(), background ? paper : RGBAZero());
	
	CalcYOffests();
}

force_inline
int SixelStream::ReadParams()
{
	LTIMING("SixelStream::ReadParams");
	
	Zero(params);
	int c = 0, n = 0, i = 0;
	for(;;) {
		while((c = Peek()) > 0x2F && c < 0x3A)
			n = n * 10 + (Get() - 0x30);
		params[i++ & 7] = n;
		if(c != ';')
			break;
		n = 0;
		Get(); // faster than Stream::Skip(1)
	}
	return i;
}

force_inline
void SixelStream::CalcYOffests()
{
	int w = buffer.GetWidth();
	for(int i = 0, y = cursor.y * w; i < 6; i++, y += w)
		coords[i] = y;
}

force_inline
void SixelStream::Return()
{
	cursor.x = 0;
}

force_inline
void SixelStream::LineFeed()
{
	size.cx = max(size.cx, cursor.x);
	cursor.x =  0;
	cursor.y += 6;
	size.cy = max(size.cy, cursor.y);
	CalcYOffests();
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

force_inline
void SixelStream::SetPalette()
{
	LTIMING("SixelStream::SetPalette");
	
	int n = ReadParams();
	if(n == 5) {
		switch(params[1]) {
		case 2: { // RGB
			RGBA& rgba = palette.At(params[0]);
			rgba.r = (params[2] * 255 + 99) / 100;
			rgba.g = (params[3] * 255 + 99) / 100;
			rgba.b = (params[4] * 255 + 99) / 100;
			break;
		}
		case 1: { // HLS
			int h = params[2];
			int l = params[3];
			int s = params[4];
			palette.At(params[0]) = sHSLColor(h, s, l);
			break;
		}
		default:
			break;
		}
	}
	else
	if(n == 1) {
		ink = palette.At(params[0]);
		ink.a = 0xFF;
	}
}

force_inline
void SixelStream::GetRasterInfo()
{
	LTIMING("SixelStream::GetRasterInfo");
	ReadParams(); // We don't use the raster info.
}

force_inline
void SixelStream::GetRepeatCount()
{
	LTIMING("SixelStream::GetRepeatCount");

	ReadParams();
	repeat += max(1, params[0]); // Repeat compression.
}

void SixelStream::AdjustBufferSize()
{
	if((cursor.x + repeat >= 4096) || (cursor.y + 6 >= 4096))
		throw Exc("Sixel canvas size is too big > (4096 x 4096)");
	ImageBuffer ibb(buffer.GetSize() += 512);
	Copy(ibb, Point(0, 0), buffer, buffer.GetSize());
	buffer = ibb;
	CalcYOffests();
}

force_inline
void SixelStream::PaintSixel(int c)
{
	LTIMING("SixelStream::PaintSixel");
	
	Size sz = buffer.GetSize();
	if((sz.cx < cursor.x + repeat) || (sz.cy < cursor.y))
		AdjustBufferSize();

	if(!repeat) {
		for(int n = 0; n < 6; ++n) {
			*(buffer + ((c >> n) & 1) * (coords[n] + cursor.x)) = ink;
		}
		size.cx = max(size.cx, ++cursor.x);
	}
	else {
		for(int n = 0; n < 6; ++n) {
			if(c & (1 << n)) {
				Fill(buffer + coords[n] + cursor.x, ink, repeat); // Takes advantage of CPU-intrinsics.
			}
		}
		size.cx = max(size.cx, cursor.x += repeat);
		repeat = 0;
	}
}

SixelStream::operator Image()
{
	Clear();

	LTIMING("SixelStream::Get");
		
	try {
		for(;;) {
			int c = Get() & 0x7F;
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
			case 0x18:
			case 0x1A:
			case 0x1B:
			case 0x1C:
				goto Finalize;
			case 0x7F:
				if(IsEof())
					goto Finalize;
				break;
			default:
				if(c > 0x3E)
					PaintSixel(c - 0x3F);
				break;
			}
		}
	}
	catch(const Exc& e) {
		LLOG(e);
	}
	
Finalize:
	return !IsError() ? Crop(buffer, 0, 1, size.cx, size.cy) : Image();
}
}