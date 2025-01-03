﻿/*
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
#ifndef __IF_NET_CORE_H__
#define __IF_NET_CORE_H__
#include "IFCommonLib_API.h"
#include "IFEventSlot.h"
#include "IFNetConnection.h"
#include "IFNetIOData.h"
#include "IFThread.h"
#include "IFQueue.h"
#include "IFHashMap.h"
#ifdef IFPLATFORM_WINDOWS_SHOP
//struct in_addr
//{
//	int data[2];
//};
#endif

class IFNet_Message;
class IFTimer;
#ifndef IFTHREAD_NOT_ENABLE
class IFThreadSyncObj;
#endif

class IFNetMsgFactory;

struct IFNetCoreEvent : public IFMemObj
{

	enum EventType
	{
		ET_NEW_CONNECTION,
		ET_CONNECT,
		ET_DISCONNECT,
		ET_DATA_RECV,
		ET_PACK_RECV,
		ET_DATA_SENDED,
		ET_ERROR_PACK,
		ET_SSL_ESTABLISH_RESULT,

	};
	const char* getEventTypeName()
	{
		const char* p[] = { 
			"NEW_CONNECTION",
			"CONNECT",
			"DISCONNECT",
			"DATA_RECV",
			"PACK_RECV",
			"DATA_SENDED",
			"ERROR_PACK",
			"SSL_ESTABLISH_RESULT", };
		return p[eventType] ;
	}

	IFNetCoreEvent(IFNetConnection* pConnection,IFNet_Message* pMsg)
		:spConnection(pConnection),eventType(ET_PACK_RECV),pPack(pMsg)
	{

	}
	IFNetCoreEvent(IFNetConnection* pConnection, IFSimpleArray<char>* data)
		:spConnection(pConnection),eventType(ET_DATA_RECV),pData(data)
	{

	}

	IFNetCoreEvent(IFNetConnection* pConnection, EventType et, bool b)
		:spConnection(pConnection),eventType(et),bConnectSuccess(b)
	{

	}
	IFNetCoreEvent(IFNetConnection* pConnection, EventType et, IFUI64 sendid, bool b)
		:spConnection(pConnection), eventType(et), bSendSuccess(b), nSendID(sendid)
	{

	}
	IFNetCoreEvent(IFNetConnection* pConnection, EventType et)
		:spConnection(pConnection),eventType(et)
	{

	}

	void fireEvent(IFNetCore* pNetCore);

	IFNetConnectionPtr spConnection;
	int eventType;

	union 
	{
		bool bConnectSuccess;
		bool bSendSuccess;
	};
	
	IFUI64 nSendID;

	IFString sAddress;
	int nPort;
	
	//recv ET_PACK_RECV
	IFRefPtr<IFNet_Message> pPack;
	//ET_DATA_RECV
	IFSimpleArray<char>* pData;

	
};


class IFCOMMON_API IFNetCore : public IFRefObj
{
public:
	IF_DECLARERTTI;

	struct AddressInfo
	{
		int family;
		char addr[64];
		int addrlen;
		IFString ip;
		int port;		
	};

	class IFCOMMON_API UDPData : public IFRefObj
	{
		IF_DECLARERTTI;
	public:
		int nLocalPort;
		AddressInfo removeAddress;

		IFSimpleArray<char> dataBuf;
	};



	typedef IFRefPtr<IFFunctor<void(const AddressInfo* info)>> AsyncGetAddressCallbackPtr;

public:
	IFEventSlot<void(IFNetCore* pCore, IFNetConnection* pConnection)> event_NewConnection;

	

	IFEventSlot<void(IFNetCore* pCore, UDPData* pData)> event_RecvUDPData;

	IFEventSlot<void(IFNetCore* pCore, int threadid)> event_OnRecvThreadStart;
	IFEventSlot<void(IFNetCore* pCore, int threadid)> event_OnRecvThreadEnd;
	IFEventSlot<void(IFNetCore* pCore, int threadid, IFNetConnection* pCon, const void* pData, int len)> event_OnRecvThreadRecv;
	//IFEventSlot<void(IFNetCore* pCore, const IFString& sAddress, IFI32 nPort, IFNetConnection* pConnection)> event_ConnectResult;
public:
	IFNetCore(void);

	static IFRefPtr<IFNetCore> createNetCore();
	
	

	bool startService(int nPort, int nMaxConnection, bool bPackagemode = true , bool bSyncEvent = true);
	bool process();
	bool waitAndProcess(int ms);

	bool stopService();

	virtual bool startListen(int nPort, bool enableSSL, bool packagemode=false) { return false; };
	virtual bool stopListen(int nPort) { return false; }
	virtual bool isListening(int nPort) { return false; }

	virtual IFNetConnectionPtr createConnection(const IFString& sAddress, int nPort, bool syncconnect = true, bool bPackagemode = true, bool bSyncEvent = true) = 0;

	bool openUDPPort(int nPort);
	bool closeUDPPort(int nPort);
	bool sendUDPData(const IFString& address, int nRemotePort, int nLocalPort, const char* pData, int nLen);
	bool sendUDPDataByAddr(const AddressInfo* address, int nLocalPort, const char* pData, int nLen);

	IFUI64 getSendPackCount();
	IFUI64 getRecvPackCount();

	IFUI64 getSendByteCount();
	IFUI64 getRecvByteCount();

	//bool isSyncEvent()
	//{
	//	return m_bSyncEvent;
	//}

	inline IFNetMsgFactory* getMsgFactory()
	{
		return m_spMsgFactory;
	}
	IFTimer* getTimer();

	int getConnectionCount();
	int getMaxConnectionCount();

	void enumConnections(IFRefPtr<IFFunctor<bool(IFNetConnection*)>> spEnumFun);

	void getHostAddressAsync(const IFString& address, int nPort, AsyncGetAddressCallbackPtr spResult);
	bool getHostAddress(const IFString& address, int nPort, AddressInfo* info);


	void addRSAPrivateKey(const IFString& sPublickKeyMD5, IFRefPtr<IFMemStream> spPirvateKey);
	IFRefPtr<IFMemStream> getRSAPrivateKey(const IFString& sPublicKeyMD5);


	bool loadSSLCA();
	bool loadSSLCA(const IFString& pem, const IFString& key);

	IFString convertNetAddr2Str(const char* s, int len);

protected:
	~IFNetCore(void);
	void fireNewConnectionEvent(IFNetConnection* pConnection);


	void pushEvent(IFNetCoreEvent* e);
protected:

	virtual bool onServiceStart() = 0;
	virtual bool onServiceStop() = 0;

	void addAutoKeepAliveConnection(IFNetConnection* pConnection);
	void removeAutoKeepAliveConnection(IFNetConnection* pConnection);
#ifndef IFPLATFORM_FREE_RTOS

	void sendAsyncMsg(const IFAsyncSendMsgInfo& info);
#endif
protected:
	int m_nMaxConnectionCount;
	int m_nPort;

	bool m_bSyncEvent;
	bool m_bPackageMode;
	bool m_bServiceStarted;

	IFUI64 m_nSendPackage;
	IFUI64 m_nRecvPackage;

	IFUI64 m_nSendBytes;
	IFUI64 m_nRecvBytes;


	IFCSLock m_ConnectionListLock;
	typedef IFSet<IFNetConnectionPtr> ConnectionList;
	ConnectionList m_Connections;

	typedef IFQueue<IFNetCoreEvent*> EventList;
	EventList m_EventList;
	//IFCSLock m_EventListLock;
	IFRefPtr<IFNetMsgFactory> m_spMsgFactory;

	typedef IFSet<IFNetConnectionPtr> ConnectionWeakList;
	ConnectionWeakList m_AutoKeepAliveList;

	friend class IFNetConnection;
	friend struct IFNetCoreEvent;

	class UDPSocketInfo : public IFRefObj
	{
	public:
		UDPSocketInfo();

		SOCKET m_Socket;
		int m_nLocalPort;
		
		IFCSLock m_DataLock;
		IFArray<IFRefPtr<UDPData>> m_Datas; 
	
	protected:
		~UDPSocketInfo();
	};
#ifndef IFTHREAD_NOT_ENABLE
	IFMap<int, IFPair<IFRefPtr<IFThread>,IFRefPtr<UDPSocketInfo>>> m_OpendUDPProcThreadList;
#endif
	void processUDPReceive( UDPSocketInfo* pSocket);

#ifndef IFTHREAD_NOT_ENABLE
	IFRefPtr<IFThread> m_spGetHostIPThread;
	void processGetHostIP(IFThread* pThread);

#endif


	struct GetAddressRequestInfo
	{
		IFString add;
		int port;
	};

	IFList<IFPair<GetAddressRequestInfo, AsyncGetAddressCallbackPtr>> m_GetHostIPQueue;
	IFCSLock m_GetHostIPQueueLock;

	IFList<IFPair<AsyncGetAddressCallbackPtr,IFPair<bool,AddressInfo>>> m_GetHostIPResultQueue;
	IFCSLock m_GetHostIPResultQueueLock;

#if !defined(IFPLATFORM_FREE_RTOS) && !defined(IFPLATFORM_WEB)

	IFArray<IFRefPtr<IFThread>> m_PackSerializeThreads;
	void processPackSerialize(IFThread* pThread, int nThreadIDX);
	IFArray<IFQueue<IFAsyncSendMsgInfo>> m_SerializeQueue;
	
	IFMap<IFString, IFRefPtr<IFMemStream>> m_RSAPrivateKeyList;
#endif
	IFRefPtr<IFTimer> m_spTimer;
#ifndef IFTHREAD_NOT_ENABLE
	IFRefPtr<IFThreadSyncObj> m_spEventSyncObj;
#endif
};

#endif //__IF_NET_CORE_H__