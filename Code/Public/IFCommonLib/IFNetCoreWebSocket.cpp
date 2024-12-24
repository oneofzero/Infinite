#include <stdlib.h>
#include <emscripten/websocket.h>
#include "IFNetCoreWebSocket.h"
#include "IFLogSystem.h"


class IFNetCoreWebSocketConnection : public IFNetConnection
{
public:
	IFString m_URL;
	virtual IFUI64 sendPack(IFRefPtr<IFMemStream> pStream, IFUI64 nSendID)
	{
		emscripten_websocket_send_binary(m_Socket, (void*)pStream->getBuffer(), (IFUI32)pStream->size());
		IFLogDebug("websocket send data len:%d\r\n", (IFUI32)pStream->size());
		return 1;
	}
	//virtual void disconnect();
	virtual bool isShouldRemove()
	{
		return false;
	}

	virtual void setNoBlock()
	{

	}
	virtual void setNoDelay()
	{

	}
	virtual SOCKET getSocket()
	{
		return m_Socket;
	}

	

	IFNetCoreWebSocketConnection(IFNetCoreWebSocket* pCore, bool bSyncEvent, bool bPackageMode, EMSCRIPTEN_WEBSOCKET_T socket, const IFString& url)
		:IFNetConnection(pCore, bSyncEvent, bPackageMode)
		,m_URL(url)
	{
		m_pWebSocketCore = pCore;
		m_Socket = socket;
		m_eConnectionState = IFNCS_NOTCONNECTED;

		emscripten_websocket_set_onopen_callback(m_Socket, this, [](int eventType, const EmscriptenWebSocketOpenEvent* websocketEvent, void* userData)
			{
				auto* pThis = (IFNetCoreWebSocketConnection*)userData;
				return pThis->onWebSocketOpen(eventType, websocketEvent);
			});

		emscripten_websocket_set_onerror_callback(m_Socket, this, [](int eventType, const EmscriptenWebSocketErrorEvent* websocketEvent, void* userData)
			{
				auto* pThis = (IFNetCoreWebSocketConnection*)userData;
				return pThis->onWebSocketError(eventType, websocketEvent);
			});
		emscripten_websocket_set_onclose_callback(m_Socket, this, [](int eventType, const EmscriptenWebSocketCloseEvent* websocketEvent, void* userData)
			{
				auto* pThis = (IFNetCoreWebSocketConnection*)userData;
				return pThis->onWebSocketClose(eventType, websocketEvent);
			});
		emscripten_websocket_set_onmessage_callback(m_Socket, this, [](int eventType, const EmscriptenWebSocketMessageEvent* websocketEvent, void* userData)
			{
				auto* pThis = (IFNetCoreWebSocketConnection*)userData;
				return pThis->onWebSocketMsg(eventType, websocketEvent);
			});

	}

	~IFNetCoreWebSocketConnection()
	{

	}


	EM_BOOL onWebSocketOpen(int eventType, const EmscriptenWebSocketOpenEvent* websocketEvent)
	{
		m_eConnectionState = IFNCS_CONNECTED;
		IFLogDebug("websocket onWebSocketOpen!%s\r\n", m_URL.c_str());
		IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_CONNECT, true);
		m_pWebSocketCore->pushEvent(pEvent);

		//event_ConnectResult(this, true);
		return true;
	}
	
	EM_BOOL onWebSocketError(int eventType, const EmscriptenWebSocketErrorEvent* websocketEvent )
	{
		IFLogDebug("websocket onWebSocketError!%s\r\n", m_URL.c_str());

		if (m_eConnectionState == IFNCS_NOTCONNECTED)
		{
			//event_ConnectResult(this, false);
			IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_CONNECT, false);
			m_pWebSocketCore->pushEvent(pEvent);

		}
		else
		{
			disconnect();
		}

		return true;
	}

	EM_BOOL onWebSocketMsg(int eventType, const EmscriptenWebSocketMessageEvent* websocketEvent)
	{
		IFLogDebug("websocket onWebSocketMsg!%s datalen:%d\r\n", m_URL.c_str(), websocketEvent->numBytes);
		setLastCommunicateTime();
		recvData((char*)websocketEvent->data, websocketEvent->numBytes);
		return true;
	}

	EM_BOOL onWebSocketClose(int eventType, const EmscriptenWebSocketCloseEvent* websocketEvent)
	{
		IFLogDebug("websocket onWebSocketClose!%s\r\n", m_URL.c_str());

		disconnect();
		return true;
	}

	void disconnect() override
	{
		if (m_Socket >0)
		{
			//FD_CLR(m_Socket, &m_pSelectCore->m_Readfds);
			//closesocket(m_Socket);
			
			if (m_eConnectionState == IFNCS_CONNECTED)
			{
				//shutdown(m_Socket,2);
				emscripten_websocket_close(m_Socket,2, "none");

				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_DISCONNECT);
				m_pWebSocketCore->pushEvent(pEvent);
				m_eConnectionState = IFNCS_DISCONNECTED;
				emscripten_websocket_delete(m_Socket);
				//closesocket(m_Socket);
			}

			m_Socket = 0;



		}
	}

	EMSCRIPTEN_WEBSOCKET_T m_Socket;
	IFNetCoreWebSocket* m_pWebSocketCore;


	friend class IFNetCoreWebSocket;
};



IFNetCoreWebSocket::IFNetCoreWebSocket()
{

}
//void IFNetCoreWebSocket::pushEvent(IFNetCoreEvent* e)
//{
//	IFNetCore::pushEvent(e);
//}


bool IFNetCoreWebSocket::startListen(int nPort, bool enableSSL, bool packagemode )
{
	return false;
}
bool IFNetCoreWebSocket::stopListen(int nPort)
{
	return false;
}

bool IFNetCoreWebSocket::onServiceStart()
{
	return true;
}

bool IFNetCoreWebSocket::onServiceStop()
{
	return true;
}



IFNetConnectionPtr IFNetCoreWebSocket::createConnection(const IFString& sAddress, int nPort, bool syncconnect , bool bPackagemode , bool bSyncEvent)
{
	EmscriptenWebSocketCreateAttributes param;
	IFString url = sAddress;
	if (!sAddress.startsWith("ws://") ||
		!sAddress.startsWith("wss://"))
	{
		url = "ws://" + sAddress;
	}

	if (nPort)
	{
		url += IFString().format(":%d", nPort);
	}

	param.url = url.c_str();
	param.protocols = "binary";
	param.createOnMainThread = true;
	auto socket = emscripten_websocket_new(&param);
	if (socket <= 0)
	{
		IFLogError("can't create websocket error:%d\r\n", socket);
		return NULL;
	}
	IFLogDebug("websocket connect:%s\r\n", url.c_str());
	return NewIFRefObj<IFNetCoreWebSocketConnection>(this, bSyncEvent, bPackagemode, socket, url);
}


IFNetCoreWebSocket::~IFNetCoreWebSocket()
{

}

IFRefPtr<IFNetCore> IFNetCore::createNetCore()
{
	return NewIFRefObj<IFNetCoreWebSocket>();
}