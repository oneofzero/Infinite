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
#ifndef __IF_SERVER_FRAME_H__
#define __IF_SERVER_FRAME_H__
#include "IFCommonLib_API.h"
#include "ifsingleton.h"
#include "IFObj.h"
#include "IFRefPtr.h"

class IFNetCore;
class IFNetConnection;
class GSMsgFactory;
class IFTimer;

class IFCOMMON_API IFServerFrame :public IFMemObj
{
public:
	IFServerFrame(void);
	~IFServerFrame(void);

	int mainLoop();

	void requestExit();

	IFNetCore* getNetCore();

	IFTimer* getTimer();
protected:


	virtual void procNewConnection(IFNetCore* pCore, IFNetConnection* pConnection);

	virtual void procDataReceive(IFNetConnection* pConnection,const void* pBuf, IFUI32 nSize);
	virtual void procDisconnect(IFNetConnection* pConnection);

	void checkTimeOut(int deltaTime);

	virtual void onInit();
	virtual void onShutdown();
	virtual void onUpdate(float fSecond);
protected:

	bool m_bExit;

	bool m_bThreadSafe;
	bool m_bPackageMode;
	int m_nListenPort;
	int m_nMaxConnectionNum;

	IFUI32 m_DisconnectTimeOut;

	IFRefPtr<IFNetCore> m_spNetCore;
	IFRefPtr<GSMsgFactory> m_spMsgFactory;
	IFRefPtr<IFTimer> m_spTimer;

};

#endif //__IF_SERVER_FRAME_H__