#pragma once
#ifndef __IF_SHA_H__
#define __IF_SHA_H__
#include "IFObj.h"
#include "IFString.h"
class IFCOMMON_API IFSHAResult :public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	IFSHAResult();
	~IFSHAResult();

	IFSHAResult& append(const IFString& s);

	IFString toString();

	const IFString& result();
private:

	void* m_context;
	IFString m_result;
};

class IFCOMMON_API IFSHA256Result :public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	IFSHA256Result();
	~IFSHA256Result();

	IFSHA256Result& append(const IFString& s);

	IFString toString();

	const IFString& result();
private:

	void* m_context;
	IFString m_result;
};

class IFCOMMON_API IFHMACResult : public IFMemObj
{
	IF_DECLARERTTI_STATIC;

public:
	IFHMACResult(const IFString& key, const IFString& hashEngine);

	IFHMACResult& append(const IFString& s);

	IFString& result();
private:

	IFSimpleArray<char> m_context;
	void* m_engine;
	IFString m_result;

};
#endif //__IF_SHA_H__