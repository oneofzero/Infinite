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
#include "IFNetCoreIOCP.h"
#include "IFLogSystem.h"
#include <WS2tcpip.h>
#include <WSPiApi.h>
#include <ws2ipdef.h>

IFNetCoreIOCP::IFNetCoreIOCP( void )
	:
	m_nExitThreadCount(0)
	,m_bProcessThreadExit(false)
	,m_WSAConnectEx(NULL)
	,m_WSAAcceptEx(NULL)
{
	m_IODataProcessFunList[IFNetCoreIOCP_IODATA::DT_ACCEPT] = &IFNetCoreIOCP::processAccept;
	m_IODataProcessFunList[IFNetCoreIOCP_IODATA::DT_CONNECT] = &IFNetCoreIOCP::processConnect;
	m_IODataProcessFunList[IFNetCoreIOCP_IODATA::DT_RECV] = &IFNetCoreIOCP::processRecvData;
	m_IODataProcessFunList[IFNetCoreIOCP_IODATA::DT_SEND] = &IFNetCoreIOCP::processSendData;

	

}


IFNetCoreIOCP::~IFNetCoreIOCP( void )
{

}

bool IFNetCoreIOCP::startListen(int nPort, bool enableSSL, bool packagemode)
{
	if (!m_WSAAcceptEx)
		return false;

	SOCKADDR_IN address;
	SOCKET listener = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	CreateIoCompletionPort((HANDLE)listener, m_hCompletionPort, (u_long)0, 0);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(nPort);
	int hr = bind(listener, (sockaddr*)&address, sizeof address);
	if (hr != 0)
	{
		IFLOG(IFLL_DEBUG, "IPV4 bind error!%d\r\n", hr);
		return false;
	}
	hr = listen(listener, SOMAXCONN);
	if (hr != 0)
	{
		IFLOG(IFLL_DEBUG, "IPV4 listen error!%d\r\n", hr);
		return false;
	}

	SOCKADDR_IN6 address6;
	ZeroMemory(&address6, sizeof(address6));
	SOCKET listener6 = WSASocket(AF_INET6, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	CreateIoCompletionPort((HANDLE)listener6, m_hCompletionPort, (u_long)0, 0);

	address6.sin6_family = AF_INET6;
	address6.sin6_addr = in6addr_any;
	address6.sin6_port = htons(nPort);
	bool haveipv6 = true;
	hr = bind(listener6, (sockaddr*)&address6, sizeof address6);
	if (hr != 0)
	{
		haveipv6 = false;
		IFLOG(IFLL_DEBUG, "IPV6 bind error!%d\r\n", WSAGetLastError());
	}
	hr = listen(listener6, SOMAXCONN);
	if (hr != 0)
	{
		haveipv6 = false;
		IFLOG(IFLL_DEBUG, "IPV6 listen error!%d\r\n", WSAGetLastError());
	}


	IFRefPtr<IFNetConnectionIOCP> spNewConnection = IFNew IFNetConnectionIOCP(this, packagemode, m_bSyncEvent);
	spNewConnection->m_nLocalPort = nPort;
	spNewConnection->requestAccept(false,listener);
	if (enableSSL)
		spNewConnection->waitEstablishSSL();
	if (haveipv6)
	{
		IFRefPtr<IFNetConnectionIOCP> spNewConnection6 = IFNew IFNetConnectionIOCP(this, packagemode, m_bSyncEvent);
		spNewConnection6->requestAccept(true, listener6);
		spNewConnection6->m_nLocalPort = nPort;
		if (enableSSL)
			spNewConnection6->waitEstablishSSL();
	}
	m_ListenerList.insert(makeIFPair(nPort, makeIFPair(listener, listener6)));
	return true;
}

bool IFNetCoreIOCP::stopListen(int nPort)
{
	auto it = m_ListenerList.find(nPort);
	if (it != m_ListenerList.end())
	{
		closesocket(it->second.first);
		closesocket(it->second.second);

		m_ListenerList.erase(it);
		return true;
	}
	return false;
}

bool IFNetCoreIOCP::onServiceStart(  )
{
	m_bProcessThreadExit = false;

	int hr = 0;
	WSADATA wsaData;
	hr = WSAStartup( 0x0202, &wsaData );
	if( hr != 0 )
		return false;

	m_hCompletionPort= CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, NULL );

	if (!loadExtensions())
		return false;


	//server need more thread
	if (m_nPort)
	{
		if (!startListen(m_nPort,false,m_bPackageMode))
			return false;
		SYSTEM_INFO systeminfo;
		GetSystemInfo(&systeminfo);

		m_nProcessThreadCount = systeminfo.dwNumberOfProcessors;
	}
	else
		m_nProcessThreadCount = 1;

	for(int i = 0; i < m_nProcessThreadCount; i ++ )
	{
		IFRefPtr<IFThread> spProcessThread = IFNew IFThread;
		m_ProcessThreads.push_back(spProcessThread);
		spProcessThread->start(makeIFFunctor(this, &IFNetCoreIOCP::processThread));

	}


	m_bStarted = true;
	IFLOG(IFLL_DEBUG, "ifnet core%p start!\r\n", this);
	return true;

}



