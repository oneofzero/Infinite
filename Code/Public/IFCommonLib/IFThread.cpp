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
#include "stdafx.h"
#include "IFThread.h"
#include "IFThreadSyncObj.h"
#ifdef WIN32
#include <process.h>
#if  WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
#include <ppltasks.h>
#endif
#else
#ifndef ANDROID
#include<signal.h>
#include<unistd.h>
#else
#include <unistd.h>
#endif
#endif
#include "IFLogSystem.h"
#ifdef MAC
#include <pthread.h>
#endif


IFThread::IFThread(void):
	IFRefObj(true),
#ifndef LINUX

#ifdef IFTHREAD_SUPPORT_STD_THREAD
	m_pThread(NULL),
#else
	m_hThread(NULL),
#endif
#else
	m_threadid(0),
#endif
	m_bRunning(false),
	m_bNeedExit(false)
{
	m_spSyncObj = IFNew IFThreadSyncObj();
}


IFThread::~IFThread(void)
{
#ifndef LINUX
#ifdef IFTHREAD_SUPPORT_STD_THREAD
	
#else
	if (m_hThread)
	{
		CloseHandle(m_hThread);
	}
#endif
#endif
}



bool IFThread::start( IFFunctor<void()>* pFunctor )
{
	m_bNeedExit = false;
#ifdef LINUX
	if (m_threadid)
	{
		terminate();
	}
	m_bRunning = true;
	m_spFunctor = pFunctor;

	pthread_create(&m_threadid, NULL, run, new IFRefPtr<IFThread>(this));


#else
#ifdef IFTHREAD_SUPPORT_STD_THREAD
	if (m_pThread)
	{
		terminate();
	}
	//m_bRunning = true;
	m_spFunctor = pFunctor;
	addRef();
	m_pThread = new std::thread(&IFThread::run, this);
#else
	if (m_bRunning)
		return false;

	if (m_hThread)
		CloseHandle(m_hThread);


	DWORD dwThreadID;
	m_spFunctor = pFunctor;
	m_hThread = CreateThread(NULL,0,run, new IFRefPtr<IFThread>(this), 0, &dwThreadID);
	//while (	!m_bRunning );
#endif
	//m_pThread->detach();
#endif
	return true;
}

bool IFThread::terminate()
{
#ifdef LINUX

	if (m_threadid)
	{
		while (m_bRunning)
		{
			sleep(10);
		}
		pthread_kill(m_threadid,0);
		m_threadid = 0;
	}
#else


#ifdef IFTHREAD_SUPPORT_STD_THREAD
	if(m_pThread)
	{
		//std::terminate();
		delete m_pThread;
	}
	m_pThread = NULL;

#else
	TerminateThread(m_hThread,0);
#endif

#endif
	return true;
}

bool IFThread::isRunning()
{
	return m_bRunning;
}
#ifdef LINUX
void* IFThread::run(void* p)
{	
	IFLOG(IFLL_TRACE, "thread begin\r\n");
	IFRefPtr<IFThread> pThread = *(IFRefPtr<IFThread>*)p;
	pThread->m_bRunning = true;
	(*pThread->m_spFunctor)();
	pThread->m_bRunning = false;
	delete (IFRefPtr<IFThread>*)p;

	return NULL;

}
#else
#ifndef IFTHREAD_SUPPORT_STD_THREAD
DWORD IFThread::run(LPVOID p)
{

	IFRefPtr<IFThread> pThread = *(IFRefPtr<IFThread>*)p;
	pThread->m_bRunning = true;
	(*pThread->m_spFunctor)();
	pThread->m_bRunning = false;
	pThread->m_spFunctor = NULL;
	delete (IFRefPtr<IFThread>*)p;
	return 0;
}
#else
void IFThread::run()

{
	m_bRunning = true;
	(*m_spFunctor)();
	m_bRunning = false;
	decRef();
}
#endif
#endif

void IFThread::sleep( IFUI32 ms )
{
#ifdef LINUX
	::usleep(ms*1000);
#else
#ifdef IFTHREAD_SUPPORT_STD_THREAD
	std::chrono::milliseconds dura( ms );
	std::this_thread::sleep_for(dura);
#else
	::Sleep(ms);
#endif
#endif
}

bool IFThread::waitExit()
{
#ifdef LINUX
	//::usleep(ms*1000);
	void* val;
	pthread_join(m_threadid, &val);
#else
#ifdef IFTHREAD_SUPPORT_STD_THREAD
	m_pThread->join();
#else
	WaitForSingleObject(m_hThread,INFINITE);
#endif
#endif
	return true;
}

int IFThread::getCurrentThreadID()
{
#if defined(LINUX) || defined(IFPLATFORM_WEB)
	return pthread_self();
#elif defined(MAC)
    pthread_t p = pthread_self();
    return p->__sig;
#else

	return ::GetCurrentThreadId();
#endif
}

void IFThread::requestExit()
{
	m_bNeedExit = true;
	m_spSyncObj->notify();
}

bool IFThread::isNeedExit()
{
	return m_bNeedExit;
}



