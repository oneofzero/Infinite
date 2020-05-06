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
#include "IFRefPtr.h"
#include "ifsingleton.h"
#include "IFList.h"
#include "IFCommonLib_API.h"
#include "IFEventSlot.h"
#include "IFThread.h"

class IFThread;

enum IFAsyncTaskState
{
	IFATS_UNKNWON,
	IFATS_WAIT_EXECUTE,
	IFATS_EXECUTING,
	IFATS_DONE,
};

class IFAsyncTask;

class IFAsyncTaskThread : public IFThread
{
public:

protected:
	~IFAsyncTaskThread();

	IFRefPtr<IFAsyncTask> m_spTask;

	friend class IFAsyncTaskMgr;
};

class IFCOMMON_API IFAsyncTask : public IFRefObj
{
	IF_DECLARERTTI;



	IFEventSlot<void(IFAsyncTask* pTask)> event_TaskDone;
public:
	IFAsyncTask();

	virtual bool cancel();

	bool isCancelled();

	IFAsyncTaskState getState();
protected:
	~IFAsyncTask();

	virtual void execute();

	bool m_bCancel;

private:
	IFAsyncTaskState m_eState;
	friend class IFAsyncTaskMgr;
};

class IFCOMMON_API IFAsyncTaskMgr :public IFSingleton<IFAsyncTaskMgr>,  public IFMemObj
{
public:
	IFAsyncTaskMgr(int nMaxThreadCount);
	~IFAsyncTaskMgr(void);

	int getMaxWorkThreadCount();

	bool addTask(IFRefPtr<IFAsyncTask> spTask);

	void process();
private:
	typedef IFList<IFRefPtr<IFAsyncTask>> TaskList; 
	TaskList m_WaitExecuteList;
	IFCSLock m_WaitExecuteListLock;

	TaskList m_ExecutedList;
	IFCSLock m_ExecutedListLock;

	int m_nMaxThreadCount;
	void workThread(IFAsyncTaskThread* pThread,bool* bExit);
	bool m_bExitWorkThread;

	typedef IFMap<IFRefPtr<IFAsyncTaskThread>,bool> ThreadList;
	ThreadList m_WorkThreadList;
};

