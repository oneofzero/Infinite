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
#include "IFRBTree.h"
#include "IFArray.h"
#include "IFRefPtr.h"
#include "IFCommonLib_API.h"
#include "IFEventSlot.h"

class IFStateMgr;

class IFCOMMON_API IFState : public IFRefObj
{
public:
	IFEventSlot<void(IFState* pThis,IFState* pPrveState)> event_Enter;
	IFEventSlot<void(IFState* pThis)> event_Leave;
public:
	IFState();

protected:
	~IFState();

	virtual void onEnter(IFState* pPrveState){};
	virtual void onLeave(){};

	void Enter(IFState* pPrveState, bool bPush);
	void Leave();


	IFArray<IFRefPtr<IFState>> m_PrevStateStack;
	IFStateMgr* m_pStateMgr;
	friend class IFStateMgr;
};

class IFCOMMON_API IFStateMgr: public IFRefObj
{
public:
	IFStateMgr();

	void addState(IFState* pState);

	void changeState(IFState* pState, bool bNeedPush = false);

	IFState* getCurState();

	bool resumePrevState();

protected:
	~IFStateMgr();

	typedef IFRBTree<IFRefPtr<IFState>> StateList;
	StateList m_StateList;
	IFRefPtr<IFState> m_spCurState;
};