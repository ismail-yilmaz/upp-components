#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG(SSH::GetName(ssh->otype, ssh->oid) << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

bool SshShell::Run0(int mode_, const String& terminal, Size pagesize)
{
	mode  = mode_;
	psize = pagesize;

	return ComplexCmd(CHSHELL, [=]() mutable {
		SshChannel::Clear();
		SshChannel::Terminal(terminal, psize);
		SshChannel::Shell();
		if(mode == CONSOLE)
			Cmd(CHSHELL, [=] { ConsoleInit(); return true; });
		Cmd(CHSHELL, [=] { return ProcessEvents(queue); });
	});
}

void SshShell::ReadWrite(String& in, const void* out, int out_len)
{
	switch(mode) {
		case GENERIC: {
			if(out_len > 0)
				WhenOutput(out, out_len);
			WhenInput();
			break;
		}
		case CONSOLE: {
			if(out_len > 0)
				ConsoleWrite(out, out_len);
#ifdef PLATFORM_POSIX
			// We need to catch the WINCH signal. To this end we'll use a POSIX compliant kernel
			// function: sigtimedwait. To speed up, we'll simply poll for the monitored event.
			ConsoleRead();
			
			sigset_t set;
			sigemptyset(&set);
			sigaddset(&set, SIGWINCH);
			sigprocmask(SIG_BLOCK, &set, NULL);
		
			struct timespec timeout;
			Zero(timeout); // Instead of waiting, we simply poll.
		
			auto rc = sigtimedwait(&set, NULL, &timeout);
			if(rc < 0 && errno != EAGAIN)
				SetError(-1, "sigtimedwait() failed.");
			if(rc > 0)
				LLOG("SIGWINCH received.");
			resized = rc > 0;
#elif PLATFORM_WIN32
			// This part is a little bit tricky. We need to handle Windows console events here.
			// But we cannot simply ignore the events we don't look for. We need to actively
			// remove them from the event queue. Otherwise they'll cause erratic behaviour, and
			// a lot of head ache. Thus to filter out these unwanted events, or the event's that
			// we don't want to get in our way, we'll first peek at the console event queue to
			// see if they met our criteria and remove them one by one as we encounter, using
			// the ReadConsoleInput method.
			
			auto rc = WaitForSingleObject(stdinput, 10);
			switch(rc) {
				case WAIT_OBJECT_0:
					break;
				case WAIT_TIMEOUT:
				case WAIT_ABANDONED:
					return;
				default:
					SetError(-1, "WaitForSingleObject() failed.");
			}

			DWORD n = 0;
			INPUT_RECORD ir[1];

			if(!PeekConsoleInput(stdinput, ir, 1, &n))
				SetError(-1, "Unable to peek console input events.");
			if(n) {
				switch(ir[0].EventType) {
					case KEY_EVENT:
						// Ignore key-ups.
						if(!ir[0].Event.KeyEvent.bKeyDown)
							break;
						ConsoleRead();
						return;
					case WINDOW_BUFFER_SIZE_EVENT:
						LLOG("WINDOW_BUFFER_SIZE_EVENT received.");
						resized = true;
						break;
					case MENU_EVENT:
					case MOUSE_EVENT:
					case FOCUS_EVENT:
						break;
					default:
						SetError(-1, "Unknown console event type encountered.");
				}
				if(!ReadConsoleInput(stdinput, ir, 1, &n))
					SetError(-1, "Unable to filter console input events.");
			}
#endif
			break;
		}
		default:
			NEVER();
	}
	Resize();
}

void SshShell::Resize()
{
	if(!resized)
		return;

	if(mode == CONSOLE)
		PageSize(GetConsolePageSize());

	int n = 0;
	do {
		n = SetPtySz(psize);
	}
	while(!IsTimeout() && !IsEof() && n < 0);
	resized = false;
}

