#include "SSH.h"

namespace Upp {

#define LLOG(x)       do { if(SSH::sTrace) RLOG("Scp #" << core->oid << " " << x); } while(false)
#define LDUMPHEX(x)	  do { if(SSH::sTraceVerbose) RDUMPHEX(x); } while(false)

bool Scp::Get(Stream& out, const String& path, Gate<int64, int64> progress)
{
	return ComplexCmd(FGET, [=, &out]() mutable {
		scp->data = Null;
		Cmd(FGET, [=]() mutable {
			if(!scp || path.IsEmpty())
				SetError(-1, "Path is not set.");
			scp->channel = libssh2_scp_recv2(core->session, path, &scp->fstat);
			if(!scp->channel && !WouldBlock())
				SetError(WouldBlock());
			if(scp->channel)
				LLOG("Channel obtained.");
			return scp->channel != NULL;
		});
		Cmd(FGET, [=, &out]() mutable {
			return DataRead(0, out, scp->fstat.st_size, pick(progress));
		});
	});
}

String Scp::Get(const String& path, Gate<int64, int64> progress)
{
	ComplexCmd(FGET, [=]() mutable {
		chdata->data.Create();
		Get(chdata->data, path, pick(progress));
	});
	return core->async ? Null : GetResult(); 
}

bool Scp::Put(Stream& in, const String& path, long mode, Gate<int64, int64> progress)
{
	return ComplexCmd(FGET, [=, &in]() mutable {
		scp->data = Null;
		Cmd(FPUT, [=, &in]() mutable {
			if(!scp || path.IsEmpty())
				SetError(-1, "Path is not set.");
			scp->channel = libssh2_scp_send64(core->session, path, mode, in.GetSize(), 0, 0);
			if(!scp->channel && !WouldBlock())
				SetError(WouldBlock());
			if(scp->channel)
				LLOG("Channel obtained.");
			return scp->channel != NULL;
		});
		Cmd(FPUT, [=, &in]() mutable {
			return DataWrite(0, in, in.GetSize(), pick(progress));
		});
	});

}


}