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
#include <stdlib.h>
#include "IFArray.h"
#include "IFString.h"
#include "IFAlloc.h"
#include <stdarg.h>
#include <locale.h>
#include <stdio.h>
#include <string.h>
#ifndef WIN32
#include <wchar.h>
#endif
#include "IFHashSet.h"
#include "IFMemStream.h"
#include "IFNumParse.h"
#include "IFUtility.h"
//#ifdef ANDROID
#include "androidwcs.h"
void utf8_wchar(const IFString &utf8, IFStringW &wide);

void wchar_utf8(const IFStringW& wide, IFString &utf8);
//#else


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

//#endif
//#if defined(ANDROID) || defined(MAC) || defined(LINUX) || defined(IFPLATFORM_WEB) || defined(IFPLATFORM_FREE_RTOS)
#include "utf8wchar.cpp"
//#endif

#ifdef IFSTRING_STANDALONE
const IFString IFString::Empty;
const IFStringW IFStringW::Empty;

template<typename CHART>
inline static IFUI32 rshash(const CHART* s, IFUI32& nLen)
{
	IFUI32 b = 378551;
	IFUI32 a = 63689;
	//IFUI32 hashval = 0;
	nLen = 0;

	IFUI32 hash = 0;
	while (s[nLen])
	{
		hash = hash * a + s[nLen];
		a *= b;
		nLen++;
	}
	return hash;
}
inline static IFUI32 rshash2(const char* s, IFUI32 nLen)
{
	if (nLen == -1)
		nLen = (int)strlen(s);
	IFUI32 b = 378551;
	IFUI32 a = 63689;
	IFUI32 hash = 0;

	int nBlockLen = nLen / sizeof(size_t);
	int nBlockMod = nLen % sizeof(size_t);
	size_t* pS = (size_t*)s;
	size_t* pEnd = pS + nBlockLen;
	for (; pS < pEnd; pS++)
	{
		hash = hash * a + (int)*pS;
		a *= b;

	}
	s = (char*)pEnd;
	for (int i = 0; i < nBlockMod; i++)
	{
		hash = hash * a + s[i];
	}
	return hash;
}


//static IFHashSet<IFString>* g_pShortStringPool = NULL;

static inline const IFWCHAR* t_strstr(const IFWCHAR* p1, const IFWCHAR* p2)
{
	return wcsstr(p1, p2);
}
static inline const char* t_strstr(const char* p1, const char* p2)
{
	return strstr(p1, p2);
}

static inline const int t_strlen(const IFWCHAR* p1)
{
	return (int)wcslen(p1);
}
static inline const int t_strlen(const char* p1)
{
	return (int)strlen(p1);
}

template<typename T>
static inline void StrCopy(T* pDest, const T* pSrc, int nLen)
{
	memcpy(pDest, pSrc, nLen*sizeof(T));
}
template<typename T>
static inline void StrMove(T* pDest, const T* pSrc, int nLen)
{
	memmove(pDest, pSrc, nLen*sizeof(T));
}



template<typename STRINGT>
void IFString_assign(STRINGT& o, const typename STRINGT::THIS_CHAR_TYPE* pStr, int nLen)
{
	if (nLen == -1)
	{
		nLen = t_strlen(pStr);
	}

	auto pNewPtr = o.makeSureSelfBuffer(nLen + 1);
	o.m_nSize = nLen;
	o.m_nRSHash = 0;
	if (nLen)
		StrCopy(pNewPtr, pStr, nLen);
	pNewPtr[o.m_nSize] = 0;
}

template<typename STRINGT>
void IFString_assign(STRINGT& o, const STRINGT& str)
{
	o.m_nSize = str.m_nSize;
	o.m_nRSHash = (str.m_nRSHash);
	o.m_nCap = str.m_nCap;
	o.m_spBuffer = (str.m_spBuffer);

	if (str.isUseSmallBuff())
	{
		StrCopy(o.m_SmallBuff, str.c_str(), o.m_nSize);
		o.m_SmallBuff[o.m_nSize] = 0;
	}

}

template<typename STRINGT>
void IFString_append(STRINGT& o, const typename STRINGT::THIS_CHAR_TYPE* sStr, int nLen)
{
	if (nLen <0)
		nLen = t_strlen(sStr);
	if (nLen <= 0)
		return;
	int nNewLen = o.m_nSize + nLen;
	auto* pNewPtr = o.makeSureSelfBuffer(nNewLen + 1);
	StrCopy(pNewPtr + o.m_nSize, sStr, nLen);
	o.m_nSize = nNewLen;
	pNewPtr[o.m_nSize] = 0;
	o.m_nRSHash = 0;
}
template<typename STRINGT>
void IFString_clear(STRINGT& o)
{
	auto* pNewPtr = o.makeSureSelfBuffer(1);
	o.m_nSize = 0;
	pNewPtr[0] = 0;
	o.m_nRSHash = 0;
}

template<typename STRINGT>
void IFString_push_back(STRINGT& o, typename STRINGT::THIS_CHAR_TYPE c)
{
	auto p = o.makeSureSelfBuffer(o.m_nSize + 2);

	p[o.m_nSize] = c;
	o.m_nSize++;
	p[o.m_nSize] = 0;
	o.m_nRSHash = 0;
}

