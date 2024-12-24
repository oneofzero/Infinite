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
#ifndef __IF_NET_CORE_EPOLL_H__
#define __IF_NET_CORE_EPOLL_H__
#include "IFNetCore.h"

class IFNetCoreEPOLL : public IFNetCore
{
public:
	IFNetCoreEPOLL();

protected:
	~IFNetCoreEPOLL();

	virtual IFNetConnectionPtr createConnection(const IFString& sAddress, int nPort,  bool syncconnect = true, bool bPackagemode = true, bool bSyncEvent = true);

	virtual bool startListen(int nPort, bool enableSSL = false, bool packagemode = false);
	virtual bool stopListen(int nPort);
	virtual bool isListening(int nPort) override;

	virtual bool onServiceStart();
	virtual bool onServiceStop();

private:

    IFRefPtr<IFThread> m_spWorkThread;
	void workThread();
	
	bool m_bExitWorkThread;
	int m_nEpoll;
	struct ListenSocketInfo
	{
		int nPort;
		bool bEnableSSL;
		bool bPackagemode;
	};
	IFHashMap<SOCKET, ListenSocketInfo> m_ListenScoket;
	friend class IFNetEPOLLConnection;

	ConnectionList m_ConnectingList;
	IFCSLock m_ConnectingLock;
};

class IFNetEPOLLConnection : public IFNetConnection
{
public:
	IFNetEPOLLConnection(IFNetCoreEPOLL* pEPOLLCore,bool bPackagemode, bool bSyncEvent, SOCKET sk=INVALID_SOCKET);
	virtual IFUI64 sendPack(IFRefPtr<IFMemStream> pStream, IFUI64 nSendID = 0);
	virtual void disconnect();
	virtual bool isShouldRemove();
	virtual void setNoBlock();
	virtual void setNoDelay();
	virtual SOCKET getSocket()
	{
		return m_Socket;
	}
protected:
	~IFNetEPOLLConnection();
	SOCKET m_Socket;
	IFNetCoreEPOLL* m_pEPOLLCore;

	class PackInfo : IFMemObj
	{
	public:
		IFRefPtr<IFMemStream> spStream;		
		int nRequestSended;
		IFUI64 nSendID;
	};
	IFList<PackInfo> m_SendPacks;
	IFCSLock m_SendPacksLock;
	bool sendPendingPacks();

	friend class IFNetCoreEPOLL;
};

#endif //__IF_NET_CORE_EPOLL_H__