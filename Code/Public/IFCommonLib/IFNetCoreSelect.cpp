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
#include "IFNetCoreSelect.h"
#include "IFLogSystem.h"
#ifdef WIN32
#include <WS2tcpip.h>
#include <WSPiApi.h>
#include <ws2ipdef.h>

#endif
#ifdef LINUX
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <errno.h>

#elif defined(MAC)||defined(IFPLATFORM_WEB)
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#endif



IFNetCoreSelect::IFNetCoreSelect()
	:m_bExitWorkThread(false)
	,m_Listener(INVALID_SOCKET)
{
}

IFNetCoreSelect::~IFNetCoreSelect()
{

}

bool IFNetCoreSelect::onServiceStart()
{
#ifdef WIN32
	WSADATA wsaData;

	int hr = WSAStartup( 0x0202, &wsaData );
	if( hr != 0 )
		return false;
#endif

	if (m_nPort)
	{
		startListen(m_nPort, false, m_bPackageMode);
	}

	m_spWorkThread = IFNew IFThread;
	m_spWorkThread->start(makeIFFunctor(this, &IFNetCoreSelect::workThread));
	return true;
}

bool IFNetCoreSelect::onServiceStop()
{
	m_bExitWorkThread = true;
	m_spWorkThread->requestExit();
	m_spWorkThread->waitExit();
	for (ConnectionList::iterator it = m_ConnectingList.begin();
		it != m_ConnectingList.end(); ++it)
	{
		(*it)->disconnect();
	}
	m_ConnectingList.clear();
	return true;
}

IFNetConnectionPtr IFNetCoreSelect::createConnection( const IFString& sAddress, int nPort, bool syncconnect /*= true*/,bool bPackagemode, bool bSyncEvent )
{

    IFRefPtr<IFNetCoreSelectConnection> pConnection = IFNew IFNetCoreSelectConnection(this, bPackagemode, bSyncEvent);

    if (!syncconnect)
    {
        getHostAddressAsync(sAddress, nPort, makeIFFunctor<void(const IFNetCore::AddressInfo*)>([=](const IFNetCore::AddressInfo* pADDR)
        {
            if (!pADDR)
            {
                pConnection->m_eConnectionState = IFNCS_CONNECTED;
                IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, false );
                pushEvent(pEvent);
                return;
            }
          /*  sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_addr = *pADDR;
            address.sin_port = htons( nPort );*/
			pConnection->m_Socket = socket(pADDR->family, SOCK_STREAM, IPPROTO_TCP);

            pConnection->setNoDelay();
            pConnection->m_nRemotePort = nPort;
			pConnection->m_sRemoteIP = pADDR->ip;// inet_ntoa(address.sin_addr);
            
            
            if (!syncconnect)
            {
                pConnection->setNoBlock();
                IFCSLockHelper lh(m_ConnectingListLock);
                m_ConnectingList.insert(pConnection);
                //return pConnection;
            }
            
            int r = connect(pConnection->m_Socket, (sockaddr*)pADDR->addr, pADDR->addrlen);
			IFLOG(IFLL_DEBUG, "CONNECT RESULT %d IS PENDING\r\n", r);
            
            if(r<0)
            {
#ifdef WIN32
                r = GetLastError();
                if (r != 10035)
                {

					IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, false );
					pushEvent(pEvent);
					IFCSLockHelper lh(m_ConnectingListLock);
					ConnectionList::iterator it = m_ConnectingList.find(pConnection);
					if (it!=m_ConnectingList.end())
						m_ConnectingList.erase(it);

					return;

				}

#endif
                
            }
            //else 
            //{
            //    if (!syncconnect)
            //    {
            //        IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, true );
            //        pushEvent(pEvent);
            //    }
            //}
           
       }));
        
        return pConnection;
    }

	IFNetCore::AddressInfo addressinfo;
	if (!IFNetCore::getHostAddress(sAddress, nPort, &addressinfo))
		return NULL;
	pConnection->m_Socket = socket(addressinfo.family, SOCK_STREAM, IPPROTO_TCP);
 
	
	pConnection->setNoDelay();
	pConnection->m_nRemotePort = nPort;
	pConnection->m_sRemoteIP = addressinfo.ip;// inet_ntoa(address.sin_addr);


	if (!syncconnect)
	{
		pConnection->setNoBlock();
		IFCSLockHelper lh(m_ConnectingListLock);
		m_ConnectingList.insert(pConnection);
		//return pConnection;
	}


	int r = connect(pConnection->m_Socket, (sockaddr*)&addressinfo.addr, addressinfo.addrlen);
	IFLOG(IFLL_DEBUG, "CONNECT RESULT %d\r\n", r);

	if(r<0)
	{
#ifdef WIN32
		r = GetLastError();
		if (r == 10035)
		{
#endif
			if (!syncconnect)
			{
				return pConnection;
			}
			else
				return NULL;
	
#ifdef WIN32
		}
        else
		{
			IFCSLockHelper lh(m_ConnectingListLock);
			ConnectionList::iterator it = m_ConnectingList.find(pConnection);
			if (it!=m_ConnectingList.end())
				m_ConnectingList.erase(it);
			return NULL;
		}
#endif


	}
	else 
	{
		if (!syncconnect)
		{
			IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, true );
			pushEvent(pEvent);
		}
	}


	pConnection->m_eConnectionState = IFNCS_CONNECTED;
	pConnection->setNoDelay();
	IFCSLockHelper lh(m_ConnectionListLock);
	m_Connections.insert(pConnection);

	return pConnection;
}