IFString::IFString()
	:m_nSize(0)
	,m_nCap(0)
	,m_nRSHash(0)
	,m_eEncoding(LOCAL_CHAR_ENCODING)
	
	

{
	m_SmallBuff[0] = 0;
	//makeSureSelfBuffer(8);
	//m_spBuffer->m[0] = 0;
}

IFString::IFString(ENCODING coding)
	:m_nSize(0)
	,m_nCap(0)
	,m_nRSHash(0)	
	,m_eEncoding(coding)
{
	m_SmallBuff[0] = 0;
	//makeSureSelfBuffer(8);
	//m_spBuffer->m[0] = 0;
}




IFString::IFString(const IFString& o)
	:m_nCap(0)
	, m_eEncoding(o.m_eEncoding)
{

	IFString_assign(*this, o);
}

IFString::IFString(const IFStringW& o,ENCODING coding )	
	:m_nSize(0)
	,m_nCap(0)
	,m_nRSHash(0)
	,m_eEncoding(coding)
	
{
	m_SmallBuff[0] = 0;
	operator = (o);
}

IFString::IFString(const char* sStr, ENCODING coding /*= LOCAL_CHAR_ENCODING*/)
	:m_nSize(0)
	,m_nCap(0)
	, m_nRSHash(0)
	, m_eEncoding(coding)
	
	

{
	m_SmallBuff[0] = 0;
	operator=(sStr);
}

IFString::IFString(const char* sStr, int nLen,ENCODING coding)	
	:m_nSize(0)
	,m_nCap(0)
	,m_nRSHash(0)
	,m_eEncoding(coding)
	
	
{
	IFString_assign(*this, sStr, nLen);
}
IFString::IFString(const char* sStr, const char* sEnd,ENCODING coding)	
	:m_nSize(0)
	,m_nCap(0)
	,m_nRSHash(0)
	,m_eEncoding(coding)	
{

	IFString_assign(*this, sStr, int(sEnd - sStr));
}




IFString& IFString::operator =(const IFString& o)
{
	m_eEncoding = o.m_eEncoding;
	IFString_assign(*this, o);
	return *this;
}

IFString& IFString::operator =(const IFStringW& o)
{

	clear();
#ifdef  IF_STRING_NO_ANSI
	wchar_utf8(o, *this);
#else
	if (m_eEncoding == EC_UTF8)
		wchar_utf8(o, *this);
	else
		wchar_gbk(o, *this);
#endif //  IF_STRING_NO_ANSI


	return *this;
}

//IFString& IFString::operator =(const char* sStr)


bool IFString::operator ==(const IFString& o) const
{
	if (!isUseSmallBuff() && m_spBuffer == o.m_spBuffer)
		return true;
	else if (m_nSize && o.m_nSize)
		return strcmp(c_str(),o.c_str()) == 0;
	else 
		return m_nSize == o.m_nSize;
}

bool IFString::operator ==(const char* s) const
{
	if (m_nSize)
		return strcmp(c_str(),s) == 0;
	else
	{
		return s[0] == 0;
	}
}

IFString& IFString::operator=(const char* sStr)
{

	IFString_assign(*this, sStr, -1);
	return *this;


}
bool IFString::operator <(const IFString& o) const
{
	if (m_spBuffer && m_spBuffer == o.m_spBuffer)
		return false;

	const char* pMe =  c_str();
	const char* pOther = o.c_str();
	while( *pMe && *pOther )
	{
		if( (IFUI8)*pMe > (IFUI8)*pOther )
			return false;
		else if( (IFUI8)*pMe < (IFUI8)*pOther )
			return true;

		++pMe;
		++pOther;
	}

	return *pMe==0 && *pOther!=0;
}


IFString IFString::operator +(const IFString& o)const
{
	if (o.m_nSize)
	{
		IFString str(*this);
		str += o;
		return str;
	}
	else
		return *this;
}

IFString IFString::operator +(const char* sStr)const
{
	IFString str(*this);
	str +=sStr;
	return str;
}

IFString& IFString::operator +=(const IFString& o)
{
	if (o.m_nSize)
	{
		return append(o.c_str(), o.length());
	}
	else
		return *this;
}

IFString& IFString::operator +=(char c)
{

	push_back(c);
	return *this;
}


IFString& IFString::operator +=(const  char* sStr)
{
	//int nAddLen = (int)strlen(sStr);
	//int nNewLen = m_nSize + nAddLen;
	//makeSureSelfBuffer(nNewLen+1);
	//memcpy(m_spBuffer->m+m_nSize, sStr, nAddLen);
	//m_nSize = nNewLen;
	//m_spBuffer->m[m_nSize] = 0;
	return append(sStr);
}

IFString& IFString::append( const char* sStr, int nLen )
{
	IFString_append(*this, sStr, nLen);
	return *this;
}


void IFString::push_back(char c)
{
	IFString_push_back(*this, c);
}

void IFString::reserve(int nSize)
{
	m_nCap = nSize;
	makeSureSelfBuffer(nSize);
	//m_MemStr.reserve(nSize);
}

void IFString::clear()
{
	//operator = ("");
	IFString_clear(*this);
}

