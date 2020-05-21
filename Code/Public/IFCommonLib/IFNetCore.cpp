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
#include "IFNetCore.h"
#include "IFNetIOData.h"
#include "IFLogSystem.h"
#include "IFTimer.h"
#include "IFNetMsgFactory.h"
#include "IFSystemAPI.h"
#include "IFPlatformDefine.h"
#include "IFFunctorDefaultParam.h"
#include "IFThreadSyncObj.h"
#include "IFRSA.h"
#include "IFAES.h"

#ifdef WIN32
#include <WS2tcpip.h>
#include <WSPiApi.h>
#include <ws2ipdef.h>

#if defined(IFPLATFORM_WP)
#include "IFNetCoreSelect.h"
#else
#include "IFNetCoreIOCP.h"
#include "IFNetCoreSelect.h"

#endif
#else
#ifdef LINUX
#include "IFNetCoreEPOLL.h"
#else
#include "IFNetCoreSelect.h"
#endif
#endif
#ifdef IFCOMMON_UNITY_SUPPORT
#define DONT_USE_SSL
#endif

#if defined(IFPLATFORM_IOS)
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

#ifdef IFPLATFORM_ANDROID
//#include "android-ifarrds/ifaddrs.h"
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#ifndef DONT_USE_SSL
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/rand.h>
#include <openssl/err.h>

SSL_CTX* GetSSLContext();
#endif



IF_DEFINERTTI(IFNetCore, IFRefObj);
IF_DEFINERTTI(IFNetCore::UDPData, IFRefObj);
IFNetCore::IFNetCore(void) :
	m_nMaxConnectionCount(1000),
	//m_nCurConnectionCount(0),
	m_nSendPackage(0),
	m_nRecvPackage(0),
	m_nSendBytes(0),
	m_nRecvBytes(0)
{
	m_spEventSyncObj = IFNew IFThreadSyncObj();
	m_spTimer = IFNew IFTimer();
	m_spMsgFactory = IFNew IFNetMsgFactory;
	m_spMsgFactory->registerMsg(makeIFFunctor<void(IFNetConnection*, IFNet_Message_EstablishEncryption_Req*)>([=](IFNetConnection* pConnection, IFNet_Message_EstablishEncryption_Req* pMsg)
		{
			auto privatekey = getRSAPrivateKey(pMsg->m_sPublicKeyMD5);
			IFRefPtr<IFNet_Message_EstablishEncryption_Res> spRet = IFNew IFNet_Message_EstablishEncryption_Res;

			if (!privatekey || !pMsg->m_sEncryptAESKey.size())
			{
				pConnection->sendMsg(spRet);
				return;
			}
			IFRefPtr<IFAES> spAES;

			{
				IFRefPtr<IFMemStream> spOut = IFNew IFMemStream;
				IFRefPtr<IFMemStream> spIn = IFNew IFMemStream(pMsg->m_sEncryptAESKey, pMsg->m_sEncryptAESKey.size());

				privatekey->seek(0, IFStream::ISSF_BEGIN);
				IFLOG(IFLL_TRACE, "RSA INPUT:%s\r\n", IFString().encodeBase64((const char*)spIn->getBuffer(), spIn->size()).c_str());
				IFLOG(IFLL_TRACE, "RSA KEY:%s\r\n", IFString().encodeBase64((const char*)privatekey->getBuffer(), privatekey->size()).c_str());
				if (!IFRSA::decryptPrivate(spOut, spIn, privatekey))
				{
					pConnection->sendMsg(spRet);
					return;
				}
				spAES = IFNew IFAES(IFString((const char*)spOut->getBuffer(), (int)spOut->size()));
			}

			{
				IFRefPtr<IFMemStream> spOut = IFNew IFMemStream;
				IFRefPtr<IFMemStream> spIn = IFNew IFMemStream(pMsg->m_sTestDataEncrypt, pMsg->m_sTestDataEncrypt.size());
				spAES->decrypt(spIn, spOut);
				spRet->m_sTestData.resize(spOut->size());
				memcpy(spRet->m_sTestData, spOut->getBuffer(), spOut->size());


				pConnection->sendMsg(spRet);

				pConnection->setDecryptFun(makeIFFunctor<IFRefPtr<IFMemStream>(IFStream*)>([=](IFStream* pIn)
					{
						IFRefPtr<IFMemStream> spOUT = IFNew IFMemStream;
						spAES->decrypt(pIn, spOUT);
						return spOUT;
					}));
				pConnection->setDefalutEncryptFun(makeIFFunctor<IFRefPtr<IFMemStream>(IFStream*)>([=](IFStream* pIn)
					{
						IFRefPtr<IFMemStream> spOUT = IFNew IFMemStream;
						spAES->encrypt(pIn, spOUT);
						return spOUT;
					}));

				return;
			}


		}));
	m_spMsgFactory->registerMsg(makeIFFunctor(&IFNetConnection::ProcEstablishEncryptRes));
	//m_spMsgFactory->registerMsg(IFNet_Message_Sproto::MsgID, makeIFFunctor(IFNet_Message_Sproto::createMsg));

	//IFNet_Message_EstablishEncryption_Req::setMsgProc();

	//IFNet_Message_EstablishEncryption_Res::setMsgProc();
//	InitializeCriticalSection(&m_ConnectionListLock);
}

