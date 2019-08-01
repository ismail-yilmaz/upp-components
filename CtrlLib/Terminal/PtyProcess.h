#ifndef _Terminal_Pty_h_
#define _Terminal_Pty_h_

#include <Core/Core.h>

#ifdef PLATFORM_POSIX
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>
#endif

namespace Upp {

#ifdef PLATFORM_POSIX
class PtyProcess : public AProcess {
public:
    PtyProcess()                                                                                                    { Init(); }
    PtyProcess(const char *cmdline, const VectorMap<String, String>& env, const char *cd = nullptr)                 { Init(); Start(cmdline, env, cd); }
    PtyProcess(const char *cmdline, const char *env = nullptr, const char *cd = nullptr)                            { Init(); Start(cmdline, nullptr, env, cd); }
    PtyProcess(const char *cmd, const Vector<String> *args, const char *env = nullptr, const char *cd = nullptr)    { Init(); Start(cmd, args, env, cd); }
    virtual ~PtyProcess()                                                                                           { Kill(); }

    PtyProcess& ConvertCharset(bool b = true)       { convertcharset = b; return *this; }
    PtyProcess& NoConvertCharset()                  { return ConvertCharset(false); }

    bool        SetSize(Size sz);
    bool        SetSize(int col, int row)           { return SetSize(Size(col, row)); }
    Size        GetSize();
    bool        SetAttrs(const termios& t);
    bool        GetAttrs(termios& t);
    bool        Start(const char *cmdline, const char *env = nullptr, const char *cd = nullptr)                         { return DoStart(cmdline, nullptr, env, cd); }
    bool        Start(const char *cmd, const Vector<String> *args, const char *env = nullptr, const char *cd = nullptr) { return DoStart(cmd, args, env, cd); }
    bool        Start(const char *cmdline, const VectorMap<String, String>& env, const char *cd = nullptr);
    void        Kill() override;

    bool        IsRunning() override;

    bool        Read(String& s) override;
    void        Write(String s) override;

    int         GetPid() const                      { return pid; }
    int         GetExitCode() override;

private:
    void        Init();
    void        Free();
    bool        Open();
    bool        ResetSignals();
    bool        DecodeExitCode(int status);
    bool        DoStart(const char *cmd, const Vector<String> *args, const char *env, const char *cd);

    int         master, slave;
    int         exit_code;
    bool        convertcharset;
    String      exit_string;
    String      sname;
    String      wread;
    String      wbuffer;
    pid_t       pid;
};
#endif
}

#endif
