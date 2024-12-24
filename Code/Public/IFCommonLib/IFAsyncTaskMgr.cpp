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
#include "IFAsyncResult.h"
#include "IFSystemAPI.h"

IF_DEFINESINGLETON(IFAsyncTaskMgr);

IFAsyncTaskMgr::IFAsyncTaskMgr(int nMax)
{
#ifndef IFTHREAD_NOT_ENABLE
	m_nMaxThreadCount = nMax;
	m_spWaitTaskSignal = NewIFRefObj<IFThreadSyncObj>();
	m_WorkThreadList.reserve(m_nMaxThreadCount);
	for (int i = 0; i < nMax; i ++ )
	{
		auto spThread = NewIFRefObj<IFThread>();
		m_WorkThreadList.push_back(spThread);
		spThread->start([=]() 
			{
				workThread(i);
			});
	}
#endif
}


IFAsyncTaskMgr::~IFAsyncTaskMgr(void)
{
#ifndef IFTHREAD_NOT_ENABLE
	for (auto spThread : m_WorkThreadList)
	{
		spThread->requestExit();
		m_spWaitTaskSignal->notify();
	}

	for (auto spThread : m_WorkThreadList)
	{
		spThread->waitExit();
	}	
#endif
}

#ifndef IFTHREAD_NOT_ENABLE

int IFAsyncTaskMgr::getMaxWorkThreadCount()
{
	return m_nMaxThreadCount;
}
#endif
void IFAsyncTaskMgr::pushAsyncResult(IFAsyncResult* pResult)
{
	m_asyncResults.push(pResult);
}
#ifndef IFTHREAD_NOT_ENABLE

void IFAsyncTaskMgr::addTaskInternal(IFAsyncTask* pTask)
{
	//auto spTask = NewIFRefObj<IFAsyncTask>(spTaskFun);
	m_waitQueue.push(pTask);
	m_spWaitTaskSignal->notify();
}

void IFAsyncTaskMgr::workThread(int threadIdx)
{
	while (!m_WorkThreadList[threadIdx]->isNeedExit())
	{
		m_spWaitTaskSignal->wait(100);
		IFRefPtr<IFAsyncTask> spTask;
		while (m_waitQueue.pop(spTask))
		{
			if(!spTask->isCancelled())
				spTask->execute();
		}
	}

}
#endif

void IFAsyncTaskMgr::process()
{

	/*IFCSLockHelper lh(m_ExecutedListLock);
	if (m_ExecutedList.size())
	{
		for (TaskList::iterator it = m_ExecutedList.begin();
			it != m_ExecutedList.end(); ++it)
		{
			(*it)->event_TaskDone(*it);
		}
		m_ExecutedList.clear();
	}*/
	decltype(m_asyncResults)::DataType spResult;
	while (m_asyncResults.pop(spResult))
	{
		spResult->notifyResult();
	}

}

void IFAsyncTaskMgr::Create()
{
	IFNew IFAsyncTaskMgr(IFNativeSystemAPI::getProcessorCount());
}

void IFAsyncTaskMgr::Destroy()
{
	delete IFAsyncTaskMgr::getSingletonPtr();
}


IF_DEFINERTTI(IFAsyncTask,IFRefObj)

IFAsyncTask::IFAsyncTask()
:IFRefObj(true)
,m_bCancel(false)
,m_eState(IFATS_WAIT_EXECUTE)
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


IFAsyncTaskState IFAsyncTask::getState()
{
	return m_eState;
}

bool IFAsyncTask::isCancelled()
{
	return m_bCancel;
}