IFNetCore::~IFNetCore(void)
{
	//	DeleteCriticalSection(&m_ConnectionListLock);
}




IFUI64 IFNetCore::getSendPackCount()
{
	return m_nSendPackage;
}

IFUI64 IFNetCore::getRecvPackCount()
{
	return m_nRecvPackage;
}

IFUI64 IFNetCore::getSendByteCount()
{
	return m_nSendBytes;
}


IFUI64 IFNetCore::getRecvByteCount()
{
	return m_nRecvBytes;
}

IFTimer* IFNetCore::getTimer()
{
	return m_spTimer;
}

int IFNetCore::getConnectionCount()
{
	return m_Connections.size();
}

int IFNetCore::getMaxConnectionCount()
{
	return m_nMaxConnectionCount;
}




void IFNetCore::enumConnections(IFRefPtr<IFFunctor<bool(IFNetConnection*)>> spEnumFun)
{
	IFCSLockHelper lh(m_ConnectionListLock);

	for (ConnectionList::iterator it = m_Connections.begin();
		it != m_Connections.end(); ++it)
	{
		if ((*spEnumFun)(*it))
		{
			return;
		}
	}
}



void IFNetCore::fireNewConnectionEvent(IFNetConnection* pConnection)
{
	if (m_bSyncEvent)
	{
		IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(pConnection, IFNetCoreEvent::ET_NEW_CONNECTION);
		pushEvent(pEvent);
	}
	else
	{
		event_NewConnection(this, pConnection);
	}
}

void IFNetCore::pushEvent(IFNetCoreEvent* e)
{
	m_EventList.push(e);
	m_spEventSyncObj->notify();

}


