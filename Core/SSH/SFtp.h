class SFtp : public Ssh {
public:
  enum Flags {
        READ        = LIBSSH2_FXF_READ,
        WRITE       = LIBSSH2_FXF_WRITE,
        APPEND      = LIBSSH2_FXF_APPEND,
        CREATE      = LIBSSH2_FXF_CREAT,
        TRUNCATE    = LIBSSH2_FXF_TRUNC,
        EXCLUSIVE   = LIBSSH2_FXF_EXCL,
     };

    enum Permissions {
        IRUSR = LIBSSH2_SFTP_S_IRUSR,
        IWUSR = LIBSSH2_SFTP_S_IWUSR,
        IXUSR = LIBSSH2_SFTP_S_IXUSR,
        IRWXU = IRUSR | IWUSR | IXUSR,
        IRGRP = LIBSSH2_SFTP_S_IRGRP,
        IWGRP = LIBSSH2_SFTP_S_IWGRP,
        IXGRP = LIBSSH2_SFTP_S_IXGRP,
        IRWXG = IRGRP | IWGRP | IXGRP,
        IROTH = LIBSSH2_SFTP_S_IROTH,
        IWOTH = LIBSSH2_SFTP_S_IWOTH,
        IXOTH = LIBSSH2_SFTP_S_IXOTH,
        IRWXO = IROTH | IWOTH | IXOTH,
        IRALL = IRUSR | IRGRP | IROTH,
        IWALL = IWUSR | IWGRP | IWOTH,
        IXALL = IXUSR | IXGRP | IXOTH,
        IRWXA = IRALL | IWALL | IXALL
    };

    class DirEntry : public Moveable<DirEntry> {
        friend class SFtp;
        public:
            String GetName() const                  { return filename; }
            int64  GetUid() const                   { return a->flags & LIBSSH2_SFTP_ATTR_UIDGID ? a->uid : -1; }
            int64  GetGid() const                   { return a->flags & LIBSSH2_SFTP_ATTR_UIDGID ? a->gid : -1; }
            int64  GetSize() const                  { return a->flags & LIBSSH2_SFTP_ATTR_SIZE ? a->filesize : -1; }
            Time   GetLastModified() const          { return a->flags & LIBSSH2_SFTP_ATTR_ACMODTIME ? TimeFromUTC(a->mtime) : Null; }
            Time   GetLastAccessed() const          { return a->flags & LIBSSH2_SFTP_ATTR_ACMODTIME ? TimeFromUTC(a->atime) : Null; }
            SFtpAttrs& GetAttrs()                   { return *a; }

            const SFtpAttrs& operator~() const      { return *a; }
            SFtpAttrs&  operator*()                 { return *a; }

            bool IsFile() const                     { return LIBSSH2_SFTP_S_ISREG(a->permissions); }
            bool IsDirectory() const                { return LIBSSH2_SFTP_S_ISDIR(a->permissions); }
            bool IsSymLink() const                  { return LIBSSH2_SFTP_S_ISLNK(a->permissions); }
            bool IsSpecial() const                  { return LIBSSH2_SFTP_S_ISCHR(a->permissions); }
            bool IsBlock() const                    { return LIBSSH2_SFTP_S_ISBLK(a->permissions); }
            bool IsPipe() const                     { return LIBSSH2_SFTP_S_ISFIFO(a->permissions); }
            bool IsSocket() const                   { return LIBSSH2_SFTP_S_ISSOCK(a->permissions); }
            bool IsReadable() const                 { return CanMode(IRUSR, IRGRP, IROTH); }
            bool IsWriteable() const                { return CanMode(IWUSR, IWGRP, IWOTH); }
            bool IsReadOnly() const                 { return IsReadable() && !IsWriteable(); }
            bool IsExecutable() const               { return !IsDirectory() && CanMode(IXUSR, IXGRP, IXOTH); }

            String ToString() const;
            String ToXml() const;

            DirEntry(const String& path);
            DirEntry(const String& path, const SFtpAttrs& attrs);
            DirEntry()                              { Zero();  }
            DirEntry(const Nuller&)                 { Zero();  }

            DirEntry(DirEntry&& e) = default;
            DirEntry& operator=(DirEntry&& e) = default;

        private:
            bool CanMode(dword u, dword g, dword o) const;
            void Zero();

            bool valid;
            String filename;
            One<SFtpAttrs> a;
     };
    typedef Vector<DirEntry> DirList;

public:
    SFtp&                   Timeout(int ms)                                         { ssh->timeout = ms; return *this; }
    SFtp&                   NonBlocking(bool b = true)                              { return Timeout(b ? 0 : Null) ;}
    SFtp&                   WaitStep(int ms)                                        { ssh->waitstep = clamp(ms, 0, INT_MAX); }
    SFtp&                   ChunkSize(int sz)                                       { if(sz >= 1024) ssh->chunk_size = sz; return *this; }

    LIBSSH2_SFTP_HANDLE*    GetHandle() const                                       { return sftp->handle; };
    Value                   GetResult() const                                       { return sftp->value; }

    // File
    SFtpHandle*             Open(const String& path, dword flags, long mode);
    SFtpHandle*             OpenRead(const String& path)                            { return Open(path, READ, IRALL); }
    SFtpHandle*             OpenWrite(const String& path)                           { return Open(path, CREATE | WRITE, IRALL | IWUSR); }
    bool                    Close(SFtpHandle* handle);
    bool                    Rename(const String& oldpath, const String& newpath);
    bool                    Delete(const String& path);
    bool                    Sync(SFtpHandle* handle);
    SFtp&                   Seek(SFtpHandle* handle, int64 position);
    int64                   GetPos(SFtpHandle* handle);

    // Read/Write
    bool                    Get(SFtpHandle* handle, Stream& out, Gate<int64, int64> progress = Null);
    String                  Get(SFtpHandle* handle, Gate<int64, int64> progress = Null);
    bool                    Get(const String& path, Stream& out, Gate<int64, int64> progress = Null);
    String                  Get(const String& path, Gate<int64, int64> progress = Null);
    bool                    Get(const String& path, Stream& out, int64 offset, Gate<int64, int64> progress = Null);
    bool                    Put(SFtpHandle* handle, Stream& in, Gate<int64, int64> progress = Null);
    bool                    Put(Stream& in, const String& path, Gate<int64, int64> progress = Null);
    bool                    Put(Stream& in, const String& path, dword flags, long mode, Gate<int64, int64> progress = Null);
    bool                    Put(Stream& in, const String& path, int64 offset, Gate<int64, int64> progress = Null);
    bool                    Append(Stream& in, const String& path, Gate<int64, int64> progress = Null);
    bool                    Append(Stream& in, const String& path, long mode, Gate<int64, int64> progress = Null);
    String                  Peek(const String& path, int64 offset, int64 length, Gate<int64, int64> progress = Null);
    bool                    Poke(const String& data, const String& path, int64 offset, int64 length, Gate<int64, int64> progress = Null);

    // Directory
    SFtpHandle*             OpenDir(const String& path);
    bool                    MakeDir(const String& path, long mode);
    bool                    RemoveDir(const String& path);
    bool                    ListDir(SFtpHandle* handle, DirList& list);
    bool                    ListDir(const String& path, DirList& list);
    String                  GetCurrentDir();
    String                  GetParentDir();

    // Symlink
    bool                    MakeLink(const String& orig, const String& link)        { return SymLink(orig, const_cast<String*>(&link), LIBSSH2_SFTP_SYMLINK); }
    bool                    ReadLink(const String& path, String& target)            { return SymLink(path, &target, LIBSSH2_SFTP_READLINK); }
    bool                    RealizePath(const String& path, String& target)         { return SymLink(path, &target, LIBSSH2_SFTP_REALPATH); }

    // Attributes
    bool                    GetAttrs(SFtpHandle* handle, SFtpAttrs& attrs);
    bool                    GetAttrs(const String& path, SFtpAttrs& attrs);
    bool                    SetAttrs(SFtpHandle* handle, const SFtpAttrs& attrs);
    bool                    SetAttrs(const String& path, const SFtpAttrs& attrs);
    DirEntry                GetInfo(const String& path);
    bool                    SetInfo(const DirEntry& entry)                          { return SetAttrs(entry.GetName(), ~entry); }
    int64                   GetSize(const String& path)                             { QueryAttr(path, SIZE); return sftp->value; }
    bool                    SetSize(const String& path, int64 size)                 { return ModifyAttr(path, SIZE, size); }
    Time                    GetLastModifyTime(const String& path)                   { QueryAttr(path, LASTMODIFIED); return sftp->value; }
    bool                    SetLastModifyTime(const String& path, const Time& time) { return ModifyAttr(path, LASTMODIFIED, time); }
    Time                    GetLastAccessTime(const String& path)                   { QueryAttr(path, LASTACCESSED); return sftp->value; }
    bool                    SetLastAccessTime(const String& path, const Time& time) { return ModifyAttr(path, LASTACCESSED, time); }

    // Tests
    bool                    FileExists(const String& path)                          { QueryAttr(path, FILE); return sftp->value; }
    bool                    DirectoryExists(const String& path)                     { QueryAttr(path, DIRECTORY); return sftp->value; }
    bool                    SymLinkExists(const String& path)                       { QueryAttr(path, SYMLINK); return sftp->value; }
    bool                    SocketExists(const String& path)                        { QueryAttr(path, SOCKET); return sftp->value; }
    bool                    PipeExists(const String& path)                          { QueryAttr(path, PIPE); return sftp->value; }
    bool                    BlockExists(const String& path)                         { QueryAttr(path, BLOCK); return sftp->value; }
    bool                    SpecialFileExists(const String& path)                   { QueryAttr(path, SPECIAL); return sftp->value; }

    // (Multithreaded I/O)
    static AsyncWork<String> AsyncGet(SshSession& session, const String& path, Gate<int64, int64> progress = Null);
    static AsyncWork<void>   AsyncGet(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress = Null);
    static AsyncWork<void>   AsyncPut(SshSession& session, String&& data, const String& target, Gate<int64, int64> progress = Null);
    static AsyncWork<void>   AsyncPut(SshSession& session, const char* source, const char* target, Gate<int64, int64> progress = Null);
	
    SFtp(SshSession& session);
    virtual ~SFtp();

    SFtp(SFtp&&) = default;
    SFtp& operator=(SFtp&&) = default;

private:
    virtual bool            Init() override;
    virtual void            Exit() override;
    virtual bool            Cleanup(Error& e) override;

    inline SFtpHandle*      HANDLE(SFtpHandle* h)                                   { return h ? h : sftp->handle; }
    int                     FStat(SFtpHandle* handle, SFtpAttrs& a, bool set);
    int                     LStat(const String& path, SFtpAttrs& a, int type);
    bool                    QueryAttr(const String& path, int attr);
    bool                    ModifyAttr(const String& path, int attr, const Value& v);
    bool                    SymLink(const String& path, String* target, int type);
    bool                    FRead(SFtpHandle* handle, Stream& out, int64 size,  Gate<int64, int64> progress = Null, bool str = false);
    bool                    FWrite(SFtpHandle* handle, Stream& out, int64 size, Gate<int64, int64> progress = Null);

    struct SFtpData {
        LIBSSH2_SFTP*       session;
        SFtpHandle*         handle;
        DirEntry            finfo;
        Value               value;
        StringStream        stream;
    };
    One<SFtpData> sftp;

    enum OpCodes{
        INIT, EXIT, START, FOPEN, FCLOSE, FSYNC, FRENAME, FDELETE, FGET, FPUT, FGETSTAT,
        FSETSTAT, DOPEN, DMAKE, DDELETE, DLIST, DGET, LINK, FQUERY, FMODIFY, FSEEK, FTELL,
        FPEEK, FPOKE
    };
    enum FileAttributes {
        FILE, DIRECTORY, SYMLINK, SOCKET, PIPE, BLOCK, SPECIAL, INFO, UID, GID, PERMISSIONS,
        SIZE, LASTMODIFIED, LASTACCESSED
    };
};
