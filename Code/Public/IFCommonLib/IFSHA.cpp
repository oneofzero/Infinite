#include "stdafx.h"
#include "IFSHA.h"

IF_DEFINERTTIROOT(IFSHAResult);
IF_DEFINERTTIROOT(IFHMACResult);
IF_DEFINERTTIROOT(IFSHA256Result);
#ifndef DONT_USE_SSL
#include <openssl/sha.h>
#include <openssl/hmac.h>


IFSHAResult::IFSHAResult()
{
	m_context = new SHA_CTX();
	SHA1_Init((SHA_CTX*)m_context);
}


IFSHAResult::~IFSHAResult()
{
	delete (SHA_CTX*)m_context;
}

IFSHAResult& IFSHAResult::append(const IFString& s)
{
	SHA1_Update((SHA_CTX*)m_context, s.c_str(), s.length());
	return *this;
}
IFString IFSHAResult::toString()
{
	IFString r;
	for (int i = 0; i < m_result.length(); i++)
	{
		r += IFString().format("%02X", (IFUI8)m_result[i]);
	}
	return r;
}

const IFString& IFSHAResult::result()
{
	
	char b[20];
	SHA1_Final((unsigned char*)b, (SHA_CTX*)m_context);
	m_result = IFString(b, 20, IFString::EC_UTF8);

	return m_result;
}

IFSHA256Result::IFSHA256Result()
{
	m_context = new SHA256_CTX();
	SHA256_Init((SHA256_CTX*)m_context);
}


IFSHA256Result::~IFSHA256Result()
{
	delete (SHA256_CTX*)m_context;
}

IFSHA256Result& IFSHA256Result::append(const IFString& s)
{
	SHA256_Update((SHA256_CTX*)m_context, s.c_str(), s.length());
	return *this;
}
IFString IFSHA256Result::toString()
{
	result();
	IFString r;
	for (int i = 0; i < m_result.length(); i++)
	{
		r += IFString().format("%02X", (IFUI8)m_result[i]);
	}
	return r;
}

const IFString& IFSHA256Result::result()
{
	if (m_result.isEmpty())
	{
		char b[32];
		SHA256_Final((unsigned char*)b, (SHA256_CTX*)m_context);
		m_result = IFString(b, 32, IFString::EC_UTF8);

	}
	
	return m_result;
}


IFHMACResult::IFHMACResult(const IFString& sKey, const IFString& hashEngine)
{
	auto alg = hashEngine.toUpper();
	const EVP_MD* engine = NULL;
	if (alg == "MD5")
	{
		engine = EVP_md5();
	}
	else
	{
		engine = EVP_sha256();
	}
	m_context.resize(sizeof(HMAC_CTX));
	auto pCtx = (HMAC_CTX*)(char*)m_context;
	HMAC_CTX_init(pCtx);
	HMAC_Init_ex(pCtx, sKey.c_str(), sKey.length(), engine, NULL);
}
IFHMACResult& IFHMACResult::append(const IFString& s)
{
	auto pCtx = (HMAC_CTX*)(char*)m_context;

	HMAC_Update(pCtx, (const unsigned char*)s.c_str(), s.length());

	return *this;
}

IFString& IFHMACResult::result()
{
	if (m_result.isEmpty())
	{
		auto pCtx = (HMAC_CTX*)(char*)m_context;
		IFUI8 MD[128];
		IFUI32 len = sizeof(MD);
		HMAC_Final(pCtx, MD, &len);
		m_result = IFString((const char*)MD, len);
	}
	
	return m_result;
}
#else
IFSHAResult::IFSHAResult()
{
}


IFSHAResult::~IFSHAResult()
{
}

IFSHAResult& IFSHAResult::append(const IFString& s)
{
	return *this;
}

IFString IFSHAResult::toString()
{
	return IFString::Empty;
}

const IFString& IFSHAResult::result()
{
	return IFString::Empty;
	
}
IFSHA256Result::IFSHA256Result()
{

}
IFSHA256Result::~IFSHA256Result()
{

}
IFSHA256Result& IFSHA256Result::append(const IFString& s)
{
	return *this;
}
IFString IFSHA256Result::toString()
{
	return m_result;
}

const IFString& IFSHA256Result::result()
{
	return m_result;
}

IFHMACResult::IFHMACResult(const IFString& key, const IFString& hashEngine)
{

}

IFHMACResult& IFHMACResult::append(const IFString& s)
{
	return *this;
}

IFString& IFHMACResult::result()
{
	return m_result;
}
#endif