bool IFNetCore::process()
{
	const int maxNum = 1000000;
	int nNum = 1000000;
	while (nNum)
	{
		//IFLOG(IFLL_DEBUG, "process event\r\n");


		IFNetCoreEvent* pEvent = NULL;
		if (m_EventList.pop(pEvent))
		{
			//IFLOG(IFLL_DEBUG, "pop event\r\n" );
			pEvent->fireEvent(this);
			//IFLOG(IFLL_DEBUG, "fire event\r\n");
			if (pEvent->eventType == IFNetCoreEvent::ET_DISCONNECT)
			{
				IFLOG(IFLL_DEBUG, "disconnect event fired!%p\r\n", pEvent->spConnection.getPtr());
				IFCSLockHelper hl(m_ConnectionListLock);
				ConnectionList::iterator it = m_Connections.find(pEvent->spConnection);
				if (it != m_Connections.end())
				{
					if (pEvent->spConnection->getAutoKeepAliveTime())
						removeAutoKeepAliveConnection(pEvent->spConnection);

					m_Connections.erase(it);

				}
			}
			delete pEvent;
		}
		else
		{
			break;
		}

		nNum--;

	}
	if (m_AutoKeepAliveList.size())
	{
		IFLOG(IFLL_DEBUG, "process keep alive %d\r\n", m_AutoKeepAliveList.size());
		IFUI32 dwCurTime = IFNativeSystemAPI::getTickCount();
		IFRefPtr<IFMemStream> spKeepAlive = IFNew IFMemStream;
		spKeepAlive->writeUI32(9);
		spKeepAlive->writeI8(0);
		spKeepAlive->writeI32(0);

		ConnectionWeakList::iterator it = m_AutoKeepAliveList.begin();
		while (it != m_AutoKeepAliveList.end())
		{
			if ((*it)->getState() == IFNCS_DISCONNECTED)
			{
				it = m_AutoKeepAliveList.erase(it);
				continue;
			}

			if (dwCurTime - (*it)->getLastCommunicateTime() > (*it)->getAutoKeepAliveTime())
			{
				IFMemStream* pKeepAlive = (*it)->getAutoKeepAlivePackage();
				if (!pKeepAlive)
					pKeepAlive = spKeepAlive;
				(*it)->sendPack(pKeepAlive);
			}
			++it;
		}
	}

	if (m_OpendUDPProcThreadList.size())
	{
		for (auto it = m_OpendUDPProcThreadList.begin(); it != m_OpendUDPProcThreadList.end(); ++it)
		{
			auto& datas = it->second.second->m_Datas;
			if (datas.size())
			{
				IFCSLockHelper lh(it->second.second->m_DataLock);
				for (auto d : datas)
				{
					event_RecvUDPData(this, d);
				}
				datas.clear();
			}

		}
	}

	if (m_GetHostIPResultQueue.size())
	{
		IFCSLockHelper lh(m_GetHostIPResultQueueLock);

		while (m_GetHostIPResultQueue.size())
		{

			auto& pr = m_GetHostIPResultQueue.front();
			if (pr.second.first)
				(*pr.first)(&pr.second.second);
			else
				(*pr.first)(NULL);

			m_GetHostIPResultQueue.pop_front();

		}
	}
	m_spTimer->update();

	return maxNum != nNum;
}

bool IFNetCore::waitAndProcess(int ms)
{
	m_spEventSyncObj->wait(ms);
	return process();
}

bool IFNetCore::startService(int nPort, int nMaxConnection, bool bPackagemode, bool bSyncEvent)
{
	m_nPort = nPort;
	m_nMaxConnectionCount = nMaxConnection;
	m_bPackageMode = bPackagemode;
	m_bSyncEvent = bSyncEvent;
	int processornum = IFNativeSystemAPI::getProcessorCount();
	if (processornum > 1)
		processornum /= 2;
	m_PackSerializeThreads.reserve(processornum);
	m_SerializeQueue.resize(processornum);
	for (int i = 0; i < processornum; i++)
	{
		IFRefPtr<IFThread> spThread = IFNew IFThread();
		m_PackSerializeThreads.push_back(spThread);
		spThread->start(makeIFDPFunctor(this, &IFNetCore::processPackSerialize, makeIFDefaultParam(spThread, i)));
	}

	return onServiceStart();
}

bool IFNetCore::stopService()
{
	while (m_OpendUDPProcThreadList.size())
	{
		closeUDPPort(m_OpendUDPProcThreadList.begin()->first);
	}

	if (m_spGetHostIPThread)
	{
		m_spGetHostIPThread->requestExit();
		m_spGetHostIPThread->waitExit();
	}

	for (auto pThread : m_PackSerializeThreads)
	{
		pThread->requestExit();
	}
	for (auto pThread : m_PackSerializeThreads)
	{
		pThread->waitExit();
	}
	if (onServiceStop())
	{
		process();
		return true;
	}


	return false;
}




void IFNetCoreEvent::fireEvent(IFNetCore* pNetCore)
{
	switch (eventType)
	{
	case ET_NEW_CONNECTION:
		pNetCore->event_NewConnection(pNetCore, spConnection);
		break;
	case ET_CONNECT:
		spConnection->event_ConnectResult(spConnection, bConnectSuccess);
		break;
	case ET_DISCONNECT:
		spConnection->event_Disconnect(spConnection);
		break;
	case ET_DATA_RECV:
		spConnection->event_RecvData(spConnection, *pData, pData->size());
		delete pData;
		break;
	case ET_PACK_RECV:
		spConnection->event_RecvPackage(spConnection, pPack);
		{
			auto pInfo = pNetCore->getMsgFactory()->getMsgInfo(pPack->getMsgID());
			if (pInfo && pInfo->spProcess)
			{
				(*pInfo->spProcess)(spConnection, pPack);
			}
		}
		//pPack->process(spConnection);
		//delete pPack;
		break;
	case ET_ERROR_PACK:
		spConnection->event_ErrorPack(spConnection);
		break;
	case ET_DATA_SENDED:
		spConnection->event_SendResult(spConnection, nSendID, bSendSuccess);
		break;
	case ET_SSL_ESTABLISH_RESULT:
		spConnection->event_SSLEstablishResult(spConnection, bConnectSuccess);
		break;
	default:
		break;
	}
}


