#include "PtyProcess.h"

namespace Upp {

#define LLOG(x)	 RLOG("PtyProcess [WIN32]: " << x);
	
#ifdef PLATFORM_WIN32

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

void sCmdToUnicode(Buffer<wchar>& cmd, const char *cmdptr)
{
	if(cmdptr) {
		WString wcmd(cmdptr);
		int len = wcmd.GetCount() + 1;
		cmd.Alloc(len);
		memcpy(cmd, ~wcmd, len * sizeof(wchar));
	}
}

void sEnvToUnicode(Buffer<wchar>& env, const char *envptr)
{
	if(envptr) {
		int len = 0;
		while(envptr[len] || envptr[len + 1]) len++;
		WString wenv(envptr, len + 1);
		env.Alloc(len + 2);
		memcpy(env, ~wenv, (len + 2) * sizeof(wchar));
	}
}

#ifdef  flagWIN10
bool Win32CreateProcess(const char *cmdptr, const char *envptr, STARTUPINFOEX& si, PROCESS_INFORMATION& pi, const char *cd)
{
	Buffer<wchar> cmd;
	sCmdToUnicode(cmd, cmdptr);
#if 0 // TODO: test this later...
	Buffer<wchar> env;
	sEnvToUnicode(env, envptr);
#endif
	return CreateProcessW(
		nullptr,
		cmd,
		nullptr,
		nullptr,
		FALSE,
		EXTENDED_STARTUPINFO_PRESENT,
		(void *) envptr,
		cd ? ~WString(cd) : nullptr,
		(LPSTARTUPINFOW) &si.StartupInfo,
		&pi);
}

#else

HANDLE WinPtyCreateProcess(const char *cmdptr, const char *envptr, const char *cd, winpty_t* hConsole)
{
	Buffer<wchar> cmd, env;
	sCmdToUnicode(cmd, cmdptr);
	sEnvToUnicode(env, envptr);

	auto hSpawnConfig = winpty_spawn_config_new(
		WINPTY_SPAWN_FLAG_AUTO_SHUTDOWN,
		cmd,
		nullptr,
		cd ? ~WString(cd) : nullptr,
		env,
		nullptr);
		
	if(!hSpawnConfig) {
		LLOG("winpty_spawn_config_new() failed.");
		return nullptr;
	}
	
	HANDLE hProcess = nullptr;
	
	auto success = winpty_spawn(
		hConsole,
		hSpawnConfig,
		&hProcess,
		nullptr,
		nullptr,
		nullptr);

	winpty_spawn_config_free(hSpawnConfig);
	
	if(!success) {
		LLOG("winpty_spawn() failed.");
		return nullptr;
	}

	return hProcess;
}
#endif

void PtyProcess::Init()
{
	hProcess       = nullptr;
	hConsole       = nullptr;
	hOutputRead    = nullptr;
	hErrorRead     = nullptr;
	hInputWrite    = nullptr;
#ifdef flagWIN10
	hProcAttrList  = nullptr;
#endif
	cSize          = Null;
	convertcharset = false;
	exit_code      = Null;
}

void PtyProcess::Free()
{
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
#ifndef flagWIN10
	if(hConsole) {
		winpty_free(hConsole);
		hConsole = nullptr;
	}
#else
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
	for(int i = 0; i < env.GetCount(); i++) {
		senv << env.GetKey(i) << "=" << env[i] << '\0';
	}
	return DoStart(cmdline, nullptr, ~senv, cd);
}

bool PtyProcess::DoStart(const char *cmd, const Vector<String> *args, const char *env, const char *cd)
{
	Kill();
	exit_code = Null;

	String command = sParseArgs(cmd, args);
	if(command.IsEmpty()) {
		LLOG("Couldn't parse arguments.");
		Free();
		return false;
	}

#ifdef flagWIN10

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

	if(!UpdateProcThreadAttribute(
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
		LLOG("Win32CreateProcess() failed.");
		Free();
		return false;
	}

#else

	auto hAgentConfig = winpty_config_new(0, nullptr);
	if(!hAgentConfig) {
		LLOG("winpty_config_new() failed.");
		Free();
		return false;
	}

	winpty_config_set_initial_size(hAgentConfig, 80, 24);
		
	hConsole = winpty_open(hAgentConfig, nullptr);
	winpty_config_free(hAgentConfig);
	if(!hConsole) {
		LLOG("winpty_open() failed.");
		Free();
		return false;
	}
	
	hInputWrite = CreateFileW(
		winpty_conin_name(hConsole),
		GENERIC_WRITE,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr);

	hOutputRead = CreateFileW(
		winpty_conout_name(hConsole),
		GENERIC_READ,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr);
	
	hErrorRead = CreateFileW(
		winpty_conerr_name(hConsole),
		GENERIC_READ,
		0,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr);

	if(!hInputWrite || !hOutputRead || !hErrorRead) {
		LLOG("Couldn't create file I/O handles.");
		Free();
		return false;
	}
	
	hProcess = WinPtyCreateProcess(~command, env, cd, hConsole);
	if(!hProcess) {
		LLOG("WinPtyCreateProcess() failed.");
		Free();
		return false;
	}
	
#endif
	return true;
}

bool PtyProcess::Read(String& s)
{
	String rread;
	constexpr int BUFSIZE = 4096;

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
}

void PtyProcess::Write(String s)
{
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
}

void PtyProcess::Kill()
{
	if(hProcess && IsRunning()) {
		TerminateProcess(hProcess, (DWORD)-1);
		exit_code = 255;
	}
	Free();
}

int PtyProcess::GetExitCode()
{
	return IsRunning() ? (int) Null : exit_code;
}

HANDLE PtyProcess::GetProcessHandle() const
{
	return hProcess;
}

bool PtyProcess::IsRunning()
{
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
	return false;
}

bool PtyProcess::SetSize(Size sz)
{
	if(hConsole) {
#ifdef flagWIN10
		COORD size;
		size.X = (SHORT) max(2, sz.cx);
		size.Y = (SHORT) max(2, sz.cy);
		if(ResizePseudoConsole(hConsole, size) == S_OK) {
#else
		if(winpty_set_size(hConsole, max(2, sz.cx), max(2, sz.cy), nullptr)) {
#endif
			LLOG("Pty size is set to: " << sz);
			cSize = sz;
			return true;
		}
	}
	cSize = Null;
	LLOG("Couldn't set pty size!");
	return false;
}

Size PtyProcess::GetSize()
{
	if(hConsole && !IsNull(cSize)) {
		LLOG("Fetched pty size: " << cSize);
		return cSize;
	}
	LLOG("Couldn't fetch pty size!");
	return Null;
}
#endif

}