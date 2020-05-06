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
#include "IFRefObj.h"
#include "IFFunctor.h"
#include "IFPlatformDefine.h"
#ifdef LINUX
#include <pthread.h>
#else
#if defined(IFPLATFORM_WINDOWS_SHOP) || defined(IFPLATFORM_WP) || defined(__APPLE__) || defined(IFPLATFORM_WEB)
#include <thread>
#define IFTHREAD_SUPPORT_STD_THREAD true
#endif
#endif

class IFThreadSyncObj;

class IFCOMMON_API IFThread : public IFRefObj
{
public:
	IFThread(void);



	bool start( IFFunctor<void()>* pFunctor );

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

#ifdef LINUX

	static void* run(void*);
#else
#ifdef IFTHREAD_SUPPORT_STD_THREAD
	void run();
#else
	static DWORD WINAPI run(LPVOID);
#endif
#endif

	IFRefPtr<IFFunctor<void()> > m_spFunctor;

	bool m_bNeedExit;
	bool m_bRunning;
	bool m_bStarted;
	IFCSLock m_Lock;
	IFRefPtr<IFThreadSyncObj> m_spSyncObj;
#ifdef LINUX

	pthread_t m_threadid;
#else
#ifdef IFTHREAD_SUPPORT_STD_THREAD
	std::thread* m_pThread;
#else
	HANDLE m_hThread;
#endif
#endif

};

