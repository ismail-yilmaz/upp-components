#include <CtrlLib/CtrlLib.h>
#include <CoRoutines/CoRoutines.h>


// GuiWebDownloader example, adapted to coroutines.

using namespace Upp;


CoRoutine<void> HttpDownload(const String& url)
{
	Progress pi;
	String path = AppendFileName(Nvl(GetDownloadFolder(), GetHomeDirFile("downloads")), GetFileName(url));
	HttpRequest http(url);
	http.Timeout(0);
	int loaded = 0;
	{
		FileOut out(path);
		http.WhenContent = [&out, &loaded](const void *ptr, int size) { out.Put(ptr, size); loaded += size; };
		while(http.Do()) {
			if(http.GetContentLength() >= 0) {
				pi.SetText("Downloading " + GetFileName(url));
				pi.Set((int)loaded, (int)http.GetContentLength());
			}
			else {
				pi.Set(0, 0);
				pi.SetText(http.GetPhaseName());
			}
			if(pi.Canceled())
				http.Abort();
			else
				co_await CoSuspend();
		}
	}
	if(http.IsFailure()) {
		DeleteFile(path);
		Exclamation("Download has failed.&\1" +
			(http.IsError()
				? http.GetErrorDesc()
					: AsString(http.GetStatusCode()) + ' ' + http.GetReasonPhrase()));
	}
}

GUI_APP_MAIN
{
	StdLogSetup(LOG_COUT|LOG_FILE);
	
	String url = "http://downloads.sourceforge.net/project/upp/upp/4179/upp-x11-src-4179.tar.gz";

	for(;;) {
		if(!EditText(url, "Download", "URL"))
			return;
		auto downloader = HttpDownload(url); // Multiple downloaders can be created/run at once.
		try
		{
			while(downloader.Do())
				Sleep(1); // Simulate work.
		}
		catch(const Exc& e)
		{
			
		}
		catch(...)
		{
			
		}
	}
}
