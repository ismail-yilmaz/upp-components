#include "SixelRaster.h"

#define LLOG(x)		 // RLOG("SixelRaster::Data " << x)
#define LTIMING(x)	 // RTIMING(x)

namespace Upp {

class SixelRaster::Data : NoCopy {
public:
    Data(Stream& sdata);

	Data&			Background(RGBA c)				{ paper = c; return *this;  }
	Data&			NoHole(bool b = true)			{ nohole = b; return *this; }
	
    Image           Get();
    operator        Image()                         { return Get(); }
    Size            GetSize() const                 { return size;  }
    int             GetRatio() const                { return 1; }
    
private:
	void			CheckHeader();
    void            Clear();
    inline void     Return();
    inline void     LineFeed();
    void            SetPalette();
    void            GetRasterInfo();
    void            GetRepeatCount();
    int				ReadParams();
    void			CalcYOffests();
    void            AdjustBufferSize();
    void            PaintSixel(int c);

private:
    Stream&         stream;
    ImageBuffer		buffer;
    Vector<RGBA>    palette;
    RGBA            ink;
    RGBA			paper;
    int             repeat;
    int				params[8];
    int				coords[6];
    Size            size;
    Point           cursor;
    bool            nohole;
};


SixelRaster::Data::Data(Stream& sixelstream)
: stream(sixelstream)
, nohole(true)
{
}

void SixelRaster::Data::Clear()
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
	cursor  = Point(0, 0);
	repeat  = 0;
	ink     = palette[0];
	ink.a   = 0xff;
	paper   = ink;

	Zero(params);
	
	buffer.Create(1024, 1024);
	Fill(buffer, buffer.GetSize(), nohole ? paper : RGBAZero());
	
	CalcYOffests();
}

force_inline
int SixelRaster::Data::ReadParams()
{
	LTIMING("Data::ReadParams");
	
	Zero(params);
	int c = 0, n = 0, i = 0;
	for(;;) {
		while((c = stream.Peek()) > 0x2F && c < 0x3A)
			n = n * 10 + (stream.Get() - 0x30);
		params[i++ & 7] = n;
		if(c != ';')
			break;
		n = 0;
		stream.Get(); // faster than Stream::Skip(1)
	}
	return i;
}

force_inline
void SixelRaster::Data::CalcYOffests()
{
	int w = buffer.GetWidth();
	for(int i = 0, y = cursor.y * w; i < 6; i++, y += w)
		coords[i] = y;
}

force_inline
void SixelRaster::Data::Return()
{
	cursor.x = 0;
}

force_inline
void SixelRaster::Data::LineFeed()
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
void SixelRaster::Data::SetPalette()
{
	LTIMING("Data::SetPalette");
	
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
void SixelRaster::Data::GetRasterInfo()
{
	LTIMING("Data::GetRasterInfo");
	(void) ReadParams(); // We don't use the raster info.
}

force_inline
void SixelRaster::Data::GetRepeatCount()
{
	LTIMING("Data::GetRepeatCount");

	(void) ReadParams();
	repeat += max(1, params[0]); // Repeat compression.
}

void SixelRaster::Data::AdjustBufferSize()
{
	if((cursor.x + repeat >= 4096) || (cursor.y + 6 >= 4096))
		throw Exc("Sixel canvas size is too big > (4096 x 4096)");
	ImageBuffer ibb(buffer.GetSize() += 512);
	Copy(ibb, Point(0, 0), buffer, buffer.GetSize());
	buffer = ibb;
	CalcYOffests();
}

force_inline
void SixelRaster::Data::PaintSixel(int c)
{
	LTIMING("Data::PaintSixel");
	
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

void SixelRaster::Data::CheckHeader()
{
	while(!stream.IsEof()) {
		int c = stream.Get();
		if(c == 0x90 || (c == 0x1B && stream.Get() == 0x50)) {
			int n = ReadParams();
			if(stream.Get() != 'q') {
				stream.SetError();
				break;
			}
			if(n >= 2)
				nohole = params[1] != 1;
			return;
		}
	}
	throw Exc("SixelRaster: Invalid sixel stream");
}

Image SixelRaster::Data::Get()
{
	Clear();

	LTIMING("Data::Get");
		
	try {
		CheckHeader();
		for(;;) {
			int c = stream.Get() & 0x7F;
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
			case -1: // eof or err
				goto Finalize;
			default:
				if(c > 0x3E && c < 0x7F)
					PaintSixel(c - 0x3F);
				break;
			}
		}
	}
	catch(const Exc& e) {
		LLOG(e);
	}
	
Finalize:
	return !stream.IsError() ? Crop(buffer, size) : Image();
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

	img = Data(stream).Get();

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