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
#include "IFNetConnection.h"
#include "IFNetGlobalDefine.h"

enum IFNetIODataOperation
{
	IFNIO_RECV,
	IFNIO_SEND,
	IFNIO_CONNECT,
	IFNIO_ACCEPT,
};
#define IFNET_IODATA_SIZE 1*1024
#define IFNET_LARGE_IODATA_SIZE 60*1024

struct IFCOMMON_API IFNetIOData : public IFMemObj
{


#if defined(WIN32) && WINAPI_FAMILY!=WINAPI_PARTITION_APP
protected:
	IFNetIOData(IFNetConnectionPtr pConnection, IFNetIODataOperation op, int nTotalSize = IFNET_IODATA_SIZE )
		:m_spConnection(pConnection),m_nOperation(op),nBufTotalSize(nTotalSize)
	{

		wsaBuf.buf = szBuf;
		wsaBuf.len = nBufTotalSize;

		ZeroMemory( &m_OverLapped, sizeof m_OverLapped );

#if  WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		m_OverLapped.hEvent = WSACreateEvent();
#endif

	}
#else
	IFNetIOData()
		:nBufTotalSize(IFNET_IODATA_SIZE)
	{

	}
#endif
	~IFNetIOData(void)
	{

	}
public:

	inline char* getBuffer()
	{
		return szBuf;
	}

	inline IFNetConnection* getConnection()
	{
		return m_spConnection;
	}

	inline IFNetIODataOperation getOperation()
	{
		return m_nOperation;
	}
#if defined(WIN32) && WINAPI_FAMILY!=WINAPI_PARTITION_APP
	inline WSABUF* getWSABuf()
	{
		return &wsaBuf;
	}
#endif

	inline void reset()
	{
#if defined(WIN32) && WINAPI_FAMILY!=WINAPI_PARTITION_APP
		wsaBuf.buf = szBuf;
		wsaBuf.len = nBufTotalSize;

#if  WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP

		ZeroMemory( &m_OverLapped, sizeof m_OverLapped );
#else
		WSAResetEvent(m_OverLapped.hEvent);
#endif
#endif


	}
#ifdef WIN32
#ifdef IFPLATFORM_WP

	HANDLE getWSAEvent()
	{
		return m_OverLapped.hEvent;
	}
#endif
#endif

public:
#if defined(WIN32) && WINAPI_FAMILY!=WINAPI_PARTITION_APP
	WSAOVERLAPPED m_OverLapped;
#endif
	IFNetConnectionPtr	m_spConnection;
	IFNetIODataOperation m_nOperation;
#if defined(WIN32) && WINAPI_FAMILY!=WINAPI_PARTITION_APP
	WSABUF wsaBuf;
#endif
	const int nBufTotalSize;
	char szBuf[IFNET_IODATA_SIZE];



	friend class IFNetCore;

	
};

struct IFNetLargeIOData : public IFNetIOData
{
#if defined(WIN32) && WINAPI_FAMILY!=WINAPI_PARTITION_APP
private:
	IFNetLargeIOData(IFNetConnectionPtr pConnection, IFNetIODataOperation op)
		:IFNetIOData(pConnection,op,IFNET_LARGE_IODATA_SIZE)
	{

	}
#else
	IFNetLargeIOData()
	{

	}

#endif
public:
	char szLargeBuf[IFNET_LARGE_IODATA_SIZE-IFNET_IODATA_SIZE];
	friend class IFNetCore;

};