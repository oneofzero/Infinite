/*
Copyright (c) 2020 HuangCong

"Anti 996" License Version 1.0 (Draft)

Permission is hereby granted to any individual or legal entity
obtaining a copy of this licensed work (including the source code,
documentation and/or related items, hereinafter collectively referred
to as the "licensed work"), free of charge, to deal with the licensed
work for any purpose, including without limitation, the rights to use,
reproduce, modify, prepare derivative works of, distribute, publish
and sublicense the licensed work, subject to the following conditions:

1. The individual or the legal entity must conspicuously display,
without modification, this License and the notice on each redistributed
or derivative copy of the Licensed Work.

2. The individual or the legal entity must strictly comply with all
applicable laws, regulations, rules and standards of the jurisdiction
relating to labor and employment where the individual is physically
located or where the individual was born or naturalized; or where the
legal entity is registered or is operating (whichever is stricter). In
case that the jurisdiction has no such laws, regulations, rules and
standards or its laws, regulations, rules and standards are
unenforceable, the individual or the legal entity are required to
comply with Core International Labor Standards.

3. The individual or the legal entity shall not induce, suggest or force
its employee(s), whether full-time or part-time, or its independent
contractor(s), in any methods, to agree in oral or written form, to
directly or indirectly restrict, weaken or relinquish his or her
rights or remedies under such laws, regulations, rules and standards
relating to labor and employment as mentioned above, no matter whether
such written or oral agreements are enforceable under the laws of the
said jurisdiction, nor shall such individual or the legal entity
limit, in any methods, the rights of its employee(s) or independent
contractor(s) from reporting or complaining to the copyright holder or
relevant authorities monitoring the compliance of the license about
its violation(s) of the said license.

THE LICENSED WORK IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN ANY WAY CONNECTION WITH THE
LICENSED WORK OR THE USE OR OTHER DEALINGS IN THE LICENSED WORK.
*/
#include "stdafx.h"
#include "IFNetCoreEPOLL.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include "IFLogSystem.h"

IFNetCoreEPOLL::IFNetCoreEPOLL()
	:m_nEpoll(0)
	,m_ListenScoket(INVALID_SOCKET)
	,m_bExitWorkThread(false)
{

}

IFNetCoreEPOLL::~IFNetCoreEPOLL()
{

}


bool IFNetCoreEPOLL::startListen(int nPort, bool enableSSL /*= false*/, bool packagemode /*= false*/)
{
	auto listenScoket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//socket_set_option($listen, SOL_SOCKET, SO_REUSEADDR, 1)
	int enable_reuseaddr = 1;
	if (setsockopt(listenScoket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable_reuseaddr, sizeof(enable_reuseaddr))!=0)
	{
		IFLOG(IFLL_ERROR, "set socket addr reuse error:%d\r\n", errno);
	}


	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(nPort);
	int hr = bind(listenScoket, (sockaddr*)&address, sizeof address);
	if (hr != 0)
	{
		IFLOG(IFLL_ERROR, "bind port %d error! %d\r\n", nPort, errno);
		return false;
	}
	hr = listen(listenScoket, SOMAXCONN);
	if (hr != 0)
	{
		IFLOG(IFLL_ERROR, "listen %d error! %d\r\n", listenScoket, errno);
		return false;
	}

	epoll_event epe;
	epe.data.fd = listenScoket;
	//epe.data.ptr = NULL;

	epe.events = EPOLLIN | EPOLLERR|EPOLLPRI;
	int result = epoll_ctl(m_nEpoll, EPOLL_CTL_ADD, listenScoket, &epe);
	if (result < 0)
	{
		IFLOG(IFLL_ERROR, "epoll %d add event listener error=%d\r\n", m_nEpoll, result);
	}
	else
	{
		IFLOG(IFLL_DEBUG, "epoll add event listener ok\r\n");
	}
	ListenSocketInfo lsi;
	lsi.nPort = nPort;
	lsi.bEnableSSL = enableSSL;
	m_ListenScoket.insert(makeIFPair(listenScoket, lsi));
	return true;
}

bool IFNetCoreEPOLL::stopListen(int nPort)
{
	for (auto it = m_ListenScoket.begin(); it!=m_ListenScoket.end(); ++it)
	{
		auto& pr = *it;
		if (pr.second.nPort == nPort)
		{
			closesocket(pr.first);
			m_ListenScoket.erase(it);
			return true;
		}
	}
	return false;
}


