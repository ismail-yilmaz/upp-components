#include "PtyProcess.h"

// Note: This code is a modification of U++/LocalProcess.
// Credits should go to U++ team.

#define LLOG(x)	// RLOG("PtyProcess: " << x);

namespace Upp {

#ifdef PLATFORM_POSIX
static void sNoBlock(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

bool sParseEnv(Vector<const char*>& out, const char* penv)
{
	if(penv) {
		const char *p = penv;
		while(*p) {
			out.Add(p);
			p += strlen(p) + 1;
		}
		if(String(penv).Find("TERM=") < 0)
			out.Add("TERM=xterm");
		out.Add(nullptr);
	}
	return !out.IsEmpty();
}

bool sParseArgs(Vector<char*>& out, const char *cmd, const Vector<String> *pargs, Buffer<char>& cmd_buf)
{
	if(pargs) {
		int n = strlen(cmd) + 1;
		for(int i = 0; i < pargs->GetCount(); i++)
			n += (*pargs)[i].GetCount() + 1;
		cmd_buf.Alloc(n + 1);
		char *p = cmd_buf;
		out.Add(p);
		int l = strlen(cmd) + 1;
		memcpy(p, cmd, l);
		p += l;
		for(int i = 0; i < pargs->GetCount(); i++) {
			out.Add(p);
			l = (*pargs)[i].GetCount() + 1;
			memcpy(p, ~(*pargs)[i], l);
			p += l;
		}
	}
	else { // parse command line for execve
		cmd_buf.Alloc(strlen(cmd) + 1);
		char *cmd_out = cmd_buf;
		const char *p = cmd;
		while(*p)
			if((byte)*p <= ' ')
				p++;
			else {
				out.Add(cmd_out);
				while(*p && (byte)*p > ' ') {
					int c = *p;
					if(c == '\\') {
						if(*++p)
							*cmd_out++ = *p++;
					}
					else if(c == '\"' || c == '\'') {
						p++;
						while(*p && *p != c)
							if(*p == '\\') {
								if(*++p)
									*cmd_out++ = *p++;
							}
							else
								*cmd_out++ = *p++;
						if(*p == c)
							p++;
					}
					else
						*cmd_out++ = *p++;
				}
				*cmd_out++ = '\0';
			}
	}
	if(out.IsEmpty())
		return false;
	out.Add(nullptr);
	return true;
}

void PtyProcess::Init()
{
	pid    =  0;
	master = -1;
	slave  = -1;
	convertcharset = false;
	exit_code = Null;
}

void PtyProcess::Free()
{
	if(master >= 0) {
		close(master);
		master = -1;
	}
	
	if(slave >= 0) {
		close(slave);
		slave = -1;
	}
	
	if(pid) {
		waitpid(pid, 0, WNOHANG | WUNTRACED);
		pid = 0;
	}
}

bool PtyProcess::Start(const char *cmdline, const VectorMap<String, String>& env, const char *cd)
{
	String senv;
	for(int i = 0; i < env.GetCount(); i++)
		senv << env.GetKey(i) << "=" << env[i] << '\0';
	return DoStart(cmdline, nullptr, ~senv, cd);
}

bool PtyProcess::DoStart(const char *cmd, const Vector<String> *args, const char *env, const char *cd)
{
	Kill();
	exit_code = Null;

	Vector<char*> vargs;
	Buffer<char> cmdbuf;
	if(!sParseArgs(vargs, cmd, args, cmdbuf)) {
		LLOG("Couldn't parse arguments.");
		Free();
		return false;
	}
	
	String fullpath = GetFileOnPath(vargs[0], getenv("PATH"), true);
	if(IsNull(fullpath)) {
		LLOG("Couldn't retrieve full path.");
		Free();
		return false;
	}

	Buffer<char> arg0(fullpath.GetCount() + 1);
	memcpy(~arg0, ~fullpath, fullpath.GetCount() + 1);
	vargs[0] = ~arg0;
	
	if((master = posix_openpt(O_RDWR | O_NOCTTY)) < 0) {
		LLOG("Couldn't open pty master.");
		Free();
		return false;
	}
	
	if(grantpt(master) < 0) {
		LLOG("grantpt() failed.");
		Free();
		return false;
	}
	
	if(unlockpt(master) < 0) {
		LLOG("unlockpt() failed.");
		Free();
		return false;
	}

	if(IsNull((sname = ptsname(master)))) {
		LLOG("ptsname() failed.");
		Free();
		return false;
	}

	pid = fork();
	if(pid < 0) {
		LLOG("fork() failed.");
		Free();
		return false;
	}
	else
	if(pid > 0) {
		// Parent process...
		close(slave);
		slave = -1;
		sNoBlock(master);
		return true;
	}
	// Child process...
	close(master);
		
	ResetSignals();
	
	if(setsid() < 0) {
		LLOG("setsid() failed.");
		Exit(1);
	}
	
	setpgid(pid, 0);
	
	if((slave = open(~sname, O_RDWR)) < 0) {
		LLOG("Couldn't open pty slave.");
		Free();
		return false;
	}

#if defined(TIOCSCTTY)
	if(ioctl(slave, TIOCSCTTY, nullptr) < 0) {
		LLOG("ioctl(TIOCSCTTY) failed.");
		Free();
		return false;
	}
#endif

#if defined(PLATFORM_SOLARIS)
	if(isastream(slave)) {
		if((ioctl(slave, I_PUSH, "pterm") < 0)) {
			LLOG("ioctl(I_PUSH) - pterm - failed.");
			Free();
			return false;
		}
		if((ioctl(slave, I_PUSH, "ldterm") < 0)) {
			LLOG("ioctl(I_PUSH) -ldterm- failed.");
			Free();
			return false;
		}
		if((ioctl(slave, I_PUSH, "ttcompat") < 0)) {
			LLOG("ioctl(I_PUSH) -ttcompat- failed.");
			Free();
			return false;
		}
	}
#endif

	if((dup2(slave, STDIN_FILENO)  != STDIN_FILENO)  ||
	   (dup2(slave, STDOUT_FILENO) != STDOUT_FILENO) ||
	   (dup2(slave, STDERR_FILENO) != STDERR_FILENO)) {
	       LLOG("dup2() failed.");
	       Free();
	       return false;
	}
	
	sNoBlock(slave);
	
	if(slave > STDERR_FILENO)
		close(slave);

	if(cd)
		(void) chdir(cd);
	
	if(env) {
		Vector<const char*> venv;
		sParseEnv(venv, env);
		execve(fullpath, (char* const*) vargs.begin(), (char* const*) venv.begin());
	}
	else {
		execv(fullpath, vargs.begin());
	}

	LLOG("execv() failed, errno = " << errno);
	exit(~errno);
	return true;
}

bool PtyProcess::Read(String& s)
{
	String rread;
	constexpr int BUFSIZE = 4096;

	bool running = IsRunning() || master >= 0;
	if(running && Wait(WAIT_READ, 0)) { // Poll
		char buffer[BUFSIZE];
		int n = 0, done = 0;
		while((n = read(master, buffer, BUFSIZE)) > 0) {
			done += n;
			rread.Cat(buffer, n);
		}
		LLOG("Read() -> " << done << " bytes read.");
		if(n == 0) {
			close(master);
			master = -1;
		}
		if(!IsNull(rread)) {
			s << (convertcharset ? FromSystemCharset(rread) : rread);
			return true;
		}
	}
	Write(Null); // Flush pending input...
	return !IsNull(rread) && running;
}

void PtyProcess::Write(String s)
{
	if(IsNull(s) && IsNull(wbuffer))
		return;
	if(convertcharset)
		s = ToSystemCharset(s);
	wbuffer.Cat(s);
	dword done = 0;
	if(master >= 0 && Wait(WAIT_WRITE, 0)) { // Poll
		int n = 0;
		while((n = write(master, ~wbuffer, min(wbuffer.GetLength(), 4096))) > 0 || n == EINTR) {
			done += n;
			if(n > 0)
				wbuffer.Remove(0, n);
		}
	}
	LLOG("Write() -> " << done << "/" << wbuffer.GetLength() << " bytes.");
}

void PtyProcess::Kill()
{
	if(IsRunning()) {
		LLOG("\nPtyProcess::Hang up, pid = " << (int) pid);
		exit_code = 255;
		kill(pid, SIGHUP); // TTYs behaves better with hang up signal.
		GetExitCode();
		int status;
		if(pid && waitpid(pid, &status, 0) == pid)
			DecodeExitCode(status);
	}
	Free();
}

int PtyProcess::GetExitCode()
{
	if(!IsRunning())
		return Nvl(exit_code, -1);
	int status;
	if(!(waitpid(pid, &status, WNOHANG | WUNTRACED) == pid && DecodeExitCode(status)))
		return -1;
	LLOG("GetExitCode() -> " << exit_code << " (just exited)");
	return exit_code;
}

bool PtyProcess::IsRunning()
{
	if(!pid || !IsNull(exit_code)) {
		LLOG("IsRunning() -> no");
		return false;
	}
	int status = 0, wp;
	if(!((wp = waitpid(pid, &status, WNOHANG | WUNTRACED)) == pid && DecodeExitCode(status)))
		return true;
	LLOG("IsRunning() -> no, just exited, exit code = " << exit_code);
	return false;
}

bool PtyProcess::DecodeExitCode(int status)
{
	if(WIFEXITED(status)) {
		exit_code = (byte)WEXITSTATUS(status);
		return true;
	}
	else if(WIFSIGNALED(status) || WIFSTOPPED(status)) {
		static const struct {
			const char *name;
			int         code;
		}
		signal_map[] = {
#define SIGDEF(s) { #s, s },
		SIGDEF(SIGHUP) SIGDEF(SIGINT) SIGDEF(SIGQUIT) SIGDEF(SIGILL) SIGDEF(SIGABRT)
		SIGDEF(SIGFPE) SIGDEF(SIGKILL) SIGDEF(SIGSEGV) SIGDEF(SIGPIPE) SIGDEF(SIGALRM)
		SIGDEF(SIGPIPE) SIGDEF(SIGTERM) SIGDEF(SIGUSR1) SIGDEF(SIGUSR2) SIGDEF(SIGTRAP)
		SIGDEF(SIGURG) SIGDEF(SIGVTALRM) SIGDEF(SIGXCPU) SIGDEF(SIGXFSZ) SIGDEF(SIGIOT)
		SIGDEF(SIGIO) SIGDEF(SIGWINCH)
#ifndef PLATFORM_BSD
		//SIGDEF(SIGCLD) SIGDEF(SIGPWR)
#endif
		//SIGDEF(SIGSTKFLT) SIGDEF(SIGUNUSED) // not in Solaris, make conditional if needed
#undef SIGDEF
		};

		int sig = (WIFSIGNALED(status) ? WTERMSIG(status) : WSTOPSIG(status));
		exit_code = (WIFSIGNALED(status) ? 1000 : 2000) + sig;
		exit_string << "\nProcess " << (WIFSIGNALED(status) ? "terminated" : "stopped") << " on signal " << sig;
		for(int i = 0; i < __countof(signal_map); i++)
			if(signal_map[i].code == sig)
			{
				exit_string << " (" << signal_map[i].name << ")";
				break;
			}
		exit_string << "\n";
		return true;
	}
	return false;
}

bool PtyProcess::ResetSignals()
{
	sigset_t set;
	sigemptyset(&set);
	if(pthread_sigmask(SIG_SETMASK, &set, nullptr) < 0) {
		LLOG("Couldn't unblock signals for child process.");
		return false;
	}
	
	// We also need to reset the signal handlers.
	// See signal.h for NSIG constant.
	
	for(int i = 1; i < NSIG; i++) {
		if(i != SIGSTOP && i != SIGKILL)
			signal(i, SIG_DFL);
	}
	return true;
}

bool PtyProcess::Wait(dword event, int ms)
{
	SocketWaitEvent we;
	we.Add((SOCKET) master, event);
	return we.Wait(ms) && we[0] & event;
}

bool PtyProcess::SetSize(Size sz)
{
	if(master >= 0) {
		winsize wsz;
		Zero(wsz);
		wsz.ws_col = max(2, sz.cx);
		wsz.ws_row = max(2, sz.cy);
		if(ioctl(master, TIOCSWINSZ, &wsz) >= 0) {
			LLOG("Pty size is set to: " << sz);
			return true;
		}
	}
	LLOG("Couldn't set pty size!");
	return false;
}

Size PtyProcess::GetSize()
{
	if(master >= 0) {
		winsize wsz;
		Zero(wsz);
		if(ioctl(master, TIOCGWINSZ, &wsz) >= 0) {
			Size sz(wsz.ws_col, wsz.ws_row);
			LLOG("Fetched pty size: " << sz);
			return sz;
		}
	}
	LLOG("Couldn't fetch pty size!");
	return Null;
}

bool PtyProcess::SetAttrs(const termios& t)
{
	if(master >= 0 && tcsetattr(master, TCSANOW, &t) >= 0) {
		LLOG("Pty attributes are set.");
		return true;
	}
	LLOG("Couldn't set pty attributes!");
	return false;
}

bool PtyProcess::GetAttrs(termios& t)
{
	if(master >= 0 && tcgetattr(master, &t) >= 0) {
		LLOG("Pty attributes are fetched.");
		return true;
	}
	LLOG("Couldn't fetch pty attributes!");
	return false;
}

#elif PLATFORM_WIN32

// Windows 10, pseudoconsole support. (Experimental)

#ifdef flagWIN10
String sParseArgs(const char *cmd, const Vector<String> *pargs)
{
	while(*cmd && (byte) *cmd <= ' ') ++cmd;

	if(!pargs)
		return cmd;

	String cmdh;
	cmdh = cmd;
	for(int i = 0; i < pargs->GetCount(); i++) {
		cmdh << ' ';
		String argument = (*pargs)[i];
		if(argument.GetCount() && argument.FindFirstOf(" \t\n\v\"") < 0)
			cmdh << argument;
		else {
			cmdh << '\"';
			const char *s = argument;
			for(;;) {
				int num_backslashes = 0;
				while(*s == '\\') {
					s++;
					num_backslashes++;
				}
				if(*s == '\0') {
					cmdh.Cat('\\', 2 * num_backslashes);
					break;
				}
				else
				if(*s == '\"') {
					cmdh.Cat('\\', 2 * num_backslashes + 1);
					cmdh << '\"';
				}
				else {
					cmdh.Cat('\\', num_backslashes);
					cmdh.Cat(*s);
				}
				s++;
			}
			cmdh << '\"';
		}
	}
	return cmdh;
}

bool Win32CreateProcess(const char *command, const char *envptr, STARTUPINFOEX& si, PROCESS_INFORMATION& pi, const char *cd)
{ // provides conversion of charset for cmdline and envptr
	WString wcmd(command);
	int n = wcmd.GetCount() + 1;
	WString wcd(cd);
	Buffer<wchar> cmd(n);
	memcpy(cmd, wcmd, n * sizeof(wchar));
#if 0 // unicode environment not necessary for now
	wchar wenvptr = nullptr;
	Buffer<wchar> env(n);
	if(envptr) {
		int len = 0;
		while(envptr[len] || envptr[len + 1])
			len++;
		WString wenv(envptr, len + 1);
		env.Alloc(len + 2);
		memcpy(env, wenv, (len + 2) * sizeof(wchar));
	}
#endif
	return CreateProcessW(
		nullptr,
		cmd,
		nullptr,
		nullptr,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT,
		(void *)envptr,
		cd ? ~wcd : nullptr,
		(LPSTARTUPINFOW) &si.StartupInfo,
		&pi);
}
#endif

void PtyProcess::Init()
{
#ifdef flagWIN10
	hProcess      = nullptr;
	hConsole      = nullptr;
	hOutputRead   = nullptr;
	hErrorRead    = nullptr;
	hInputWrite   = nullptr;
	hProcAttrList = nullptr;
	cSize         = Null;
#endif
	convertcharset = false;
	exit_code = Null;
}

void PtyProcess::Free()
{
#ifdef flagWIN10
	if(hProcess) {
		CloseHandle(hProcess);
		hProcess = nullptr;
	}

	if(hOutputRead) {
		CloseHandle(hOutputRead);
		hOutputRead = nullptr;
	}

	if(hErrorRead) {
		CloseHandle(hErrorRead);
		hErrorRead = nullptr;
	}

	if(hInputWrite) {
		CloseHandle(hInputWrite);
		hInputWrite = nullptr;
	}

	if(hConsole) {
		ClosePseudoConsole(hConsole);
		hConsole = nullptr;
	}

	if(hProcAttrList) {
		DeleteProcThreadAttributeList(hProcAttrList);
		hProcAttrList = nullptr;
	}
#endif
}

bool PtyProcess::Start(const char *cmdline, const VectorMap<String, String>& env, const char *cd)
{
	String senv;
	for(int i = 0; i < env.GetCount(); i++)
		senv << env.GetKey(i) << "=" << env[i] << '\0';
	return DoStart(cmdline, nullptr, ~senv, cd);
}

bool PtyProcess::DoStart(const char *cmd, const Vector<String> *args, const char *env, const char *cd)
{
	Kill();
	exit_code = Null;
#ifdef flagWIN10
	String command = sParseArgs(cmd, args);
	if(command.IsEmpty()) {
		LLOG("Couldn't parse arguments.");
		Free();
		return false;
	}

	HANDLE hOutputReadTmp, hOutputWrite;
	HANDLE hInputWriteTmp, hInputRead;
	HANDLE hErrorWrite;

	HANDLE hp = GetCurrentProcess();

	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = nullptr;
	sa.bInheritHandle = TRUE;

	CreatePipe(&hInputRead, &hInputWriteTmp, &sa, 0);
	CreatePipe(&hOutputReadTmp, &hOutputWrite, &sa, 0);
	
	DuplicateHandle(hp, hInputWriteTmp, hp, &hInputWrite, 0, FALSE, DUPLICATE_SAME_ACCESS);
	DuplicateHandle(hp, hOutputReadTmp, hp, &hOutputRead, 0, FALSE, DUPLICATE_SAME_ACCESS);
	DuplicateHandle(hp, hOutputWrite,   hp, &hErrorWrite, 0, TRUE,  DUPLICATE_SAME_ACCESS);
	
	CloseHandle(hInputWriteTmp);
	CloseHandle(hOutputReadTmp);

	COORD size;
	size.X = 80;
	size.Y = 24;
	
	if(CreatePseudoConsole(size, hInputRead, hOutputWrite, 0, &hConsole) != S_OK) {
		LLOG("CreatePseudoConsole() failed.");
		Free();
		return false;
	}

	PROCESS_INFORMATION pi;
	STARTUPINFOEX si;
	ZeroMemory(&si, sizeof(STARTUPINFOEX));
	si.StartupInfo.cb = sizeof(STARTUPINFOEX);

	size_t listsize;  // FIXME: Use Upp's allocators (or Upp::Buffer<>) here.
	InitializeProcThreadAttributeList(nullptr, 1, 0, (PSIZE_T) &listsize);
	hProcAttrList = (PPROC_THREAD_ATTRIBUTE_LIST) HeapAlloc(GetProcessHeap(), 0, listsize);
	if(!hProcAttrList) {
		LLOG("HeapAlloc(): Out of memory error.");
		Free();
		return false;
	}

	if(!InitializeProcThreadAttributeList(hProcAttrList, 1, 0, (PSIZE_T) &listsize)) {
		LLOG("InitializeProcThreadAttributeList() failed.");
		Free();
		return false;
	}

	if (!UpdateProcThreadAttribute(
			hProcAttrList,
			0,
			PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE,
			hConsole,
			sizeof(hConsole),
			nullptr,
			nullptr)) {
		LLOG("UpdateProcThreadAttribute() failed.");
		Free();
		return false;
	}

	si.lpAttributeList        = hProcAttrList;
	si.StartupInfo.dwFlags    = STARTF_USESTDHANDLES;
	si.StartupInfo.hStdInput  = hInputRead;
	si.StartupInfo.hStdOutput = hOutputWrite;
	si.StartupInfo.hStdError  = hErrorWrite;

	bool h = Win32CreateProcess(~command, env, si, pi, cd);

	CloseHandle(hErrorWrite);
	CloseHandle(hInputRead);
	CloseHandle(hOutputWrite);

	if(h) {
		hProcess = pi.hProcess;
		CloseHandle(pi.hThread);
	}
	else {
		LLOG("CreateProcess() failed.");
		Free();
		return false;
	}
	return true;
#else
	LLOG("PtyProcess requires at least Windows 10.");
	return false;
#endif
}

bool PtyProcess::Read(String& s)
{
	String rread;
	constexpr int BUFSIZE = 4096;

#ifdef flagWIN10
	s = rbuffer;
	rbuffer.Clear();
	bool running = IsRunning();
	char buffer[BUFSIZE];
	dword n = 0;

	while(hOutputRead
		&& PeekNamedPipe(hOutputRead, nullptr, 0, nullptr, &n, nullptr)
		&& n
		&& ReadFile(hOutputRead, buffer, sizeof(buffer), &n, nullptr) && n)
			rread.Cat(buffer, n);

	while(hErrorRead
		&& PeekNamedPipe(hErrorRead, nullptr, 0, nullptr, &n, nullptr)
		&& n
		&& ReadFile(hErrorRead, buffer, sizeof(buffer), &n, nullptr) && n)
			rread.Cat(buffer, n);

	LLOG("Read() -> " << rread.GetLength() << " bytes read.");

	if(!IsNull(rread)) {
		s << (convertcharset ? FromOEMCharset(rread) : rread);
	}
	return !IsNull(rread) && running;
#else
	return false;
#endif
}

void PtyProcess::Write(String s)
{
#ifdef flagWIN10
	if(IsNull(s) && IsNull(wbuffer))
		return;
	if(convertcharset)
		s = ToSystemCharset(s);
	wbuffer.Cat(s);
	dword done = 0;
	if(hInputWrite) {
		bool ret = true;
		dword n = 0;
		for(int wn = 0; ret && wn < wbuffer.GetLength(); wn += n) {
			ret = WriteFile(hInputWrite, ~wbuffer, min(wbuffer.GetLength(), 4096), &n, nullptr);
			done += n;
			if(n > 0)
				wbuffer.Remove(0, n);
			String hr = rbuffer;
			rbuffer.Clear();
			Read(rbuffer);
			rbuffer = hr + rbuffer;
		}
	}
	LLOG("Write() -> " << done << "/" << wbuffer.GetLength() << " bytes.");
#endif
}

void PtyProcess::Kill()
{
#ifdef flagWIN10
	if(hProcess && IsRunning()) {
		TerminateProcess(hProcess, (DWORD)-1);
		exit_code = 255;
	}
#endif
	Free();
}

int PtyProcess::GetExitCode()
{
#ifdef flagWIN10
	return IsRunning() ? (int) Null : exit_code;
#else
	return (int) Null;
#endif
}

HANDLE PtyProcess::GetProcessHandle() const
{
#ifdef flagWIN10
	return hProcess;
#else
	return nullptr;
#endif
}

bool PtyProcess::IsRunning()
{
#ifdef flagWIN10
	dword exitcode;
	if(!hProcess)
		return false;
	if(GetExitCodeProcess(hProcess, &exitcode) && exitcode == STILL_ACTIVE)
		return true;
	dword n;
	if(PeekNamedPipe(hOutputRead, nullptr, 0, nullptr, &n, nullptr) && n)
		return true;
	exit_code = exitcode;
	LLOG("IsRunning() -> no, just exited, exit code = " << exit_code);
#endif
	return false;
}

bool PtyProcess::SetSize(Size sz)
{
#ifdef flagWIN10
	if(hConsole) {
		COORD size;
		size.X = (SHORT) max(2, sz.cx);
		size.Y = (SHORT) max(2, sz.cy);
		if(ResizePseudoConsole(hConsole, size) == S_OK) {
			LLOG("Pty size is set to: " << sz);
			cSize = sz;
			return true;
		}
	}
	cSize = Null;
	LLOG("Couldn't set pty size!");
#endif
	return false;
}

Size PtyProcess::GetSize()
{
#ifdef flagWIN10
	// GetScreenBufferInfo() function does not seem to work here.
	// TODO: Check the MS docs and find a better way, if possbile.
	if(hConsole && !IsNull(cSize)) {
		LLOG("Fetched pty size: " << cSize);
		return cSize;
	}
	LLOG("Couldn't fetch pty size!");
#endif
	return Null;
}
#endif
}