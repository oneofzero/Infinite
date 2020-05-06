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
#include "IFCommonLib.h"
#include "IFServerFrame.h"
#include "IFLogSystem.h"
#include "IFNetCore.h"
#include "IFNetConnection.h"
#include "IFNetMsgFactory.h"
#include "IFSystemAPI.h"


IFServerFrame::IFServerFrame(void)
	:m_bExit(false)
	,m_nListenPort(30186)
	,m_nMaxConnectionNum(3000)
	,m_DisconnectTimeOut(300*1000)
	,m_bPackageMode(true)
	,m_bThreadSafe(true)
{
	//IFCommonLibInit();

	m_spTimer = IFNew IFTimer;

	m_spTimer->addFunctor(makeIFFunctor(this,&IFServerFrame::checkTimeOut), 20*1000,0);
}


IFServerFrame::~IFServerFrame(void)
{
	//IFCommonLibShutdown();
}

int IFServerFrame::mainLoop()
{
	m_spNetCore = IFNetCore::createNetCore();
	if (!m_spNetCore->startService(m_nListenPort,m_nMaxConnectionNum,m_bPackageMode, m_bThreadSafe ))
	{
		IFLOG(IFLL_FATAL, "can't start service!");
		return 0;
	}

	m_spNetCore->event_NewConnection.AddSelfHoldHandle(makeIFFunctor(this, &IFServerFrame::procNewConnection));

	onInit();
	IFLOG(IFLL_INFO, "Server Start! port=%d\r\n", m_nListenPort);
	IFUI64 dwTime = IFNativeSystemAPI::getMicrosSec();
	IFUI64 dwTotalTime = 0;
	while (!m_bExit)
	{
		m_spNetCore->process();

		IFUI64 dwCurTime = IFNativeSystemAPI::getMicrosSec();
		while ((dwCurTime-dwTime) < 1000)
		{
			m_spNetCore->process();
			IFThread::sleep(1);
			dwCurTime = IFNativeSystemAPI::getMicrosSec();
		}
		//m_spTimer->update(int((dwCurTime-dwTime)/1000));
		m_spTimer->update();

		onUpdate(float(dwCurTime-dwTime)/1000000.0f);
		//dwTotalTime += dwCurTime-dwTime;
		//IFLOG(IFLL_ERROR,"updatetime=%d  %d\r\n", dwCurTime-dwTime, dwTotalTime);
		dwTime = dwCurTime;
	}

	onShutdown();

	m_spNetCore->stopService();
	IFLOG(IFLL_INFO, "Server Stop!\r\n");

	return 0;
}


void IFServerFrame::procNewConnection( IFNetCore* pCore, IFNetConnection* pConnection )
{
	//IFLOG(IFLL_INFO, "new connection :%s(%d)\r\n", pConnection->getRemoteIP().c_str(), pConnection->getRemotePort());

	//pConnection->event_RecvData.AddSelfHoldHandle(makeIFFunctor(this, &IFServerFrame::procDataReceive));
	pConnection->event_Disconnect.AddSelfHoldHandle(makeIFFunctor(this,&IFServerFrame::procDisconnect));
	//IFRefPtr<IFNetConnection> spConnection = pConnection;
	//auto timeroutproc = [spConnection]()
	//{
	//	if(spConnection->getUserData())
	//		return;
	//	spConnection->disconnect();
	//};
	//必须在五秒后关联上否则 就断开链接 
	//m_spTimer->addFunctor(makeIFFunctor<void()>(timeroutproc), 5000);
}

void IFServerFrame::procDataReceive( IFNetConnection* pConnection,const void* pBuf, IFUI32 nSize )
{/*
	IFRefPtr<IFMemStream> spStream = IFNew IFMemStream(pBuf, nSize);
	int nMsgID = spStream->readI32();
	IFNet_Message* spMessage = IFNetMsgFactory::getSingleton().createMsg(nMsgID);
	if(spMessage)
	{
		try
		{
			spMessage->deserialize(spStream);

		}
		catch(IFStreamReadException& )
		{
			char buf[32]={0};

			pConnection->sendPack(IFNew IFMemStream((const void*)buf,32));
			IFLOG(IFLL_WARNING, "无法解析的错误消息\r\n");
		}
		spMessage->process(pConnection);

		delete spMessage;
	}
	else
	{

		IFLOG(IFLL_WARNING, "无法解析的错误消息\r\n");
	}
	*/
}

void IFServerFrame::requestExit()
{
	m_bExit = true;
}

void IFServerFrame::procDisconnect( IFNetConnection* pConnection )
{
	//IFLOG(IFLL_INFO, "disconnect connection :%s(%d)\r\n", pConnection->getRemoteIP().c_str(), pConnection->getRemotePort());

}

void IFServerFrame::onInit()
{

}

void IFServerFrame::onShutdown()
{

}

IFNetCore* IFServerFrame::getNetCore()
{
	return m_spNetCore;
}

IFTimer* IFServerFrame::getTimer()
{
	return m_spTimer;
}

void IFServerFrame::checkTimeOut()
{
	IFUI32 curTime = IFNativeSystemAPI::getTickCount();
	auto ckTime = [=](IFNetConnection* pConnection)->bool
	{
		if (curTime - pConnection->getLastCommunicateTime() >= m_DisconnectTimeOut)
			pConnection->disconnect();
		return false;
	};
	m_spNetCore->enumConnections(makeIFFunctor<bool(IFNetConnection*)>(ckTime));
}

void IFServerFrame::onUpdate( float fSecond )
{
	
}
