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
#include "stdafx.h"
#include "IFGUID.h"
#include "IFStream.h"
#include <time.h>
#include "IFSystemAPI.h"
#include <math.h>
#include "IFUtility.h"

IF_DEFINERTTIROOT(IFGUID);

static bool nSrand = false;
IFGUID::IFGUID(void)
{
}


IFGUID::~IFGUID(void)
{
}

bool IFGUID::generate()
{
#ifdef WIN32
	int hr = CoCreateGuid (&m_uuid);

	return hr == 0;
#elif defined(IFPLATFORM_LINUX) || defined(MAC) || defined(IFPLATFORM_WEB)
    uuid_generate(m_uuid);
    return true;
#elif defined(IFPLATFORM_ANDROID)
	if (!nSrand)
	{
		srand(IFNativeSystemAPI::getMicrosSec());
		nSrand = true;
	}
	m_uuid.Data1 = rand();
	m_uuid.Data2 = rand();
	m_uuid.Data3 = rand();
	for (int i = 0; i < 8; i ++ )
		m_uuid.Data4[i] = rand();
	return true;
#else

#endif // 
}

IFString IFGUID::toString() const
{
#if defined(WIN32)||defined(IFPLATFORM_ANDROID)
	return IFString().format("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		m_uuid.Data1,m_uuid.Data2,m_uuid.Data3,
		m_uuid.Data4[0],m_uuid.Data4[1],
		m_uuid.Data4[2],m_uuid.Data4[3],
		m_uuid.Data4[4],m_uuid.Data4[5],
		m_uuid.Data4[6],m_uuid.Data4[7]
		);
#else
	auto uuidb = (IFUI8*)&m_uuid;
    return IFString().format("%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                             *(IFUI32*)&uuidb[0],*(IFUI16*)&uuidb[4],*(IFUI16*)&uuidb[6],
		uuidb[8], uuidb[9],
		uuidb[10], uuidb[11],
		uuidb[12], uuidb[13],
		uuidb[14], uuidb[15]
                             );
#endif
}


bool IFGUID::fromString(const IFString& s)
{
	IFArray<IFString> sl;
	int Data1;
	IFUI16 Data2,Data3;
	IFUI8 Data4[8];

	USplitStrings(&sl,s, "-");
	if (sl.size()!=5)
		return false;
	if (sl[0].size()!=8)
		return false;
	if (sl[1].size()!=4 || sl[2].size()!=4)
		return false;
	Data1 = sl[0].toUint32(16);
	Data2 = sl[1].toUint32(16);
	Data3 = sl[2].toUint32(16);

	if (sl[3].size()!=4)
		return false;
	Data4[0] = sl[3].sub(0,2).toUint32(16);
	Data4[1] = sl[3].sub(2,2).toUint32(16);

	if (sl[4].size()!=12)
		return false;
	for (int i = 0; i < 3; i ++)
	{
		Data4[i*2+2] = sl[4].sub(i*4,2).toUint32(16);
		Data4[i*2+3] = sl[4].sub(i*4+2,2).toUint32(16);

	}

#if defined(WIN32)||defined(IFPLATFORM_ANDROID)
		m_uuid.Data1 = Data1,m_uuid.Data2=Data2,m_uuid.Data3=Data3;
		for (int i = 0; i < 8; i ++)
		{
			m_uuid.Data4[i] = Data4[i];
		}


#else
		*(IFUI32*)&m_uuid[0] = Data1,*(IFUI16*)&m_uuid[4] = Data2,*(IFUI16*)&m_uuid[6]=Data3;
		for (int i = 0; i < 8; i ++)
		{
			m_uuid[i+8] = Data4[i];
		}


#endif

	return true;

}


void IFGUID::serialize( IFStream* pStream ) const
{
	pStream->write(&m_uuid, sizeof(m_uuid));
}

void IFGUID::deserialize( IFStream* pStream )
{
	pStream->read(&m_uuid, sizeof(m_uuid));
}

bool IFGUID::isEqual( const IFGUID& o ) const
{
#ifdef MAC
	return memcmp(m_uuid,o.m_uuid,sizeof(m_uuid)) == 0;
#else
	return memcmp(&m_uuid,&o.m_uuid,sizeof(m_uuid)) == 0;
#endif
}

bool IFGUID::operator<( const IFGUID& o ) const
{
	IFUI64* pA[2],*pB[2];
#if defined(MAC)||defined(IFPLATFORM_LINUX)
	//memcmp(m_uuid,o.m_uuid,sizeof(m_uuid));
	pA[0] = (IFUI64*)(&((char*)m_uuid)[0]);
	pB[0] = (IFUI64*)(&((char*)m_uuid)[8]);
	pA[1] = (IFUI64*)(&((char*)o.m_uuid)[0]);
	pB[1] = (IFUI64*)(&((char*)o.m_uuid)[8]);

#else
	//memcmp(&m_uuid,&o.m_uuid,sizeof(m_uuid));
	pA[0] = (IFUI64*)(&((char*)&m_uuid)[0]);
	pB[0] = (IFUI64*)(&((char*)&m_uuid)[8]);
	pA[1] = (IFUI64*)(&((char*)&o.m_uuid)[0]);
	pB[1] = (IFUI64*)(&((char*)&o.m_uuid)[8]);

#endif
	if (*pA[0] < *pA[1])
		return true;
	else if (*pA[1] < *pA[0])
		return false;
	else
		return *pB[0] < *pB[1];
	
}
