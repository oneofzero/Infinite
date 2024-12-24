#pragma once
#ifndef __IF_REMOTE_LOG_STREAM_H__
#define __IF_REMOTE_LOG_STREAM_H__
#include "IFLogSystem.h"
#include "IFNetCore.h"

class IFCOMMON_API IFRemoteConsoleStream : public IFStream
{
protected:
	~IFRemoteConsoleStream()
	{

	}
public:

	void init(IFNetCore* pNetCore, const IFString& sAddr, int nPort);

	const IFString& getName()
	{
		return IFString::Empty;
	}
	virtual IFUI32 read(void* pDestData, IFUI32 nSize)
	{
		return 0;
	}
	virtual IFUI32 write(const void* pSourceData, IFUI32 nSize);


	virtual IFI64 seek(IFI64 nSeek, IFUI32 nFrom)
	{
		return 0;
	}
	virtual IFI64 tell()const
	{
		return 0;
	}

	virtual bool isEnd()const
	{
		return false;
	}

	virtual bool isVaild()const
	{
		return true;
	}

	void flush()
	{
		
	}

private:
	IFEventHandle<void(IFNetConnection* pCon, bool ok)> m_handleConnect;
	void procConnect(IFNetConnection* pCon, bool ok);
	IFEventHandle<void(IFNetConnection* pCon)> m_handleDisConnect;
	void procDisconnect(IFNetConnection* pCon);

	void sendCached();

	void rawsend(const void* p, int len);

	IFRefPtr<IFNetConnection> m_spOutCon;
	IFRefPtr<IFNetCore> m_spNetCore;
	IFQueue<IFSimpleArray<char>*> m_buff;
};

#endif