bool IFNetCoreSelect::startListen(int nPort, bool enableSSL /*= false*/, bool packagemode /*= false*/)
{
	auto listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	int enable_reuseaddr = 1;
	if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable_reuseaddr, sizeof(enable_reuseaddr)) != 0)
	{
#if defined(IFPLATFORM_WEB)
		IFLOG(IFLL_ERROR, "set socket addr reuse error\r\n");
#else
		IFLOG(IFLL_ERROR, "set socket addr reuse error:%d\r\n", errno);
#endif
	}

	sockaddr_in addr;

	ZeroMemory(&(addr.sin_zero), 8);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(nPort);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenSocket, (struct sockaddr*)&addr, sizeof(addr)) != 0)
	{
		closesocket(listenSocket);
		return false;
	}

	if (listen(listenSocket, SOMAXCONN) != 0)
	{
		closesocket(listenSocket);
		return false;
	}
	IFCSLockHelper lh(m_ListenersLock);
	SocketInfo si;
	si.socket = listenSocket;
	si.usePackageMode = packagemode;
	si.useSSL = enableSSL;
	si.localPort = nPort;
	m_Listener.insert(makeIFPair(nPort, si));
	return true;
}

bool IFNetCoreSelect::stopListen(int nPort)
{
	IFCSLockHelper lh(m_ListenersLock);
	auto it = m_Listener.find(nPort);
	if (it == m_Listener.end())
		return false;
	closesocket(it->second.socket);
	m_Listener.erase(it);
	return true;
}

