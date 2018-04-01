#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)


bool SFtp::Cleanup(Error& e)
{
	if(!Ssh::Cleanup(e) || !IsComplexCmd())
		return false;
	ssh->ccmd = -1;
	if(sftp->handle) {
		LLOG("Cleaning up...");
		ssh->status = CLEANUP;
		Close(NULL);
	}
	return true;
}

bool SFtp::Init()
{
	if(!ssh->session)
		SetError(-1, "SSH session is invalid.");
	sftp->session = libssh2_sftp_init(ssh->session);
	if(sftp->session) {
		LLOG("Session successfully initialized.");
		return true;
	}
	if(!WouldBlock())
		SetError(-1);
	return false;
}

void SFtp::Exit()
{
	Cmd(EXIT, [=]() mutable {
		if(!sftp->session)
			return true;
		if(WouldBlock(libssh2_sftp_shutdown(sftp->session)))
			return false;
		sftp->session = NULL;
		sftp->handle = NULL;
		ssh->init = false;
		LLOG("Session deinitalized.");
		return true;
	});
}

int SFtp::FStat(SFtpHandle* handle, SFtpAttrs& a, bool set)
{
	auto rc = libssh2_sftp_fstat_ex(HANDLE(handle), &a, set);
	return WouldBlock(rc) ?  1 : rc;
}

int SFtp::LStat(const String& path, SFtpAttrs& a, int type)
{
	auto rc = libssh2_sftp_stat_ex(sftp->session, ~path, path.GetLength(), type, &a);
	return WouldBlock(rc) ?  1 : rc;
}

SFtpHandle* SFtp::Open(const String& path, dword flags, long mode)
{
	auto b = Cmd(FOPEN, [=]() mutable {
		ASSERT(sftp->session);
		sftp->handle = libssh2_sftp_open(sftp->session, path, flags, mode);
		if(!sftp->handle && !WouldBlock())
			SetError(-1);
		if(sftp->handle)
			LLOG(Format("File '%s' is successfully opened.", path));
		return sftp->handle != NULL;
	});
	return b ? sftp->handle : NULL;
}

bool SFtp::Close(SFtpHandle* handle)
{
	return Cmd(FCLOSE, [=]() mutable {
		auto rc = libssh2_sftp_close_handle(HANDLE(handle));
		if(!WouldBlock(rc) && rc != 0)
			SetError(-1, "Unable to close handle.");
		if(rc == 0) {
			LLOG("File handle freed.");
			sftp->handle = NULL;
		}
		return rc == 0;
	});
}

bool SFtp::Rename(const String& oldpath, const String& newpath)
{
	return Cmd(FRENAME, [=]() mutable {
		ASSERT(sftp->session);
		auto rc = libssh2_sftp_rename(sftp->session, oldpath, newpath);
		if(!WouldBlock(rc) && rc != 0)
			SetError(rc);
		if(rc == 0)
			LLOG(Format("'%s' is successfully renamed to '%s'", oldpath, newpath));
		return rc == 0;
	});
}

bool SFtp::Delete(const String& path)
{
	return Cmd(FDELETE, [=]() mutable {
		ASSERT(sftp->session);
		auto rc = libssh2_sftp_unlink(sftp->session, path);
		if(!WouldBlock(rc) && rc != 0)
			SetError(rc);
		if(rc == 0)
			LLOG(Format("File '%s' is successfully deleted", path));
		return rc == 0;
	});
}

bool SFtp::Sync(SFtpHandle* handle)
{
	return Cmd(FSYNC, [=]() mutable {
		auto rc = libssh2_sftp_fsync(HANDLE(handle));
		if(!WouldBlock(rc) && rc != 0)
			SetError(rc);
		if(rc == 0)
			LLOG("File successfully synchronized to disk.");
		return rc == 0;
	});
}

SFtp& SFtp::Seek(SFtpHandle* handle, int64 position)
{
	Cmd(FSEEK, [=]() mutable {
		LLOG("Seeking to offset " << position);
		libssh2_sftp_seek64(HANDLE(handle), position);
		return true;
	});
	return *this;
}

int64 SFtp::GetPos(SFtpHandle* handle)
{
	Cmd(FTELL, [=]() mutable {
		sftp->value = (int64) libssh2_sftp_tell64(HANDLE(handle));
		LLOG("File position: " << sftp->value);
		return true;
	});
	return ssh->async ? Null : pick(sftp->value);
}