void IFNetCore::addAutoKeepAliveConnection(IFNetConnection* pConnection)
{
	m_AutoKeepAliveList.insert(pConnection);
}

void IFNetCore::removeAutoKeepAliveConnection(IFNetConnection* pConnection)
{
	ConnectionWeakList::iterator it = m_AutoKeepAliveList.find(pConnection);
	if (it != m_AutoKeepAliveList.end())
	{
		m_AutoKeepAliveList.erase(it);
	}
}


void IFNetCore::sendAsyncMsg(const IFAsyncSendMsgInfo& info)
{
	IFLOG(IFLL_TRACE, "sendAsyncMsg 1");
	int nThreadIdx = ((IFUI64)(void*)info.spConnection.getPtr()) % m_SerializeQueue.size();
	auto& queue = m_SerializeQueue[nThreadIdx];
	IFLOG(IFLL_TRACE, "sendAsyncMsg 2");

	queue.push(info);
	IFLOG(IFLL_TRACE, "sendAsyncMsg 3");
	m_PackSerializeThreads[nThreadIdx]->getSyncObj()->notify();
	IFLOG(IFLL_TRACE, "sendAsyncMsg 4");
}

IFRefPtr<IFNetCore> IFNetCore::createNetCore()
{
#ifdef WIN32
#if defined(IFPLATFORM_WP)
	return IFNew IFNetCoreSelect;
#else
	return IFNew IFNetCoreIOCP;
	//return IFNew IFNetCoreSelect;
#endif
#else
#ifdef LINUX
	return IFNew IFNetCoreEPOLL;
#else
	return IFNew IFNetCoreSelect;
#endif
#endif

}

bool IFNetCore::openUDPPort(int nPort)
{
	auto it = m_OpendUDPProcThreadList.find(nPort);
	if (it != m_OpendUDPProcThreadList.end())
		return false;
	//SOCKET m_Socket =  WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );

	IFRefPtr<UDPSocketInfo> pInfo = IFNew UDPSocketInfo;
	pInfo->m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	pInfo->m_nLocalPort = nPort;


	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(nPort); ///监听端口
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	int so_broadcast = 1;
	setsockopt(pInfo->m_Socket, SOL_SOCKET, SO_BROADCAST, (char*)&so_broadcast, sizeof(so_broadcast));
	int enable_reuseaddr = 1;
	if (setsockopt(pInfo->m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable_reuseaddr, sizeof(enable_reuseaddr)) != 0)
	{
		IFLOG(IFLL_ERROR, "set socket addr reuse error:%d\r\n", errno);
	}

	if (bind(pInfo->m_Socket, (struct sockaddr*) & local, sizeof local) != 0)
	{
		return false;
	}

	pInfo->addRef();
	IFRefPtr<IFThread> spThread = IFNew IFThread;
	spThread->start(makeIFDPFunctor(this, &IFNetCore::processUDPReceive, IFFunctorParam<UDPSocketInfo*>(pInfo)));
	m_OpendUDPProcThreadList[nPort] = makeIFPair(spThread, pInfo);

	return true;
}

bool IFNetCore::closeUDPPort(int nPort)
{
	auto it = m_OpendUDPProcThreadList.find(nPort);
	if (it == m_OpendUDPProcThreadList.end())
		return false;
	closesocket(it->second.second->m_Socket);
	it->second.second->m_Socket = INVALID_SOCKET;
	while (it->second.first->isRunning())
	{
		IFThread::sleep(1);
	}
	m_OpendUDPProcThreadList.erase(it);
	return true;
}

