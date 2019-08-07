#include "Sixel.h"

// TODO: Needs optimixation.

#define LLOG(x)		// RLOG("SixelRenderer: " << x)
#define LTIMING(x)	// RTIMING(x)

namespace Upp {

SixelRenderer::SixelRenderer(const String& data)
: sixelstream(data)
{
	Clear();
}

SixelRenderer::SixelRenderer(const String& data, Size sz)
: sixelstream(data)
{
	Clear();
	SetSize(sz);
}

SixelRenderer::SixelRenderer(const String& data, Info _info)
: sixelstream(data)
{
	Clear();
	info = _info;
	SetSize(info.size);
}

void SixelRenderer::Clear()
{
	info   = Info();
	ink    = SColorText;
	repeatcount = 1;
}

SixelRenderer& SixelRenderer::SetSize(Size sz)
{
	buffer.Create((info.size = sz));
	return *this;
}

Image SixelRenderer::Get()
{
	cursor = Point(0, 0);

	FillBuffer();

	LTIMING("SixelRenderer::Get");
	
	while(!sixelstream.IsEof()) {
		int c = sixelstream.Get();
		switch(c) {
		case '$':
			cursor.x = 0;
			break;
		case '-':
			cursor.x  = 0;
			cursor.y += 6;
			break;
		case '#':
			SetColors();
			break;
		case '!':
			SetRepeatCount();
			break;
		case '\"':
			SetRasterInfo();
			break;
		case 0x3F ... 0x7E:
			DrawSixel(c - 0x3F);
			break;
		}
	}

	Premultiply(buffer);
	Image img = buffer;
	return pick(img);
}

Color HslColorf(double h, double s, double l)
{
	// This function is based on an HSL->RGB conversion formula which can be found on Wikipedia:
	// https://en.wikipedia.org/wiki/HSL_and_HSV?oldformat=true#HSL_to_RGB_alternative
	
	auto fn = [=] (double n) -> double
	{
		double a = s * min(l, 1.0 - l);
		double k = modulo(h / (30.0 / 360.0) + n, 12.0);
		return l - a * max(min(k - 3.0, 9.0 - k * 1.0), -1.0);
	};

	return Color(fn(0) * 255, fn(8) * 255, fn(4) * 255);
}

void SixelRenderer::SetColors()
{
ONCELOCK {
	colortable = {
		{ 0,  Black()    },
		{ 1,  Blue()     },
		{ 2,  Red()      },
		{ 3,  Green()    },
		{ 4,  Magenta()  },
		{ 5,  Cyan()     },
		{ 6,  Yellow()   },
		{ 7,  White()    },
		{ 8,  Gray()     },
		{ 9,  LtBlue()   },
		{ 10, LtRed()    },
		{ 11, LtGreen()  },
		{ 12, LtMagenta()},
		{ 13, LtCyan()   },
		{ 14, LtYellow() },
		{ 15, White()    },
	};
}
	Vector<int> params;
	
	GetNumericParameters(params, ';');
	
	if(params.GetCount() == 5) {
		int index = params[0];
		if(params[1] == 1) {	// HSL
			double h = params[2] * 1.0;
			double l = params[3] / 100.0;
			double s = params[4] / 100.0;
			SetColorRegister(index, HslColorf(h, s, l));
		}
		else
		if(params[1] == 2) {	// RGB
			byte r = params[2] * 255 / 100;
			byte g = params[3] * 255 / 100;
			byte b = params[4] * 255 / 100;
			SetColorRegister(index, Color(r, g, b));
		}
	}
	else
	if(params.GetCount() == 1) {
		Color *c = colortable.FindPtr(params[0]);
		if(c) ink = *c;
	}
}

void SixelRenderer::SetColorRegister(int i, const Color& c)
{
	int ii = colortable.Find(i);
	if(ii >= 0)
		colortable.Unlink(i);
	colortable.Put(i, c);
}

void SixelRenderer::SetRasterInfo()
{
	Vector<int> params;
	
	GetNumericParameters(params, ';');

	if(params.GetCount() == 4) {
		info.aspectratio = max(params[0] / params[1], 1);
		Size size = Size(params[2], params[3]);
		if(GetSize() != size) {
			SetSize(size);
			FillBuffer();
		}
	}
}

void SixelRenderer::SetRepeatCount()
{
	Vector<int> params;
	
	GetNumericParameters(params);
	
	if(params.GetCount() == 1)
		repeatcount = params[0];
}

void SixelRenderer::DrawSixel(int c)
{
	if(buffer.IsEmpty()) {
		SetSize(Size(640, 480));
		FillBuffer();
	}
	
	repeatcount = max(1, repeatcount);
	int ratio   = max(info.aspectratio, 1);
	
	Rect r = GetSize();

	while(repeatcount-- > 0) {
		for(int i = 0; i < ratio && r. Contains(cursor); i++) {
			for(int j = 0; j < 6; j++) {
				if(cursor.y + j < r.bottom) {
					if(c & (1 << j))
						buffer[cursor.y + j][cursor.x] = ink;
				}
			}
			cursor.x++;
		}
	}
}

void SixelRenderer::GetNumericParameters(Vector<int>& v, int delimiter)
{
	String number;
	
	while(!sixelstream.IsEof()) {
		int c = sixelstream.Peek();
		if(c >= '0' && '9' >= c) {
			number.Cat(sixelstream.Get());
		}
		else {
			v.Add(Nvl(StrInt(number), 0));
			if(c != delimiter || IsNull(delimiter))
				break;
			number.Clear();
			sixelstream.Skip(1);
		}
	}
}

void SixelRenderer::FillBuffer()
{
	if(info.nohole)
		Fill(buffer, info.size, paper);
}

Image RenderSixelImage(const String& sixeldata, const Size& sizehint, Color paper)
{
	VTInStream vts;
	Image img;
	
	vts.WhenDcs = [&img, &sizehint, &paper](const VTInStream::Sequence& seq)
	{
		bool nohole = seq.GetInt(2, 0) != 1;
		int  grid   = seq.GetInt(3, 0); // Omitted.
		int  ratio = 1;
	
		switch(seq.GetInt(1, 1)) {
		case 0 ... 1:
		case 5 ... 6:
			ratio = 2;
			break;
		case 3 ... 4:
			ratio = 3;
			break;
		default:
			ratio = 1;
			break;
		}

		img = SixelRenderer(seq.payload)
				.SetSize(sizehint)
					.SetAspectRatio(ratio)
						.SetPaper(paper)
							.NoColorHole(nohole);
	};

	vts.Parse(sixeldata, false);
	return pick(img);
}
}