void SshShell::ConsoleInit()
{
#ifdef PLATFORM_WIN32
	stdinput = GetStdHandle(STD_INPUT_HANDLE);
	if(!stdinput)
		SetError(-1, "Unable to obtain a handle for stdin.");
	stdoutput = GetStdHandle(STD_OUTPUT_HANDLE);
	if(!stdoutput)
		SetError(-1, "Unable to obtain a handle for stdout.");
#endif
	ConsoleRawMode();
}

#ifdef PLATFORM_POSIX
void SshShell::ConsoleRead()
{
	if(!EventWait(STDIN_FILENO, WAIT_READ))
		return;

	Buffer<char> buffer(ssh->chunk_size);
	auto n = read(STDIN_FILENO, buffer, ssh->chunk_size);
	if(n > 0)
		Send(String(buffer, n));
	else
	if(n == -1 && errno != EAGAIN)
		SetError(-1, "Couldn't read input from console.");
}

void SshShell::ConsoleWrite(const void* buffer, int len)
{
	if(!EventWait(STDOUT_FILENO, WAIT_WRITE))
		return;
	auto n = write(STDOUT_FILENO, buffer, len);
	if(n == -1 && errno != EAGAIN)
		SetError(-1, "Couldn't write output to console.");
}

void SshShell::ConsoleRawMode(bool b)
{
	if(!channel || mode != CONSOLE)
		return;

	if(b) {
		termios nflags;
		Zero(nflags);
		Zero(tflags);
		tcgetattr(STDIN_FILENO, &nflags);
		tflags = nflags;
		cfmakeraw(&nflags);
		tcsetattr(STDIN_FILENO, TCSANOW, &nflags);
	}
	else
		tcsetattr(STDIN_FILENO, TCSANOW, &tflags);
}

Size SshShell::GetConsolePageSize()
{
	winsize wsz;
	Zero(wsz);
	if(ioctl(STDIN_FILENO, TIOCGWINSZ, &wsz) == 0)
		return Size(wsz.ws_col, wsz.ws_row);
	LLOG("Warning: ioctl() failed. Couldn't read local terminal page size.");
	return Null;
}

#elif PLATFORM_WIN32

void SshShell::ConsoleRead()
{
	DWORD n = 0;
	const int RBUFSIZE = 1024 * 16;
	Buffer<char> buffer(RBUFSIZE);
	if(!ReadConsole(stdinput, buffer, RBUFSIZE, &n, NULL))
		SetError(-1, "Couldn't read input from console.");
	if(n > 0)
		Send(String(buffer, n));
}

void SshShell::ConsoleWrite(const void* buffer, int len)
{
	DWORD n = 0;
	if(!WriteConsole(stdoutput, buffer, len, &n, NULL))
		SetError(-1, "Couldn't Write output to console.");
}

void SshShell::ConsoleRawMode(bool b)
{
	if(!channel || mode != CONSOLE)
		return;

	if(b) {
		GetConsoleMode(stdinput, &tflags);
		DWORD nflags = tflags;
		nflags &= ~ENABLE_LINE_INPUT;
		nflags &= ~ENABLE_ECHO_INPUT;
		nflags |= ENABLE_WINDOW_INPUT;
		SetConsoleMode(stdinput, nflags);
	}
	else
		SetConsoleMode(stdinput, tflags);
}

Size SshShell::GetConsolePageSize()
{
	CONSOLE_SCREEN_BUFFER_INFO cinf;
	Zero(cinf);
	if(GetConsoleScreenBufferInfo((HANDLE) _get_osfhandle(1), &cinf))
		return Size(cinf.dwSize.X, cinf.dwSize.Y);
	LLOG("Warning: Couldn't read local terminal page size.");
	return Null;
}

#endif

SshShell::SshShell(SshSession& session)
: SshChannel(session)
{
	ssh->otype	= SHELL;
	mode		= GENERIC;
	resized		= false;
#ifdef PLATFORM_WIN32
	stdinput	= NULL;
	stdoutput	= NULL;
#endif
	Zero(tflags);
}

SshShell::~SshShell()
{
	ConsoleRawMode(false);
}

}