#define IS_WRITE_SPACE(c) ( (c)==' ' || (c) == '\r' || (c)=='\n' || (c)=='\t')


IFString IFString::trimStart(char c) const
{
	auto p = c_str();

	int  i = 0;
	for (; i < (int)size(); i++)
	{
		if ( c && c == p[i] || IS_WRITE_SPACE(p[i]) )
			continue;
		break;
	}
	
	return IFString(p + i, size() - i, m_eEncoding);
	
}

IFString IFString::trimEnd(char c) const
{
	auto p = c_str();

	int  i = size()-1;
	for (; i >=0; i--)
	{
		if (c && c == p[i] || IS_WRITE_SPACE(p[i]))
			continue;
		break;
	}

	return IFString(p , i+1, m_eEncoding);
}

template<typename STRINGT>
void IFString_erase(STRINGT& o, int nPos, int nSize)
{
	if (nSize == 0 || nSize>o.m_nSize)
		return;
	auto pOldPtr = o.selfptr();
	auto pNewPtr = o.makeSureSelfBuffer(o.m_nSize);
	int nNeedCopyMoveSize = o.m_nSize - (nPos + nSize);
	if (nNeedCopyMoveSize > 0)
	{
		StrMove(&pNewPtr[nPos], &pOldPtr[nPos + nSize], nNeedCopyMoveSize);
	}

	o.m_nSize -= nSize;
	pNewPtr[o.m_nSize] = 0;
	o.m_nRSHash = 0;
}

void IFString::erase(int nPos,int nSize)
{
	IFString_erase(*this, nPos, nSize);
	//m_MemStr.erase(nPos,nSize);

}



template<typename T, typename STRINGT>
STRINGT& IFString_insert(STRINGT& o, int nPos, const STRINGT& s)
{
	if (!s.size())
		return o;
	int nNewSize = o.m_nSize + s.size();
	int nMoveSize = o.m_nSize - nPos;
	T* pOldPtr;
	typename STRINGT::BufferPtr spOldPtr;
	if (o.isUseSmallBuff())
	{
		pOldPtr = o.selfptr();
	}
	else
	{
		spOldPtr = o.m_spBuffer;
		pOldPtr = spOldPtr->m;
	}
	T* pNewPtr = o.makeSureSelfBuffer(nNewSize);

	if (nMoveSize)
		StrMove(&pNewPtr[nPos + s.size()], &pOldPtr[nPos], nMoveSize);
	StrCopy(&pNewPtr[nPos], s.c_str(), s.size());

	o.m_nSize += s.size();
	pNewPtr[o.m_nSize] = 0;
	o.m_nRSHash = 0;
	return o;
}


IFString& IFString::insert(int nPos, const IFString& s)
{
	
	return IFString_insert<char>(*this, nPos, s);
}

template<typename STRINGT>
int IFString_find(const STRINGT& o,const STRINGT& other, int nOffset, bool nocase)
{
	if (!other.m_nSize || !o.m_nSize)
		return -1;
	auto pMe = o.c_str() + nOffset;
	auto pOther = other.c_str();
	if (nocase)
	{
		auto *cp = pMe;
		const typename STRINGT::THIS_CHAR_TYPE *s1, *s2;

		while (*cp)
		{
			s1 = cp;
			s2 = pOther;

			while (*s1 && *s2)
			{
				int d = *s1 - *s2;
				if (d)
				{
					if ((*s1 >= 'A' && *s1 <= 'Z') ||
						(*s1 >= 'a' && *s1 <= 'z'))
					{
						if (d != 'A' - 'a' &&
							d != 'a' - 'A')
							break;
					}
					else
						break;
				}

				s1++, s2++;
			}

			if (!*s2)
				return int(cp - pMe) + nOffset;

			cp++;
		}
		return -1;
	}
	else
	{
		auto pFind = t_strstr(pMe, pOther);
		if (pFind == NULL)
			return -1;
		else
			return int(pFind - pMe)+nOffset;
	}
}


int IFString::find(const IFString& other, int noffset, bool nocase) const
{
	return IFString_find(*this, other, noffset, nocase);
}

void IFString::setUTF8Codeing( bool b )
{
	if (b)
		m_eEncoding = EC_UTF8;
	else
		m_eEncoding = EC_ANSI;

}

bool IFString::isUTF8Codeing() const
{
	return m_eEncoding == EC_UTF8;
}

void IFString::resize( int nSize )
{
	char* pNewPtr = makeSureSelfBuffer(nSize+1);
	
	m_nSize = nSize;
	pNewPtr[m_nSize] = 0;
	m_nRSHash = 0;

}

IFString IFString::operator-( const IFString& o ) const
{
	return IFString();
}

IFString IFString::convertEncoding(ENCODING encoding) const
{
	if (encoding == m_eEncoding)
		return *this;
	IFStringW  ws = *this;
#ifndef IF_STRING_NO_ANSI
	if (encoding == ENCODING::EC_ANSI)
		return ws.toANSIString();		
	else
#endif
		return ws.toUTF8String();
	
}