bool SFtp::FRead(SFtpHandle* handle, Stream& out, int64 size, Gate<int64, int64> progress, bool str)
{
	auto peek = size > 0;
	Buffer<char> buffer(ssh->chunk_size);

	auto rc = libssh2_sftp_read(handle, buffer, ssh->chunk_size);
	if(!WouldBlock(rc) && rc < 0)
		SetError(rc);
	if(rc > 0) {
		if(peek && out.GetSize() < size) {
			out.Put64(buffer, out.GetSize() + rc > size ? size : rc);
		}
		else
		if(!peek) {
			out.Put64(buffer, rc);
		}
		if(progress(peek ? size : sftp->finfo.GetSize(), out.GetSize()))
			SetError(-1, "File read aborted.");
	}
	auto b = rc == 0 || (peek && out.GetSize() == size);
	if(b) {
		LLOG(Format("%d of %d bytes successfully read.", out.GetSize(), peek ? size : sftp->finfo.GetSize()));
		if(peek || str) sftp->value = pick(sftp->stream.GetResult());
	}
	return b;
}

bool SFtp::FWrite(SFtpHandle* handle, Stream& in, int64 size, Gate<int64, int64> progress)
{
	auto poke = size > 0;
	ssize_t write_size = poke ? size : in.GetSize();
	auto remaining = static_cast<ssize_t>(write_size - ssh->packet_length);
	auto rc = libssh2_sftp_write(HANDLE(handle), in.Get(remaining), remaining);
	if(rc < 0) {
		if(!WouldBlock(rc))
			SetError(rc);
		return false;
	}
	ssh->packet_length += rc;
	if(progress(write_size, ssh->packet_length))
		SetError(-1, "File write aborted");
	auto b = rc == 0 && remaining == 0;
	if(b) {
		LLOG(Format("%ld of %ld bytes successfully written.",
				(int64) ssh->packet_length, (int64) write_size));
		ssh->packet_length = 0;
	}
	return b;
}

bool SFtp::Get(SFtpHandle* handle, Stream& out, Gate<int64, int64> progress)
{
	sftp->finfo = Null;
	return Cmd(START, [=, &out]() mutable {
		if(OpCode() == START) {
			if(FStat(HANDLE(handle), *sftp->finfo, false) <= 0) OpCode() = FGET;
			else return false;
		}
		return FRead(HANDLE(handle), out, 0, pick(progress));
	});
}

String SFtp::Get(SFtpHandle* handle, Gate<int64, int64> progress)
{
	sftp->finfo = Null;
	sftp->stream.Create();
	Cmd(START, [=]() mutable {
		if(OpCode() == START) {
			if(FStat(HANDLE(handle), *sftp->finfo, false) <= 0) OpCode() = FGET;
			else return false;
		}
		return FRead(HANDLE(handle), sftp->stream, 0, progress, true);
	});
	return ssh->async ? Null : pick(sftp->value);
}

bool SFtp::Get(const String& path, Stream& out, Gate<int64, int64> progress)
{
	return ComplexCmd(FGET, [=, &out]() mutable {
		OpenRead(path);
		Get(NULL, out, pick(progress));
		Close(NULL);
	});
}

bool SFtp::Get(const String& path, Stream& out, int64 offset, Gate<int64, int64> progress)
{
	return ComplexCmd(FGET, [=, &out]() mutable {
		OpenRead(path);
		Seek(NULL, offset);
		Get(NULL, out, pick(progress));
		Close(NULL);
	});
}

String SFtp::Get(const String& path, Gate<int64, int64> progress)
{
	ComplexCmd(FGET, [=]() mutable {
		OpenRead(path);
		Get(NULL, pick(progress));
		Close(NULL);
	});
	return ssh->async ? Null : pick(sftp->value);
}

bool SFtp::Put(SFtpHandle* handle, Stream& in, Gate<int64, int64> progress)
{
	ssh->packet_length = 0;
	return Cmd(FPUT, [=, &in]() mutable {
		return FWrite(HANDLE(handle), in, 0, pick(progress));
	});
}

bool SFtp::Put(Stream& in, const String& path, dword flags, long mode, Gate<int64, int64> progress)
{
	return ComplexCmd(FPUT, [=, &in]() mutable {
		Open(path, flags, mode);
		Put(NULL, in, pick(progress));
		Close(NULL);
	});
}

bool SFtp::Put(Stream& in, const String& path, Gate<int64, int64> progress)
{
	return Put(in, path, CREATE | WRITE | TRUNCATE,	IWUSR | IRALL, pick(progress));
}

bool SFtp::Put(Stream& in, const String& path, int64 offset, Gate<int64, int64> progress)
{
	return ComplexCmd(FPUT, [=, &in] {
		OpenWrite(path);
		Seek(NULL, offset);
		Put(NULL, in, pick(progress));
		Close(NULL);
	});
}