bool IFNetCoreIOCP::onServiceStop()
{
	for (auto& soks:m_ListenerList)
	{
		closesocket(soks.second.first);
		closesocket(soks.second.second);

	}
	m_ListenerList.clear();
	//if (m_Listener!=INVALID_SOCKET)
	//{
	//	closesocket(m_Listener);
	//}

	//while (m_Connections.size())
	//{
	//	(*m_Connections.begin())->disconnect();
	//}
	for (ConnectionList::iterator it = m_Connections.begin(); it != m_Connections.end(); ++it)
	{
		(*it)->disconnect();
	}

	m_Connections.clear();
	for (ConnectionList::iterator it = m_AcceptConnections.begin(); it != m_AcceptConnections.end(); ++it)
	{
		(*it)->disconnect();
	}
	m_AcceptConnections.clear();
	m_AutoKeepAliveList.clear();
	m_bProcessThreadExit = true;
	for (int i = 0; i < m_ProcessThreads.size(); i ++ )
	{
		m_ProcessThreads[i]->waitExit();
	}
	m_ProcessThreads.clear();
	return true;
}

//bool IFNetCoreIOCP::acceptNewConnection()
//{
//	IFRefPtr<IFNetConnectionIOCP> spAccept = IFNew IFNetConnectionIOCP(this, m_bPackageMode, m_bSyncEvent);
//	
//	return true;
//}
/*
IFNetCoreIOCP_IODATA_Accept* IFNetCoreIOCP::createAcceptData(IFNetConnectionIOCP* pConnection)
{
	IFNetCoreIOCP_IODATA_Accept* pData = IFNew IFNetCoreIOCP_IODATA_Accept();
	ZeroMemory(&pData->m_OverLapped, sizeof(pData->m_OverLapped));
	pData->m_Type = IFNetCoreIOCP_IODATA::DT_ACCEPT;
	pData->m_spConnection = pConnection;
	return pData;
}
*
IFNetCoreIOCP_IODATA_Connect* IFNetCoreIOCP::createConnectData( IFNetConnectionIOCP* pConnection )
{
	IFNetCoreIOCP_IODATA_Connect* pData = IFNew IFNetCoreIOCP_IODATA_Connect();
	ZeroMemory(&pData->m_OverLapped, sizeof(pData->m_OverLapped));

	pData->m_Type = IFNetCoreIOCP_IODATA::DT_CONNECT;
	pData->m_spConnection = pConnection;
	return pData;

}


IFNetCoreIOCP_IODATA_Send* IFNetCoreIOCP::createSendData( IFNetConnectionIOCP* pConnection, IFMemStream* pPack )
{
	IFNetCoreIOCP_IODATA_Send* pData = IFNew IFNetCoreIOCP_IODATA_Send(pPack);
	pData->m_spConnection = pConnection;
	return pData;
}


void IFNetCoreIOCP::freeIOData( IFNetCoreIOCP_IODATA* pData )
{
	pData->m_spConnection = NULL;
	pData->FreeMe(pData);
}
*/

