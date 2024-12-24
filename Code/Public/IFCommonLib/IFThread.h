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
#ifndef __IF_THREAD_H__
#define __IF_THREAD_H__

#include "IFRefObj.h"
#include "IFFunctor.h"
#include "IFPlatformDefine.h"

#ifdef IFTHREAD_USE_STD_THREAD
#include <thread>
#elif defined(IFTHREAD_USE_PTHREAD)
#include <pthread.h>
#endif

#ifndef IFTHREAD_NOT_ENABLE

#include "IFThreadSyncObj.h"

class IFCOMMON_API IFThread : public IFRefObj
{
public:
	IFThread(void);

	template<typename FUNT>
	bool start(FUNT fun)
	{
		return start(makeIFFunctor<void()>(fun));
	}

	bool start( IFRefPtr<IFFunctor<void()>> pFunctor );

	void requestExit();

	bool isNeedExit();

	bool waitExit();

	bool terminate();

	bool isRunning();

	IFThreadSyncObj* getSyncObj()
	{
		return m_spSyncObj;
	}

	static void sleep(IFUI32 ms);
	static int getCurrentThreadID();
protected:
	~IFThread(void);

#ifdef IFTHREAD_USE_PTHREAD

	static void* run(void*);
#elif defined(IFTHREAD_USE_STD_THREAD)
	void run();
#elif defined(IFTHREAD_USE_EMBED_THREAD)
#else
	static DWORD WINAPI run(LPVOID);
#endif

	IFRefPtr<IFFunctor<void()> > m_spFunctor;

	bool m_bNeedExit;
	bool m_bRunning;
	//bool m_bStarted;
	//IFCSLock m_Lock;
	IFRefPtr<IFThreadSyncObj> m_spSyncObj;
#ifdef IFTHREAD_USE_PTHREAD

	pthread_t m_threadid;
#elif defined(IFTHREAD_USE_STD_THREAD)
	std::thread* m_pThread;
#elif defined(IFTHREAD_USE_EMBED_THREAD)
#else
	HANDLE m_hThread;
#endif

};

#endif

#endif //__IF_THREAD_H__