#include "stdafx.h"
#include "IFThreadSyncObj.h"
#ifndef WIN32
#include <pthread.h>
#include <sys/time.h>
#endif

IFThreadSyncObj::IFThreadSyncObj()
{
#ifdef WIN32
	m_Event = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
#ifdef ANDROID
	pthread_mutexattr_t  att = PTHREAD_MUTEX_RECURSIVE_NP;
#else
	pthread_mutexattr_t  att;
	pthread_mutexattr_init(&att);
	pthread_mutexattr_settype(&att, PTHREAD_MUTEX_RECURSIVE);
#endif

	pthread_cond_init(&m_Event, NULL);
	pthread_mutex_init(&m_Mutex, &att);
#endif
}

bool IFThreadSyncObj::wait(IFUI32 dwTimeout)
{
#ifdef WIN32
	return WAIT_OBJECT_0 == WaitForSingleObject(m_Event, dwTimeout);
#else
	pthread_mutex_lock(&m_Mutex);
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

	pthread_mutex_unlock(&m_Mutex);
	//if(r!=0)
	//  {
	//    printf("wait error:%d\n", r);
	//  }
	return r==0;
#endif
}

void IFThreadSyncObj::notify()
{
#ifdef WIN32
	SetEvent(m_Event);
#else
	pthread_cond_signal(&m_Event);
#endif
}

IFThreadSyncObj::~IFThreadSyncObj()
{
#ifdef WIN32
	CloseHandle(m_Event);
#else
	pthread_cond_destroy(&m_Event);
	pthread_mutex_destroy(&m_Mutex);
#endif
}
