#include "stdafx.h"
#include "IFSHA.h"

#ifdef IFCOMMON_UNITY_SUPPORT
#define DONT_USE_SSL
#endif

#ifndef DONT_USE_SSL
#include <openssl/sha.h>



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
#endif