IFString& IFString::format( const char* sFormat, ... )
{
#ifdef IFPLATFORM_FREE_RTOS
	char buf[512];
#else
	char buf[32 * 1024];
#endif

#ifdef WIN32
	va_list vlist;
	va_start(vlist, sFormat );
	int n = _vsnprintf_s(buf, _TRUNCATE, sFormat,  vlist );
	va_end(vlist);
#else
#if defined(LINUX) || defined(MAC) || defined(IFPLATFORM_WEB) || defined(IFPLATFORM_FREE_RTOS) ||defined(IFPLATFORM_EMBED_NOSYS)
	va_list vlist;
	va_start(vlist, sFormat );
	int n = vsnprintf(buf,sizeof(buf), sFormat, vlist  );
	va_end(vlist);
#endif
#endif
	IFString_assign(*this, buf, n);
	return *this;
}

template<typename STRINGT>
void IFString_upper(STRINGT& o)
{
	if (o.m_nSize)
	{
		auto p = o.makeSureSelfBuffer(o.m_nSize + 1);

		while (*p)
		{
			if (*p >= 'a' && *p <= 'z')
			{
				*p -= 'a' - 'A';
			}
			p++;
		}

		o.m_nRSHash = 0;

	}
}

template<typename STRINGT>
void IFString_lower(STRINGT& o)
{
	if (o.m_nSize)
	{
		auto p = o.makeSureSelfBuffer(o.m_nSize + 1);

		while (*p)
		{
			if (*p >= 'A' && *p <= 'Z')
			{
				*p += 'a' - 'A';
			}
			p++;
		}

		o.m_nRSHash = 0;

	}
}

void IFString::upper()
{
	IFString_upper(*this);

}

void IFString::lower()
{
	IFString_upper(*this);

}

template<typename STRINGT>
int IFString_find_first_of(const STRINGT& o, typename STRINGT::THIS_CHAR_TYPE c, int noffset)
{
	int nSize = o.size();
	auto pTtr = o.c_str();
	for (int i = noffset; i < nSize; i++)
	{
		if (c == pTtr[i])
		{
			return i;
		}
	}
	return -1;
}

template<typename STRINGT>
int IFString_find_last_of(const STRINGT& o, typename STRINGT::THIS_CHAR_TYPE c, int noffset)
{
	int nSize = o.size() - noffset - 1;
	auto pTtr = o.c_str();
	for (int i = nSize; i >= 0; i--)
	{
		if (c == pTtr[i])
		{
			return i;
		}
	}
	return -1;
}


int IFString::find_first_of( char c , int noffset) const
{
	return IFString_find_first_of(*this, c, noffset);
}

int IFString::find_last_of( char c , int noffset) const
{
	return IFString_find_last_of(*this, c, noffset);

}

bool IFString::endsWith(const IFString& ends) const
{
	if (length() < ends.length())
		return false;

	auto c = memcmp(c_str() + length() - ends.length(), ends.c_str(), ends.length());
	return c == 0;
}

bool IFString::startsWith(const IFString& starts) const
{
	if (length() < starts.length())
		return false;
	
	return memcmp(c_str(), starts.c_str(), starts.length()) == 0;
}

IFString IFString::convertTo( ENCODING cd ) const
{
#ifdef IF_STRING_NO_ANSI
	return *this;
#else
	if (cd == m_eEncoding)
		return *this;
	else if (cd == EC_UTF8)
	{
		return IFStringW(*this).toUTF8String();
	}
	else
	{
		return IFStringW(*this).toANSIString();
	}
#endif
}

const char IFString::UTF8Flag[3]={(char)-17,(char)-69,(char)-65};

IFString IFString::sub(int index, int size) const
{
	if (index > (int)length())
		return IFString::Empty;
	size = IFMin(size, (int)length() - index);
	return IFString(c_str()+index, size);
}

IFI32 IFString::toInt32( int nRadix /*= 10*/ ) const
{
	return strtol(c_str(), NULL, nRadix);
}

IFUI32 IFString::toUint32( int nRadix /*= 10*/ ) const
{
	return strtoul(c_str(), NULL, nRadix);
}

IFUI64 IFString::toUint64( int nRadix /*= 10*/ ) const
{
#ifndef WIN32
	return strtoull(c_str(), NULL, nRadix);
#else
	return _strtoui64(c_str(), NULL, nRadix);
#endif
}

IFI64 IFString::toInt64( int nRadix /*= 10*/ ) const
{
#ifndef WIN32
	return strtoll(c_str(), NULL, nRadix);
#else
	return _strtoi64(c_str(), NULL, nRadix);
#endif
}

IFString& IFString::loadInt(int n)
{
	char* end = IFNumParse::int32buf(n, m_SmallBuff);
	*end = 0;
	m_nSize = (int)(end - m_SmallBuff);
	m_spBuffer = NULL;
	return *this;

}
IFString& IFString::loadInt64(IFI64 n)
{
	char* end = IFNumParse::int64buf(n, m_SmallBuff);
	*end = 0;
	m_nSize = (int)(end - m_SmallBuff);
	m_spBuffer = NULL;
	return *this;

}
IFString& IFString::loadFloat(float f)
{
	char* end = IFNumParse::float2buf(f, m_SmallBuff);
	*end = 0;
	m_nSize = (int)(end - m_SmallBuff);
	m_spBuffer = NULL;
	return *this;

}
IFString& IFString::loadDouble(double f)
{
	char* end = IFNumParse::double2buf(f, m_SmallBuff);
	*end = 0;
	m_nSize = (int)(end - m_SmallBuff);
	m_spBuffer = NULL;
	return *this;
}
//IFUI32 IFString::toRSHash() const

