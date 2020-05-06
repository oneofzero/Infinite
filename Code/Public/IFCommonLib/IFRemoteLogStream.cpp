#include "stdafx.h"
#include "IFRemoteLogStream.h"
#include "IFDateTime.h"

void IFRemoteConsoleStream::init(IFNetCore* pNetCore, const IFString& sAddr, int nPort)
{
	m_spNetCore = pNetCore;
	m_spOutCon = m_spNetCore->createConnection(sAddr, nPort, false, false, true);
	m_handleConnect = makeIFFunctor(this, &IFRemoteConsoleStream::procConnect);
	m_handleDisConnect = makeIFFunctor(this, &IFRemoteConsoleStream::procDisconnect);

	m_handleConnect.connectSlot(m_spOutCon->event_ConnectResult);
	
}

IFUI32 IFRemoteConsoleStream::write(const void* pSourceData, IFUI32 nSize)
{
	if (m_spOutCon && m_spOutCon->isConnected() )
	{
		sendCached();
		rawsend(pSourceData, nSize);
	}
	else
	{
		m_buff.push(IFNew IFSimpleArray<char>(nSize, (const char*)pSourceData));
	}
	return nSize;
}

void IFRemoteConsoleStream::procConnect(IFNetConnection* pCon, bool ok)
{
	m_handleConnect.disconnectAllSlot();


	if (! ok)
	{
		m_spOutCon = m_spNetCore->createConnection(pCon->getRemoteIP(), pCon->getRemotePort(), false, false, true);
		m_handleConnect.connectSlot(m_spOutCon->event_ConnectResult);

	}
	else
	{
		m_spOutCon->setMaxSendBufferSize(1024 * 1024 * 10);
		m_handleDisConnect.connectSlot(m_spOutCon->event_Disconnect);
		sendCached();
	}
}

void IFRemoteConsoleStream::procDisconnect(IFNetConnection* pCon)
{
	m_spOutCon = m_spNetCore->createConnection(pCon->getRemoteIP(), pCon->getRemotePort(), false, false, true);
	m_handleConnect.connectSlot(m_spOutCon->event_ConnectResult);
}

void IFRemoteConsoleStream::sendCached()
{
	if (m_buff.m_nSize)
	{
		IFSimpleArray<char>* pData;
		while (m_buff.pop(pData))
		{

			rawsend(*pData, pData->size());
			//m_spOutCon->sendData(*pData, pData->size());

			delete pData;
		}
	}
}

void IFRemoteConsoleStream::rawsend(const void* p, int len)
{
	auto dt = IFDateTime::now().toDetail();
	IFString s;
	s.format("[%02d:%02d:%02d %03d]", dt.nHour, dt.nMinute, dt.nSecond, dt.nMilliSecond);
	m_spOutCon->sendData(s.c_str(), s.length());
	m_spOutCon->sendData(p, len);

}

