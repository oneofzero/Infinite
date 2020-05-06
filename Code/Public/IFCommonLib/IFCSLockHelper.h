/*
The MIT License (MIT)
Copyright © 2014 Huang Cong

Permission is hereby granted, free of charge, to any person obtaining a 
copy of this software and associated documentation files (the “Software”), 
to deal in the Software without restriction, including without limitation 
the rights to use, copy, modify, merge, publish, distribute, sublicense, 
and/or sell copies of the Software, and to permit persons to whom the 
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS 
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
THE SOFTWARE.
*/
#pragma once
#include "IFCommonLib_API.h"
#include "IFObj.h"
#include "IFPlatformDefine.h"
#ifdef WIN32

class IFCOMMON_API IFCSLock  : public IFMemObj
{
private:
	IFCSLock(const IFCSLock& o )
	{

	}
	IFCSLock& operator = (const IFCSLock& o)
	{
		return *this;
	}
public:
	IFCSLock()
	{
#if defined(IFPLATFORM_WINDOWS_SHOP) || defined(IFPLATFORM_WP)
		InitializeCriticalSectionEx(&m_lock,0,0);
#else
		InitializeCriticalSection(&m_lock);
#endif
	}

	~IFCSLock()
	{
		DeleteCriticalSection(&m_lock);
	}
	
	inline void lock()
	{
		EnterCriticalSection(&m_lock);

	}

	inline void unlock()
	{
		LeaveCriticalSection(&m_lock);

	}

	operator  LPCRITICAL_SECTION()
	{
		return &m_lock;
	}

	CRITICAL_SECTION m_lock;
};

class IFCSLockEmpty  : public IFMemObj
{
public:
	inline void lock()
	{

	}
	inline void unlock()
	{

	}

};

#else

#include <pthread.h>

class IFCOMMON_API IFCSLock : public IFMemObj
{
public:
	IFCSLock()
	{
	    #ifdef ANDROID
		pthread_mutexattr_t  att = PTHREAD_MUTEX_RECURSIVE_NP;
		#else
		pthread_mutexattr_t  att;
		pthread_mutexattr_init(&att);
		pthread_mutexattr_settype(&att, PTHREAD_MUTEX_RECURSIVE);
		#endif
		pthread_mutex_init(&m_lock, &att);
	}

	~IFCSLock()
	{
		pthread_mutex_destroy(&m_lock);
	}

	inline void lock()
	{
		pthread_mutex_lock(&m_lock);

	}

	inline void unlock()
	{
		pthread_mutex_unlock(&m_lock);

	}

	operator  pthread_mutex_t*()
	{
		return &m_lock;
	}
	//pthread_mutex_t mtx
	pthread_mutex_t m_lock;
};


#	endif

template<class LOCK>
class  IFLockHelper
{
public:
	IFLockHelper(LOCK& cs):m_pCS(&cs)
	{
		m_pCS->lock();
	}
	IFLockHelper(LOCK* cs):m_pCS(cs)
	{
		if(m_pCS)
			m_pCS->lock();
	}
	~IFLockHelper()
	{
		if(m_pCS)
			m_pCS->unlock();
	}

	LOCK* m_pCS;
};
typedef IFLockHelper<IFCSLock> IFCSLockHelper;