void IFNetCoreIOCP::processThread()
{
	BOOL bIOFailed = FALSE;
	DWORD dwTransBytes = 0;
	DWORD dwWaitTime = 500;
	SOCKET socket;
	DWORD dwLastError = 0;
	IFNetCoreIOCP_IODATA* pIOData = NULL;

	while (true)
	{
		if (!GetQueuedCompletionStatus( m_hCompletionPort,
			&dwTransBytes,(PULONG_PTR) &socket, 
			(LPOVERLAPPED*)&pIOData, dwWaitTime ))
		{
			dwLastError = GetLastError();
			if(dwLastError == WAIT_TIMEOUT)
			{

					if (m_bProcessThreadExit)
						break;
					else
						continue;
			}
			else if (pIOData)	//error 
			{
			
				(this->*m_IODataProcessFunList[pIOData->m_Type])(pIOData,dwTransBytes, false);

			}
		}
		else if (pIOData)
		{
			(this->*m_IODataProcessFunList[pIOData->m_Type])(pIOData,dwTransBytes, true);
			//freeIOData(pIOData);
		}
		else
		{
			IFLOG(IFLL_ERROR,"!!!!\r\n");
		}
	}

	//no io data need exit
	IFLOG(IFLL_DEBUG, "if net core process exit!\r\n");


}




IFNetConnectionPtr IFNetCoreIOCP::createConnection( const IFString& sAddress, int nPort, bool syncconnect, /*= true*/ bool bPackagemode, bool bSyncEvent )
{
	IFRefPtr<IFNetConnectionIOCP> spConnection = IFNew IFNetConnectionIOCP(this, bPackagemode, bSyncEvent);

	//SOCKET sk =  WSASocket( AF_INET, SOCK_STREAM, IPPROTO_TCP, 0 , 0, WSA_FLAG_OVERLAPPED);

	if (syncconnect)
	{
		if (spConnection->connect(sAddress,nPort))
			return spConnection;
		else
			return NULL;
	}
	else
	{
		if (spConnection->requestConnect(sAddress, nPort))
			return spConnection;
		return NULL;
	}

}




void IFNetCoreIOCP::processAccept(IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess)
{	
	IFNetCoreIOCP_IODATA_Accept* pAcceptData = (IFNetCoreIOCP_IODATA_Accept*)pData;
	IFRefPtr<IFNetConnectionIOCP> spNewConnection = IFNew IFNetConnectionIOCP(this, pAcceptData->m_spConnection->isPackageMode(), m_bSyncEvent);

	spNewConnection->requestAccept(pData->m_spConnection->m_bIPV6,pAcceptData->m_ListenSocket);
	spNewConnection->m_nLocalPort = pData->m_spConnection->m_nLocalPort;
	if (pData->m_spConnection->m_ssl)
		spNewConnection->waitEstablishSSL();
	if (!bSuccess)
	{
		pAcceptData->decRef();
		//delete pAcceptData;
		return;
	}
	//if (m_AcceptConnections.size() < 10)
	//{
	//	for (int i = 0; i < 100; i ++ )
	//	{
	//		IFRefPtr<IFNetConnectionIOCP> spNewConnection = IFNew IFNetConnectionIOCP(this);
	//		spNewConnection->requestAccept();
	//	}

	//}

	

	int err = setsockopt( pData->m_spConnection->getSocket(), 
		SOL_SOCKET, 
		SO_UPDATE_ACCEPT_CONTEXT, 
		(char *)&pAcceptData->m_ListenSocket, 
		sizeof(pAcceptData->m_ListenSocket) );

	
	sockaddr_in* localaddress;
	sockaddr_in* remoteaddress;
	int localaddsize,remoteaddsize;
	m_WSAGetAcceptExSockAddrs(
		pAcceptData->m_buf,
		0,
		sizeof(SOCKADDR_IN)+16,
		sizeof(SOCKADDR_IN)+16,
		(sockaddr**)&localaddress,
		&localaddsize,
		(sockaddr**)&remoteaddress,
		&remoteaddsize
		);

	//int socksize = sizeof(sockaddr_in) + 16+2;
	//sockaddr_in* pAddress = (sockaddr_in*)&pAcceptData->m_buf[];
	IFNetCore::AddressInfo adinfo;
	memcpy(adinfo.addr, remoteaddress, remoteaddsize);
	adinfo.addrlen = remoteaddsize;
	char ip[128] = { 0 };
	if (pData->m_spConnection->m_bIPV6)
	{
		sockaddr_in6* remoteaddress6 = (sockaddr_in6*)remoteaddress;
		adinfo.family = AF_INET6 ;
		inet_ntop(AF_INET6, &(remoteaddress6->sin6_addr), ip, sizeof(ip));

		adinfo.ip = ip;
		adinfo.port = htons(remoteaddress6->sin6_port);
	}
	else
	{
		adinfo.family =  AF_INET;
		inet_ntop(AF_INET, &(remoteaddress->sin_addr), ip, sizeof(ip));
		adinfo.ip = ip;
		adinfo.port = ntohs(remoteaddress->sin_port);
	}

	addConnectedConnection(pAcceptData->m_spConnection);
	pAcceptData->m_spConnection->onConnect(&adinfo);
	fireNewConnectionEvent(pAcceptData->m_spConnection);
	if (dwTransBytes)
	{
		pAcceptData->m_spConnection->recvData(pAcceptData->m_buf, dwTransBytes);
	}

	removeAcceptConnection(pAcceptData->m_spConnection);

	pAcceptData->decRef();
	//delete pAcceptData;

}