bool SFtp::Append(Stream& in, const String& path, long mode, Gate<int64, int64> progress)
{
	return Put(in, path, WRITE | CREATE | APPEND, mode, pick(progress));
}

bool SFtp::Append(Stream& in, const String& path, Gate<int64, int64> progress)
{
	return Put(in, path, WRITE | CREATE | APPEND, IRALL | IWUSR, pick(progress));
}

String SFtp::Peek(const String& path, int64 offset, int64 length, Gate<int64, int64> progress)
{
	ComplexCmd(FPEEK, [=]() mutable {
		sftp->stream.Create();
		OpenRead(path);
		Seek(NULL, offset);
		Cmd(FGET, [=]{ return FRead(HANDLE(NULL), sftp->stream, length, pick(progress)); });
		Close(NULL);
	});
	return ssh->async ? Null : pick(sftp->value);
}

bool SFtp::Poke(const String& data, const String& path, int64 offset, int64 length, Gate<int64, int64> progress)
{
	return ComplexCmd(FPOKE, [=]() mutable {
		sftp->stream.Open(data);
		OpenWrite(path);
		Seek(NULL, offset);
		Cmd(FPUT, [=]{ return FWrite(HANDLE(NULL), sftp->stream, length, pick(progress)); });
		Close(NULL);
	});
}

SFtpHandle*	 SFtp::OpenDir(const String& path)
{
	auto b = Cmd(DOPEN, [=]() mutable {
		ASSERT(sftp->session);
		sftp->handle = libssh2_sftp_opendir(sftp->session, path);
		if(!sftp->handle && !WouldBlock())
			SetError(-1);
		if(sftp->handle)
			LLOG(Format("Directory '%s' is successfully opened.", path));
		return sftp->handle != NULL;
	});
	return b ? sftp->handle : NULL;
}

bool SFtp::MakeDir(const String& path, long mode)
{
	return Cmd(DMAKE, [=]() mutable {
		ASSERT(sftp->session);
		auto rc = libssh2_sftp_mkdir(sftp->session, path, mode);
		if(!WouldBlock(rc) && rc != 0)
			SetError(rc);
		if(rc == 0)
			LLOG(Format("Directory '%s' is succesfully created.", path));
		return rc == 0;
	});
}

bool SFtp::RemoveDir(const String& path)
{
	return Cmd(DDELETE, [=]() mutable {
		ASSERT(sftp->session);
		auto rc = libssh2_sftp_rmdir(sftp->session, path);
		if(!WouldBlock(rc) && rc != 0)
			SetError(rc);
		if(rc == 0)
			LLOG(Format("Directory '%s' is succesfully deleted.", path));
		return rc == 0;
	});
}

bool SFtp::ListDir(SFtpHandle* handle, DirList& list)
{
	return Cmd(DLIST, [=, &list]() mutable {
		char label[512];
		char longentry[512];
		SFtpAttrs attrs;
		int rc = libssh2_sftp_readdir_ex(
				HANDLE(handle),
				label, sizeof(label),
				longentry, sizeof(longentry),
				&attrs);
		if(rc > 0) {
			DirEntry& entry	= list.Add();
			entry.filename	= label;
			*entry.a		= attrs;
			entry.valid		= true;
//			DDUMP(entry);
			return false;
		}
		if(rc == 0) {
			LLOG(Format("Directory listing is successful. (%d entries)", list.GetCount()));
		}
		else
		if(!WouldBlock(rc))
			SetError(rc);
		return rc == 0;
	});
}

bool SFtp::ListDir(const String& path, DirList& list)
{
	return ComplexCmd(DLIST, [=, &list]() mutable {
		OpenDir(path);
		ListDir(NULL, list);
		Close(NULL);
	});
}

String SFtp::GetCurrentDir()
{
	ComplexCmd(DGET, [=]() mutable {
		SymLink(".", NULL, LIBSSH2_SFTP_REALPATH);
	});
	return ssh->async ? Null : pick(sftp->value);
}

String SFtp::GetParentDir()
{
	ComplexCmd(DGET, [=]() mutable {
		SymLink("..", NULL, LIBSSH2_SFTP_REALPATH);
	});
	return ssh->async ? Null : pick(sftp->value);
}