IFUI32 IFString::RSHash(const char* s, int nLen) 
{
	//IFUI32 nlen = 0;
	//return ::rshash(s, nlen);
	return ::rshash2(s, nLen);
}



template<typename T, typename STRINGT>
T* IFString_makeSureSelfBuffer(STRINGT& s, int nBufInitialSize)
{
	s.m_nCap = IFMax(s.m_nCap, nBufInitialSize);

	if (s.m_nCap <= IFSTRING_SMALL_LEN)
	{
		if (s.m_spBuffer)
		{
			if (s.m_nSize)
			{
				StrCopy(s.m_SmallBuff, s.m_spBuffer->m, s.m_nSize);
				s.m_SmallBuff[s.m_nSize] = 0;
			}
			s.m_spBuffer = NULL;
		}
		return s.m_SmallBuff;
		//nBufInitialSize = 16;
	}

	if (!s.m_spBuffer || s.m_spBuffer->getRefCount() > 1)
	{
		if (s.m_nCap < s.m_nSize + 1)
			s.m_nCap = s.m_nSize + 1;

		auto spBuf = STRINGT::RefBuffer::alloc(s.m_nCap);
		//spBuf->m.resize(nBufInitialSize);
		if (s.m_spBuffer)
		{
			StrCopy(spBuf->m, s.m_spBuffer->m, IFMin(s.m_nCap, s.m_spBuffer->size()));
		}
		else if (s.m_nSize)
		{
			StrCopy(spBuf->m, s.m_SmallBuff, s.m_nSize);
		}
		spBuf->m[s.m_nSize] = 0;
		s.m_spBuffer = spBuf;

	}



	if (s.m_nCap > s.m_spBuffer->size())
	{

		int nNewSize = s.m_spBuffer->size() * 2;
		if (nNewSize < s.m_nCap)
			nNewSize = s.m_nCap;
		s.m_spBuffer = s.m_spBuffer->clone(nNewSize);

	}
	return s.m_spBuffer->m;
}

char* IFString::makeSureSelfBuffer(int nBufInitialSize)
{
	return IFString_makeSureSelfBuffer<char>(*this, nBufInitialSize);
	
}

template<typename T, typename STRINGT>
void IFString_replace(STRINGT& o, T oldVal, T newVal, IFUI32 nBegin, IFUI32 nEnd)
{
	nEnd = IFMin(nEnd, (IFUI32)o.m_nSize);
	T* pC = o.makeSureSelfBuffer(o.m_nSize);

	for (IFUI32 i = nBegin; i < nEnd; i++)
	{
		if (pC[i] == oldVal)
			pC[i] = newVal;
	}
	o.m_nRSHash = 0;
}

template<typename STRINGT>
void IFString_replace(STRINGT& o, const STRINGT& oldVal, const STRINGT& newVal, IFUI32 nBegin, IFUI32 nEnd)
{
	IFUI32 nPos = nBegin;


	while (true)
	{
		nPos = o.find(oldVal, nPos);
		if (nPos == -1 || nPos >= nEnd)
			return;
		if (oldVal.size() == newVal.size())
		{
			auto* pOBuf = o.makeSureSelfBuffer(o.m_nSize);
			memcpy(pOBuf + nPos, newVal.c_str(), oldVal.size());
		}
		else
		{
			o.erase(nPos, oldVal.size());
			o.insert(nPos, newVal);
		}
		nPos += newVal.size();
	}
	o.m_nRSHash = 0;
}


void IFString::replace(char oldVal, char newVal, IFUI32 nBegin /*= 0*/, IFUI32 nEnd/*=-1*/)
{
	IFString_replace(*this, oldVal, newVal, nBegin, nEnd);

}

void IFString::replace(const IFString& oldVal, const IFString& newVal, IFUI32 nBegin /*= 0*/, IFUI32 nEnd/*=-1*/)
{
	IFString_replace(*this, oldVal, newVal, nBegin, nEnd);

}

IFString IFString::replace(char oldVal, char newVal, IFUI32 nBegin, IFUI32 nEnd) const
{
	IFString newS;
	IFString_replace(newS, oldVal, newVal, nBegin, nEnd);
	return newS;
}

IFString IFString::toURLString() const
{
	IFString curl;
	for (const char* p = c_str(); *p; p++)
	{
		unsigned char c = *p;
		if ( (c>='a'&& c<='z') || (c>='A'&& c<='Z') ||
			(c>='0' && c<='9') ||
			//!#$&'()*+,/:;=?@-._~
			c=='!' || c=='#' || c=='$' || c=='&' || c=='\'' || c=='(' || c==')' || c=='*'||c=='+'||c==','||c=='/'||c==':'||c==';'||c=='='||
			c=='?'||c=='@'||c=='-'||c=='.'||c=='_'||c=='~')
		{
			curl.push_back(c);
		}
		else
		{
			curl += IFString().format("%%%02X", (int)c);
		}

	}

	return curl;
}
IFString IFString::FromURLString(const IFString& s)
{
	IFString o(EC_UTF8);
	for (int i = 0; i < (int)s.size(); i++)
	{
		if (s[i] == '%' && i + 2 < (int)s.size())
		{
			auto c = IFString(&s[i] + 1, &s[i] + 3).toInt32(16);
			o.push_back(c);
			i += 2;

		}
		else if (s[i] == '+')
			o.push_back(' ');
		else
		{
			o.push_back(s[i]);
		}
	}
	return o;
}

