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
	String app;

	if(pargs) {
		app = cmd;
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

void PtyProcess::Init()
{
	pid = 0;
	master = slave = -1;
	convertcharset = false;
	exit_code = Null;
}

void PtyProcess::Free()
{
	if(master >= 0)
		close(master);
	if(slave  >= 0)
		close(slave);
	master = slave  = -1;
	if(pid) waitpid(pid, 0, WNOHANG | WUNTRACED);
	pid = 0;
}

bool PtyProcess::Open()
{
	if((master = posix_openpt(O_RDWR | O_NOCTTY)) < 0) {
		LLOG("Couldn't open pty master.");
		return false;
	}
	
	if(grantpt(master) < 0) {
		LLOG("grantpt() failed.");
		return false;
	}
	
	if(unlockpt(master) < 0) {
		LLOG("unlockpt() failed.");
		return false;
	}
	
	if(IsNull((sname = ptsname(master)))) {
		LLOG("ptsname() failed.");
		return false;
	}

	return true;
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

	if(!Open())
		return  false;

	Vector<char*> vargs;
	Buffer<char> cmdbuf;
	if(!sParseArgs(vargs, cmd, args, cmdbuf)) {
		LLOG("Couldn't parse arguments.");
		return false;
	}
	
	String fullpath = GetFileOnPath(vargs[0], getenv("PATH"), true);
	if(IsNull(fullpath)) {
		LLOG("Couldn't retrieve full path.");
		return false;
	}

	Buffer<char> arg0(fullpath.GetCount() + 1);
	memcpy(~arg0, ~fullpath, fullpath.GetCount() + 1);
	vargs[0] = ~arg0;

	pid = fork();
	if(pid < 0) {
		LLOG("fork() failed.");
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
		Exit(-1);
	}
	
	setpgid(pid, 0);
	
	if((slave = open(~sname, O_RDWR)) < 0) {
		LLOG("Couldn't open pty slave.");
		return false;
	}

#if defined(TIOCSCTTY)
	if(ioctl(slave, TIOCSCTTY, nullptr) < 0) {
		LLOG("ioctl(TIOCSCTTY) failed.");
		return false;
	}
#endif
	

#if defined(PLATFORM_SOLARIS) // Does U++ even run on Solaris?
	if(isastream(slave)) {
		if((ioctl(slave, I_PUSH, "pterm") < 0)) {
			LLOG("ioctl(I_PUSH) - pterm - failed.");
			return false;
		}
		if((ioctl(slave, I_PUSH, "ldterm") < 0)) {
			LLOG("ioctl(I_PUSH) -ldterm- failed.");
			return false;
		}
		if((ioctl(slave, I_PUSH, "ttcompat") < 0)) {
			LLOG("ioctl(I_PUSH) -ttcompat- failed.");
			return false;
		}
	}
#endif

	if((dup2(slave, STDIN_FILENO)  != STDIN_FILENO)  ||
	   (dup2(slave, STDOUT_FILENO) != STDOUT_FILENO) ||
	   (dup2(slave, STDERR_FILENO) != STDERR_FILENO)) {
	       LLOG("dup2() failed.");
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
	bool running = IsRunning() || master >= 0;
	fd_set rd, ex;
	timeval tval;
	Zero(tval);
	FD_ZERO(&rd);
	FD_ZERO(&ex);
	FD_SET(master, &rd);
	FD_SET(master, &ex);	// TODO: Implement packet mode.
	int ev = select(master + 1, &rd, nullptr, &ex, &tval);
	if(ev > 0 && FD_ISSET(master, &rd)) {
		LLOG("Read() -> select");
		int n = 0, done = 0;
		const int BUFSIZE = 4096;
		char buffer[BUFSIZE];
		while((n = read(master, buffer, BUFSIZE)) > 0) {
			done += n;
			rread.Cat(buffer, n);
		}
		LLOG("Read() -> " << done << " bytes");
		if(n == 0) {
			close(master);
			master = -1;
		}
		if(!IsNull(rread)) {
			s << (convertcharset ? FromSystemCharset(rread) : rread);
			return true;
		}
	}
	return !IsNull(rread) && !running;
}

void PtyProcess::Write(String s)
{
	if(IsNull(s) && wbuffer.IsEmpty())
		return;
	if(convertcharset)
		s = ToSystemCharset(s);
	wbuffer.Cat(s);
	if (master >= 0) {
		int ret = 1;
		for(int wn = 0; (ret > 0 || errno == EINTR) && wn < wbuffer.GetLength(); wn += ret) {
			ret = write(master, ~wbuffer + wn, wbuffer.GetLength() - wn);
			if(ret > 0)
				wbuffer.Remove(0, ret);
		}
	}
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
			LLOG("Fetched pys size: " << sz);
			return sz;
		}
	}
	LLOG("Couldn't get pty size!");
	return Null;
}

bool PtyProcess::SetAttrs(const termios& t)
{
	if(master >= 0 && tcsetattr(master, TCSAFLUSH, &t) >= 0) {
		LLOG("Pty attributes set.");
		return true;
	}
	LLOG("Couldn't set pty attributes!");
	return false;
}

bool PtyProcess::GetAttrs(termios& t)
{
	if(master >= 0 && IsRunning() && tcgetattr(master, &t) >= 0) {
		LLOG("Pty attributes fetched.");
		return true;
	}
	LLOG("Couldn't get pty attributes!");
	return false;

}
#endif
}