bool SFtp::SymLink(const String& path, String* target, int type)
{
	if(type == LIBSSH2_SFTP_SYMLINK)
		return Cmd(LINK, [=]() mutable {
			ASSERT(sftp->session);
			ASSERT(target);
			int rc = libssh2_sftp_symlink_ex(
						sftp->session,
						path,
						path.GetLength(),
						const_cast<char*>(target->Begin()),
						target->GetLength(),
						type
					);
			if(!WouldBlock(rc) && rc != 0)
				SetError(rc);
			if(rc == 0)
				LLOG(Format("Symbolic link '%s' for path '%s' is successfult created.", target, path));
			return rc == 0;
		});
	else
		return Cmd(LINK, [=]() mutable {
			ASSERT(sftp->session);
			Buffer<char> buf(512, 0);
			int rc = libssh2_sftp_symlink_ex(
						sftp->session,
						path,
						path.GetLength(),
						buf,
						512,
						type
					);
			if(!WouldBlock(rc) && rc <= 0)
				SetError(rc);
			if(rc > 0) {
				LLOG("Symbolic link operation is successful.");
				if(target)
					target->Set(buf, rc);
				else
					sftp->value = pick(String(buf, rc));
			}
			return rc > 0;
		});
}

bool SFtp::GetAttrs(SFtpHandle* handle, SFtpAttrs& attrs)
{
	return Cmd(FGETSTAT, [=, &attrs]() mutable {
		auto rc = FStat(handle, attrs, false);
		if(rc < 0)	SetError(rc);
		if(rc == 0)	LLOG("File attributes successfully retrieved.");
		return rc == 0;
	});
}

bool SFtp::GetAttrs(const String& path, SFtpAttrs& attrs)
{
	return Cmd(FGETSTAT, [=, &attrs]() mutable {
		auto rc = LStat(path, attrs, LIBSSH2_SFTP_STAT);
		if(rc < 0)	SetError(rc);
		if(rc == 0) LLOG(Format("File attributes of '%s' is successfully retrieved.", path));
		return rc == 0;
	});
}

bool SFtp::SetAttrs(SFtpHandle* handle, const SFtpAttrs& attrs)
{
	return Cmd(FSETSTAT, [=, &attrs]() mutable {
		auto rc = FStat(handle, const_cast<SFtpAttrs&>(attrs), true);
		if(rc < 0)	SetError(rc);
		if(rc == 0)	LLOG("File attributes successfully modified.");
		return rc == 0;
	});
}

bool SFtp::SetAttrs(const String& path, const SFtpAttrs& attrs)
{
	return Cmd(FSETSTAT, [=, &attrs]() mutable {
		auto rc = LStat(path,  const_cast<SFtpAttrs&>(attrs), LIBSSH2_SFTP_SETSTAT);
		if(rc < 0)	SetError(rc);
		if(rc == 0)	LLOG(Format("File attributes of '%s' is successfully modified.", path));
		return rc == 0;
	});
}

bool SFtp::QueryAttr(const String& path, int attr)
{
	return ComplexCmd(FQUERY, [=]() mutable {
		sftp->finfo = Null;
		sftp->value = Null;
		GetAttrs(path, *sftp->finfo);
		Cmd(FQUERY, [=] {
			sftp->finfo.filename = path;
			switch(attr) {
				case FILE:
					sftp->value = sftp->finfo.IsFile();
					break;
				case DIRECTORY:
					sftp->value = sftp->finfo.IsDirectory();
					break;
				case SOCKET:
					sftp->value = sftp->finfo.IsSocket();
					break;
				case SYMLINK:
					sftp->value = sftp->finfo.IsSymLink();
					break;
				case PIPE:
					sftp->value = sftp->finfo.IsPipe();
					break;
				case BLOCK:
					sftp->value = sftp->finfo.IsBlock();
					break;
				case SPECIAL:
					sftp->value = sftp->finfo.IsSpecial();
					break;
				case SIZE:
					sftp->value = sftp->finfo.GetSize();
					break;
				case LASTMODIFIED:
					sftp->value = sftp->finfo.GetLastModified();
					break;
				case LASTACCESSED:
					sftp->value = sftp->finfo.GetLastAccessed();
					break;
				case INFO:
					sftp->finfo.valid = true;
					sftp->value = RawPickToValue(pick(sftp->finfo));
					sftp->finfo = Null;
					break;
				default:
					break;
			}
			return true;
		});
	});
}

bool SFtp::ModifyAttr(const String& path, int attr, const Value& v)
{
	return ComplexCmd(FMODIFY, [=]() mutable {
		sftp->finfo = Null;
		GetAttrs(path, *sftp->finfo);
		Cmd(FMODIFY, [=]{
			auto& attrs = *sftp->finfo;
			switch(attr) {
				case SIZE:
					attrs.flags |= LIBSSH2_SFTP_ATTR_SIZE;
					attrs.filesize = v.To<int64>();
					break;
				case LASTMODIFIED:
					attrs.flags |= LIBSSH2_SFTP_ATTR_ACMODTIME;
					attrs.mtime = GetUTCSeconds(v);
					break;
				case LASTACCESSED:
					attrs.flags |= LIBSSH2_SFTP_ATTR_ACMODTIME;
					attrs.atime = GetUTCSeconds(v);
					break;
				default:
					break;
			}
			return true;
		});
		SetAttrs(path, ~sftp->finfo);
	});
}