void IFNetCoreIOCP::processConnect( IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess )
{
	IFNetCoreIOCP_IODATA_Connect* pConnectData = (IFNetCoreIOCP_IODATA_Connect*)pData;

	IFNetConnectionIOCP* pConnection = pConnectData->m_spConnection;


	if (bSuccess)
		pConnection->onConnect( &pConnectData->m_addr);
	else
	{
		IFLOG(IFLL_DEBUG,"connect failed!\r\n");
	}
	if (pConnection->isSyncEvent())
	{
		IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, bSuccess);
		pushEvent(pEvent);
	}
	else
	{
		pConnection->event_ConnectResult(pConnection,  bSuccess);

	}
	pConnectData->decRef();
	//delete pConnectData;
}
void IFNetCoreIOCP::processRecvData( IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess )
{
	IFNetCoreIOCP_IODATA_Recv* pRD = (IFNetCoreIOCP_IODATA_Recv*)pData;
	if (IFRefPtr<IFNetConnectionIOCP> spConnection = pData->m_spConnection)
	{
		if (dwTransBytes == 0 || !bSuccess)
		{

			spConnection->disconnect();
			//spConnection->m_spRecvData->m_spConnection = NULL;
		}
		else
		{
			spConnection->setLastCommunicateTime();
			spConnection->recvData(pRD->m_buf, dwTransBytes);
			spConnection->requestRecvData();
			ATOMIC_INC_INT64((volatile LONG64*)&m_nRecvPackage);
			ATOMIC_ADD_INT64((volatile LONG64*)&m_nRecvBytes, (LONG64)dwTransBytes);
		}

	}
	pRD->decRef();

}

void IFNetCoreIOCP::processSendData( IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess )
{
	IFNetCoreIOCP_IODATA_Send* pSendData = (IFNetCoreIOCP_IODATA_Send*)pData;
	if (dwTransBytes == 0||!bSuccess)
	{
		pData->m_spConnection->fireSendDoneEvent(pSendData->m_nSendID, false);
		pData->m_spConnection->disconnect();
	}
	else
	{
		pData->m_spConnection->setLastCommunicateTime();
		
		ATOMIC_ADD_INT32((volatile long*)&pData->m_spConnection->m_nSendPendingBytes, -(long)dwTransBytes);
		ATOMIC_ADD_INT64((volatile LONG64*)&m_nSendBytes, (LONG64)dwTransBytes);
		ATOMIC_INC_INT64((volatile LONG64*)&m_nSendPackage);
		pData->m_spConnection->fireSendDoneEvent(pSendData->m_nSendID, true);
	}
	


	pSendData->decRef();
	//delete pSendData;
}

//bool IFNetCoreIOCP::convertSockAddress( sockaddr_in* pOut, const IFString& sAddress, int nPort )
//{
//	ZeroMemory(pOut,sizeof(pOut));
//	sockaddr_in& address = *pOut; 
//	address.sin_family = AF_INET;
//	address.sin_addr.s_addr = inet_addr( sAddress.c_str() );
//	if(address.sin_addr.s_addr == 0xffffffff)
//	{
//		hostent* pHostent = gethostbyname(sAddress.c_str());
//		if(pHostent)
//		{
//			address.sin_addr = *((in_addr*)pHostent->h_addr_list[0]);
//		}
//		else
//			return false;
//	}
//
//	address.sin_port = htons( nPort );
//	return true;
//}

void IFNetCoreIOCP::addAcceptConnection( IFNetConnectionIOCP* pConnection )
{
	IFCSLockHelper lh(m_AcceptConnectionsLock);
	m_AcceptConnections.insert(pConnection);
}

