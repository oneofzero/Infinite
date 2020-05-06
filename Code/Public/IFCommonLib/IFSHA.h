#pragma once
#include "IFString.h"
class IFCOMMON_API IFSHAResult
{
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