bool IFNetCore::sendUDPData(const IFString& address, int nRemotePort, int nLocalPort, const char* pData, int nLen)
{
	auto it = m_OpendUDPProcThreadList.find(nLocalPort);
	if (it == m_OpendUDPProcThreadList.end())
		return false;

	sockaddr_in server;
	int len = sizeof(server);
	server.sin_family = AF_INET;
	server.sin_port = htons(nRemotePort); ///server的监听端口
	server.sin_addr.s_addr = inet_addr(address.c_str()); ///server的地址
	if (server.sin_addr.s_addr == 0xffffffff)
	{
		hostent* pHostent = gethostbyname(address.c_str());
		if (pHostent)
		{
			IFLOG(IFLL_DEBUG, "reslove host%s sucess\r\n", address.c_str());

			server.sin_addr = *((in_addr*)pHostent->h_addr_list[0]);
		}
		else
		{
			IFLOG(IFLL_ERROR, "cant reslove host%s\r\n", address.c_str());
		}
	}

	int nSendBytes = sendto(it->second.second->m_Socket, pData, nLen, 0, (struct sockaddr*) & server, len);
	return nSendBytes == nLen;
}

bool IFNetCore::sendUDPDataByAddr(const AddressInfo* address, int nLocalPort, const char* pData, int nLen)
{
	auto it = m_OpendUDPProcThreadList.find(nLocalPort);
	if (it == m_OpendUDPProcThreadList.end())
		return false;
	int nSendBytes = sendto(it->second.second->m_Socket, pData, nLen, 0, (struct sockaddr*)address->addr, address->addrlen);
	return nSendBytes == nLen;
}



void IFNetCore::processUDPReceive(UDPSocketInfo* pSocket)
{
	char buf[1024 * 64];
	while (pSocket->m_Socket != INVALID_SOCKET)
	{
		sockaddr_in from;
#ifdef WIN32
		int fromlen = sizeof(from);
#else
		socklen_t fromlen = sizeof(from);
#endif	
		int nrecvLen = recvfrom(pSocket->m_Socket, buf, sizeof(buf), 0, (struct sockaddr*) & from, &fromlen);
		if (nrecvLen < 0)
		{
#ifdef WIN32
			if (WSAGetLastError() == WSAECONNRESET)
			{
				continue;
			}
#endif
			pSocket->decRef();
			return;
		}
		else if (nrecvLen == 0)
		{
			//IFThread::sleep(1);
			continue;
		}
		IFRefPtr<UDPData> spUDPData = IFNew UDPData;
		spUDPData->nRemotePort = ntohs(from.sin_port);
		spUDPData->remoteAddress = inet_ntoa(from.sin_addr);
		spUDPData->nLocalPort = pSocket->m_nLocalPort;
		spUDPData->dataBuf.resize(nrecvLen);
		memcpy(spUDPData->dataBuf, buf, nrecvLen);

		IFCSLockHelper lh(pSocket->m_DataLock);
		pSocket->m_Datas.push_back(spUDPData);
	}
	pSocket->decRef();
}

void IFNetCore::getHostAddressAsync(const IFString& address, int nPort, AsyncGetAddressCallbackPtr spResult)
{
	//in_addr addr;
	/*addr.s_addr = inet_addr( address.c_str() );
	if (addr.s_addr!=0xffffffff)
	{
		(*spResult)(&addr);
	}
	else
	{*/
	IFCSLockHelper lh(m_GetHostIPQueueLock);
	GetAddressRequestInfo info;
	info.add = address;
	info.port = nPort;
	m_GetHostIPQueue.push_back(makeIFPair(info, spResult));
	if (!m_spGetHostIPThread)
	{
		m_spGetHostIPThread = IFNew IFThread;
		m_spGetHostIPThread->start(makeIFDPFunctor(this, &IFNetCore::processGetHostIP, IFFunctorParam<IFThread*>(m_spGetHostIPThread)));
	}
	//}
}

