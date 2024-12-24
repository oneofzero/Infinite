#include "stdafx.h"
#include "IFThreadSyncObj.h"
#include "IFAtomicOperation.h"
#ifdef IFTHREAD_USE_PTHREAD
#include <pthread.h>
#include <sys/time.h>
#endif

IFThreadSyncObj::IFThreadSyncObj()
{
#ifdef WIN32
	m_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
#	ifdef IFTHREAD_USE_PTHREAD
#	ifdef ANDROID
	pthread_mutexattr_t  att = PTHREAD_MUTEX_RECURSIVE_NP;
#	else
	pthread_mutexattr_t  att;
	pthread_mutexattr_init(&att);
	pthread_mutexattr_settype(&att, PTHREAD_MUTEX_RECURSIVE);
#	endif

#	ifdef IFPLATFORM_FREE_RTOS
	//pthread_cond_init(&m_Event, NULL);
	m_Event = PTHREAD_COND_INITIALIZER;
#	else
	pthread_cond_init(&m_Event, NULL);
#	endif

	pthread_mutex_init(&m_Mutex, &att);
	m_notified = false;
#	endif
#endif
}

bool IFThreadSyncObj::wait(IFUI32 dwTimeout)
{
#ifdef WIN32
	return WAIT_OBJECT_0 == WaitForSingleObject(m_Event, dwTimeout);

#else
#	ifdef IFTHREAD_USE_PTHREAD
	pthread_mutex_lock(&m_Mutex);
	if (m_notified)
	{
		m_notified = false;
		pthread_mutex_unlock(&m_Mutex);
		return true;
	}
	int r;
	if (dwTimeout == 0xffffffff)
		r = pthread_cond_wait(&m_Event, &m_Mutex);
	else
	{
		struct timeval now;
		int rt;
		timespec tv;
		gettimeofday(&now, NULL);


		tv.tv_sec = now.tv_sec + (dwTimeout/1000);
		tv.tv_nsec = (now.tv_usec + (dwTimeout % 1000) * 1000) * 1000;


		r = pthread_cond_timedwait(&m_Event, &m_Mutex, &tv);
	}
	m_notified = false;
	pthread_mutex_unlock(&m_Mutex);
	//if(r!=0)
	//  {
	//    printf("wait error:%d\n", r);
	//  }
	return r==0;
#	else
	return false;
#endif

#endif
}

void IFThreadSyncObj::notify()
{
	ATOMIC_INC_INT32(&m_nSetCount);
#ifdef WIN32
	SetEvent(m_Event);
#else
#	ifdef IFTHREAD_USE_PTHREAD
	pthread_mutex_lock(&m_Mutex);
	m_notified = true;
	pthread_cond_signal(&m_Event);
	pthread_mutex_unlock(&m_Mutex);
#	else
#	endif
#endif
}

IFThreadSyncObj::~IFThreadSyncObj()
{
#ifdef WIN32
	CloseHandle(m_Event);
#else
#	ifdef IFTHREAD_USE_PTHREAD
	pthread_cond_destroy(&m_Event);
	pthread_mutex_destroy(&m_Mutex);
#	else

#	endif
#endif
}