bool IFNetCoreEPOLL::onServiceStart()
{
	signal(SIGPIPE, SIG_IGN);
	m_nEpoll = epoll_create(m_nMaxConnectionCount);
	if (!m_nEpoll)
	{
		IFLOG(IFLL_ERROR,"epoll create failed! max count=%d\r\n", m_nMaxConnectionCount);
		return false;
	}
	if (m_nPort)
	{
		if (!startListen(m_nPort))
			return false;

	}
	
	m_spWorkThread = IFNew IFThread;
	m_spWorkThread->start(makeIFFunctor(this, &IFNetCoreEPOLL::workThread));
	return true;
}

bool IFNetCoreEPOLL::onServiceStop()
{
    m_bExitWorkThread = true;
    m_spWorkThread->waitExit();
	m_ConnectingList.clear();



}

void IFNetCoreEPOLL::workThread()
{
	epoll_event evts[64];
	IFSimpleArray<char> buf(1024 * 1024); //[32*1024];
	IFLOG(IFLL_DEBUG,"epoll work thread start!\r\n");

	while(!m_bExitWorkThread)
	{
		if( int ec = epoll_wait(m_nEpoll, evts, 64, 500))
		{
			//IFLOG(IFLL_DEBUG,"epoll get events num = %d\r\n", ec);

			for (int i = 0; i < ec; i ++ )
			{
				auto& evt = evts[i];
				//IFLOG(IFLL_DEBUG,"event%d=%d\r\n",i,evts[i].events);
				//IFLOG(IFLL_DEBUG, "event ptr:%p fd:%d", evts[i].data.ptr, evt.data.fd);
				IFNetEPOLLConnection* pConnection = (IFNetEPOLLConnection*)evts[i].data.ptr;

				auto listenIT = m_ListenScoket.find(evt.data.fd);
				if (listenIT != m_ListenScoket.end())
				{
					auto listenSocket = evt.data.fd;
					sockaddr_in remoteAddress;
					socklen_t ADDSIZE = sizeof(sockaddr_in);

					SOCKET sk = accept(listenSocket, (sockaddr*)&remoteAddress, &ADDSIZE);
					IFRefPtr<IFNetEPOLLConnection> spEPOLLConnection = IFNew IFNetEPOLLConnection(this, m_bPackageMode, m_bSyncEvent, sk);
					spEPOLLConnection->setNoDelay();
					spEPOLLConnection->setNoBlock();
					spEPOLLConnection->m_eConnectionState = IFNCS_CONNECTED;
					if (listenIT->second.bEnableSSL)
						spEPOLLConnection->waitEstablishSSL();

					char ip[128];
					inet_ntop(AF_INET, &(remoteAddress.sin_addr), ip, sizeof(ip));
					spEPOLLConnection->m_sRemoteIP = ip;
					spEPOLLConnection->m_nRemotePort = ntohs(remoteAddress.sin_port);
					spEPOLLConnection->m_nLocalPort = listenIT->second.nPort;


					epoll_event epe;
					//epe.data.fd = m_ListenScoket;
					epe.data.ptr = spEPOLLConnection;
					//epe.data.fd = spEPOLLConnection->m_Socket;

					epe.events = EPOLLOUT| EPOLLIN | EPOLLERR | EPOLLPRI |EPOLLET;
					int result = epoll_ctl(m_nEpoll, EPOLL_CTL_ADD, spEPOLLConnection->m_Socket, &epe);
					{
						IFCSLockHelper lh2(m_ConnectionListLock);
						m_Connections.insert(spEPOLLConnection);
						IFLOG(IFLL_DEBUG, "new connection accepted!\r\n", sk);
					}
					IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(spEPOLLConnection, IFNetCoreEvent::ET_NEW_CONNECTION);
					pushEvent(pEvent);
				}
				else if (pConnection)
				{


					if (evts[i].events & EPOLLOUT)
					{
						IFLOG(IFLL_TRACE, "EPOLLOUT&\r\n");
						pConnection->setLastCommunicateTime();
						if (pConnection->m_eConnectionState == IFNCS_NOTCONNECTED)
						{
							//if (evts[i].events & EPOLLIN)
							//{
							//	{
							//		IFCSLockHelper lh(m_ConnectingLock);
							//		ConnectionList::iterator it = m_ConnectingList.find(pConnection);
							//		if (it != m_ConnectingList.end())
							//			m_ConnectingList.erase(it);
							//	}
							//	IFLOG(IFLL_ERROR, "EPOLLIN& disconnect error!\r\n");
							//	IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, false );
							//	pushEvent(pEvent);
							//	pConnection->disconnect();
							//}
							//else
							{
								int error = 0;
								socklen_t len = sizeof(int);
								if ((0 == getsockopt(pConnection->m_Socket, SOL_SOCKET, SO_ERROR, &error, &len)) && 0 == error)
								{

									
									{
										IFCSLockHelper lh(m_ConnectionListLock);
										m_Connections.insert(pConnection);
									}
									{
										IFCSLockHelper lh(m_ConnectingLock);
										ConnectionList::iterator it = m_ConnectingList.find(pConnection);
										if (it != m_ConnectingList.end())
											m_ConnectingList.erase(it);
									}
									epoll_event epe;
									//epe.data.fd = m_ListenScoket;
									epe.data.ptr = pConnection;
									epe.events = EPOLLIN|EPOLLOUT | EPOLLERR | EPOLLPRI | EPOLLET;
									if (epoll_ctl(m_nEpoll, EPOLL_CTL_MOD, pConnection->m_Socket, &epe)<0)
									{
										IFLOG(IFLL_DEBUG, "epoll EPOLL_CTL_MOD error!%d\r\n", errno);
									}

									IFLOG(IFLL_DEBUG, "epoll connect ok!%d\r\n", evts[i].events);

									pConnection->m_eConnectionState = IFNCS_CONNECTED;
									IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, true);
									pushEvent(pEvent);


								}
								else
								{
									IFLOG(IFLL_ERROR, "connect error!%d\r\n", error);
									{
										IFCSLockHelper lh(m_ConnectingLock);
										ConnectionList::iterator it = m_ConnectingList.find(pConnection);
										if (it != m_ConnectingList.end())
											m_ConnectingList.erase(it);
									}
									IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_CONNECT, false);
									pushEvent(pEvent);
									pConnection->disconnect();
								}
							}												
						}
						
						if(pConnection->m_eConnectionState == IFNCS_CONNECTED)
						{
							IFLOG(IFLL_TRACE, "EPOLL SENDEDDATA!\r\n");
							pConnection->sendPendingPacks();
						}
					}
					if (evts[i].events & EPOLLIN)
					{
						if (pConnection->m_eConnectionState == IFNCS_CONNECTED )
						{
							pConnection->setLastCommunicateTime();
							int rs = 0;
							do
							{
								rs = recv(pConnection->m_Socket, buf, buf.size(), 0);
								if (rs > 0)
								{
									//buf[rs] = 0;
									//IFLOG(IFLL_DEBUG,"recv[%d]\r\n",rs);

									pConnection->recvData(buf, rs);
								}
								else if (rs == 0)
								{
									IFLOG(IFLL_DEBUG, "recv error disconnect %d\r\n", errno);
									pConnection->disconnect();

								}
								else if (rs < 0)
								{
									if (errno != EAGAIN)
									{
										IFLOG(IFLL_DEBUG, "recv error disconnect %d\r\n", errno);
										pConnection->disconnect();
									}

								}
							} while (rs>0);
							
							
						}
						else
						{
							IFLOG(IFLL_ERROR, "connection not connect!cant recv data!\r\n");
						}


					}
					
					if(evts[i].events & (EPOLLERR | EPOLLPRI))
					{
						if (evts[i].events &EPOLLPRI)
						{
							IFLOG(IFLL_ERROR, "recv error disconnect EPOLLPRI!\r\n");
						}
						pConnection->disconnect();
					}
					evts[i].events = 0;
				}
				
				else
				{
					IFLOG(IFLL_ERROR,"unknown event!%d\r\n", evts[i].data.fd)
				}
			}
		}

		//IFThread::sleep(1000);

	}
	IFLOG(IFLL_DEBUG,"epoll work thread exit!\r\n");

}