static char base64enctable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char base64dectable[]={

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255,

	255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,

	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28,

	29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,

	49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,

	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
}


;

IFString& IFString::encodeBase64(const char* pData, int nLen)
{
	int n = nLen;
	const char* p = pData;
	clear();
	for (;n>2;n-=3)
	{
		auto a = (IFUI8)(p[0]&0xFC)>>2;
		auto b = (IFUI8)(p[0]&0x3)<<4|((p[1]&0xf0)>>4);
		auto c = (IFUI8)((p[1]&0xf)<<2)|((p[2]&0xc0)>>6);
		auto d = (IFUI8)p[2]&0x3f;
		p+=3;
		push_back(base64enctable[a]);
		push_back(base64enctable[b]);
		push_back(base64enctable[c]);
		push_back(base64enctable[d]);
	}
	char buf[3]={0};
	if (n)
	{
		for (int i = 0; i < n; i++)
		{
			buf[i]=*p;p++;
		}
		p=buf;
		auto a = (IFUI8)(p[0]&0xFC)>>2;
		auto b = (IFUI8)(p[0]&0x3)<<4|((p[1]&0xf0)>>4);
		auto c = (IFUI8)((p[1]&0xf)<<2)|((p[2]&0xc0)>>6);
		push_back(base64enctable[a]);
		push_back(base64enctable[b]);
		if (n==2)
		{
			push_back(base64enctable[c]);
			push_back('=');
		}
		else
		{
			push_back('=');
			push_back('=');
		}

	}
	return *this;
}


IFString IFString::encodeBase64() const
{
	IFString s;
	s.encodeBase64(c_str(), length());
	return s;
}

IFString IFString::encodeHEX(bool upper) const
{
	IFString r;
	r.reserve(length() * 2);
	for (int i = 0; i < (int)length(); i++)
	{
		r += IFString().format(upper ? "%02X":"%02x", (IFUI8)(*this)[i]);
	}
	return r;
}

IFString& IFString::encodeBase64(const IFString& s)
{
	return encodeBase64(s.c_str(), s.length());
}

bool IFString::decodeBase64(IFSimpleArray<char>& buf) const
{
	int nLen = length();
	if (nLen%4!=0 || nLen < 4)
		return false;
	auto p = (const IFUI8*)c_str();
	int nOutLen = nLen/4*3;
	if ((*this)[length()-1]=='=')
		nOutLen--;
	if ((*this)[length()-2]=='=')
		nOutLen--;
	buf.resize(nOutLen);
	char* pOut = buf;
	char* pOutEnd = pOut + nOutLen;
	for (;nLen;nLen-=4)
	{
		char a = base64dectable[p[0]];
		char b = base64dectable[p[1]];
		char c = base64dectable[p[2]];
		char d = base64dectable[p[3]];
		*pOut = (a<<2)|((b>>4)&0x3);pOut++;
		if (pOut<pOutEnd)
		{
			*pOut = ((b&0xf)<<4)|((c>>2)&0xf);pOut++;
			if (pOut<pOutEnd)
			{
				*pOut = (c<<6)|(d&0x3f);pOut++;
			}
		}


		p+=4;
		//pOut+=3;
	}


	return true;
}

IFRefPtr<IFMemStream> IFString::decodeBase64() const
{
	IFSimpleArray<char> buf;
	if (!decodeBase64(buf))
	{
		return NULL;
	}
	IFRefPtr<IFMemStream> spStream = IFNew IFMemStream();
	spStream->write(buf, buf.size());
	spStream->seek(0, IFStream::ISSF_BEGIN);
	return spStream;
}

IFArray<IFString> IFString::split(const IFString& sep) const
{
	StringList sl;
	USplitStrings(&sl, *this, sep.c_str());
	return sl;
}

IFString IFString::toUpper() const
{
	IFString s = *this;
	s.upper();
	return s;
}

IFString IFString::toLower() const
{
	IFString s = *this;
	s.lower();
	return s;
}


#endif


//////////////////////////////////////////////////////////////////////////


IFStringW::IFStringW() :m_nSize(0),m_nCap(0), m_nRSHash(0)
{
	m_SmallBuff[0] = 0;
	//makeSureSelfBuffer(8);
	//m_spBuffer->m[0] = 0;
}
IFStringW::IFStringW(const IFWCHAR* sStr) : m_nSize(0), m_nCap(0), m_nRSHash(0)
{
	operator = (sStr);
}

IFStringW::IFStringW(const IFStringW& o)
	:m_nSize(0), m_nCap(0)
{
	IFString_assign(*this, o);
}

IFStringW::IFStringW(const IFString& o) :m_nSize(0), m_nCap(0), m_nRSHash(0)
{
	operator=(o);
}



