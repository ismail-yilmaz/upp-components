#ifndef _SSH_SSH_h
#define _SSH_SSH_h

#include <Core/Core.h>

#include "libssh2/libssh2.h"
#include "libssh2/libssh2_sftp.h"
#include "libssh2/libssh2_publickey.h"

namespace Upp {

INITIALIZE(SSH);

typedef LIBSSH2_SFTP_HANDLE     SFtpHandle;
typedef LIBSSH2_SFTP_ATTRIBUTES SFtpAttrs;
typedef libssh2_knownhost       SshHost;
typedef LIBSSH2_AGENT           SshAgent;

namespace SSH {
    extern bool sTrace;
    extern int  sTraceVerbose;
    extern String GetName(int type, int64 id);
}

class Ssh;
class SshSession;
class SFtp;
class SshChannel;
class Scp;
class SshExec;
class SshHosts;

#include "Core.h"
#include "Session.h"
#include "SFtp.h"
#include "Channels.h"
#include "Hosts.h"
}
#endif
