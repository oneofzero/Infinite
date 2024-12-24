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
#ifndef __IF_MD5_H__
#define __IF_MD5_H__
#include "IFCommonLib_API.h"
#include "ifsingleton.h"
#include "IFString.h"
#include "IFStream.h"

struct IFCOMMON_API IFMD5Binary
{

	IFUI8 data[16];
		
	IFString toString() const;

};

struct IFCOMMON_API IFMD5Result : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:

	IFMD5Result(IFStream* sInput);
	IFMD5Result(const IFString& s);
	IFMD5Result();
	~IFMD5Result();

	void calcMD4(IFStream* sInput);


	void init();
	void append(const void* data, int len);
	IFMD5Result& finish();

	inline bool operator <(const IFMD5Result& o) const
	{
		if( *(IFUI64*)&m_Result[0] < *(IFUI64*)&o.m_Result[0] )
		{
			return true;
		}
		else if(*(IFUI64*)&m_Result[0] == *(IFUI64*)&o.m_Result[0] )
		{
			return *(IFUI64*)&m_Result[8] < *(IFUI64*)&o.m_Result[8];
		}
		else
		{
			return false;
		}
	}

	inline bool isEqual(const IFMD5Result& o) const
	{
		return *(IFUI64*)&m_Result[0] == *(IFUI64*)&o.m_Result[0]  &&
			*(IFUI64*)&m_Result[8] == *(IFUI64*)&o.m_Result[8] ;
	}

	IFString toString() const;

	void serialize(IFStream* pStream) const;
	void deserialize(IFStream* pStream);



	union
	{
		IFUI8 m_Result[16];
		IFMD5Binary m_Binary;
	};
	

private:
	void* m_pMD5State;

};

inline IFStream& operator << (IFStream& stream, const IFMD5Result& o)
{
	o.serialize(&stream);
	return stream;
}

inline IFStream& operator >> (IFStream& stream, IFMD5Result& o)
{
	o.deserialize(&stream);
	return stream;
}
#endif //__IF_MD5_H__