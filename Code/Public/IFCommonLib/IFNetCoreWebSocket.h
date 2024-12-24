#pragma once
#include "IFNetCore.h"
class IFCOMMON_API IFNetCoreWebSocket : public IFNetCore
{
public:

	IFNetCoreWebSocket();
	virtual IFNetConnectionPtr createConnection(const IFString& sAddress, int nPort, bool syncconnect = true, bool bPackagemode = true, bool bSyncEvent = true);

	virtual bool startListen(int nPort, bool enableSSL = false, bool packagemode = false) override;
	virtual bool stopListen(int nPort) override;

	//void pushEvent(IFNetCoreEvent* e);
protected:
	~IFNetCoreWebSocket();


	virtual bool onServiceStart();
	virtual bool onServiceStop();


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

	friend class IFNetCoreWebSocketConnection;
};