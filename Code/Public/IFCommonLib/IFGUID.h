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
#include "IFObj.h"
#include "IFString.h"
#include "IFPlatformDefine.h"
#include "IFCommonLib_API.h"
#ifdef WIN32
#include <objbase.h>
#elif defined(IFPLATFORM_LINUX) || defined(MAC) || defined(IFPLATFORM_WEB)
#include <uuid/uuid.h>
#endif

class IFStream;

class IFCOMMON_API IFGUID : public IFMemObj
{
public:
	IFGUID(void);
	~IFGUID(void);

	bool generate();

	IFString toString() const;
	bool fromString(const IFString& s);

	void serialize(IFStream* pStream) const;
	void deserialize(IFStream* pStream);
	bool isEqual(const IFGUID& o) const;
	bool operator <(const IFGUID& o) const;
protected:
#ifdef WIN32
	GUID m_uuid;
#elif defined(IFPLATFORM_ANDROID)
	struct UUID
	{
		int Data1;
		IFUI16 Data2,Data3;
		IFI8 Data4[8];
	};
	UUID m_uuid;
#else
	uuid_t m_uuid;
#endif
};

inline IFStream& operator << (IFStream& stream, const IFGUID& o)
{
	o.serialize(&stream);
	return stream;
}

inline IFStream& operator >> (IFStream& stream, IFGUID& o)
{
	o.deserialize(&stream);
	return stream;
}