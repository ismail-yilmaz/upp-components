// Useful EscPainter macros.
// Taken from uppsrc/ide/LayDes package.

Color(r, g, b)
{ 
	c.r = r;
	c.g = g;
	c.b = b;
	return c;
}

Point(x, y)
{ 
	p.x = x;
	p.y = y;
	return p;
}

Size(cx, cy)
{
	sz.cx = cx;
	sz.cy = cy;
	return sz;
}

Rect(l, t, r, b)
{
	e.left = l;
	e.top = t;
	e.right = r;
	e.bottom = b;
	return e;
}

RectC(x, y, cx, cy)
{
	e.left = x;
	e.top = y;
	e.right = x + cx;
	e.bottom = y + cy;
	return e;
}

Polar(p, r, a)
{
	pt.x = p.x + r * cos(a);
	pt.y = p.y + r * sin(a);
	return pt;
}
