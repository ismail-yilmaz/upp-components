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
typedef LIBSSH2_AGENT			SshAgent;

namespace SSH {
    extern bool sTrace;
    extern bool sTraceVerbose;
    extern String GetName(int type, int64 id);
}

class Ssh;
class SshSession;
class SFtp;
class SshChannel;
class SshExec;
class Scp;
class SshHosts;

#include "Core.h"
#include "Session.h"
#include "SFtp.h"
#include "Channels.h"
#include "Hosts.h"

void ssh_keyboard_callback(const char *name, int name_len, const char *instruction,
    int instruction_len, int num_prompts, const LIBSSH2_USERAUTH_KBDINT_PROMPT *prompts,
    LIBSSH2_USERAUTH_KBDINT_RESPONSE *responses, void **abstract);
}
#endif
