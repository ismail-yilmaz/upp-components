#include "HtmlTools.h"

namespace Upp {
	
#define MLOG(x) //  LOG(x)

#ifdef UPP_HEAP

static std::atomic<int64> UPP_Tidy_alloc;

static void TIDY_CALL *tidy_alloc(size_t size)
{
	size_t alloc = size + sizeof(int64);
	int64 *aptr = (int64 *)MemoryAllocSz(alloc);
	*aptr++ = (int64)alloc;
	UPP_Tidy_alloc += alloc;
	MLOG("UPP_Tidy_MALLOC(" << (int64)size << ", size " << size
	     << ") -> " << aptr << ", MemorySize: " << GetMemoryBlockSize(aptr)
	     << ", total = " << (int64) UPP_Tidy_alloc << ", thread: " << Thread::GetCurrentId());
	return aptr;
}

static void TIDY_CALL tidy_free(void *ptr)
{
	if(!ptr)
		return;
	int64 *aptr = (int64 *)ptr - 1;
	UPP_Tidy_alloc -= *aptr;
	MLOG("UPP_Tidy_FREE(" << ptr << ", size " << *aptr
	                     << "), MemorySize: " << GetMemoryBlockSize(aptr) << ", total = " << (int64)  UPP_Tidy_alloc << ", thread: " << Thread::GetCurrentId());
	if(*aptr !=  GetMemoryBlockSize(aptr))
		Panic("tidy_free corrupted");
	MemoryFree(aptr);
}

static void TIDY_CALL *tidy_realloc(void *ptr, size_t size)
{
	MLOG("tidy_realloc " << ptr << ", " << size);
	if(!ptr) {
		MLOG("UPP_Tidy_REALLOC by Alloc:");
		return tidy_alloc(size);
	}
	int64 *aptr = (int64 *)ptr - 1;
	if((int64)(size + sizeof(int64)) <= *aptr) { // TODO: Do we really want this?
		MLOG("UPP_Tidy_REALLOC(" << ptr << ", " << (int64)size << ", alloc " << *aptr << ") -> keep same block" << ", thread: " << Thread::GetCurrentId());
		return ptr;
	}
	size_t newalloc = size + sizeof(int64);
	int64 *newaptr = (int64 *)MemoryAllocSz(newalloc);
	if(!newaptr) {
		MLOG("UPP_Tidy_REALLOC(" << ptr << ", " << (int64)size << ", alloc " << newalloc << ") -> fail" << ", thread: " << Thread::GetCurrentId());
		return NULL;
	}
	*newaptr++ = newalloc;
	memcpy(newaptr, ptr, min<int>((int)(*aptr - sizeof(int64)), (int)size));
	UPP_Tidy_alloc += newalloc - *aptr;
	MLOG("UPP_Tidy_REALLOC(" << ptr << ", " << (int64)size << ", alloc " << newalloc
	     << ") -> " << newaptr << ", total = " << (int64) UPP_Tidy_alloc << ", thread: " << Thread::GetCurrentId());
	MemoryFree(aptr);
	return newaptr;	
}

static void TIDY_CALL tidy_panic(ctmbstr msg)
{
	Panic((const char *) msg);
}

INITIALIZER(HtmlTools)
{
	tidySetMallocCall(tidy_alloc);
	tidySetReallocCall(tidy_realloc);
	tidySetFreeCall(tidy_free);
	tidySetPanicCall(tidy_panic);
}

#endif
}