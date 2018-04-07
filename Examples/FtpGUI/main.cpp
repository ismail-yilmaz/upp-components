#include "FtpGUI.h"
#define TFILE <FtpGUI/FtpGui.t>
#include <Core/t.h>

void FtpGUI::Error()
{
	ErrorOK(DeQtf(browser.GetErrorDesc()));
}

void FtpGUI::Sync()
{
	connect.Enable(!IsNull(url));
	connect.SetLabel(connected ? t_("Disconnect") : t_("Connect"));
	dirup.Enable(workdir != basedir);
	upload.Enable(connected && !IsNull(workdir));
}

void FtpGUI::Action()
{
	auto f = list.Get(list.GetCursor());
	if(f.isdir) {
		if(browser.SetDir(f.name))
			LoadDirectory();
		else
			Error();
	}
	else
		Download(f.name);
}

void FtpGUI::Dirup()
{
	if(browser.DirUp()) {
		auto s = browser.GetDir();
		if(!browser.IsError()) {
			LoadDirectory();
			return;
		}
	}
	Error();
}

void FtpGUI::Connect()
{
	connected = browser.Connect(~url);
	if(connected) {
		basedir = workdir = browser.GetDir();
		if(browser.IsError())
			Error();
		LoadDirectory();
	}
	else
		Error();
}

void FtpGUI::Disconnect()
{
	if(connected) {
		browser.Disconnect();
		connected = false;
		basedir = Null;
		workdir = Null;
		list.Clear();
		Sync();
	}
}

void FtpGUI::LoadDirectory()
{
	Ftp::DirList ls;

	if(!browser.ListDir(Null, ls)) {
		Error();
		return;
	}
	
	workdir = browser.GetDir();
	if(browser.IsError()) {
		Error();
		return;
	}

	list.Clear();

	for(auto& e : ls) {
		if(e.IsSymLink())
			continue;
		list.Add(
			e.GetName(),
			e.IsFile()
				? CtrlImg::File()
				: CtrlImg::Dir(),
			StdFont().Bold(e.IsDirectory()),
			SColorText(),
			e.IsDirectory(),
			e.GetSize(),
			e.GetLastModified(),
			SLtBlue(),
			Null, Null,
			RawPickToValue(pick(e))
		);
	}
	Sync();
}

void FtpGUI::Download(const String& f)
{
	auto dir = SelectDirectory();
	if(!IsNull(dir)) {
		auto src  = UnixPath(AppendFileName(workdir, f));
		auto dest = NativePath(AppendFileName(dir, f));
		Transfer(GET, src, dest);
	}
}

void FtpGUI::Upload()
{
	auto src = SelectFileOpen("*");
	if(!IsNull(src)) {
		auto dest = UnixPath(AppendFileName(workdir, GetFileName(src)));
		Transfer(PUT, src, dest);
	}
}

void FtpGUI::Transfer(OpCode cmd, const String& src, const String& dest)
{

	Progress pi(this, src);
	pi.Create();
	
	browser.WhenProgress = [&pi] (int64 done, int64 total) {
		pi.SetText(Format(t_("%1:s of %2:s is transferred"),
			FormatFileSize(done),
			FormatFileSize(total)));
		return pi.SetCanceled(int(done), int(total));
	};
	
	switch(cmd) {
		case GET: {
			pi.Title(t_("Downloading ") << GetFileName(src));
			auto data = browser.Get(src);
			if(browser.IsError())
				Error();
			else
			if(!SaveFile(dest, data))
				ErrorOK(t_("Unable to save file."));
			break;
		}
		case PUT: {
			FileIn fi(src);
			if(!fi)
				ErrorOK(t_("Unable to load file."));
			pi.Title(t_("Uploading ") << GetFileName(src));
			if(!browser.Put(fi, dest))
				Error();
			break;
		}
	}
	browser.WhenProgress = Null;
}

void FtpGUI::Rename()
{
	auto oldname = list.GetCurrentName();
	if(IsNull(oldname))
		return;
	String newname = oldname;
	if(!EditTextNotNull(newname, t_("Rename file"), t_("File name")) ||
		PromptYesNo(DeQtf(Format(t_("Do yo really want to rename '%s' to '%s'?"), oldname, newname))) == IDNO)
		return;
	if(!browser.Rename(list.GetCurrentName(), newname))
		Error();
	else
		LoadDirectory();
}

void FtpGUI::Delete()
{
	auto f = list.Get(list.GetCursor());
	auto path = f.name;
	if(IsNull(path) || PromptYesNo(DeQtf(Format(t_("Do yo really want to delete '%s'?"), path))) == IDNO)
		return;
	f.isdir
		? browser.RemoveDir(path)
		: browser.Delete(path);
	if(browser.IsError())
		Error();
	else
		LoadDirectory();
}

void FtpGUI::MakeDir()
{
	String newdir;
	if(!EditTextNotNull(newdir, t_("New Directory"), t_("Name")))
		return;
	if(!browser.MakeDir(newdir))
		Error();
	else
		LoadDirectory();
}

void FtpGUI::Menu(Bar& bar)
{
	if(!list.IsCursor() || !connected)
		return;
	
	auto f = list.Get(list.GetCursor());
	
	bar.Add(f.isdir ? t_("Browse") : t_("Download"), THISFN(Action));
	bar.Separator();
	bar.Add(t_("New Folder"), THISFN(MakeDir));
	bar.Add(t_("Rename"), THISFN(Rename));
	bar.Add(t_("Delete"), THISFN(Delete));
}

void FtpGUI::Serialize(Stream& s)
{
	s % url;
	if(s.IsLoading())
		Sync();
}

FtpGUI::FtpGUI()
{
	connected = false;
	CtrlLayout(*this, t_("Ftp GUI Example"));
	Sizeable().Zoomable().CenterScreen();
	Icon(CtrlImg::Share());
	dirup.SetImage(CtrlImg::DirUp());
	upload.SetImage(CtrlImg::Plus());
	dirup.Tip(t_("Go one directory up..."));
	upload.Tip(t_("Upload a file"));
	url.WhenAction      = [=]{ Sync(); };
	dirup.WhenAction    = [=]{ Dirup(); };
	upload.WhenAction   = [=]{ if(connected) Upload(); };
	list.WhenLeftDouble = [=]{ if(list.IsCursor()) Action(); };
	list.WhenBar        = THISFN(Menu);
	connect.WhenAction  = [=]{ !connected ? Connect() : Disconnect(); };
	browser.WhenWait    = [=]{ ProcessEvents(); };
	Sync();
}

GUI_APP_MAIN
{
	SetLanguage(GetSystemLNG());
	Ftp::Trace();
	FtpGUI ftpbrowser;
	LoadFromFile(ftpbrowser);
	ftpbrowser.Run();
	StoreToFile(ftpbrowser);
}
