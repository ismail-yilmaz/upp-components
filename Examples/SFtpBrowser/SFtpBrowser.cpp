#include "SFtpBrowser.h"

#define TFILE <SFtpBrowser/SFtpBrowser.t>
#include <Core/t.h>

#define IMAGECLASS Images
#define IMAGEFILE <SFtpBrowser/SFtpBrowser.iml>
#include <Draw/iml_source.h>

void SFtpBrowser::Workdir(const String& s)
{
	workdir = s;
	Image ico = workdir == basedir ? CtrlImg::Home() : CtrlImg::Dir();
	auto ss = AttrText(s)
		.SetImage(ico)
		.Bold();
	auto i = dir.Find(ss);
	if(i < 0 && !s.IsEmpty()) {
		dir.Add(ss);
		dir.GoEnd();
	}
	else
	if(i >= 0)
		dir.SetIndex(i);
}

void SFtpBrowser::Close()
{
	if(browser && browser->InProgress())
		browser->Abort();
	if(session.InProgress())
		session.Abort();
	Disconnect();
	TopWindow::Close();
}

void SFtpBrowser::Sync()
{
	Ssh::Trace(settings.log);
	dir.Enable(connected);
	mkdir.Enable(connected);
	dirup.Enable(!basedir.IsEqual(GetWorkdir()));
	upload.Enable(connected && !IsNull(workdir));
	settings.password.Enable(!settings.keyauth);
	settings.passphrase.Enable(settings.keyauth);
	settings.prikey.Enable(settings.keyauth);
	settings.pubkey.Enable(settings.keyauth);
}

void SFtpBrowser::Info()
{
	if(list.IsCursor()) {
		const FileList::File& f = list.Get(list.GetCursor());
		filename = " " + f.name;
		filesize = f.isdir
			? String(t_("Directory"))
			: FormatFileSize(f.length);
		filetime = Format(f.time);
	}
	else Summary();
}

