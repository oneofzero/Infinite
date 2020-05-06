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
#include "IFCommonLib_API.h"
#include "IFNetCore.h"
#include "WinSock2.h"
#include "mswsock.h"
#include "IFAtomicOperation.h"

class IFNet_Message;
class IFTimer;


class IFNetConnectionIOCP;

class IFNetCoreIOCP_IODATA : public IFMemObj
{
public:


	enum DATA_TYPE
	{
		DT_ACCEPT,
		DT_CONNECT,
		DT_RECV,
		DT_SEND,
		DT_NUM,
	};
public:
	IFNetCoreIOCP_IODATA(IFNetConnectionIOCP* pConnection, DATA_TYPE dt, void (*freeFun)(IFNetCoreIOCP_IODATA* pData))
		:m_Type(dt)
		,m_spConnection(pConnection)
		,m_nRefCount(0)
		,m_FreeFun(freeFun)
	{
		memset(&m_OverLapped,0, sizeof(m_OverLapped));
	}

	void addRef()
	{
		ATOMIC_INC_INT32(&m_nRefCount);
	}

	void decRef()
	{
		if (ATOMIC_DEC_INT32(&m_nRefCount) == 0)
		{
			m_FreeFun(this);
		}
	}

	WSAOVERLAPPED m_OverLapped;
	DATA_TYPE m_Type;
	int m_nRefCount;
	void (*m_FreeFun)(IFNetCoreIOCP_IODATA* pData);
	IFRefPtr<IFNetConnectionIOCP> m_spConnection;
protected:
	~IFNetCoreIOCP_IODATA()
	{

	}
};

struct IFNetCoreIOCP_IODATA_Accept : public IFNetCoreIOCP_IODATA
{
public:
	IFNetCoreIOCP_IODATA_Accept(IFNetConnectionIOCP* pConnection)
		:IFNetCoreIOCP_IODATA(pConnection,DT_ACCEPT,freeMe)
	{
	}

	static void freeMe(IFNetCoreIOCP_IODATA* pData)
	{
		delete (IFNetCoreIOCP_IODATA_Accept*)pData;
	}

	char m_buf[1024];
	SOCKET m_ListenSocket;
protected:
	~IFNetCoreIOCP_IODATA_Accept()
	{

	}
};
struct IFNetCoreIOCP_IODATA_Connect : public IFNetCoreIOCP_IODATA
{
public:
	IFNetCoreIOCP_IODATA_Connect(IFNetConnectionIOCP* pConnection)
		:IFNetCoreIOCP_IODATA(pConnection,DT_CONNECT,freeMe)
	{

	}
	static void freeMe(IFNetCoreIOCP_IODATA* pData)
	{
		delete (IFNetCoreIOCP_IODATA_Connect*)pData;
	}
	//sockaddr_in m_addr;
	IFNetCore::AddressInfo  m_addr;
protected:
	~IFNetCoreIOCP_IODATA_Connect()
	{
	}
};

struct IFNetCoreIOCP_IODATA_Recv : public IFNetCoreIOCP_IODATA
{
public:
	IFNetCoreIOCP_IODATA_Recv(IFNetConnectionIOCP* pConnection)
		:IFNetCoreIOCP_IODATA(pConnection,DT_RECV,freeMe)
	{
		m_Type = IFNetCoreIOCP_IODATA::DT_RECV;
		m_WSABuf.buf = m_buf;

	}
	static void freeMe(IFNetCoreIOCP_IODATA* pData)
	{
		delete (IFNetCoreIOCP_IODATA_Recv*)pData;
	}
	inline void reset()
	{
		m_WSABuf.len = sizeof(m_buf);
	}
	WSABUF m_WSABuf;
	char m_buf[32*1024];
protected:
	~IFNetCoreIOCP_IODATA_Recv()
	{
	}
};

struct IFNetCoreIOCP_IODATA_Send : public IFNetCoreIOCP_IODATA
{
public:
	IFNetCoreIOCP_IODATA_Send(IFNetConnectionIOCP* pConnection,IFMemStream* pPack)
		:IFNetCoreIOCP_IODATA(pConnection,DT_RECV, freeMe)
	{
		m_Type = IFNetCoreIOCP_IODATA::DT_SEND;
		m_spSendPackage = pPack;
		m_WSABuf.buf = (char*)pPack->getBuffer();
		m_WSABuf.len = (IFUI32)pPack->size();
	}

	static void freeMe(IFNetCoreIOCP_IODATA* pData)
	{
		delete (IFNetCoreIOCP_IODATA_Send*)pData;
	}
	IFUI64 m_nSendID;
	WSABUF m_WSABuf;

	IFRefPtr<IFMemStream> m_spSendPackage;
protected:
	~IFNetCoreIOCP_IODATA_Send()
	{

	}
};

class IFCOMMON_API IFNetCoreIOCP : public IFNetCore
{public:
	IFNetCoreIOCP(void);

	IFNetConnectionPtr createConnection(const IFString& sAddress, int nPort, bool syncconnect = true,  bool bPackagemode = true, bool bSyncEvent = true);
	virtual bool startListen(int nPort, bool enableSSL = false, bool packagemode = false);
	virtual bool stopListen(int nPort);


	IFRefPtr<IFStream> createSendPack();

	int getSendPackCount();
	int getRecvPackCount();

	int getSendByteCount();
	int getRecvByteCount();

	bool isMultiThread();


	IFNetIOData* AllocIOData(IFNetConnection* pConnection, IFNetIODataOperation op, bool bNeedLargeSize = false);
	void FreeIOData(IFNetIOData* pData);

	//inline void addSendPack(){m_nSendPackage++;}
	//inline void addRecvPack(){m_nRecvPackage++;};
	IFTimer* getTimer();

