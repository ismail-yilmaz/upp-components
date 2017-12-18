#include "BrowserCtrl.h"

namespace Upp {

BrowserCtrl& BrowserCtrl::ActiveDir(const String& s)
{
	auto ss = AttrText(s)
		.SetImage(CtrlImg::Dir())
		.Bold();
	auto i = dir.Find(ss);
	if(i < 0 && !s.IsEmpty()) {
		dir.Add(ss);
		dir.GoEnd();
	}
	else
	if(i >= 0) dir.SetIndex(i);
	return *this;
}

void BrowserCtrl::Sync()
{
	int rx = 8;
	if(mkdir.IsShown()) {
		mkdir.RightPosZ(rx, 24);
		rx += mkdir.GetSize().cx - 4;
	}
	if(dirup.IsShown()) {
		dirup.RightPosZ(rx, 24);
		rx += dirup.GetSize().cx - 4;
	}
	dir.HSizePosZ(84, rx);
	if(IsShown())
		Refresh();
}

BrowserCtrl& BrowserCtrl::ShowInfoPanel(bool b)
{
	filesize.Show(b);
	filename.Show(b);
	filetime.Show(b);
	filelist.VSizePosZ(28, b ? 20 : 4);
	return *this;
}

void BrowserCtrl::Action()
{
	if(IsCursor()) {
		auto& f = GetFileInfo();
		if(f.isdir) {
			ActiveDir(GetFullPath(f.name));
			Load();
		}
		else Select();
	}
}

void BrowserCtrl::Rename()
{
	if(IsCursor()) {
		if(!filelist.IsEdit()) {
			auto&  f = GetFileInfo();
			String s = f.name;
			if(EditTextNotNull(s, t_("Rename File"), t_("Name")))
				Relabel(f.name, s);
		}
		else filelist.StartEdit();
	}
}

void BrowserCtrl::MkDir()
{
	String s;
	if(EditTextNotNull(s, t_("New Directory"), t_("Name")))
		if(WhenMkDir(GetFullPath(s)))
			Reload(s);
}

void BrowserCtrl::Delete()
{
	if(IsCursor()) {
		auto& e = GetFileInfo();
		auto rc = true;
		if(ask)
			rc = PromptYesNo(DeQtf(Format(t_("Do yo really want to delete '%s'?"), e.name)));
		if(rc && WhenDelete(GetFullPath(e.name)))
			Reload();
	}
}

void BrowserCtrl::DirUp()
{
	if(WhenDirUp())
		Load();
}

bool BrowserCtrl::Load()
{
	auto rc = WhenLoad();
	if(rc) {
		dirup.Enable(GetActiveDir() != basedir);
		mkdir.Enable(!basedir.IsEmpty());
		sortbyext
		? Upp::SortByExt(filelist)
		: Upp::SortByName(filelist);
		Summary();
	}
	return rc;
}

void BrowserCtrl::Reload(const String& s)
{
	Load();
	if(!s.IsEmpty()) {
		filelist.FindSetCursor(s);
		Info();
	}
}

void BrowserCtrl::Select()
{
	if(IsCursor()) {
		selected.Clear();
		for(auto i = 0; i < filelist.GetCount(); i++) {
			if(filelist.IsSelected(i))
				selected.Add(GetFullPath(filelist[i].name));
		}
		if(!selected.IsEmpty())
			WhenSel();
	}
}

void BrowserCtrl::Info()
{
	if(IsCursor()) {
		auto& f = GetFileInfo();
		filename = " " + f.name;
		filesize = f.isdir
			? String(t_("Directory"))
			: FormatFileSize(f.length);
		filetime = Format(f.time);
	}
	else Summary();
}

void BrowserCtrl::Summary()
{
	int nd = 0, nf = 0, sz = 0;
	for(auto i = 0; i < filelist.GetCount(); i++) {
		auto& f = filelist.Get(i);
		if(f.isdir)
			nd++;
		else
		if(!f.hidden)
			nf++;
		sz += f.length;
	}
	filename = Format(t_(" %d folder(s), %d file(s) "), nd, nf);
	filesize = FormatFileSize(sz);
	filetime = "";
}

void BrowserCtrl::Relabel(const String& s1, const String& s2)
{
	if(s1 == s2)
		return;
	bool rc = true;
	if(ask)
		rc = PromptYesNo(Format(t_("Do you really want to rename '%s' to '%s'?"), s1, s2));
	if(rc && WhenRename(s1, GetFullPath(s2)))
		Reload(s2);
}

void BrowserCtrl::Drag()
{
	if(dnd)
		WhenDrag();
}

void BrowserCtrl::Drop(PasteClip& d)
{
	if(dnd && !IsDragAndDropSource()) {
		if(AcceptFiles(d)) {
			selected = GetFiles(d);
			if(!selected.IsEmpty())
				WhenDropFiles();
		}
		else WhenDrop(d);
	}
}

String BrowserCtrl::GetFullPath(const String& s)
{
	if(!IsFullPath(s))
		return AppendFileName(GetActiveDir(), s);
	return s;
}

void BrowserCtrl::Clear()
{
	dir.Clear();
	filelist.Clear();
	dirup.Disable();
	mkdir.Disable();
	~filename = Null;
	~filesize = Null;
	~filetime = Null;
	basedir	  = Null;
	path      = Null;
}


void BrowserCtrl::KillCurSel()
{
	filelist.ClearSelection();
	filelist.KillCursor();
	filelist.EndEdit();
}

void BrowserCtrl::ContextMenu(Bar& bar)
{
	if(IsCursor()) {
		if(stdmenu) {
			auto& f = GetFileInfo();
			bar.Sub((bool) filelist.GetCount(),
			    t_("Sort list"),
			    THISFN(SortMenu))
				.Image(CtrlImg::sort());
			bar.Separator();
			bar.Add(IsBaseDir(), t_("Dir up..."), THISFN(DirUp))
				.Key(K_CTRL_BACKSPACE)
				.Image(CtrlImg::DirUp());
			bar.Add(t_("Reload directory"),
				[=]{ Reload(filelist.GetCurrentName()); })
				.Key(K_F5)
				.Image(CtrlImg::Toggle());
			bar.Separator();
			bar.Add(WhenSel,
			    !f.isdir ? t_("Select") : t_("Browse..."), THISFN(Action))
				.Key(K_CTRL_S)
				.Image(!f.isdir ? CtrlImg::selection() : CtrlImg::Dir());
			bar.Separator();
			bar.Add(WhenMkDir, t_("Create directory"), THISFN(MkDir))
				.Key(K_CTRL_N)
				.Image(CtrlImg::MkDir());
			bar.Separator();
			bar.Add(WhenRename, t_("Rename"), THISFN(Rename))
				.Key(K_CTRL_R);
			bar.Add(WhenDelete, t_("Delete"), THISFN(Delete))
				.Key(K_CTRL_DELETE)
				.Image(CtrlImg::Remove());
			if(WhenBar)
				bar.Separator();
		}
		WhenBar(bar);
	}
}

void BrowserCtrl::SortMenu(Bar& bar)
{
	bar.Add(t_("by name"), [=] { sortbyext = false; Upp::SortByName(filelist); })
		.Check(!sortbyext);
	bar.Add(t_("by extension"), [=] { sortbyext = true; Upp::SortByExt(filelist); })
		.Check(sortbyext);
}

void BrowserCtrl::Serialize(Stream& s)
{
	filelist.SerializeSettings(s);
	s % dnd;
	s % ask;
	s % sortbyext;
	s % stdmenu;
}

BrowserCtrl::BrowserCtrl()
{
	CtrlLayout(*this);
	dirup.Disable();
	dirup.SetImage(CtrlImg::DirUp());
	dirup.WhenAction = THISFN(DirUp);
	dirup.Tip(t_("Dir up"));
	mkdir.Disable();
	mkdir.SetImage(CtrlImg::MkDir());
	mkdir.WhenAction = THISFN(MkDir);
	mkdir.Tip(t_("Create directory"));
	dnd       = false;
	ask       = true;
	sortbyext = false;
	stdmenu   = false;
	filelist.Columns(2);
	filelist.WhenBar = THISFN(ContextMenu);
	filelist.WhenLeftDouble = THISFN(Action);
	filelist.WhenLeftClick  = THISFN(Info);
	filelist.WhenRename = THISFN(Relabel);
	filelist.WhenDrag = THISFN(Drag);
	filelist.WhenDrop = THISFN(Drop);
	dir.WhenAction = [=] { Load(); };
	Sync();
}
}
