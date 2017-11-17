#include <Core/Core.h>
#include <SSH/SSH.h>

using namespace Upp;

// This SSH/SFtp/Exec example demonstrates:
//
// 1) Blocking and non-blocking operations, using NonBocking() switch.
// 2) Pick behaviour.
// 3) Multithreading support, and MT error management. (experimental)
// 4) Higher level methods. (such as testing the existence of files, dirs, sockets, etc.)
// 5) Getting files directly via/to streams.
// 6) Getting files as strings.
// 7) Peeking at remote files.
// 8) "Harvesing" results in non-blocking mode.
// 9) Getting directory listing for single file, using SshExec.
//


const char	*host = "test.rebex.net"; // A well known [S]FTP public test server.
const char	*user = "demo";
const char	*pass = "password";
const char	*dir  = "/pub/example";
const char	*file = "/pub/example/readme.txt";
const char	*cmd  = "ls -l ";

void GetDirectoryList(SshSession& session)
{
	DLOG("---- Getting directory listing of " << dir << " (blocking):");
	auto sftp = session.CreateSFtp();
	SFtp::DirList list;
	if(sftp.ListDir(dir, list)) {
		for(auto& e : list)
			DDUMP(e);		// SFtp::DirEntry has ToString() method.
		                    // a ToXml() method will be added too.
	}
	else DDUMP(sftp.GetErrorDesc());
}

void GetFileSize(SshSession& session)
{
	DLOG("---- Getting file size of " << file << " (non-blocking):");
	auto sftp = session.CreateSFtp();
	sftp.NonBlocking().GetSize(file);
	while(sftp.Do())
		Sleep(1);
	if(sftp.IsError())
		DDUMP(sftp.GetErrorDesc());
	else
		DDUMP((int64) sftp.GetResult());	// In non-blocking mode, results can be
											// "harvested" using GetResult() method.
}

void FileExists(SshSession& session)
{
	DLOG("---- Test if " << file << " exists (non-blocking):");
	auto sftp = session.CreateSFtp();
	sftp.NonBlocking().FileExists(file);
	while(sftp.Do())
		Sleep(1);
	if(sftp.IsError())
		DDUMP(sftp.GetErrorDesc());
	else
		DDUMP((bool) sftp.GetResult());
}

void GetFileViaStreams(SshSession& session)
{
	DLOG("---- Downloading file " << file << " directly to Cout() (blocking):");
	auto sftp = session.CreateSFtp();
	if(!sftp.Get(file, Cout()))
		DDUMP(sftp.GetErrorDesc());
	else
		DLOG("Done.");
}

void GetFileViaAsyncWorker(SshSession& session)
{
	// ! EXPERIMANTAL !
	DLOG("---- MT and error handling, using AsyncWork (worker threads):");
	try {
		
		SFtp::AsyncGet(session, "nosuchfile").Get();
	}
	catch(Ssh::Error& e) {
		DDUMP(e);
	}
}

void GetFileAsString(SshSession& session)
{
	DLOG("---- Downloading file " << file << " as string (non-blocking) and demonstrating pick behaviour:");
	auto sftp = session.CreateSFtp();
	sftp.NonBlocking().Get(file);
	while(sftp.Do())
		Sleep(1);

	auto sftp2 = pick(sftp);

	DLOG("'sftp' is picked!");

	if(sftp2.IsError())
		DDUMP(sftp2.GetErrorDesc());
	else
		DDUMP((String) sftp2.GetResult());
}


void PeekFile(SshSession& session)
{
	int64 offset = 12;
	int64 length = 38;
	DLOG("---- Peeking at file " << file << " at offset " <<  offset << " with length: " << length << " (blocking):");
	auto sftp = session.CreateSFtp();
	String peeked = sftp.Peek(file, offset, length);
	if(sftp.IsError())
		DDUMP(sftp.GetErrorDesc());
	else
		DDUMP(peeked);
}

void GetFileInfoViaExec(SshSession& session)
{
	DLOG("---- Getting directory listing of " << file << " using SshExec (blocking):");
	auto exec = session.CreateExec();
	DDUMP(exec(String(cmd) + file, Cout(), Cerr()));
	
}

CONSOLE_APP_MAIN
{

//	Ssh::Trace();
//	Ssh::TraceVerbose();

	StdLogSetup(LOG_COUT|LOG_FILE);

	DLOG("Starting to work...");
	SshSession session;
	if(session.Timeout(10000).Connect(host, 22, user, pass)) {
		GetDirectoryList(session);
		GetFileSize(session);
		FileExists(session);
		GetFileViaStreams(session);
		GetFileViaAsyncWorker(session);
		GetFileAsString(session);
		PeekFile(session);
		GetFileInfoViaExec(session);
	}
	else Cerr() << session.GetErrorDesc() << '\n';
}