void IFNetCoreIOCP::addConnectedConnection( IFNetConnectionIOCP* pConnection )
{
	IFCSLockHelper lh(m_ConnectionListLock);
	m_Connections.insert(pConnection);
}
void IFNetCoreIOCP::removeConnectedConnection( IFNetConnectionIOCP* pConnection )
{
	IFCSLockHelper lh(m_ConnectionListLock);
	ConnectionList::iterator it = m_Connections.find(pConnection);
	if (it != m_Connections.end())
	{
		m_Connections.erase(it);
		removeAutoKeepAliveConnection(pConnection);
	}
}

void IFNetCoreIOCP::removeAcceptConnection( IFNetConnectionIOCP* pConnection )
{
	IFCSLockHelper lh(m_AcceptConnectionsLock);
	ConnectionList::iterator it = m_AcceptConnections.find(pConnection);
	if (it != m_AcceptConnections.end())
		m_AcceptConnections.erase(it);
}


bool IFNetCoreIOCP::loadExtensions()
{
	IFRefPtr<IFNetConnectionIOCP> spCon = IFNew IFNetConnectionIOCP(this, false, m_bSyncEvent);
	spCon->m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidConnect = WSAID_CONNECTEX;
	GUID GuidGetSockaddr = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes;



	WSAIoctl(spCon->getSocket(), 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidAcceptEx, 
		sizeof(GuidAcceptEx),
		&m_WSAAcceptEx, 
		sizeof(m_WSAAcceptEx), 
		&dwBytes, 
		NULL, 
		NULL);

	int err = WSAGetLastError();

	WSAIoctl(spCon->getSocket(), 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidConnect, 
		sizeof(GuidConnect),
		&m_WSAConnectEx, 
		sizeof(m_WSAConnectEx), 
		&dwBytes, 
		NULL, 
		NULL);

	WSAIoctl(spCon->getSocket(), 
		SIO_GET_EXTENSION_FUNCTION_POINTER, 
		&GuidGetSockaddr, 
		sizeof(GuidGetSockaddr),
		&m_WSAGetAcceptExSockAddrs, 
		sizeof(m_WSAGetAcceptExSockAddrs), 
		&dwBytes, 
		NULL, 
		NULL);

	return m_WSAAcceptEx&&m_WSAConnectEx&&m_WSAGetAcceptExSockAddrs;
}





IFNetConnectionIOCP::IFNetConnectionIOCP(IFNetCoreIOCP* pCore,bool bPackagemode, bool bSyncEvent)
	:IFNetConnection(pCore,bSyncEvent,bPackagemode)
	,m_pIOCPCore(pCore), m_bIPV6(false)
{
	m_pRecvData = IFNew IFNetCoreIOCP_IODATA_Recv(NULL);
	m_pRecvData->addRef();
	m_Socket = INVALID_SOCKET;
}


IFUI64 IFNetConnectionIOCP::sendPack( IFRefPtr<IFMemStream> pStream, IFUI64 nSendID)
{
	IFUI64 nCurSendID = nSendID==0?getSendID():nSendID;
	if (m_eConnectionState != IFNCS_CONNECTED)
	{
		IFLOG(IFLL_ERROR, "send error: not connected\r\n");
		//fireSendDoneEvent(m_nSendID, false);
		return 0;
	}
	if (m_nSendPendingBytes>m_nMaxSendBufferSize)
	{
		IFLOG(IFLL_ERROR, "send error: buf overflow (%d)\r\n", m_nMaxSendBufferSize)
		//fireSendDoneEvent(m_nSendID, false);
		return 0;
	}
	IFNetCoreIOCP_IODATA_Send* pSendData = IFNew IFNetCoreIOCP_IODATA_Send(this, pStream);
	pSendData->m_nSendID = nCurSendID;
	//IFNetCoreIOCP_IODATA_Send* pSendData2 = pSendData;
	DWORD dwSendSize = 0;
	pSendData->addRef();
	pSendData->addRef();
	if(WSASend(m_Socket, &pSendData->m_WSABuf, 1, &dwSendSize, 0, (LPOVERLAPPED)&pSendData->m_OverLapped, NULL )== SOCKET_ERROR)
	{
		DWORD dwLastError = WSAGetLastError();
		if( dwLastError != ERROR_IO_PENDING )
		{
			IFLOG(IFLL_ERROR,"send error!\r\n");
			//m_pIOCPCore->freeIOData(pSendData);
			pSendData->decRef();
			pSendData->decRef();

			disconnect();
			return 0;
		};
	}
	pSendData->decRef();
	ATOMIC_ADD_INT32((volatile long*)&m_nSendPendingBytes, (long)pStream->size());

	

	return nCurSendID;
}



