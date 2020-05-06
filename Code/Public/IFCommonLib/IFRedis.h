#pragma once
#include "IFRefObj.h"
#include "IFRefPtr.h"
#include "IFFunctor.h"
class IFString;
class IFNetCore;
class IFNetConnection;

class IFRedisReturn;

typedef IFRefPtr<IFFunctor<void(IFRedisReturn* pReturn)>> RedisReturnCallbackPtr;
class IFCOMMON_API IFRedis : public IFRefObj
{
public:
	IFRedis();

	bool connect(IFNetCore* pNetCore, const IFString& sAddress, int nPort);

	void query(const IFString& sCmd, RedisReturnCallbackPtr spCallback);

protected:
	~IFRedis(void);

	IFRefPtr<IFNetConnection> m_spConnection;
};

