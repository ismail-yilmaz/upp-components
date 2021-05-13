#ifndef _Upp_PtyProcess_h_
#define _Upp_PtyProcess_h_

#include <Core/Core.h>

#ifdef PLATFORM_POSIX
    #include <sys/ioctl.h>
    #include <sys/wait.h>
    #include <termios.h>
#elif PLATFORM_WIN32
	#include <windows.h>
	#include "lib/libwinpty.h"
#endif

namespace Upp {

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

#ifdef PLATFORM_POSIX
    bool        SetAttrs(const termios& t);
    bool        GetAttrs(termios& t);
    Gate<termios&> WhenAttrs;
#endif

    bool        Start(const char *cmdline, const char *env = nullptr, const char *cd = nullptr)                         { return DoStart(cmdline, nullptr, env, cd); }
    bool        Start(const char *cmd, const Vector<String> *args, const char *env = nullptr, const char *cd = nullptr) { return DoStart(cmd, args, env, cd); }
    bool        Start(const char *cmdline, const VectorMap<String, String>& env, const char *cd = nullptr);
    void        Kill() final;

    bool        IsRunning() override;

    bool        Read(String& s) override;
    void        Write(String s) override;

    int         GetExitCode() override;

#ifdef PLATFORM_POSIX
    int         GetPid() const                      { return pid; }
#elif PLATFORM_WIN32
    HANDLE      GetProcessHandle() const;
#endif

private:
    void        Init();
    void        Free();
    bool        DoStart(const char *cmd, const Vector<String> *args, const char *env, const char *cd);

#ifdef PLATFORM_POSIX
    bool        ResetSignals();
    bool        Wait(dword event, int ms = 10);
    bool        DecodeExitCode(int status);

    int         master, slave;
    String      exit_string;
    String      sname;
    pid_t       pid;
#elif PLATFORM_WIN32
	#ifdef flagWIN10
    // Windows 10 pseudoconsole API support. (Experimental)
    HPCON       hConsole;
    PPROC_THREAD_ATTRIBUTE_LIST hProcAttrList;
    #else
    // WinPty backend. (default)
    winpty_t*   hConsole;
    #endif
    HANDLE      hProcess;
    HANDLE      hOutputRead;
    HANDLE      hErrorRead;
    HANDLE      hInputWrite;
    DWORD       dwProcessId;
    Size        cSize;
    String      rbuffer;
#endif
    String      wbuffer;
    int         exit_code;
    bool        convertcharset;
};
}

#endif