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
#ifndef __IF_NET_CONNECTION_H__
#define __IF_NET_CONNECTION_H__
#ifdef WIN32
#include <WinSock2.h>
#if defined(WINAPI_FAMILY) && WINAPI_PARTITION_APP == WINAPI_FAMILY
typedef int SOCKET;
#define NO_WINSOCK_API TRUE
#endif
#else
#ifndef IFPLATFORM_EMBED_NOSYS
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define INVALID_SOCKET  0
#define closesocket close
#endif

#endif
#include "IFRefObj.h"
#include "IFNetGlobalDefine.h"
#include "IFList.h"
#include "IFEventSlot.h"
#include "IFString.h"
#include "IFMap.h"
#include "IFFIFOStream.h"
#include "IFMemStream.h"

#ifndef IFPLATFORM_FREE_RTOS
class IFNet_Message;
class IFNet_Message_EstablishEncryption_Res;
#endif

struct IFNetIOData;
class IFNetCore;
class IFAES;

enum IFNetConnectionState
{
	IFNCS_UNKNOWN,
	IFNCS_NOTCONNECTED,
	IFNCS_CONNECTED,
	IFNCS_DISCONNECTTING,
	IFNCS_DISCONNECTED,
};

typedef IFFunctor<IFRefPtr<IFMemStream>(IFStream*)> IFNetEncryptFunctor;
#pragma pack(push)
#pragma pack(8)
class IFCOMMON_API IFNetRecvPackage : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum {PACKAGE_HEAD_SIZE=sizeof(IFUI32)};

	IFNetRecvPackage()
		:IFRefObj(true)
		,m_nCurBufSize(0)
		,m_nCurPackLenBufSize(0)
	{

	}

	bool isFull()
	{
		return m_nCurPackLenBufSize == PACKAGE_HEAD_SIZE && m_nCurBufSize == m_Buf.size();
	}


	IFUI32 getRemainSize()
	{
		return m_Buf.size() - m_nCurBufSize;
	}



	int write(const void* p, IFUI32 nSize);
	
	union 
	{
		char m_LenBuf[PACKAGE_HEAD_SIZE];
		IFUI32 m_PackLen;
	};

	const void* getBufferPtr()
	{
		return m_Buf;
	}
	int getBufferSize()
	{
		return m_Buf.size();
	}

	IFSimpleArray<char> m_Buf;
	int m_nCurBufSize;
	int m_nCurPackLenBufSize;

private:
	virtual ~IFNetRecvPackage();
	IFNetRecvPackage(const IFNetRecvPackage& o)
	{

	}
};

class IFCOMMON_API IFNetDepacker : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFNetDepacker();


	bool write(const void* pData, int nLen);

	bool hasPackage();

	IFRefPtr<IFNetRecvPackage> getPackage();

private:
	typedef IFList<IFRefPtr<IFNetRecvPackage>> Packages;
	Packages m_Packages;
	//IFRefPtr<IFMemStream> m_spSizeBuf;
	IFCSLock m_Lock;
	int m_nPackLenBufSize;
	union 
	{
		IFUI32 m_PackLen;
		char m_PackLenBuf[4];

	};
};

class IFCOMMON_API IFNetConnection : public IFRefObj
{
private:
	IF_DECLARERTTI;
	

public:

	IFEventSlot<void(IFNetConnection* pConnection, bool bSuccess)> event_ConnectResult;

	IFEventSlot<void(IFNetConnection* pConnection)> event_Disconnect;
	//receive data thread fire in  work threads if netcore work in multithread;
	IFEventSlot<void(IFNetConnection* pConnection,const void* pBuf, IFUI32 nSize)> event_RecvData;
	//receive data thread fire in  work threads if netcore work in multithread;
	IFEventSlot<void(IFNetConnection* pConnection,const void* pBuf, IFUI32 nSize)> event_RecvUnKnownPackage;
	//receive package event only fire in main process thread
#ifndef IFPLATFORM_FREE_RTOS
	IFEventSlot<void(IFNetConnection* pConnection,IFNet_Message* pPackage)> event_RecvPackage;

	IFEventSlot<void(IFNetConnection* pConnection)> event_ErrorPack;
#endif

	IFEventSlot<void(IFNetConnection* pConnection, IFUI64 nSendID, bool bSendOK)> event_SendResult;

	IFEventSlot<void(IFNetConnection* pConnection)> event_EncryptionEstablished;

	IFEventSlot<void(IFNetConnection* pConnection, bool)> event_SSLEstablishResult;
public:
	//包预处理函数 ，如果返回false 将不进行解包处理
	typedef IFRefPtr<IFFunctor<bool(IFMemStream*)>> PackProcessFunPtr;
public:


	virtual void disconnect() = 0;
	virtual IFUI64 sendPack(IFRefPtr<IFMemStream> pStream, IFUI64 nSendID = 0) = 0; //return sendid if 0  send failed! 
	virtual bool isShouldRemove() = 0;
	virtual SOCKET getSocket() = 0;

	void setPackageMode(bool b);
	bool isPackageMode();