IFNetConnectionPtr IFNetCoreEPOLL::createConnection( const IFString& sAddress, int nPort,  bool syncconnect, bool bPackagemode, bool bSyncEvent )
{
	IFRefPtr<IFNetEPOLLConnection> spConnection = IFNew IFNetEPOLLConnection(this, bPackagemode, bSyncEvent);
	spConnection->setNoDelay();
	if (!syncconnect)
	{
		spConnection->setNoBlock();

		getHostAddressAsync(sAddress, nPort, makeIFFunctor<void(const IFNetCore::AddressInfo*)>([=](const IFNetCore::AddressInfo* pADDR)
		{
			if (!pADDR)
			{
				spConnection->m_eConnectionState = IFNCS_CONNECTED;
				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(spConnection, IFNetCoreEvent::ET_CONNECT, false );
				pushEvent(pEvent);
				return;
			}
			/*sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr = *pADDR;
			address.sin_port = htons( nPort );*/
			IFLOG(IFLL_DEBUG, "connect to %s:%d\r\n", pADDR->ip.c_str(), pADDR->port);

			if(connect(spConnection->m_Socket, (sockaddr*)&pADDR->addr, pADDR->addrlen)<0)
			{

				if (errno == EINPROGRESS)
				{
					epoll_event epe;
					//epe.data.fd = spConnection->m_Socket;
					epe.data.ptr = spConnection;

					epe.events = EPOLLOUT | EPOLLERR | EPOLLPRI | EPOLLET;
					int result = epoll_ctl(m_nEpoll, EPOLL_CTL_ADD, spConnection->m_Socket, &epe);
					spConnection->m_eConnectionState = IFNCS_NOTCONNECTED;
					spConnection->m_sRemoteIP = pADDR->ip;
					spConnection->m_nRemotePort = pADDR->port;
					IFCSLockHelper lh(m_ConnectingLock);
					m_ConnectingList.insert(spConnection);
					return;
				}
				else
				{
					spConnection->m_eConnectionState = IFNCS_CONNECTED;
					IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(spConnection, IFNetCoreEvent::ET_CONNECT, false);
					pushEvent(pEvent);
					return;

				}
				
				
				
			}
			else 
			{
				IFLOG(IFLL_DEBUG,"sync connect ok!\r\n");
			}

			epoll_event epe;
			//epe.data.fd = spConnection->m_Socket;
			epe.data.ptr = spConnection;

			epe.events = EPOLLOUT|EPOLLIN|EPOLLERR | EPOLLPRI | EPOLLET;
			int result = epoll_ctl(m_nEpoll, EPOLL_CTL_ADD, spConnection->m_Socket, &epe);
			spConnection->m_eConnectionState = IFNCS_CONNECTED;
			spConnection->m_sRemoteIP = pADDR->ip;
			spConnection->m_nRemotePort = pADDR->port;
			IFCSLockHelper lh2(m_ConnectionListLock);
			m_Connections.insert(spConnection);

		}));

		return spConnection;
	}

	IFNetCore::AddressInfo addressinfo;
	if (!IFNetCore::getHostAddress(sAddress, nPort, &addressinfo))
		return NULL;

	//sockaddr_in address;
	//address.sin_family = AF_INET;
	//address.sin_addr.s_addr = inet_addr( sAddress.c_str() );
	//if(address.sin_addr.s_addr == 0xffffffff)
	//{
	//	hostent* pHostent = gethostbyname(sAddress.c_str());
	//	if(pHostent)
	//	{
	//		IFLOG(IFLL_DEBUG,"reslove host%s sucess\r\n", sAddress.c_str());

	//		address.sin_addr = *((in_addr*)pHostent->h_addr_list[0]);
	//	}
	//	else
	//	{
	//		IFLOG(IFLL_ERROR,"cant reslove host%s\r\n", sAddress.c_str());
	//	}
	//}
	//address.sin_port = htons( nPort );



	if(connect(spConnection->m_Socket, (sockaddr*)&addressinfo.addr, addressinfo.addrlen)<0)
	{
		if (syncconnect)
			return NULL;
		else
		{
			epoll_event epe;
			//epe.data.fd = spConnection->m_Socket;
			epe.data.ptr = spConnection;

			epe.events = EPOLLIN|EPOLLOUT|EPOLLERR | EPOLLPRI | EPOLLET;
			int result = epoll_ctl(m_nEpoll, EPOLL_CTL_ADD, spConnection->m_Socket, &epe);
			spConnection->m_eConnectionState = IFNCS_NOTCONNECTED;

			IFCSLockHelper lh(m_ConnectingLock);
			m_ConnectingList.insert(spConnection);
			return spConnection;
		}
	}
	else 
	{
		IFLOG(IFLL_DEBUG,"sync connect ok!\r\n");
	}
	spConnection->setNoBlock();
	
	epoll_event epe;
	//epe.data.fd = spConnection->m_Socket;
	epe.data.ptr = spConnection;

	epe.events = EPOLLOUT|EPOLLIN|EPOLLERR | EPOLLPRI | EPOLLET;
	int result = epoll_ctl(m_nEpoll, EPOLL_CTL_ADD, spConnection->m_Socket, &epe);
	spConnection->m_eConnectionState = IFNCS_CONNECTED;
	IFCSLockHelper lh2(m_ConnectionListLock);
	m_Connections.insert(spConnection);
	return spConnection;
}



