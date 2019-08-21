#include "Page.h"

#define LLOG(x)	 // RLOG("VTPage: " << x);

namespace Upp {

// WARNING: VTPage's interface is not yet stable. It is subject to change!

void VTLine::Adjust(int cx, const VTCell& filler)
{
	if(cx < GetCount())
		wrapped = false;
	SetCount(cx, filler);
	invalid = true;
}

void VTLine::ShiftLeft(int begin, int end, int n, const VTCell& filler)
{
	Insert(end, filler, n);
	Remove(begin - 1, n);
	wrapped = false;
	invalid = true;
}

void VTLine::ShiftRight(int begin, int end, int n, const VTCell& filler)
{
	Insert(begin - 1, filler, n);
	Remove(end, n);
	wrapped = false;
	invalid = true;
}

bool VTLine::Fill(int begin, int end, const VTCell& filler, dword flags)
{
	int n = GetCount();
	begin = clamp(begin, 1, n);
	end   = clamp(end,   1, n);

	for(int i = begin; i <= end; i++)
		At(i - 1).Fill(filler, flags);

	NotSpecial();
	
	bool b = begin != end;
	if(b) Invalidate();
	return b;
}

void VTLine::SetSpecial(dword id_, word row_) const
{
	special = true;
	invalid = true;
	id  = id_;
	row = row_;
}

void VTLine::NotSpecial() const
{
	special = false;
	invalid = true;
	id  = 0;
	row = 0;
}

VTLine::VTLine()
{
	special = false;
	invalid = true;
	wrapped = false;
	id  = 0;
	row = 0;
}

VTPage& VTPage::OriginMode(bool b)
{
	cursor.relative = b;
	ClearEol();
	return *this;
}

VTPage& VTPage::WrapAround(bool b)
{
	cursor.wrap = b;
	ClearEol();
	return *this;
}

VTPage& VTPage::History(bool b)
{
	if(!b && !saved.IsEmpty())
		EraseHistory();
	scrollback = b;
	return *this;
}

void VTPage::SetCell(int x, int y, const VTCell& cell)
{
	VTLine& line  = lines[y - 1];
	line[x - 1]   = cell;
	line.Invalidate();
}

int VTPage::AddCell(const VTCell& cell)
{
	if(cursor.wrap && cursor.eol) {
		lines[cursor.y - 1].Wrap();
		NewLine();
	}
	SetCell(cell);
	int next = cursor.x + 1;
	if(next <= margin.right)
		MoveRight();
	else
		SetEol(true);
	return next;
}

VTPage& VTPage::InsertCell(const VTCell& cell)
{
	InsertCells(cursor.x, 1);
	AddCell(cell);
	return *this;
}

VTPage& VTPage::RepeatCell(int n)
{
	const VTCell& c = GetCell(cursor.x - 1, cursor.y);
	for(int i = 0; i < n; i++) AddCell(c);
	return *this;
}

VTPage& VTPage::NextTab(int n)
{
	for(int cnt = 0, i = cursor.x + 1; i <= margin.right && cnt < n; i++) {
		if(IsTabstop(i)) {
			MoveToColumn(GetRelX(i));
			cnt++;
		}
		else
		if(i == margin.right)
			MoveEnd();
	}
	return *this;
}

VTPage& VTPage::PrevTab(int n)
{
	for(int cnt = 0, i = cursor.x - 1; i >= margin.left && cnt < n; i--) {
		if(IsTabstop(i)) {
			MoveToColumn(GetRelX(i));
			cnt++;
		}
		else
		if(i == margin.left)
			MoveHome();
	}
	return *this;
}

VTPage& VTPage::SetTabs(int tsz)
{
	tabsize = tsz;
	tabs.Clear();
	for(int i = 1; i <= size.cx; i += tabsize)
		tabs.Set(i, true);
	tabsync = true;
	return *this;
}

void VTPage::GetTabs(Vector<int>& tabstops)
{
	for(int i = 1; i <= size.cx; i++)
		if(IsTabstop(i))
			tabstops.Add(i);
}

VTPage& VTPage::EraseLine(dword flags)
{
	FillLine(cursor.y, 1, size.cx, cellattrs, flags);
	return *this;
}

VTPage& VTPage::EraseLeft(dword flags)
{
	FillLine(cursor.y, 1, cursor.x, cellattrs, flags);
	return *this;
}

VTPage& VTPage::EraseRight(dword flags)
{
	FillLine(cursor.y, cursor.x, size.cx, cellattrs, flags);
	return *this;
}

VTPage& VTPage::ErasePage(dword flags)
{
	RectFill(GetRect(), cellattrs, flags);
	return *this;
}

VTPage& VTPage::EraseBefore(dword flags)
{
	EraseLeft(flags);
	RectFill(Rect(1, 1, size.cx, cursor.y - 1), cellattrs, flags);
	return *this;
}

VTPage& VTPage::EraseAfter(dword flags)
{
	EraseRight(flags);
	RectFill(Rect(1, cursor.y + 1, size.cx, size.cy), cellattrs, flags);
	return *this;
}

VTPage& VTPage::FillLine(int pos, const VTCell& filler, dword flags)
{
	return FillLine(cursor.y, 1, size.cx, filler, flags);
}

VTPage& VTPage::FillLine(int pos, int b, int e, const VTCell& filler, dword flags)
{
	pos = clamp(pos - 1, 0, size.cy - 1);
	if(lines[pos].Fill(b, e, filler, flags))
		ClearEol();
	return *this;
}

VTPage& VTPage::FillRect(const Rect& r, const VTCell& filler, dword flags)
{
	Rect rr = AdjustRect(r, MARGIN_VERT);
	RectFill(rr, filler, flags);
	return *this;
}

VTPage& VTPage::FillRect(const Rect& r, dword chr)
{
	VTCell filler = cellattrs;
	filler.chr = chr;
	return FillRect(r, filler, VTCell::FILL_NORMAL);
}

VTPage& VTPage::CopyRect(const Rect& r, const Point& pt)
{
	Rect rr = AdjustRect(r, MARGIN_VERT);
	Size sz = rr.GetSize() + 1;

	Buffer<VTCell> pcell(sz.cx * sz.cy);
	
	int pass = 0;
	while(pass < 2) {
		int pos = 0;
		if(pass == 1)
			rr.Set(AdjustPoint(pt, MARGIN_VERT), rr.GetSize());
		for(int i = rr.top; i <= rr.bottom; i++) {
			VTLine& line = lines[i - 1];
			for(int j = rr.left; j <= rr.right; j++) {
				VTCell& a  = line[j - 1];
				VTCell& b  = *(pcell + pos);
				if(pass == 0)
					b = a;
				else {
					a = b;
				}
				pos++;
			}
			if(pass == 1)
				line.Invalidate();
		}
		pass++;
	}
	return *this;
}

VTPage& VTPage::FillStream(const Rect& r, const VTCell& filler, dword flags)
{
	Rect rr = AdjustRect(r, MARGIN_VERT);
	Rect pr = GetRelRect();
	for(int i = rr.top; i <= rr.bottom; i++) {
		if(rr.top == rr.bottom)
			FillLine(i, rr.left, rr.right, filler, flags);
		else {
			if(i == rr.top)
				FillLine(i, rr.left, pr.right, filler, flags);
			else
			if(i == rr.bottom)
				FillLine(i, pr.left, rr.right, filler, flags);
			else
				FillLine(i, pr.left, pr.right, filler, flags);
		}
	}
	return *this;
}

VTPage& VTPage::FillStream(const Rect& r, dword chr)
{
	VTCell filler = cellattrs;
	filler.chr = chr;
	return FillStream(r, filler, VTCell::FILL_NORMAL);
}

void VTPage::RectFill(const Rect& r, const VTCell& filler, dword flags)
{
	for(int i = r.top; i <= r.bottom; i++)
		FillLine(i, r.left, r.right, filler, flags);
}

void VTPage::GetRectArea(Rect r, Event<const VTCell&, const Point&> consumer)
{
	Rect rr = AdjustRect(r, MARGIN_NONE);
	
	for(int i = rr.top; i <= rr.bottom; i++) {
		const VTLine& line = lines[i - 1];
		for(int j = rr.left; j <= rr.right; j++)
			consumer(line[j - 1], Point(j, i));
	}
}

void VTPage::GetRelRectArea(Rect r, Event<const VTCell&, const Point&> consumer)
{
	GetRectArea(AdjustRect(r, MARGIN_VERT), consumer);
}

VTPage& VTPage::SetVertMargins(int t, int b)
{
	margin.top = max(1, t);
	margin.bottom = b == 0 || b <= t || b > size.cy ? size.cy : b;
	LLOG("Vertical margins set to: " << Point(margin.top, margin.bottom));
	return MoveTopLeft();
}

VTPage& VTPage::SetHorzMargins(int l, int r)
{
	margin.left = max(1, l);
	margin.right  = r == 0 || r <= l || r > size.cx ? size.cx : r;
	LLOG("Horizontal margins set to: " << Point(margin.left, margin.right));
	return MoveTopLeft();
}

VTPage& VTPage::SetMargins(int l, int t, int r, int b)
{
	return SetVertMargins(t, b).SetHorzMargins(l, r);
}

VTPage& VTPage::MoveHorz(int col, bool rel, bool scrl)
{
	cursor.x = AdjustCol(col, rel, scrl);
	if(scrl && col < margin.left)
		ScrollLeft(margin.left - col);
	else
	if(scrl && col > margin.right)
		ScrollRight(col - margin.right);
	else
		ClearEol();
	if(events)
		WhenCursor();
	return *this;
}

VTPage& VTPage::MoveVert(int row, bool rel, bool scrl)
{
	cursor.y = AdjustRow(row, rel, scrl);
	if(scrl && row < margin.top)
		ScrollUp(margin.top - row);
	else
	if(scrl && row > margin.bottom)
		ScrollDown(row - margin.bottom);
	else
		ClearEol();
	if(events)
		WhenCursor();
	return *this;
}

VTPage& VTPage::SpecialLines(int n, dword id)
{
	bool b = events;
	ForbidEvents();
	for(int i = 0; i < n; i++) {
		lines[cursor.y - 1].SetSpecial(id, i);
		NewLine();
	}
	PermitEvents(b);
	if(events)
		WhenScroll();
	return *this;
}

void VTPage::LineInsert(int pos, int n, const VTCell& attrs)
{
	if((n = AdjustVStep(pos, n)) > 0) {
		int scrolled = 0;
		for(int i = 0; i < n; i++) {
			VTLine& line = lines.Insert(pos - 1);
			line.Adjust(size.cx, attrs);
			lines.Remove(margin.bottom);
			scrolled++;
		}
		Invalidate(pos - 1, margin.bottom);
		if(events && scrolled > 0)
			WhenScroll();
	}
	if(n > 0)
		ClearEol();
}

void VTPage::LineRemove(int pos, int n, const VTCell& attrs)
{
	if((n = AdjustVStep(pos, n)) > 0) {
		int scrolled = 0;
		for(int i = 0; i < n; i++) {
			VTLine& line = lines.Insert(margin.bottom);
			line.Adjust(size.cx, attrs);
			if(scrollback && GetRelY(pos) == 1)
				AddToHistory(pos);
			lines.Remove(pos - 1);
			scrolled++;
		}
		Invalidate(pos - 1, margin.bottom);
		if(events && scrolled > 0)
			WhenScroll();
	}
	if(n > 0)
		ClearEol();
}

void VTPage::CellInsert(int pos, int n, const VTCell& attrs, bool pan)
{
	if((n = AdjustHStep(pos, n)) > 0) {
		if(pan) {
			for(int y = margin.top; y <= margin.bottom; y++)
				lines[y - 1].ShiftRight(pos, margin.right, n, attrs);
		}
		else
			lines[cursor.y - 1].ShiftRight(pos, margin.right, n, attrs);
	}
	if(n > 0)
		ClearEol();
}

void VTPage::CellRemove(int pos, int n, const VTCell& attrs, bool pan)
{
	if((n = AdjustHStep(pos, n)) > 0) {
		if(pan)
			for(int y = margin.top; y <= margin.bottom; y++)
				lines[y - 1].ShiftLeft(pos, margin.right, n, attrs);
		else
			lines[cursor.y - 1].ShiftLeft(pos, margin.right, n, attrs);
	}
	if(n > 0)
		ClearEol();
}

int VTPage::AdjustCol(int& col, bool rel, bool scrl) const
{
	Rect r = cursor.relative || (rel && scrl) ? margin : Rect(1, 1, size.cx, size.cy);
	return clamp(rel ? (col += cursor.x) : (r.left - 1 + col), r.left, r.right);
}

int VTPage::AdjustRow(int& row, bool rel, bool scrl) const
{
	Rect r = cursor.relative || (rel && scrl) ? margin : Rect(1, 1, size.cx, size.cy);
	return clamp(rel ? (row += cursor.y) : (r.top - 1 + row), r.top, r.bottom);
}

Point VTPage::AdjustPoint(Point pt, dword delta)
{
	if(IsNull(pt)) {
		pt.x = 1;
		pt.y = 1;
	}
	
	if(cursor.relative) {
		pt.x += margin.left - 1;
		pt.y += margin.top  - 1;
	}
	return pt;
}

Rect VTPage::AdjustRect(Rect r, dword delta)
{
	r = Nvl(r, GetRect());
	return Rect(AdjustPoint(r.TopLeft(), delta), AdjustPoint(r.BottomRight(), delta));
}

Point VTPage::GetPos() const
{
	Point pt = cursor;
	pt.y += saved.GetCount();
	return pt;
}

Point VTPage::GetRelPos() const
{
	Point pt = cursor;
	if(cursor.relative) {
		pt.x += 1 - margin.left;
		pt.y += 1 - margin.top;
	}
	return pt;
}

const VTLine& VTPage::GetLine(int i) const
{
	ASSERT(i >= 0 && i < GetLineCount());

	int slen = saved.GetCount();
	int llen = lines.GetCount();
	if(slen && i < slen)
		return saved[i];
	else
	if(llen && i >= slen)
		return lines[i - slen];
	
	static VTLine *l;
	ONCELOCK {
		static VTLine empty;
		l = &empty;
	}
	return *l;
}

void VTPage::Invalidate(int begin, int end)
{
	int b = min(begin, end);
	int e = max(begin, end);
	for(int i = b; i < e; i++)
		lines[i].Invalidate();
}

VTPage& VTPage::SetSize(Size sz)
{
	Size prevsize = size;
	size = Nvl(sz, Size(MINCOL, MINROW));
	if(prevsize != size)
		margin.Set(1, 1, size.cx, size.cy);
	if(lines.IsEmpty())
		cursor.Clear();
	if(HasHistory()) {
		if(prevsize.cy < size.cy)
			UnwindHistory(prevsize);
		else
		if(prevsize.cy > size.cy)
			RewindHistory(prevsize);
	}
	lines.SetCount(size.cy);
	for(VTLine& line : lines)
		line.Adjust(size.cx, cellattrs);
	if(tabsync)
		SetTabs(tabsize);
	return MoveTo(cursor);
}

void VTPage::UnwindHistory(const Size& prevsize)
{
	int delta =  min(size.cy - prevsize.cy, saved.GetCount());
	while(delta-- > 0) {
		lines.Insert(0, pick(saved.Top()));
		saved.Drop();
		cursor.y++;
	}
}

void VTPage::RewindHistory(const Size& prevsize)
{
	int delta = min(cursor.y - size.cy, lines.GetCount());
	while(delta-- > 0) {
		saved.Add(pick(lines[0]));
		lines.Remove(0, 1);
	}
}

bool VTPage::AddToHistory(int pos)
{
	if(margin != Rect(1, 1, size.cx, size.cy))
		return false;
	AdjustHistorySize();
	saved.AddPick(pick(lines[pos - 1]));
	return true;
}

void VTPage::AdjustHistorySize()
{
	int count = saved.GetCount();
	if(count > historysize)
		saved.Remove(0, count - historysize);
}

void VTPage::Shrink()
{
	lines.Shrink();
	saved.Shrink();
}

VTPage& VTPage::Reset()
{
	cursor = Null;
	backup = Null;
	tabsync = false;
	events  = true;
	SetTabs(tabsize);
	SetSize(size);
	WrapAround(false);
	OriginMode(false);
	ErasePage();
	EraseHistory();
	MoveTopLeft();
	return *this;
}

void VTPage::Serialize(Stream& s)
{
	int version = 1;
	s / version;
	if(version >= 1) {
		s % tabsize;
		s % scrollback;
		s % historysize;
	}
}

VTPage::VTPage()
{
	size = Null;
	tabsize = 8;
	historysize = 1024;
	scrollback  = false;
	Reset();
}
}