void IFNetCoreSelect::workThread()
{
	int maxfd = 0;

	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 100000;
	//char buf[1024*32];
	IFSimpleArray<char> buf(1024 * 32);
	IFLogDebug("IFNetCoreSelect::workThread Start!");

	while (!m_spWorkThread->isNeedExit())
	{


		if (m_ConnectingList.size() == 0 && m_Connections.size() == 0 && m_Listener.size()==0)
		{
			if (m_bExitWorkThread)
				break;

			IFThread::sleep(10);
			continue;
		}
		fd_set readfds;// =  m_Readfds;
		fd_set writefds;// = m_ConnectingFds;
		fd_set errorfds;// = m_ConnectingFds;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&errorfds);
		{
			IFCSLockHelper lh(m_ConnectionListLock);

			ConnectionList::iterator it = m_Connections.begin();
			while ( it != m_Connections.end())
			{
				IFNetCoreSelectConnection* pCon = (IFNetCoreSelectConnection*)(IFNetConnection*)(*it);
				if (!pCon || pCon->m_Socket == INVALID_SOCKET)
				{					
					it = m_Connections.erase(it);
					continue;
				}
				FD_SET(pCon->m_Socket, &readfds);
				FD_SET(pCon->m_Socket, &errorfds);
				if (pCon->m_Socket>maxfd)
					maxfd = pCon->m_Socket;
				++it;
			}
		}
		{
			IFCSLockHelper lh(m_ConnectingListLock);
			ConnectionList::iterator it = m_ConnectingList.begin();
			while (it != m_ConnectingList.end())
			{
				IFNetCoreSelectConnection* pCon = (IFNetCoreSelectConnection*)(IFNetConnection*)(*it);
				FD_SET(pCon->m_Socket, &writefds);
				FD_SET(pCon->m_Socket, &errorfds);
				if (pCon->m_Socket>maxfd)
					maxfd = pCon->m_Socket;
				++it;
			}
		}
		if(m_Listener.size())
		{
			for (auto& lser : m_Listener)
			{
				FD_SET(lser.second.socket, &readfds);
				if (lser.second.socket > maxfd)
					maxfd = lser.second.socket;
			}
		}


		int ret = select(maxfd+1,&readfds,  &writefds, &errorfds, &tv);
		if (ret>0)
		{
			IFLogTrace("SELECT RESULT %d\r\n", ret);
			IFCSLockHelper llh(m_ListenersLock);
			SocketInfo* pSockInfo = NULL;
			if (m_Listener.size())
			{
				for (auto& lser : m_Listener)
				{
					if (FD_ISSET(lser.second.socket, &readfds))
					{
						pSockInfo = &lser.second;
						break;
					}
					
				}				
			}

			//accept select
			if (pSockInfo)
			{
				sockaddr_in remoteAddress;
#ifndef WIN32
				socklen_t
#else
				int
#endif

				ADDSIZE = sizeof(remoteAddress);

				SOCKET newClient = accept(pSockInfo->socket, (sockaddr*)&remoteAddress, &ADDSIZE);
				//FD_SET(newClient, &m_Readfds);
				IFRefPtr<IFNetCoreSelectConnection> spConnection = IFNew IFNetCoreSelectConnection(this, pSockInfo->usePackageMode, m_bSyncEvent, newClient);
				char ip[128];
				inet_ntop(AF_INET, &(remoteAddress.sin_addr), ip, sizeof(ip));
				spConnection->m_sRemoteIP = ip;
				spConnection->m_nRemotePort = ntohs(remoteAddress.sin_port);
				spConnection->m_nLocalPort = pSockInfo->localPort;
				spConnection->m_eConnectionState = IFNCS_CONNECTED;
				{
					IFCSLockHelper lh2(m_ConnectionListLock);
					m_Connections.insert(spConnection);
				}

				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(spConnection, IFNetCoreEvent::ET_NEW_CONNECTION);
				pushEvent(pEvent);
				if (pSockInfo->useSSL)
					spConnection->waitEstablishSSL();
			}
			else
			{
				{
					IFCSLockHelper lh(m_ConnectionListLock);

					ConnectionList::iterator it = m_Connections.begin();
					for (; it != m_Connections.end();)
					{
						IFNetCoreSelectConnection* pCon = (IFNetCoreSelectConnection*)(IFNetConnection*)(*it);
						if (FD_ISSET(pCon->m_Socket, &readfds))
						{
							int nLen = recv(pCon->m_Socket, buf, buf.size(), 0);
							if (nLen > 0)
							{
								pCon->setLastCommunicateTime();
								pCon->recvData(buf, nLen);
							}
							else /*if (nLen<0)*/
							{
								pCon->disconnect();
								//removeAutoKeepAliveConnection((*it));
								it = m_Connections.erase(it);
                                //++it;
								continue;
							}

						}
						if (FD_ISSET(pCon->m_Socket, &errorfds))
						{
							//pCon->m_eConnectionState = IFNCS_NOTCONNECTED;
							pCon->disconnect();
							IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pCon, IFNetCoreEvent::ET_CONNECT, false );
							pushEvent(pEvent);
							IFCSLockHelper lh(m_ConnectingListLock);
							it  = m_ConnectingList.erase(it);
							continue;
						}
						++it;
					}

				}
				{
					IFCSLockHelper lh(m_ConnectingListLock);
					ConnectionList::iterator it = m_ConnectingList.begin();
					while (it != m_ConnectingList.end())
					{
						IFNetCoreSelectConnection* pCon = (IFNetCoreSelectConnection*)(IFNetConnection*)(*it);
                        if (FD_ISSET(pCon->m_Socket, &errorfds))
						{
							//pCon->m_eConnectionState = IFNCS_NOTCONNECTED;
							pCon->disconnect();
							IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pCon, IFNetCoreEvent::ET_CONNECT, false );
							pushEvent(pEvent);
							it  = m_ConnectingList.erase(it);
							continue;
						}
						if (FD_ISSET(pCon->m_Socket, &writefds))
						{

							//FD_CLR(pCon->m_Socket, &m_ConnectingFds);
							//FD_SET(pCon->m_Socket,&m_Readfds);

							pCon->m_eConnectionState = IFNCS_CONNECTED;
							{
								IFCSLockHelper lh2(m_ConnectionListLock);
								m_Connections.insert(*it);
							}
							IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pCon, IFNetCoreEvent::ET_CONNECT,true );
							pushEvent(pEvent);

							it  = m_ConnectingList.erase(it);

							continue;
						}

						++ it;
					}
				}
			}

		}
		else if (ret==-1)
		{
#ifdef WIN32
			DWORD err = GetLastError();
			IFLOG(IFLL_ERROR,"select error%d\r\n", err);
#endif
			//break;
		}
		else if (ret == 0)
		{
			if (m_bExitWorkThread)
				break;
		}



	}
}