SFtp::DirEntry SFtp::GetInfo(const String& path)
{
	QueryAttr(path, INFO);
	if(!ssh->async && !IsError()) {
		auto& e = const_cast<DirEntry&>(sftp->value.To<DirEntry>());
		return pick(e);
	}
	return Null;
}

SFtp::SFtp(SshSession& session)
: Ssh()
{
	sftp.Create();
	ssh->otype		= SFTP;
	sftp->session	= NULL;
	sftp->handle	= NULL;
	sftp->value		= Null;
	ssh->session	= session.GetHandle();
	ssh->socket		= &session.GetSocket();
	ssh->timeout	= session.GetTimeout();
	ssh->whendo     = session.WhenDo.Proxy();
}

SFtp::~SFtp()
{
	if(sftp && sftp->session) { // Picked?
		Ssh::Exit();
		Exit();
	}
}

String SFtp::DirEntry::ToString() const
{
	if(!valid) return "<N/A>";
	const char *hypen = "-", *r = "r", *w = "w", *x = "x";
	return Format("%c%c%c%c%c%c%c%c%c%c %5<d %5<d %12>d %s %s %s",
				(IsFile()
				? *hypen : (IsDirectory()
				? 'd' : (IsSymLink()
				? 'l' : (IsSocket()
				? 's' : (IsPipe()
				? 'p' : (IsBlock()
				? 'b' : (IsSpecial()
				? 'c' : 'o' ))))))),
				((a->permissions & IRUSR) ? *r : *hypen),
				((a->permissions & IWUSR) ? *w : *hypen),
				((a->permissions & IXUSR) ? *x : *hypen),
				((a->permissions & IRGRP) ? *r : *hypen),
				((a->permissions & IWGRP) ? *w : *hypen),
				((a->permissions & IXGRP) ? *x : *hypen),
				((a->permissions & IROTH) ? *r : *hypen),
				((a->permissions & IWOTH) ? *w : *hypen),
				((a->permissions & IXOTH) ? *x : *hypen),
				GetUid(),
				GetGid(),
				GetSize(),
				AsString(GetLastModified()),
				AsString(GetLastAccessed()),
				GetName());
}

String SFtp::DirEntry::ToXml() const
{
	if(!valid) return XmlTag("<N/A>").Text("<N/A>");
	const char *hypen = "-", *r = "r", *w = "w", *x = "x";
	return XmlTag("sftp:direntry")
			("type", (IsFile()
				? "file" : (IsDirectory()
				? "directory" : (IsSymLink()
				? "symlink" : (IsSocket()
				? "socket" : (IsPipe()
				? "pipe" : (IsBlock()
				? "block-special" : (IsSpecial()
				? "character-special" : "other")
			)))))))
			("uid", AsString(GetUid()))
			("gid", AsString(GetGid()))
			("size", AsString(GetSize()))
			("modified", AsString(GetLastModified()))
			("accessed", AsString(GetLastAccessed()))
			("permissions", Format("%c%c%c%c%c%c%c%c%c",
				((a->permissions & IRUSR) ? *r : *hypen),
				((a->permissions & IWUSR) ? *w : *hypen),
				((a->permissions & IXUSR) ? *x : *hypen),
				((a->permissions & IRGRP) ? *r : *hypen),
				((a->permissions & IWGRP) ? *w : *hypen),
				((a->permissions & IXGRP) ? *x : *hypen),
				((a->permissions & IROTH) ? *r : *hypen),
				((a->permissions & IWOTH) ? *w : *hypen),
				((a->permissions & IXOTH) ? *x : *hypen)
			))
			.Text(GetName());
}

bool SFtp::DirEntry::CanMode(dword u, dword g, dword o) const
{
	return a->flags & LIBSSH2_SFTP_ATTR_PERMISSIONS &&
		   a->permissions & o ||
		   a->permissions & g ||
		   a->permissions & u;
}

void SFtp::DirEntry::Zero()
{
	a.Create();
	Upp::Zero(*a);
	valid = false;
}

SFtp::DirEntry::DirEntry(const String& path)
: DirEntry()
{
	filename = path;
}

SFtp::DirEntry::DirEntry(const String& path, const SFtpAttrs& attrs)
: DirEntry()
{
	filename = path;
	*a = attrs;
}
}