void SFtpBrowser::Summary()
{
	if(!connected)
		return;
	
	int nd = 0, nf = 0, sz = 0;
	for(int i = 0; i < list.GetCount(); i++) {
		const FileList::File& f = list.Get(i);
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

void SFtpBrowser::DirUp()
{
	String path;
	if(browser->RealizePath(GetWorkdir() + "/..", path)) {
		Workdir(path);
		LoadDir();
	}
	else BrowserError();
}

void SFtpBrowser::Action()
{
	if(!list.IsCursor())
		return;
	const FileList::File& f = list.Get(list.GetCursor());
	String fname = UnixPath(AppendFileName(GetWorkdir(), f.name));
	if(f.isdir) {
		Workdir(fname);
		LoadDir();
	}
	else Download(fname);
}

void SFtpBrowser::Connect()
{
	if(session.InProgress())
		return;
	
	if(settings.keyauth) {
		session.PublicKeyAuth();
		session.Keys(~settings.prikey, ~settings.pubkey, ~settings.passphrase);
	}
	else session.PasswordAuth();
	
	Progress pi(this);
	pi.SetTotal(5);
	session.WhenPhase = [&pi] (int phase) {
		switch(phase) {
		case SshSession::PHASE_DNS:
			pi.Create();
			pi.SetText(t_("Resolving name"));
			pi.SetPos(1);
			break;
		case SshSession::PHASE_CONNECTION:
			pi.SetText(t_("Connecting"));
			pi.SetPos(2);
			break;
		case SshSession::PHASE_HANDSHAKE:
			pi.SetText(t_("Starting handshake"));
			pi.SetPos(3);
			break;
		case SshSession::PHASE_AUTHORIZATION:
			pi.SetText(t_("Authenticating"));
			pi.SetPos(4);
			break;
		case SshSession::PHASE_SUCCESS:
			pi.SetText(t_("Client successfully connected to server"));
			pi.SetPos(5);
			pi.Close();
			break;
		}
	};

	session.Timeout(settings.timeout * 1000);
	if((connected = session.Connect(
		~settings.host,
		~settings.port,
		~settings.user,
		~settings.password
		))) {
		browser.Attach(new SFtp(session));
		filesystem.Mount(*browser);
		basedir = browser->GetDefaultDir();
		Workdir(basedir);
		LoadDir();
	}
	else SessionError();
}

void SFtpBrowser::Disconnect()
{
	if(!connected)
		return;
	browser.Clear();
	session.Disconnect();
	basedir = Null;
	workdir = Null;
	filename = "";
	filesize = "";
	filetime = "";
	list.Clear();
	dir.Clear();
	connected = false;
	Sync();
}

void SFtpBrowser::LoadDir()
{
	if(browser->InProgress())
		return;
	SFtp::DirList ls;
	if(!browser->ListDir(GetWorkdir(), ls)) {
		BrowserError();
		return;
	}
	list.Clear();
	for(const auto& e : ls) {
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
			SLtBlue()
		);
	}
	sortbyext ? SortByExt(list) : SortByName(list);
	Summary();
	Sync();
}

void SFtpBrowser::Upload()
{
	String src = SelectFileOpen("*");
	if(!IsNull(src)) {
		String dest = AppendFileName(workdir, GetFileName(src));
		Transfer(PUT, src, UnixPath(dest));
	}
}

void SFtpBrowser::Download(const String& src)
{
	String dest = AppendFileName(~settings.dldir, GetFileName(src));
	Transfer(GET, src, NativePath(dest));
}

void SFtpBrowser::Transfer(int opcode, const String src, const String& dest)
{
	SFtp worker(session);
	Progress pi(this, src);

	worker.WhenProgress = [&pi](int64 done, int64 total)
	{
		pi.SetText(
			Format(
				t_("%1:s of %2:s is transferred"),
				FormatFileSize(done),
				FormatFileSize(total)
			)
		);
		return pi.SetCanceled(int(done), int(total));
	};
	pi.Create();
	switch(opcode) {
		case GET: {
			pi.Title(t_("Downloading ") << GetFileName(src));
			FileOut fout(dest);
			if(fout && !worker.LoadFile(fout, src))
				ErrorOK(DeQtf(worker.GetErrorDesc()));
			break;
		}
		case PUT: {
			pi.Title(t_("Uploading ") << GetFileName(src));
			FileIn fin(src);
			if(fin && !worker.SaveFile(dest, fin))
				ErrorOK(DeQtf(worker.GetErrorDesc()));
			break;
		}
	}
}

void SFtpBrowser::Rename()
{
	String oldname = list.Get(list.GetCursor()).name;
	String newname = oldname;
	if(!EditTextNotNull(newname, t_("Rename file"), t_("File name")) ||
		!PromptYesNo(DeQtf(Format(t_("Do yo really want to rename '%s' to '%s'?"), oldname, newname))))
		return;
	String oldpath = AppendFileName(GetWorkdir(), oldname);
	String newpath = AppendFileName(GetWorkdir(), newname);
	if(!browser->Rename(UnixPath(oldpath), UnixPath(newpath)))
		BrowserError();
	else LoadDir();
}

void SFtpBrowser::Delete()
{
	const FileList::File& f = list.Get(list.GetCursor());
	String path = UnixPath(AppendFileName(GetWorkdir(), f.name));
	if(!PromptYesNo(DeQtf(Format(t_("Do yo really want to delete '%s'?"), path))))
		return;
	f.isdir	? browser->RemoveDir(path) : browser->Delete(path);
	if(browser->IsError())
		BrowserError();
	else LoadDir();
}

void SFtpBrowser::MakeDir()
{
	String newdir;
	if(!EditTextNotNull(newdir, t_("New Directory"), t_("Name")))
		return;
	if(!browser->MakeDir(UnixPath(AppendFileName(GetWorkdir(), newdir)), SFtp::IRWXU))
		BrowserError();
	else LoadDir();
}

void SFtpBrowser::Settings()
{
	settings.Backup();
	if(settings.ExecuteOK())
		Sync();
}

void SFtpBrowser::MainMenu(Bar& bar)
{
	bar.Sub(t_("File"), THISFN(FileMenu));
	bar.Sub(t_("Help"), THISFN(HelpMenu));
}

void SFtpBrowser::FileMenu(Bar& bar)
{
	bar.Add(!connected ? t_("Connect") : t_("Disconnect"),
			!connected ? THISFN(Connect) : THISFN(Disconnect))
		.Image(connected ? Images::Disconnect() : Images::Connect());
	bar.Separator();
	bar.Add(t_("Settings"), THISFN(Settings))
		.Image(Images::Configure());
	bar.Separator();
	bar.Add(t_("Exit"), THISFN(Close));
}

void SFtpBrowser::HelpMenu(Bar& bar)
{
	bar.Add(t_("About"), [=] {
	static const char *msg =
		"[ [ This example demonstrates the following features of the SSH "
		"package for [*^http`:`/`/www`.ultimatepp`.org^ Ultimate`+`+]:&][ &][i160;O0; GUI "
		"integration.&][i150;O0; Password and public key authentication.&][i150;O0; Basic sftp"
		" operations: file upload, download, rename, delete, mkdir.&][ &][2 &][ [1 SSH package"
		" uses ][^https`:`/`/www`.libssh2`.org`/^1 libssh2][1 , a client`-side C library "
		"implementing the SSH2 protocol.]&][ [1 SFtp browser example uses a subset of SILK "
		"icons from ][^http`:`/`/www`.famfamfam`.com`/lab`/icons`/silk`/^1 Mark James]"
		"[1 . ]&][*_@3;2 ]]";
		PromptOK(msg);
	}).Image(Images::Help());
}

void SFtpBrowser::ContextMenu(Bar& bar)
{
	if(!list.IsCursor() || !connected)
		return;
	bar.Sub(list.GetCount(), t_("Sort list"), THISFN(SortMenu))
		.Image(CtrlImg::sort());
	bar.Separator();
	const FileList::File& f = list.Get(list.GetCursor());
	bar.Add(f.isdir ? t_("Browse") : t_("Download"), THISFN(Action))
		.Image(f.isdir ? Images::Browse() : Images::Download())
		.Key(K_CTRL_A);
	bar.Separator();
	bar.Add(t_("Create a new directory"), THISFN(MakeDir))
		.Image(CtrlImg::MkDir())
		.Key(K_CTRL_N);
	bar.Separator();
	bar.Add(t_("Rename"), THISFN(Rename))
		.Image(Images::Rename())
		.Key(K_CTRL_R);
	bar.Add(t_("Delete"), THISFN(Delete))
		.Image(CtrlImg::Remove())
		.Key(K_DELETE);
}

void SFtpBrowser::SortMenu(Bar& bar)
{
	bar.Add(t_("by name"), [=] { sortbyext = false; Upp::SortByName(list); })
		.Check(!sortbyext);
	bar.Add(t_("by extension"), [=] { sortbyext = true; Upp::SortByExt(list); })
		.Check(sortbyext);
}

void SFtpBrowser::Serialize(Stream& s)
{
	int version = 1;
	s % version;
	settings.Serialize(s);
	if(s.IsLoading())
		Sync();
}

SFtpBrowser::SFtpBrowser()
{
	CtrlLayout(*this, t_("SFtp Browser Example"));
	CtrlLayoutOKCancel(settings, t_("Browser Settings"));
	Icon(CtrlImg::Share());
	Sizeable().Zoomable().CenterScreen();
	Icon(CtrlImg::Share());
	dirup.SetImage(CtrlImg::DirUp());
	mkdir.SetImage(CtrlImg::MkDir());
	upload.SetImage(CtrlImg::Plus());
	dirup.Tip(t_("Go one directory up..."));
	upload.Tip(t_("Upload a file"));
	upload << THISFN(Upload);
	dirup  << THISFN(DirUp);
	mkdir  << THISFN(MakeDir);
	list.WhenLeftDouble = THISFN(Action);
	list.WhenLeftClick  = THISFN(Info);
	list.WhenBar = THISFN(ContextMenu);
	session.WhenWait = THISFN(UpdateGui);
	dir << THISFN(LoadDir);
	settings.password.Password();
	settings.passphrase.Password();
	settings.dldir.SetData(GetTempPath());
	settings.timeout.SetData(30);
	settings.log << THISFN(Sync);
	settings.keyauth << THISFN(Sync);
	prikeysel.Attach(settings.prikey);
	pubkeysel.Attach(settings.pubkey);
	dirsel.Attach(settings.dldir);
	AddFrame(mainmenu);
	mainmenu.Set(THISFN(MainMenu));
	sortbyext = false;
	connected = false;
	Sync();
}

GUI_APP_MAIN
{
	SetLanguage(GetSystemLNG());
	Ssh::Trace();
	SFtpBrowser sftpbrowser;
	LoadFromFile(sftpbrowser);
	sftpbrowser.Run();
	StoreToFile(sftpbrowser);
}