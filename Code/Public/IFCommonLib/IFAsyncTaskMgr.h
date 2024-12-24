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
#ifndef __IF_ASYNC_TASK_MGR_H__
#define __IF_ASYNC_TASK_MGR_H__

#include "IFRefPtr.h"
#include "ifsingleton.h"
#include "IFList.h"
#include "IFCommonLib_API.h"
#include "IFEventSlot.h"
#include "IFThread.h"
#include "IFQueue.h"
//#include "IFAsyncResult.h"
//#ifndef IFTHREAD_NOT_ENABLE

class IFThread;

enum IFAsyncTaskState
{
	IFATS_WAIT_EXECUTE,
	IFATS_EXECUTING,
	IFATS_DONE,
};

class IFAsyncTask;



class IFCOMMON_API IFAsyncTask : public IFRefObj
{
	IF_DECLARERTTI;

public:

	virtual bool cancel();

	bool isCancelled();

	IFAsyncTaskState getState();
protected:
	//IFAsyncTask(IFRefPtr<IFFunctor<void()>> spTaskFun);
	IFAsyncTask();
	~IFAsyncTask();

	virtual void execute() = 0;

	bool m_bCancel;

	IFAsyncTaskState m_eState;
	//IFRefPtr<IFFunctor<void()>> m_spTaskFun;
	friend class IFAsyncTaskMgr;
};



class IFAsyncResult;
template<typename T>
class IFAsyncResultT;
template<typename T>
class IFAsyncTaskWithResult;
class IFCOMMON_API IFAsyncTaskMgr :public IFMemObj,  public IFSingleton<IFAsyncTaskMgr>
{
public:

#ifndef IFTHREAD_NOT_ENABLE

	int getMaxWorkThreadCount();

	template<typename FUN>
	IFRefPtr<IFAsyncTaskWithResult<bool>> addTask(FUN fun)
	{
		return addTask(makeIFFunctor<bool()>(fun));
	}
	template<typename RT, typename FUN>
	IFRefPtr<IFAsyncTaskWithResult<RT>> addTask(FUN fun)
	{
		return addTask(makeIFFunctor<RT()>(fun));
	}

	template<typename RT>
	IFRefPtr<IFAsyncTaskWithResult<RT>> addTask(IFRefPtr<IFFunctor<RT()>> spTaskFun);
	//IFRefPtr<IFAsyncTask> addTask(IFRefPtr<IFFunctor<void()>> spTaskFun);
	
#endif
	void process();

	static void Create();
	static void Destroy();

private:
	IFAsyncTaskMgr(int nMaxThreadCount);
	~IFAsyncTaskMgr(void);

	void pushAsyncResult(IFAsyncResult* pResult);

#ifndef IFTHREAD_NOT_ENABLE

	void addTaskInternal(IFAsyncTask* pTask);

	int m_nMaxThreadCount;
	void workThread(int threadIdx);
	typedef IFArray<IFRefPtr<IFThread>> ThreadList;
	ThreadList m_WorkThreadList;
	IFRefPtr<IFThreadSyncObj> m_spWaitTaskSignal;
	IFQueue<IFRefPtr<IFAsyncTask>> m_waitQueue;

#endif

	IFQueue<IFRefPtr<IFAsyncResult>> m_asyncResults;

	//IFQueue<IFRefPtr<IFAsyncTask>> m_ComplelteQueue;


	

	template<typename T>
	friend class IFAsyncResultT;
};
#ifndef IFTHREAD_NOT_ENABLE
#include "IFAsyncResult.h"
template<typename RT>
inline IFRefPtr<IFAsyncTaskWithResult<RT>> IFAsyncTaskMgr::addTask(IFRefPtr<IFFunctor<RT()>> spTaskFun)
{
	IFRefPtr<IFAsyncTaskWithResult<RT>> spTask =  IFNew IFAsyncTaskWithResult<RT>(spTaskFun);

	addTaskInternal(spTask);
	return spTask;
}
#endif
//#endif

#endif IFPH_IF_ASYNC_TASK_MGR_H