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
#ifndef __IF_NET_CORE_SELECT_H__
#define __IF_NET_CORE_SELECT_H__
#include "IFCommonLib_API.h"
#include "IFNetCore.h"

class IFCOMMON_API IFNetCoreSelect : public IFNetCore
{
public:

	IFNetCoreSelect();
	virtual IFNetConnectionPtr createConnection(const IFString& sAddress, int nPort, bool syncconnect = true, bool bPackagemode = true, bool bSyncEvent = true);

	virtual bool startListen(int nPort, bool enableSSL = false, bool packagemode = false) override;
	virtual bool stopListen(int nPort) override;
protected:
	~IFNetCoreSelect();


	virtual bool onServiceStart();
	virtual bool onServiceStop();

	void workThread();
	IFRefPtr<IFThread> m_spWorkThread;

	bool m_bExitWorkThread;

	struct SocketInfo
	{
		SOCKET socket;
		int localPort;
		bool usePackageMode;
		bool useSSL;

	};
	IFHashMap<int, SocketInfo> m_Listener;


	ConnectionList m_ConnectingList;
	IFCSLock m_ConnectingListLock;
	IFCSLock m_ListenersLock;

	friend class IFNetCoreSelectConnection;
};

class IFNetCoreSelectConnection : public IFNetConnection
{
public:
	virtual IFUI64 sendPack(IFRefPtr<IFMemStream> pStream, IFUI64 nSendID);
	virtual void disconnect();
	virtual bool isShouldRemove();

	virtual void setNoBlock();
	virtual void setNoDelay();
	virtual SOCKET getSocket()
	{
		return m_Socket;
	}
private:

	IFNetCoreSelectConnection(IFNetCoreSelect* pCore,bool bPackagemode, bool bSyncEvent, SOCKET sock = INVALID_SOCKET);

	~IFNetCoreSelectConnection();
	SOCKET m_Socket;

	IFNetCoreSelect* m_pSelectCore;

	IFCSLock m_lock;

	friend class IFNetCoreSelect;
};

#endif //__IF_NET_CORE_SELECT_H__