bool IFNetCore::getHostAddress(const IFString& address, int nPort, AddressInfo* adinfo)
{

	if (address.size() == 0)
	{
#if defined(IFPLATFORM_IOS) 
		ifaddrs* interfaces;
		if (getifaddrs(&interfaces) == 0)
		{
			auto ptemp = interfaces;
			while (ptemp)
			{
				if (ptemp->ifa_addr->sa_family == AF_INET)
				{
					if (strcmp(ptemp->ifa_name, "en0") == 0)
					{
						char ip[128];
						inet_ntop(AF_INET, &(((struct sockaddr_in*) ptemp->ifa_addr)->sin_addr), ip, sizeof(ip));
						adinfo->ip = ip;
						break;

					}
				}
				ptemp = ptemp->ifa_next;
			}

			freeifaddrs(interfaces);
			return true;
		}
#elif defined(IFPLATFORM_ANDROID)

		int i = 0;
		int sockfd;
		ifconf conf;
		unsigned char buf[512];
		ifreq* req;
		//初始化ifconf
		conf.ifc_len = 512;
		conf.ifc_buf = (char*)buf;
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (sockfd == INVALID_SOCKET)
		{
			IFLOG(IFLL_ERROR, "get socket error!%d\r\n", errno);
			return false;
		}
		ioctl(sockfd, SIOCGIFCONF, &conf); //获取所有接口信息
		//接下来一个一个的获取IP地址
		req = (ifreq*)buf;

		for (i = (conf.ifc_len / sizeof(req)); i > 0; i--)
		{
			// if(ifreq->ifr_flags == AF_INET){ //for ipv4
			char* ip = inet_ntoa(((struct sockaddr_in*) & (req->ifr_addr))->sin_addr);
			//__android_log_print(ANDROID_LOG_INFO, "test", "%s", ip);
			if (strcmp("127.0.0.1", ip) != 0)
			{
				adinfo->ip = ip;
				break;
			}

			IFLOG(IFLL_DEBUG, "ip list:%s %d %d %s\r\n", ip, req->ifr_flags, AF_INET, req->ifr_name)
				req++;
		}
		closesocket(sockfd);
		return true;
#endif
	}

	addrinfo* adi;
	addrinfo hadi;
	ZeroMemory(&hadi, sizeof(hadi));
	hadi.ai_family = AF_INET;
	hadi.ai_flags = AI_CANONNAME;


	int r = getaddrinfo(address.c_str(), NULL, &hadi, &adi);
	if (r != 0)
	{
		IFLOG(IFLL_ERROR, "getaddrinfo error:%s\r\n", gai_strerror(r));
		return false;
	}
	const struct sockaddr* sa = adi->ai_addr;
	char ip[128] = { 0 };
	if (sa->sa_family == AF_INET)
	{
		inet_ntop(AF_INET, &(((struct sockaddr_in*)sa)->sin_addr), ip, sizeof(ip));

		sockaddr_in* ipv4 = (sockaddr_in*)sa;
		ipv4->sin_port = htons(nPort);
	}
	else if (sa->sa_family == AF_INET6)
	{
		inet_ntop(AF_INET6, &(((struct sockaddr_in6*)sa)->sin6_addr), ip, sizeof(ip));

		sockaddr_in6* ipv6 = (sockaddr_in6*)sa;
		ipv6->sin6_port = htons(nPort);
	}
	else
		return false;
	adinfo->ip = ip;
	adinfo->port = nPort;
	adinfo->family = adi->ai_family;
	memcpy(adinfo->addr, sa, adi->ai_addrlen);
	adinfo->addrlen = adi->ai_addrlen;
	freeaddrinfo(adi);
	return true;
}

void IFNetCore::addRSAPrivateKey(const IFString& sPublickKeyMD5, IFRefPtr<IFMemStream> spPirvateKey)
{
	m_RSAPrivateKeyList[sPublickKeyMD5] = spPirvateKey;
}

IFRefPtr<IFMemStream> IFNetCore::getRSAPrivateKey(const IFString& sPublicKeyMD5)
{
	auto it = m_RSAPrivateKeyList.find(sPublicKeyMD5);
	if (it != m_RSAPrivateKeyList.end())
		return it->second;
	return NULL;
}

