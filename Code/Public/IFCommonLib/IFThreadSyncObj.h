#pragma once
#include "IFRefObj.h"

class IFCOMMON_API IFThreadSyncObj : public IFRefObj
{
public:
	IFThreadSyncObj();

	bool wait(IFUI32 dwTimeout);
	void notify();

protected:
	~IFThreadSyncObj();
#ifdef WIN32

	HANDLE m_Event;
#else
	pthread_cond_t m_Event;
	pthread_mutex_t m_Mutex;
#endif
};