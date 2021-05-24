#include "PtyProcess.h"

namespace Upp {

#define LLOG(x)	// RLOG("PtyProcess [POSIX]: " << x);
	
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
		int i = FindMatch(out, [](const char *p) { return String(p).Find("TERM=") >= 0; });
		if(i < 0)
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

	struct termios tio; Zero(tio);
	if(WhenAttrs && GetAttrs(tio) && WhenAttrs(tio)) {
		LLOG("Setting user-defined termios flags for initial pty setup.");
		SetAttrs(tio);
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

#endif
}