bool IFNetCore::loadSSLCA()
{
	return loadSSLCA("../ca/ca-cert.pem", "../ca/ca-key.pem");
	//#ifndef DONT_USE_SSL
	//	if (!SSL_CTX_load_verify_locations(GetSSLContext(), "../ca/ca-cert.pem", NULL))
	//	{
	//		IFLOG(IFLL_ERROR, "SSL_CTX_load_verify_locations error!\r\n");
	//		return false;
	//	}
	//	if (SSL_CTX_use_certificate_file(GetSSLContext(), "../ca/ca-cert.pem", SSL_FILETYPE_PEM) <= 0)
	//	{
	//		IFLOG(IFLL_ERROR, "SSL_CTX_use_certificate_file error!\r\n");
	//
	//		return false;
	//	}
	//
	//	if (SSL_CTX_use_PrivateKey_file(GetSSLContext(), "../ca/ca-key.pem", SSL_FILETYPE_PEM) <= 0)
	//	{
	//		IFLOG(IFLL_ERROR, "SSL_CTX_use_PrivateKey_file error!\r\n");
	//
	//		return false;
	//	}
	//	if (!SSL_CTX_check_private_key(GetSSLContext()))
	//	{
	//		IFLOG(IFLL_ERROR, "SSL_CTX_check_private_key error!\r\n");
	//
	//		return false;
	//	}
	//
	//	return true;
	//#else
	//	IFLOG(IFLL_ERROR, "OPENSSL NOT ENABLE!\r\n");
	//	return false;
	//#endif
	//

}



bool IFNetCore::loadSSLCA(const IFString& pem, const IFString& key)
{
#ifndef DONT_USE_SSL
	if (!SSL_CTX_load_verify_locations(GetSSLContext(), pem.c_str(), NULL))
	{
		IFLOG(IFLL_ERROR, "SSL_CTX_load_verify_locations error!\r\n");
		return false;
	}
	if (SSL_CTX_use_certificate_file(GetSSLContext(), pem.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		IFLOG(IFLL_ERROR, "SSL_CTX_use_certificate_file error!\r\n");

		return false;
	}

	if (SSL_CTX_use_PrivateKey_file(GetSSLContext(), key.c_str(), SSL_FILETYPE_PEM) <= 0)
	{
		IFLOG(IFLL_ERROR, "SSL_CTX_use_PrivateKey_file error!\r\n");

		return false;
	}
	if (!SSL_CTX_check_private_key(GetSSLContext()))
	{
		IFLOG(IFLL_ERROR, "SSL_CTX_check_private_key error!\r\n");

		return false;
	}

	return true;
#else
	IFLOG(IFLL_ERROR, "OPENSSL NOT ENABLE!\r\n");
	return false;
#endif
}

void IFNetCore::processGetHostIP(IFThread* pThread)
{
	while (!pThread->isNeedExit())
	{
		if (!m_GetHostIPQueue.size())
		{
			IFThread::sleep(1);
			continue;
		}
		m_GetHostIPQueueLock.lock();

		auto pr = m_GetHostIPQueue.front();
		m_GetHostIPQueue.pop_front();
		m_GetHostIPQueueLock.unlock();


		//hostent* pHostent = gethostbyname(pr.first.c_str());

		IFCSLockHelper lh(m_GetHostIPResultQueueLock);
		AddressInfo adinfo;
		if (getHostAddress(pr.first.add, pr.first.port, &adinfo))
		{
			//in_addr addr = *((in_addr*)pHostent->h_addr_list[0]);

			m_GetHostIPResultQueue.push_back(makeIFPair(pr.second, makeIFPair(true, adinfo)));
		}
		else
		{
			m_GetHostIPResultQueue.push_back(makeIFPair(pr.second, makeIFPair(false, adinfo)));
		}

	}
}



void IFNetCore::processPackSerialize(IFThread* pThread, int nThreadIDX)
{
	IFLOG(IFLL_TRACE, "serailze thread start!\r\n");
	while (!pThread->isNeedExit())
	{
		IFLOG(IFLL_TRACE, "serailze thread wait!\r\n");

		pThread->getSyncObj()->wait(10000000);
		IFLOG(IFLL_TRACE, "serailze thread wait ok!\r\n");

		auto& queue = m_SerializeQueue[nThreadIDX];
		IFAsyncSendMsgInfo msg;
		while (queue.pop(msg))
		{
			IFLOG(IFLL_TRACE, "send serialize msg!\r\n");
			auto package = msg.spMsg->toPackage(msg.bCompress, msg.pFun);
			IFLOG(IFLL_TRACE, "send serialize msg!\r\n");
			msg.spConnection->sendPack(package, msg.nSendID);
			IFLOG(IFLL_TRACE, "send msg ok!\r\n");
		}
	}
}

IFNetCore::UDPSocketInfo::UDPSocketInfo()
	:IFRefObj(true)
{

}

IFNetCore::UDPSocketInfo::~UDPSocketInfo()
{
	if (m_Socket != INVALID_SOCKET)
		closesocket(m_Socket);

}