IFUI64 IFNetCoreSelectConnection::sendPack( IFRefPtr<IFMemStream> pStream, IFUI64 nSendID)
{
	if (m_eConnectionState != IFNCS_CONNECTED)
		return 0;
	setLastCommunicateTime();
	int r = send(m_Socket, (const char*)pStream->getBuffer(), pStream->size(), 0);
	if (r<0)
	{
#ifdef WIN32
		r = GetLastError();
		IFLOG(IFLL_ERROR, "send error!%d\r\n", r);
#elif defined(IFPLATFORM_WEB)
		IFLOG(IFLL_ERROR, "send error!send size:%d\r\n", pStream->size());

#else
		IFLOG(IFLL_ERROR, "send error!%d send size:%d\r\n", errno, pStream->size());

#endif

		disconnect();
		return 0;
	}
	return 1;
}

IFNetCoreSelectConnection::IFNetCoreSelectConnection( IFNetCoreSelect* pCore,bool bPackagemode, bool bSyncEvent,SOCKET sock )
	:IFNetConnection(pCore,  bSyncEvent,bPackagemode)
	,m_pSelectCore(pCore)
{
	if (sock!=INVALID_SOCKET)
		m_Socket = sock;

}

IFNetCoreSelectConnection::~IFNetCoreSelectConnection()
{
	if (m_Socket != INVALID_SOCKET)
	{
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}

}


void IFNetCoreSelectConnection::disconnect()
{
	IFCSLockHelper lh(m_lock);
	if (m_Socket != INVALID_SOCKET)
	{
		//FD_CLR(m_Socket, &m_pSelectCore->m_Readfds);
		//closesocket(m_Socket);
		if (m_eConnectionState == IFNCS_CONNECTED)
		{
			//shutdown(m_Socket,2);

			IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this,IFNetCoreEvent::ET_DISCONNECT);
			m_pSelectCore->pushEvent(pEvent);
			m_eConnectionState = IFNCS_DISCONNECTED;

			closesocket(m_Socket);
		}

		m_Socket = INVALID_SOCKET;



	}
}

bool IFNetCoreSelectConnection::isShouldRemove()
{
	return false;
}


void IFNetCoreSelectConnection::setNoBlock()
{
	u_long ul = 1L;
#ifdef WIN32
	int nRet = ioctlsocket(m_Socket, FIONBIO, (u_long FAR *)&ul);
#else
	fcntl(m_Socket,F_SETFL, fcntl(m_Socket,F_GETFD,0)|O_NONBLOCK) ;

#endif
}

void IFNetCoreSelectConnection::setNoDelay()
{
	int bOptVal = 1;
	int bOptLen = sizeof(int);
	int br = setsockopt(m_Socket,IPPROTO_TCP, TCP_NODELAY , (char*)&bOptVal, bOptLen);

}

IFRefPtr<IFNetCore> IFNetCore::createNetCore()
{
	return IFNew IFNetCoreSelect;
}