	IFUI64 sendData(const void* pData, IFUI32 nLen);
	IFUI64 sendString(const char* sString);
#ifndef IFPLATFORM_FREE_RTOS
	IFUI64 sendMsg(IFNet_Message* pMsg, bool bCompress = false, IFNetEncryptFunctor* pFun = NULL);
#endif
	

	IFNetConnectionState getState() const;

	const IFString& getRemoteIP() const;
	int getRemotePort() const;

	bool isConnected() const
	{
		return getState() ==  IFNCS_CONNECTED;
	}

	IFNetCore* getNetCore();
	void setUserData(IFRefPtr<IFRefObj> spUserData);
	IFRefObj* getUserData()
	{
		return m_spUserData;
	}

	IFUI32 getSendPendingBytes()
	{
		return m_nSendPendingBytes;
	}

	void setMaxSendBufferSize(IFUI32 nMaxNum = 1024*512);
	IFUI32 getMaxSendBufferSize();

	void setLastCommunicateTime();
	IFUI32 getLastCommunicateTime();

	void setPackPreProcessFun(PackProcessFunPtr spFun);

	void setAutoKeepAliveTime(IFUI32 timeoutMS, IFRefPtr<IFMemStream> spKeepAlivePackage = NULL);
	IFUI32 getAutoKeepAliveTime();

	IFMemStream* getAutoKeepAlivePackage()
	{
		return m_spKeepAlivePackage;
	}

	//建立加密链接
	void establishEncryption(IFMemStream* pPublicKey);
	bool isEncryption();
	//void setEncryptionPrivateKey(IFMemStream* pRivateKey);

	//客户端建立sll 链接
	bool establishSSL(const IFString& hostname);
	//服务端等待客户端建立sll链接
	bool waitEstablishSSL();

	void setDecryptFun(IFNetEncryptFunctor* pDecryptFun);
	void setDefalutEncryptFun(IFNetEncryptFunctor* pEncryptFun);

	bool isSyncEvent()
	{
		return m_bSyncEvent;
	}

	int getLocalPort()
	{
		return m_nLocalPort;
	}

protected:
	IFNetConnection(IFNetCore* pNetCore, bool bSyncEvent, bool bPackageMode);

	~IFNetConnection(void);



	bool recvData(const char* pData, IFUI32 nLen);

	void fireRecvPackageEvent(IFNetRecvPackage* pPackage);
	
	void fireSendDoneEvent(IFUI64 nSendID, bool bOK);
#ifndef IFPLATFORM_FREE_RTOS

	static void ProcEstablishEncryptRes(IFNetConnection* pCon, IFNet_Message_EstablishEncryption_Res* pRes);
	void procEstablishEncryptRes(IFNet_Message_EstablishEncryption_Res* pRes);
#endif

	IFUI64 getSendID();

	int m_nLocalPort;
	int m_nRemotePort;
	IFString m_sRemoteIP;

	IFNetCore* m_pNetCore;

	//IFRefPtr<IFNetPack> m_spNotCompletePack;
	IFNetConnectionState m_eConnectionState;

	//IFNetIOData* m_pRecvDataBuf;

	//IFRefPtr<IFFIFOStream> m_FIFO;
#ifndef IFPLATFORM_FREE_RTOS

	IFRefPtr<IFNetDepacker> m_spDepacker;
	IFRefPtr<IFNetEncryptFunctor> m_spDecryptFun;
	IFRefPtr<IFNetEncryptFunctor> m_spDefaultEncryptFun;
	IFArray<IFNet_Message*> m_PackageList;
#endif
	IFRefPtr<IFRefObj> m_spUserData;
	
	IFUI32 m_LastCommunicateTime;
	PackProcessFunPtr m_spPackPreProcessFun;
	IFUI32 m_nSendPendingBytes;
	IFUI32 m_nMaxSendBufferSize;
	IFUI32 m_nAutoKeepAliveTimeOutMS;
	static IFUI64 m_nSendID;
	IFRefPtr<IFMemStream> m_spKeepAlivePackage;
	IFSimpleArray<IFUI8> m_EncryptTestData;
	IFRefPtr<IFAES> m_spAES;
	bool m_bEncryption;

	bool m_bPackageMode;
	bool m_bSyncEvent;
	IFUI16 m_nNotCompletePackLen;
	//openssl support;
	struct ssl_st* m_ssl;
	struct bio_st* m_rbio, *m_wbio;
	int ssl_state;
	bool ssl_decrpyted;
	enum
	{
		SSL_NONE,
		SSL_ESTABLISHING,
		SSL_ESTABLISED,
	};

	friend class IFNetCore;

};
#ifndef IFPLATFORM_FREE_RTOS

struct IFAsyncSendMsgInfo : public IFMemObj
{
	IFRefPtr<IFNetConnection> spConnection;
	IFRefPtr<IFNet_Message> spMsg;
	IFUI64 nSendID;
	bool bCompress;
	IFRefPtr<IFNetEncryptFunctor> pFun;
};
#endif
#pragma pack(pop)

typedef IFRefPtr<IFNetConnection> IFNetConnectionPtr;


#endif //__IF_NET_CONNECTION_H__