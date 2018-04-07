#include "SFtpGUI.h"

#define TFILE <SFtpGUI/SFtpGUI.t>
#include <Core/t.h>

void SFtpGUI::Error()
{
	ErrorOK(DeQtf(browser->GetErrorDesc()));
}

void SFtpGUI::Sync()
{
	connect.Enable(!IsNull(url));
	connect.SetLabel(connected ? t_("Disconnect") : t_("Connect"));
	dirup.Enable(workdir != basedir);
	upload.Enable(connected && !IsNull(workdir));
}

void SFtpGUI::Dirup()
{
	String path;
	auto b = browser->RealizePath(workdir + "/..", path);
	if(b) {
		 workdir = path;
		 LoadDirectory();
	}
	else
		Error();
}

void SFtpGUI::Action()
{
	auto f = list.Get(list.GetCursor());
	if(f.isdir) {
		workdir = f.data;
		LoadDirectory();
	}
	else
		Download(f.data);
}

void SFtpGUI::Connect()
{
	session.Timeout(60000);
	connected = session.Connect(~url);
	if(connected) {
		browser.Attach(new SFtp(session));
		basedir = browser->GetCurrentDir();
		if(!browser->IsError()) {
			workdir = basedir;
			LoadDirectory();
		}
		else
			Error();
	}
	else
		ErrorOK(DeQtf(session.GetErrorDesc()));
}

void SFtpGUI::Disconnect()
{
	if(connected) {
		browser.Clear();
		session.Disconnect();
		connected = false;
		basedir = Null;
		workdir = Null;
		list.Clear();
		Sync();
	}
}

void SFtpGUI::LoadDirectory()
{
	SFtp::DirList ls;

	if(!browser->ListDir(workdir, ls)) {
		Error();
		return;
	}

	list.Clear();

	for(auto& e : ls) {
		if(e.IsSymLink()      ||
		   e.GetName() == "." ||
		   e.GetName() == "..")
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
			UnixPath(AppendFileName(workdir, e.GetName()))
		);
	}
	Sync();
}

void SFtpGUI::Upload()
{
	auto src = SelectFileOpen("*");
	if(!IsNull(src)) {
		auto dest = UnixPath(AppendFileName(workdir, GetFileName(src)));
		Transfer(PUT, src, dest);
	}
}

void SFtpGUI::Download(const String& f)
{
	auto dir = SelectDirectory();
	if(!IsNull(dir)) {
		auto name = GetFileName(f);
		auto dest = NativePath(AppendFileName(dir, name));
		Transfer(GET, f, dest);
	}
}

void SFtpGUI::Transfer(OpCode cmd, const String& src, const String& dest)
{
	Progress pi(this, src);
	pi.Create();

	auto progress = [&pi] (int64 done, int64 total) {
		pi.SetText(Format(t_("%1:s of %2:s is transferred"),
			FormatFileSize(done),
			FormatFileSize(total)));
		return pi.SetCanceled(int(done), int(total));
	};

	switch(cmd) {
		case GET: {
			pi.Title(t_("Downloading ") << GetFileName(src));
			auto data = browser->Get(src, pick(progress));
			if(browser->IsError())
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
			if(!browser->Put(fi, dest, pick(progress)))
				Error();
			break;
		}
	}
}

void SFtpGUI::Rename()
{
	auto path = list.Get(list.GetCursor()).data.To<String>();
	if(IsNull(path))
		return;
	auto oldname = GetFileName(path);
	auto newname = oldname;
	if(!EditTextNotNull(newname, t_("Rename file"), t_("File name")) ||
		PromptYesNo(DeQtf(Format(t_("Do yo really want to rename '%s' to '%s'?"), oldname, newname))) == IDNO)
		return;
	auto newpath = UnixPath(GetFileFolder(path));
	if(!browser->Rename(path, UnixPath(AppendFileName(newpath, newname))))
		Error();
	else
		LoadDirectory();
}

void SFtpGUI::Delete()
{
	auto f = list.Get(list.GetCursor());
	auto path = f.data;
	if(IsNull(path) || PromptYesNo(DeQtf(Format(t_("Do yo really want to delete '%s'?"), path))) == IDNO)
		return;
	f.isdir
		? browser->RemoveDir(path)
		: browser->Delete(path);
	if(browser->IsError())
		Error();
	else
		LoadDirectory();
}

void SFtpGUI::MakeDir()
{
	String newdir;
	if(!EditTextNotNull(newdir, t_("New Directory"), t_("Name")))
		return;
	if(!browser->MakeDir(UnixPath(AppendFileName(workdir, newdir)), SFtp::IRWXU))
		Error();
	else
		LoadDirectory();
}

void SFtpGUI::Menu(Bar& bar)
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

void SFtpGUI::Serialize(Stream& s)
{
	s % url;
	if(s.IsLoading())
		Sync();
}

SFtpGUI::SFtpGUI()
{
	connected = false;
	CtrlLayout(*this, t_("SFtp GUI Example"));
	Sizeable().Zoomable().CenterScreen();
	Icon(CtrlImg::Share());
	dirup.SetImage(CtrlImg::DirUp());
	upload.SetImage(CtrlImg::Plus());
	dirup.Tip(t_("Go one directory up..."));
	upload.Tip(t_("Upload a file"));
	url.WhenAction      = [=]{ Sync(); };
	dirup.WhenAction    = [=]{ Dirup(); };
	upload.WhenAction	= [=]{ if(connected) Upload(); };
	list.WhenLeftDouble = [=]{ if(list.IsCursor()) Action(); };
	list.WhenBar        = THISFN(Menu);
	connect.WhenAction  = [=]{ !connected ? Connect() : Disconnect(); };
	session.WhenWait    = [=]{ ProcessEvents(); };
	Sync();
}

GUI_APP_MAIN
{
	SetLanguage(GetSystemLNG());
	Ssh::Trace();
	SFtpGUI sftpbrowser;
	LoadFromFile(sftpbrowser);
	sftpbrowser.Run();
	StoreToFile(sftpbrowser);
}