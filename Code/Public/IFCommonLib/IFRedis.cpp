#include "stdafx.h"
#include "IFRedis.h"
#include "IFNetCore.h"

IFRedis::IFRedis(void)
{
}


IFRedis::~IFRedis(void)
{
}

bool IFRedis::connect(IFNetCore* pNetCore, const IFString& sAddress, int nPort)
{
	m_spConnection = pNetCore->createConnection(sAddress, nPort,true,false,true);
	m_spConnection->event_RecvData.AddSelfHoldHandleL([=](IFNetConnection* pConnection, const void* pBuf, int nSize)
	{

	});
	return !!m_spConnection;
}

void IFRedis::query(const IFString& sCmd, RedisReturnCallbackPtr spCallback)
{
	m_spConnection->sendData(sCmd.c_str(), sCmd.length());
}
