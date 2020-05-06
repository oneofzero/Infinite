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
#include "IFAsyncTaskMgr.h"
#include "IFThread.h"
#include "IFFunctorDefaultParam.h"

IF_DEFINESINGLETON(IFAsyncTaskMgr);

IFAsyncTaskMgr::IFAsyncTaskMgr(int nMax)
	:m_nMaxThreadCount(nMax)
{
	for (int i = 0; i < nMax; i ++ )
	{
		IFRefPtr<IFAsyncTaskThread> spThread = IFNew IFAsyncTaskThread;
		m_WorkThreadList[spThread] = false;
		spThread->start(makeIFDPFunctor(this, &IFAsyncTaskMgr::workThread, makeIFDefaultParam<IFAsyncTaskThread*,bool*>(spThread,&m_WorkThreadList[spThread])));
	}
}


IFAsyncTaskMgr::~IFAsyncTaskMgr(void)
{
	for (ThreadList::iterator it = m_WorkThreadList.begin();
		it!=m_WorkThreadList.end(); ++it)
	{
		IFRefPtr<IFAsyncTask> spTask = it->first->m_spTask;
		if (spTask)
			spTask->cancel();
		it->second = true;
	}
	for (ThreadList::iterator it = m_WorkThreadList.begin();
		it!=m_WorkThreadList.end(); ++it)
	{
		it->first->waitExit();
	}

	m_WaitExecuteList.clear();
	m_ExecutedList.clear();
}



int IFAsyncTaskMgr::getMaxWorkThreadCount()
{
	return m_nMaxThreadCount;
}

bool IFAsyncTaskMgr::addTask( IFRefPtr<IFAsyncTask> spTask )
{
	if (spTask->getState() != IFATS_UNKNWON)
		return false;
	spTask->m_eState = IFATS_WAIT_EXECUTE;
	IFCSLockHelper lh(m_WaitExecuteListLock);
	m_WaitExecuteList.push_back(spTask);
	return true;
}

void IFAsyncTaskMgr::workThread(IFAsyncTaskThread* pThread, bool* bExit )
{
	while (!*bExit)
	{
		IFRefPtr<IFAsyncTask> spTask;
		m_WaitExecuteListLock.lock();
		if (m_WaitExecuteList.size())
		{
			spTask = m_WaitExecuteList.front();
			m_WaitExecuteList.pop_front();
		}
		m_WaitExecuteListLock.unlock();

		if (spTask)
		{
			pThread->m_spTask = spTask;
			spTask->m_eState = IFATS_EXECUTING;
			spTask->execute();
			pThread->m_spTask = NULL;
			spTask->m_eState = IFATS_DONE;
			m_ExecutedListLock.lock();
			m_ExecutedList.push_back(spTask);
			m_ExecutedListLock.unlock();
		}
		else
			IFThread::sleep(10);
	}

}

void IFAsyncTaskMgr::process()
{
	IFCSLockHelper lh(m_ExecutedListLock);
	if (m_ExecutedList.size())
	{
		for (TaskList::iterator it = m_ExecutedList.begin();
			it != m_ExecutedList.end(); ++it)
		{
			(*it)->event_TaskDone(*it);
		}
		m_ExecutedList.clear();
	}
}


IF_DEFINERTTI(IFAsyncTask,IFRefObj)

IFAsyncTask::IFAsyncTask()
:IFRefObj(true)
,m_bCancel(false)
,m_eState(IFATS_UNKNWON)
{

}

IFAsyncTask::~IFAsyncTask()
{

}

bool IFAsyncTask::cancel()
{
	m_bCancel = true;
	return true;
}

void IFAsyncTask::execute()
{

}

IFAsyncTaskState IFAsyncTask::getState()
{
	return m_eState;
}

bool IFAsyncTask::isCancelled()
{
	return m_bCancel;
}

IFAsyncTaskThread::~IFAsyncTaskThread()
{

}