IFNetEPOLLConnection::IFNetEPOLLConnection( IFNetCoreEPOLL* pEPOLLCore ,bool bPackagemode, bool bSyncEvent, SOCKET sk/*=INVALID_SOCKET*/)
	:IFNetConnection(pEPOLLCore, bSyncEvent, bPackagemode )
	,m_pEPOLLCore(pEPOLLCore)
{
	if (sk!=INVALID_SOCKET)
		m_Socket = sk;
	else
		m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&so_linger, sizeof(so_linger));
}

IFNetEPOLLConnection::~IFNetEPOLLConnection()
{
	if (m_Socket)
	{
		closesocket(m_Socket);
	}
}

bool IFNetEPOLLConnection::sendPendingPacks()
{
	IFCSLockHelper lh(m_SendPacksLock);
	//auto it = m_SendPacks.begin();
	while ( m_SendPacks.size())
	{
		auto&pk = m_SendPacks.front();
		while (pk.nRequestSended< pk.spStream->size())
		{
			if (getState() != IFNCS_CONNECTED)
			{
				IFLOG(IFLL_ERROR, "send error! not connect!\r\n");
				return false;
			}
			int r = send(m_Socket, (const char*)pk.spStream->getBuffer() + pk.nRequestSended, pk.spStream->size() - pk.nRequestSended, 0);
			if (r < 0)
			{
				if (errno != EAGAIN)
				{
					IFLOG(IFLL_ERROR, "send error!%d\r\n", errno);
					disconnect();
					return false;
				}
				else
				{
					return true;
				}
			}
			pk.nRequestSended += r;		
			ATOMIC_ADD_INT32((volatile long*)&m_nSendPendingBytes, -r);

		}
		//it = m_SendPacks.erase(it);
		m_SendPacks.pop_front();
		IFLOG(IFLL_TRACE, "send pack data pop!%d\n", m_SendPacks.size());
		
	}
	return true;
}

