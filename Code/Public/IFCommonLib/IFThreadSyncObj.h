#pragma once
#ifndef __IF_THREAD_SYNC_OBJ_H__
#define __IF_THREAD_SYNC_OBJ_H__
#ifndef IFTHREAD_NOT_ENABLE
#include "IFRefObj.h"

class IFCOMMON_API IFThreadSyncObj : public IFRefObj
{
public:
	IFThreadSyncObj();

	bool wait(IFUI32 dwTimeout);
	void notify();

protected:
	~IFThreadSyncObj();
	int m_nSetCount;
#ifdef IFTHREAD_USE_PTHREAD
	pthread_cond_t m_Event;
	pthread_mutex_t m_Mutex;
	bool m_notified;
#elif defined(IFTHREAD_USE_STD_THREAD)

#elif defined(IFTHREAD_USE_EMBED_THREAD)
#else
	HANDLE m_Event;
#endif
};
#endif

#endif //__IF_THREAD_SYNC_OBJ_H__