	int getConnectionCount();
	int getMaxConnectionCount();

	void enumConnections(IFRefPtr<IFFunctor<bool(IFNetConnection*)>> spEnumFun);

	//IFNetCoreIOCP_IODATA_Accept* createAcceptData(IFNetConnectionIOCP* pConnection);
	//IFNetCoreIOCP_IODATA_Connect* createConnectData(IFNetConnectionIOCP* pConnection);
	//IFNetCoreIOCP_IODATA_Send* createSendData(IFNetConnectionIOCP* pConnection, IFMemStream* pPack);

	//void freeIOData(IFNetCoreIOCP_IODATA* pData);

	HANDLE getCompletionPort()
	{
		return m_hCompletionPort;
	}
	LPFN_ACCEPTEX getAcceptEx()
	{
		return m_WSAAcceptEx;
	}

	LPFN_CONNECTEX getConnectEx()
	{
		return m_WSAConnectEx;
	}


	void addConnectedConnection(IFNetConnectionIOCP* pConnection);
	void removeConnectedConnection(IFNetConnectionIOCP* pConnection);

	void addAcceptConnection(IFNetConnectionIOCP* pConnection);
	void removeAcceptConnection(IFNetConnectionIOCP* pConnection);

	static bool convertSockAddress(sockaddr_in* pOut, const IFString& sAddress, int nPort);
protected:
	~IFNetCoreIOCP(void);

	virtual bool onServiceStart();
	virtual bool onServiceStop();
private:
	bool loadExtensions();

	IFArray<IFRefPtr<IFThread>> m_ProcessThreads;
	void processThread();
	bool m_bProcessThreadExit;

	void removeConnection(IFNetConnection* pConnection );

	void pushMessage(IFNetConnection* pConnect, IFNet_Message* pMsg);

	//bool acceptNewConnection();

	void processAccept(IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess);
	void processConnect(IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess);
	void processRecvData(IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess);
	void processSendData(IFNetCoreIOCP_IODATA* pData, DWORD dwTransBytes, bool bSuccess);
private:
	HANDLE m_hCompletionPort;
	typedef IFMap<int, IFPair<SOCKET, SOCKET>> LISTEN_SOCK_LIST;
	//SOCKET m_Listener;
	//SOCKET m_Listener6;
	LISTEN_SOCK_LIST m_ListenerList;
	bool m_bStarted;
	bool m_bProcessConnectDone;

	int m_nThreadCount;
	int m_nExitThreadCount;
	int m_nProcessThreadCount;

	//IFCSLock m_ConnectionListLock;
	//typedef IFRBTree<IFNetConnectionPtr> ConnectionList;
	//ConnectionList m_Connections;
	IFCSLock m_AcceptConnectionsLock;
	ConnectionList m_AcceptConnections;

	//IFMemPool<IFNetIOData> m_IODataPool;
	//IFMemPool<IFNetLargeIOData> m_LargeIODataPool;

	IFCSLock m_NewConnectionsListLock;
	ConnectionList m_NewConnectionsList;

	typedef IFList<IFPair<IFString,IFI32> > ConnectAddressList;
	IFCSLock m_ConnectQueueLock;
	ConnectAddressList m_ConnectQueue;


	struct ConnectInfo
	{
	public:
		ConnectInfo(const IFString& _sadd, IFI32 _port, IFNetConnection* pCon ):
			sAddress(_sadd),nPort(_port), spConnection(pCon)
		{

		}
		IFString sAddress;
		IFI32 nPort;
		IFNetConnectionPtr spConnection;
	};

	typedef IFList<ConnectInfo> ConnectedEventList;
	IFCSLock m_ConnectedEventListLock;
	ConnectedEventList m_ConnectedEventList;

	IFRefPtr<IFTimer> m_spTimer;

	typedef IFList<IFPair<IFNetConnectionPtr, IFNet_Message*>> MessageList;
	MessageList m_MessageList;
	IFCSLock m_MessageListLock;

	IFRefPtr<IFNetConnectionIOCP> m_spListenConnection;
	friend class IFNetConnection;

	typedef void (IFNetCoreIOCP::*IODATAPROCESSFUN)(IFNetCoreIOCP_IODATA*, DWORD, bool);
	IODATAPROCESSFUN m_IODataProcessFunList[IFNetCoreIOCP_IODATA::DT_NUM];

	LPFN_ACCEPTEX m_WSAAcceptEx;
	LPFN_CONNECTEX m_WSAConnectEx;
	LPFN_GETACCEPTEXSOCKADDRS m_WSAGetAcceptExSockAddrs;

	friend class IFNetConnectionIOCP;
};

class IFNetConnectionIOCP : public IFNetConnection
{
public:

	IFNetConnectionIOCP(IFNetCoreIOCP* pNetCore,bool bPackagemode, bool bSyncEvent);
	virtual IFUI64 sendPack(IFRefPtr<IFMemStream> pStream, IFUI64 nSendID );
	virtual void disconnect();
	virtual bool connect(const IFString& sAddress, int nPort);

	virtual bool isShouldRemove();

	virtual bool requestAccept(bool ipv6support, SOCKET listensocket);
	virtual bool requestConnect(const IFString& sAddress, int nPort);
	virtual bool requestRecvData();

	void setNoDelay();

	SOCKET getSocket()
	{
		return m_Socket;
	}

protected:

	~IFNetConnectionIOCP();

	virtual bool onConnect(const IFNetCore::AddressInfo*  addressinfo);

	SOCKET m_Socket;
	IFNetCoreIOCP* m_pIOCPCore;
	IFNetCoreIOCP_IODATA_Recv* m_pRecvData;
	bool m_bIPV6;
	friend class IFNetCoreIOCP;
	IFCSLock m_Lock;
};