IFUI64 IFNetEPOLLConnection::sendPack( IFRefPtr<IFMemStream> pStream , IFUI64 nSendID )
{
	nSendID = nSendID == 0 ? getSendID() : nSendID;
	{
		IFCSLockHelper lh(m_SendPacksLock);
		PackInfo pkinfo;
		pkinfo.nRequestSended = 0;
		pkinfo.spStream = pStream;
		pkinfo.nSendID = nSendID;
		m_SendPacks.push_back(pkinfo);
		ATOMIC_ADD_INT32(&m_nSendPendingBytes, pStream->size());
	}
	sendPendingPacks();
	
	return nSendID;
}

void IFNetEPOLLConnection::disconnect()
{
	IFCSLockHelper lh(m_SendPacksLock);
	if (m_Socket != INVALID_SOCKET)
	{
		//FD_CLR(m_Socket, &m_pSelectCore->m_Readfds);
		//closesocket(m_Socket);
		//if (m_eConnectionState == IFNCS_CONNECTED ||
		//	m_eConnectionState == IFNCS_NOTCONNECTED)
		//{
			//shutdown(m_Socket,2);
			epoll_event epe = { 0,{0} };
			
			if (epoll_ctl(m_pEPOLLCore->m_nEpoll, EPOLL_CTL_DEL, m_Socket, &epe)<0)
			{
				IFLOG(IFLL_ERROR, "EPOLL_CTL_DEL error:%d\r\n", errno);
			}
			IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this,IFNetCoreEvent::ET_DISCONNECT);
			m_pEPOLLCore->pushEvent(pEvent);
			m_eConnectionState = IFNCS_DISCONNECTED;
			closesocket(m_Socket);
			m_Socket = INVALID_SOCKET;

		//}

		IFLOG(IFLL_DEBUG,"%p disconnect\r\n", this);
		//m_Socket = INVALID_SOCKET;



	}
	m_SendPacks.clear();
}

bool IFNetEPOLLConnection::isShouldRemove()
{
	return false;
}

void IFNetEPOLLConnection::setNoBlock()
{
	fcntl(m_Socket,F_SETFL, fcntl(m_Socket,F_GETFD,0)|O_NONBLOCK) ;

}

void IFNetEPOLLConnection::setNoDelay()
{
	int bOptVal = true;
	int bOptLen = sizeof(int);
	int br = setsockopt(m_Socket,IPPROTO_TCP, TCP_NODELAY , (char*)&bOptVal, bOptLen);

}