IFNetConnectionIOCP::~IFNetConnectionIOCP()
{
	disconnect();
}

bool IFNetConnectionIOCP::isShouldRemove()
{
	return false;
}

void IFNetConnectionIOCP::disconnect()
{
	IFREFHOLDTHIS();
	{
		IFCSLockHelper lh(&m_Lock);
		if (m_Socket!=INVALID_SOCKET)
		{

			m_pRecvData->m_spConnection = NULL;

			m_pRecvData->decRef();
			m_pRecvData = NULL;

			closesocket(m_Socket);

			m_Socket = INVALID_SOCKET;
			if (m_eConnectionState == IFNCS_CONNECTED)
			{
				m_eConnectionState = IFNCS_DISCONNECTED;
				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this,IFNetCoreEvent::ET_DISCONNECT);
				m_pIOCPCore->pushEvent(pEvent);
			}
			//if (m_eConnectionState != IFNCS_UNKNOWN)
			//	m_pIOCPCore->removeConnectedConnection(this);

		}

	}



}

void IFNetConnectionIOCP::setNoDelay()
{
	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	int br = setsockopt(m_Socket,IPPROTO_TCP, TCP_NODELAY , (char*)&bOptVal, bOptLen);
}

bool IFNetConnectionIOCP::connect( const IFString& sAddress, int nPort )
{

	IFNetCore::AddressInfo address; 
	if (!m_pIOCPCore->getHostAddress( sAddress, nPort, &address))
		return false;
	DWORD dwLastError;
	m_Socket = WSASocket(address.family, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	m_bIPV6 = address.family == AF_INET6;
	if(WSAConnect(m_Socket, (SOCKADDR*)address.addr, address.addrlen, NULL, NULL, NULL, NULL) == SOCKET_ERROR )
	{
		dwLastError = GetLastError();
		return false;
	}
	CreateIoCompletionPort( (HANDLE)m_Socket, m_pIOCPCore->getCompletionPort(), (ULONG_PTR)0, 0 );

	return onConnect(&address);
}

bool IFNetConnectionIOCP::onConnect(const IFNetCore::AddressInfo*  addressinfo )
{
	setNoDelay();
	setLastCommunicateTime();

	m_nRemotePort = addressinfo->port;// ntohs(pRemoteAddress->sin_port);
	m_sRemoteIP = addressinfo->ip;//inet_ntoa(pRemoteAddress->sin_addr);
	m_eConnectionState = IFNCS_CONNECTED;

	m_pIOCPCore->addConnectedConnection(this);

	if(!requestRecvData())
	{
		return false;
	}

	return true;
}

bool IFNetConnectionIOCP::requestAccept(bool ipv6support, SOCKET listensocket)
{
	IFNetCoreIOCP_IODATA_Accept* pAD = IFNew IFNetCoreIOCP_IODATA_Accept(this);//m_pIOCPCore->createAcceptData(this);
	DWORD dwRecvBytes = 0;
	if (ipv6support)
		m_Socket = WSASocket(AF_INET6, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	else
		m_Socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort((HANDLE)m_Socket, m_pIOCPCore->getCompletionPort(), (u_long)0, 0);

	pAD->addRef();
	pAD->addRef();
	m_bIPV6 = ipv6support;
	//SOCKET listensocket = ipv6support ? m_pIOCPCore->m_Listener6 : m_pIOCPCore->m_Listener;
	int nBuffSize = ipv6support ? sizeof(sockaddr_in6) + 16 : sizeof(sockaddr_in) + 16;
	pAD->m_ListenSocket = listensocket;
	if (!(m_pIOCPCore->getAcceptEx())(listensocket, m_Socket, pAD->m_buf, 0, nBuffSize,
		nBuffSize, &dwRecvBytes, (LPWSAOVERLAPPED)&pAD->m_OverLapped))
	{
		DWORD dwLastError = WSAGetLastError();
		if (dwLastError!= ERROR_IO_PENDING)
		{
			//m_pIOCPCore->freeIOData(pAD);
			pAD->decRef();
			pAD->decRef();
			IFLOG(IFLL_ERROR, "REQUEST ACCEPT ERROR!%d\r\n", dwLastError);
			return false;
		}
	}
	pAD->decRef();

	m_pIOCPCore->addAcceptConnection(this);
	return true;
}

bool IFNetConnectionIOCP::requestConnect( const IFString& sAddress, int nPort )
{
	IFRefPtr<IFNetConnectionIOCP> thisCon = this;
	m_pIOCPCore->getHostAddressAsync(sAddress, nPort, makeIFFunctor<void(const IFNetCore::AddressInfo*)>([=](const IFNetCore::AddressInfo* pADD)
	{
		auto connecterr=[=]()
		{
			if (thisCon->isSyncEvent())
			{
				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(thisCon, IFNetCoreEvent::ET_CONNECT, false);
				m_pIOCPCore->pushEvent(pEvent);
			}
			else
			{
				event_ConnectResult(thisCon,  false);

			}
		};
		if (!pADD)
		{
			connecterr();
			return;
		}
		else
		{
			//sockaddr_in address;// = *pOut; 
			//address.sin_family = AF_INET;
			//address.sin_addr.s_addr = inet_addr( sAddress.c_str() );
			//address.sin_port = htons( nPort );

			m_Socket = WSASocket(pADD->family, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (pADD->family == AF_INET6)
			{
				sockaddr_in6 bindaddr;
				ZeroMemory(&bindaddr, sizeof(bindaddr));
				bindaddr.sin6_family = pADD->family;
				bindaddr.sin6_addr = in6addr_any;
				bindaddr.sin6_port = 0;
				int hr = bind(m_Socket, (sockaddr*)&bindaddr, sizeof bindaddr);
				if (hr != 0)
				{
					IFLOG(IFLL_ERROR, "bind socket error %d", hr);
				}
			}
			else
			{
				sockaddr_in bindaddr;
				bindaddr.sin_family = pADD->family;
				bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
				bindaddr.sin_port = 0;
				int hr = bind(m_Socket, (sockaddr*)&bindaddr, sizeof bindaddr);
				if (hr != 0)
				{
					IFLOG(IFLL_ERROR, "bind socket error %d", hr);
				}
			}
			m_bIPV6 = pADD->family == AF_INET6;

			CreateIoCompletionPort((HANDLE)m_Socket, m_pIOCPCore->getCompletionPort(), (u_long)0, 0);

			IFNetCoreIOCP_IODATA_Connect* pCD = IFNew IFNetCoreIOCP_IODATA_Connect(this);//m_pIOCPCore->createConnectData(this);
			pCD->m_addr = *pADD;

			pCD->addRef();
			pCD->addRef();

			if (!(m_pIOCPCore->getConnectEx())( m_Socket, (sockaddr*)pADD->addr, pADD->addrlen, NULL, 0,  NULL, (LPWSAOVERLAPPED)&pCD->m_OverLapped))
			{
				DWORD dwLastError = WSAGetLastError();
				if (dwLastError!= ERROR_IO_PENDING)
				{
					pCD->decRef();
					pCD->decRef();

					//delete pCD;
					connecterr();

					return;
				}
			}
			pCD->decRef();

			int err = setsockopt( m_Socket, 
				SOL_SOCKET, 
				SO_UPDATE_CONNECT_CONTEXT, 
				NULL, 
				0 );
		}

	}));

	//if (!IFNetCoreIOCP::convertSockAddress(&address,sAddress,nPort))
	//	return false;

	//	pCD->addRef();


	return true;

}

bool IFNetConnectionIOCP::requestRecvData()
{
	DWORD dwRecvSize;
	DWORD dwFlag = 0;
	IFCSLockHelper lh(&m_Lock);

	if (m_pRecvData)
	{
		m_pRecvData->reset();
		m_pRecvData->m_spConnection = this;
		m_pRecvData->addRef();

	}
	else
		return false;
	//m_RecvData.reset();
	//m_RecvData.m_spConnection = this;
	HRESULT hr  = WSARecv( m_Socket, &(m_pRecvData->m_WSABuf), 1, &dwRecvSize, &dwFlag, (LPWSAOVERLAPPED)(&m_pRecvData->m_OverLapped),NULL  );
	if( hr == SOCKET_ERROR )
	{
		int nLastError = WSAGetLastError();
		if( nLastError != ERROR_IO_PENDING )
		{
			m_pRecvData->decRef();
			IFLOG(IFLL_ERROR, "recv error! disconnect\r\n");
			
			disconnect();

			return false;
		}
	}
	return true;
}