IFStringW::IFStringW(const IFWCHAR* sStr, int nLen) :m_nSize(0), m_nCap(0), m_nRSHash(0)
{
	IFString_assign(*this, sStr, nLen);

}
IFStringW::IFStringW(const IFWCHAR* sStr, const IFWCHAR* sEnd) :m_nSize(0), m_nCap(0), m_nRSHash(0)
{
	IFString_assign(*this, sStr, int(sEnd - sStr));
}

IFStringW::~IFStringW()
{

}

IFStringW& IFStringW::operator =(const IFStringW& o)
{
	IFString_assign(*this, o);
	return *this;
}

IFStringW& IFStringW::operator =(const IFString& o)
{
	m_nRSHash = 0;
	clear();
#ifdef IF_STRING_NO_ANSI
	utf8_wchar(o, *this);
#else
    if (o.isUTF8Codeing())
        utf8_wchar(o,*this);
    else
        gbk_wchar(o, *this);
#endif
	return *this;
}

IFStringW& IFStringW::operator =(const IFWCHAR* sStr)
{
	IFString_assign(*this, sStr,-1);
	return *this;
}


bool IFStringW::operator ==(const IFStringW& o) const
{
	if (m_spBuffer && m_spBuffer == o.m_spBuffer)
		return true;
#ifdef ANDROID
	if(o.length() == length())
	{

		const IFWCHAR* pMe = c_str();
		const IFWCHAR* pOt = o.c_str();

		while ((*pMe && *pOt) && (*pMe == *pOt))
		{
			pMe++,pOt++;
		}

		return *pMe == *pOt;
	}
	else
		return false;
#else
	if (m_nSize && (m_nSize == o.m_nSize))
	{
		return wcscmp(c_str(),o.c_str()) == 0;
	}
	else
		return o.m_nSize==m_nSize;
#endif
}

bool IFStringW::operator ==(const IFWCHAR* o) const
{
#ifdef ANDROID
	const IFWCHAR* pMe = c_str();
	while ((*pMe && *o) && (*pMe == *o))
	{
		pMe++,o++;
	}

	return *pMe == *o;
#else
	if (m_nSize)
		return wcscmp( c_str(), o) == 0;
	else
		return o[0] == 0;
#endif
}
bool IFStringW::operator <(const IFStringW& o) const
{
	const IFWCHAR* pMe = c_str();
	const IFWCHAR* pOther = o.c_str();
	while( *pMe && *pOther )
	{
		if( *pMe > *pOther )
			return false;
		else if( *pMe < *pOther )
			return true;

		++pMe;
		++pOther;
	}

	return *pMe==0 && *pOther!=0;
}


IFStringW IFStringW::operator +(const IFStringW& o)const
{
	IFStringW str(*this);
	str += o;
	return str;
	
	//return operator +((const IFWCHAR*)o.m_MemStr.getBuffer());
}

IFStringW IFStringW::operator +(const IFWCHAR* sStr)const
{
	IFStringW str(*this);
	str += sStr;
	return str;
}

IFStringW& IFStringW::operator +=(const IFStringW& o)
{
	IFString_append(*this, o.c_str(), o.size());

	return *this;
}

IFStringW& IFStringW::operator +=(IFWCHAR c)
{

	push_back(c);

	return *this;
}


IFStringW& IFStringW::operator +=(const  IFWCHAR* sStr)
{
	IFString_append(*this, sStr, -1);
	return *this;
}

//IFWCHAR& IFStringW::operator [](int nIndex)
//{
//	return  ((IFWCHAR*)m_MemStr.getBuffer())[nIndex];
//}
//
//const IFWCHAR& IFStringW::operator [](int nIndex) const
//{
//	return  ((const IFWCHAR*)m_MemStr.getBuffer())[nIndex];
//}

void IFStringW::reserve(int nSize)
{
	makeSureSelfBuffer(nSize+1);
	//m_MemStr.reserve(nSize);
}

void IFStringW::clear()
{
	IFString_clear(*this);
}

void IFStringW::push_back(IFWCHAR c)
{

	IFString_push_back(*this, c);

}

void IFStringW::erase(int nPos,int nSize)
{
	IFString_erase(*this, nPos, nSize);
}


IFStringW& IFStringW::insert(int nPos, const IFStringW& s)
{
	return IFString_insert<IFWCHAR>(*this, nPos, s);
	//if (!s.size())
	//	return;
	//int nNewSize = m_nSize + s.size();
	//makeSureSelfBuffer(nNewSize);
	//int nMoveSize = m_nSize - nPos;
	//if (nMoveSize)
	//	memmove(&m_spBuffer->m[nPos+s.size()], &m_spBuffer->m[nPos], nMoveSize*sizeof(wchar_t));
	//memcpy(&m_spBuffer->m[nPos], s.c_str(), s.size()*sizeof(wchar_t));
	//m_nSize += s.size();
	//m_spBuffer->m[m_nSize] = 0;
	//m_nRSHash = 0;
}


int IFStringW::find(const IFStringW& other, int nOffset, bool nocase) const
{
	return IFString_find(*this, other, nOffset, nocase);

}

int IFStringW::find_first_of( IFWCHAR c, int noffset) const
{
	return IFString_find_first_of(*this, c, noffset);
}


int IFStringW::find_last_of( IFWCHAR c,int noffset ) const
{
	return IFString_find_last_of(*this, c, noffset);
}




IFString IFStringW::toUTF8String() const
{
	IFString sutf8;
	sutf8.setUTF8Codeing(true);
	wchar_utf8(*this, sutf8);
	return sutf8;

}

#ifndef IF_STRING_NO_ANSI
IFString IFStringW::toANSIString() const
{

    IFString ansi;
	ansi.reserve(size() * 2 + 2);
	ansi.setUTF8Codeing(false);
    wchar_gbk(*this, ansi);
    return ansi;

}
#endif

void IFStringW::resize( int nSize )
{
	auto* pNewPtr = makeSureSelfBuffer(nSize+1);
	m_nSize = nSize;
	pNewPtr[nSize] = 0;
}

void IFStringW::upper()
{
	IFString_upper(*this);

}


IFStringW& IFStringW::format( const IFWCHAR* sFormat, ... )
{
#ifdef IFPLATFORM_FREE_RTOS
	IFWCHAR buf[512];
#else
	IFWCHAR buf[32*1024];
#endif
	va_list vlist;
	va_start(vlist, sFormat );
	//vswprintf(buf,32*1024, sFormat, vlist  );
	int nLen = android_vwsprintf(buf, IFArraySize(buf), sFormat, vlist);
	va_end(vlist);
	IFString_assign(*this, buf, nLen);
	//*this= buf;
	/*
#ifdef WIN32
	IFWCHAR buf[32*1024];
	va_list vlist;
	va_start(vlist, sFormat );
	_vsnwprintf_s(buf, _TRUNCATE, sFormat,  vlist );
	va_end(vlist);
	*this = buf;

#else
#ifdef ANDROID
	IFWCHAR buf[32*1024];
	__va_list vlist;
	va_start(vlist, sFormat );
	//vswprintf(buf,32*1024, sFormat, vlist  );
	android_vwsprintf(buf,sFormat,vlist);
	va_end(vlist);
	*this= buf;
#else
    IFWCHAR buf[32*1024];
    IFWCHAR convbuf[32*1024];
    va_list vlist;
	va_start(vlist, sFormat );
    int n = 0;
    while(*sFormat)
    {
        convbuf[n] = *sFormat;
        n ++;
        if (*sFormat==L'%' )
        {
            if(*(sFormat+1)==L'S')
            {
                convbuf[n] = L's';
                n ++;
                sFormat ++;
            }
            else if (*(sFormat+1)==L's')
            {
                convbuf[n] = L'S';
                n ++;
                sFormat ++;
            }

        }


        sFormat ++;

    }
    convbuf[n]=0;



	vswprintf(buf, 1024, convbuf,vlist);
	va_end(vlist);
	*this= buf;
#endif
#endif
	*/
	return *this;
}

IFStringW IFStringW::sub( int index, int size ) const
{
	if (size<=0)
		return IFStringW::Empty;
	else if (index+size>m_nSize)
	{
		size = length()-index;
	}
	return IFStringW(c_str()+index, size);
}

IFI32 IFStringW::toInt32(int nRadix) const
{
#ifndef WIN32
	return strtol(toUTF8String().c_str(), NULL, nRadix);
#else
	return wcstol(c_str(), NULL, nRadix);
#endif
}

IFUI32 IFStringW::toUint32(int nRadix) const
{
#ifndef WIN32
	return (IFUI32)strtoul(toUTF8String().c_str(), NULL, nRadix);
#else
	return wcstoul(c_str(), NULL, nRadix);
#endif
}

IFUI64 IFStringW::toUint64( int nRadix /*= 10*/ ) const
{
#ifndef WIN32
	return (IFUI32)strtoull(toUTF8String().c_str(), NULL, nRadix);
#else
	return _wcstoui64(c_str(), NULL, nRadix);
#endif
}


IFI64 IFStringW::toInt64( int nRadix /*= 10*/ ) const
{
#ifndef WIN32
	return strtoll(toUTF8String().c_str(), NULL, nRadix);
#else
	return _wcstoi64(c_str(), NULL, nRadix);
#endif
}


IFStringW IFStringW::toUpper() const
{
	IFStringW s=*this;
	s.upper();
	return s;
}

//IFUI32 IFStringW::toRSHash() const



IFUI32 IFStringW::RSHash(const IFWCHAR	* s, int nLen /*= -1*/)
{
	IFUI32 nlen = 0;
	return ::rshash(s, nlen);
}

IFWCHAR* IFStringW::makeSureSelfBuffer(int nBufInitialSize)
{
	return IFString_makeSureSelfBuffer<IFWCHAR>(*this, nBufInitialSize);
}

void IFStringW::replace(IFWCHAR oldVal, IFWCHAR newVal, IFUI32 nBegin /*= 0*/, IFUI32 nEnd/*=-1*/)
{
	IFString_replace(*this, oldVal, newVal, nBegin, nEnd);
}

void IFStringW::replace(const IFStringW& oldVal, const IFStringW& newVal, IFUI32 nBegin /*= 0*/, IFUI32 nEnd/*=-1*/)
{
	IFString_replace(*this, oldVal, newVal, nBegin, nEnd);

}

