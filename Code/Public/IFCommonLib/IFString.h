﻿/*
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
#ifndef __IF_STRING_H__
#define __IF_STRING_H__
#include "IFRefPtr.h"
#include "IFArray.h"
class IFStringW;
class IFMemStream;
#ifdef WIN32

#define LOCAL_CHAR_ENCODING IFString::EC_ANSI
#else
#define LOCAL_CHAR_ENCODING IFString::EC_UTF8
//#define IF_STRING_NO_ANSI
#endif
#define IFSTRING_STANDALONE



template<typename T>
class IFStringBuffer : public IFRefObj
{
public:

	static IFRefPtr<IFStringBuffer<T>> alloc(int nSize)
	{
		void *pBuf = IFAlloc::Alloc(sizeof(IFStringBuffer<T>) + nSize*sizeof(T));
		return new(pBuf) IFStringBuffer<T>(nSize);
	}
	int m_nSize;

	T m[1];
	inline int size() const
	{
		return m_nSize;
	}
	IFRefPtr<IFStringBuffer<T>> clone(int nNewSize)
	{
		auto pNewBuffer = alloc(nNewSize);
		memcpy(pNewBuffer->m, m, IFMin(nNewSize, m_nSize)*sizeof(T));

		return pNewBuffer;
	}

protected:
	IFStringBuffer(int nSize)
		:IFRefObj(true)
		,m_nSize(nSize)
	{

	}
	~IFStringBuffer()
	{
		
	}
	virtual void release()
	{
		this->~IFStringBuffer();
		IFAlloc::Dealloc(this);
	}

};
#define IFSTRING_SMALL_LEN 35
#ifndef IFSTRING_STANDALONE


#else

template<typename T,typename STRINGT>
T* IFString_makeSureSelfBuffer(STRINGT& o, int nBufInitialSize);

template<typename T, typename STRINGT>
STRINGT& IFString_insert(STRINGT& o, int nPos, const STRINGT& s);

template<typename T, typename STRINGT>
void IFString_replace(STRINGT& o,T oldVal, T newVal, IFUI32 nBegin, IFUI32 nEnd);

template<typename STRINGT>
void IFString_replace(STRINGT& o,const STRINGT& oldVal, const STRINGT& newVal, IFUI32 nBegin, IFUI32 nEnd);

template<typename STRINGT>
int IFString_find(const STRINGT& o, const STRINGT& other, int noffset, bool nocase);

template<typename STRINGT>
int IFString_find_first_of(const STRINGT& o, typename STRINGT::THIS_CHAR_TYPE c, int noffset);

template<typename STRINGT>
int IFString_find_last_of(const STRINGT& o, typename STRINGT::THIS_CHAR_TYPE c, int noffset);

template<typename STRINGT>
void IFString_erase(STRINGT& o, int nPos, int nSize);
template<typename STRINGT>
void IFString_assign(STRINGT& o, const typename STRINGT::THIS_CHAR_TYPE* pStr, int nlen);
template<typename STRINGT>
void IFString_assign(STRINGT& o, const STRINGT& pStr);
template<typename STRINGT>
void IFString_append(STRINGT& o, const typename STRINGT::THIS_CHAR_TYPE* str, int nlen);
template<typename STRINGT>
void IFString_clear(STRINGT& o);
template<typename STRINGT>
void IFString_push_back(STRINGT& o, typename STRINGT::THIS_CHAR_TYPE c);
template<typename STRINGT>
void IFString_upper(STRINGT& o);

#define DEFINE_FRIEND_FUN() \
	friend THIS_CHAR_TYPE* IFString_makeSureSelfBuffer<THIS_CHAR_TYPE>(THIS_TYPE&o, int nBufInitialSize);\
	friend THIS_TYPE& IFString_insert<THIS_CHAR_TYPE>(THIS_TYPE& o, int nPos, const THIS_TYPE& s);\
	friend void IFString_replace<THIS_TYPE>(THIS_TYPE& o,const THIS_TYPE& oldVal, const THIS_TYPE& newVal, IFUI32 nBegin, IFUI32 nEnd);\
	friend void IFString_replace<THIS_CHAR_TYPE,THIS_TYPE>(THIS_TYPE& o,THIS_CHAR_TYPE oldVal, THIS_CHAR_TYPE newVal, IFUI32 nBegin, IFUI32 nEnd);\
	friend int IFString_find<THIS_TYPE>(const THIS_TYPE& o,const THIS_TYPE& other, int noffset, bool nocase);\
	friend int IFString_find_first_of<THIS_TYPE>(const THIS_TYPE& o, typename THIS_TYPE::THIS_CHAR_TYPE c, int noffset);\
	friend int IFString_find_last_of<THIS_TYPE>(const THIS_TYPE& o, typename THIS_TYPE::THIS_CHAR_TYPE c, int noffset);\
	friend void IFString_erase<THIS_TYPE>(THIS_TYPE& o, int nPos, int nSize);\
	friend void IFString_assign<THIS_TYPE>(THIS_TYPE& o, const THIS_TYPE& pStr);\
	friend void IFString_assign<THIS_TYPE>(THIS_TYPE& o, const typename THIS_TYPE::THIS_CHAR_TYPE* pStr, int nlen);\
	friend void IFString_append<THIS_TYPE>(THIS_TYPE& o, const typename THIS_TYPE::THIS_CHAR_TYPE* str, int nlen);\
	friend void IFString_clear<THIS_TYPE>(THIS_TYPE& o);\
	friend void IFString_push_back<THIS_TYPE>(THIS_TYPE& o, typename THIS_TYPE::THIS_CHAR_TYPE c);\
	friend void IFString_upper<THIS_TYPE>(THIS_TYPE& o);\




class IFCOMMON_API IFString : public IFMemObj
{
public:
	typedef IFString THIS_TYPE;
	typedef char THIS_CHAR_TYPE;
	
public:
	enum ENCODING
	{
		EC_ANSI = 0,
		EC_UTF8 = 1,
	};
public:
	IFString();
	IFString(const char* sStr, ENCODING coding = LOCAL_CHAR_ENCODING);
		

	IFString(const IFString& o);
	IFString(const IFStringW& o,ENCODING coding = LOCAL_CHAR_ENCODING);
	IFString(const char* sStr, const char* sEnd,ENCODING coding = LOCAL_CHAR_ENCODING);
	IFString(const char* sStr, int nLen,ENCODING coding = LOCAL_CHAR_ENCODING);
	IFString(ENCODING coding);
	inline ~IFString(void)
	{

	}
	IFString& operator =(const IFString& o);
	IFString& operator =(const IFStringW& o);
	IFString& operator =(const  char* sStr);


	bool operator ==(const IFString& o) const;
	bool operator ==(const char* s) const;

	bool operator <(const IFString& o) const;
	bool operator !=(const IFString& o) const
	{
		return !((*this)==(o));
	}

	bool operator != (const char* s) const
	{
		return !((*this)==(s));
	}

	IFString operator +(const IFString& o)const;
	IFString operator +(const  char* sStr)const;
	IFString& operator +=(const IFString& o);
	IFString& operator +=(const  char* sStr);
	IFString& operator +=( char sStr);

	IFString& append(const char* sStr, int nLen=-1);


	IFString operator -( const IFString& o ) const;

	inline char& operator[](int nIndex)
	{
		makeSureSelfBuffer(m_nSize);
		m_nRSHash = 0;
		return  selfptr()[nIndex];
	}
	inline const char& operator[](int nIndex)const
	{
		return  c_str()[nIndex];
	}

	inline operator const char*()
	{
		return c_str();
	}

	inline const char* c_str()const
	{
		return (m_spBuffer)?m_spBuffer->m:m_SmallBuff;
	} ;

	char* w_str()
	{
		makeSureSelfBuffer(m_nSize);
		return selfptr();
	}

	
	IFUI32 size()const{return m_nSize;};
	IFUI32 length()const{return m_nSize;}

	bool isEmpty() const
	{
		return size() == 0;
	}

	void push_back(char c);

	void reserve(int nSize);

	void resize(int nSize);

	void clear();

	IFString trimStart(char c = 0) const;
	IFString trimEnd(char c = 0) const;
	IFString trim(char c = 0) const
	{
		return trimStart(c).trimEnd(c);
	}

	//void push_back(char c);

	void erase(int nPos,int nSize = 1);
	IFString& insert(int nPos, const IFString& s);

	int find(const IFString& other, int noffset = 0, bool nocase = false) const ;
	int find_first_of(char c, int noffset = 0) const;
	int find_last_of(char c, int noffset = 0) const;
	bool endsWith(const IFString& end) const;
	bool startsWith(const IFString& start) const;


	void replace(char oldVal, char newVal, IFUI32 nBegin = 0, IFUI32 nEnd=-1);
	void replace(const IFString& oldVal, const IFString& newVal, IFUI32 nBegin = 0, IFUI32 nEnd=-1);
	IFString replace(char oldVal, char newVal, IFUI32 nBegin = 0, IFUI32 nEnd = -1) const;


	bool isUTF8Codeing() const;
	void setUTF8Codeing(bool b);
	ENCODING getEncoding() const { return m_eEncoding; }

	IFString convertEncoding(ENCODING encoding) const;

	IFString& format(const char* sFormat, ... );

	void upper();
	IFString toUpper() const;

	void lower();
	IFString toLower() const;

	IFString convertTo(ENCODING cd) const;
	IFString sub(int index, int size) const;
	IFString sub(int index) const
	{
		return sub(index, size() - index);
	}

	IFI32 toInt32(int nRadix = 10) const;
	IFUI32 toUint32(int nRadix = 10) const;
	IFUI64 toUint64(int nRadix = 10) const;
	IFI64 toInt64(int nRadix = 10) const;

	IFString& loadInt64(IFI64 n);
	IFString& loadInt(int n);
	IFString& loadDouble(double f);
	IFString& loadFloat(float f);


	inline IFUI32 toRSHash() const
	{
		if (m_nRSHash)
			return m_nRSHash;
		m_nRSHash = RSHash(c_str(), (int)size());
		return m_nRSHash;
	}
	
	IFString toURLString() const;

	IFString& encodeBase64(const char* pData, int nLen);
	IFString& encodeBase64(const IFString& s);
	IFString encodeBase64() const;

	IFString encodeHEX(bool upper) const;


	bool decodeBase64(IFSimpleArray<char>& buf) const;
	IFRefPtr<IFMemStream> decodeBase64() const;

	IFArray<IFString> split(const  IFString& sep) const;

	static const IFString Empty;

	static const char UTF8Flag[3];

	static IFUI32 RSHash(const char* s, int nLen=-1);

	static IFString FromBuffer(const void* p, int nLen, ENCODING coding = LOCAL_CHAR_ENCODING)
	{
		return IFString((const char*)p, nLen, coding);
	}

	static IFString FromURLString(const IFString& s);

	static IFString LoadInt64(IFI64 n)
	{
		IFString s;
		s.loadInt64(n);
		return s;
	}
	static IFString LoadInt(int n)
	{
		IFString s;
		s.loadInt(n);
		return s;
	}
	static IFString LoadDouble(double f)
	{
		IFString s;
		s.loadDouble(f);
		return s;
	}
	static IFString LoadFloat(float f)
	{
		IFString s;
		s.loadFloat(f);
		return s;
	}

private:
	char* makeSureSelfBuffer(int nBufInitialSize);
	
	DEFINE_FRIEND_FUN();

	inline char* selfptr()
	{
		return (char*)c_str();
	}

	inline bool isUseSmallBuff() const
	{
		return m_spBuffer==NULL;
	}

	typedef IFStringBuffer<char> RefBuffer;
	typedef IFRefPtr<RefBuffer> BufferPtr;

	BufferPtr m_spBuffer;
	char m_SmallBuff[IFSTRING_SMALL_LEN+1];
	int m_nSize;
	int m_nCap;
	mutable IFUI32 m_nRSHash;
	ENCODING m_eEncoding;
	friend class IFStringW;

};

inline IFString operator+(const char* a, const IFString& b)
{
	IFString s = a;
	return s + b;
}
#endif

class IFCOMMON_API IFStringW : public IFMemObj
{
public:
	typedef IFStringW THIS_TYPE;
	typedef IFWCHAR THIS_CHAR_TYPE;

public:
	IFStringW();
	IFStringW(const IFWCHAR* sStr);
	IFStringW(const IFStringW& o);
	IFStringW(const IFString& o );
	IFStringW(const IFWCHAR* sStr, const IFWCHAR* sEnd);
	IFStringW(const IFWCHAR* sStr, int nLen);
	~IFStringW(void);
	IFStringW& operator =(const IFStringW& o);
	IFStringW& operator =(const IFString& o);
	IFStringW& operator =(const  IFWCHAR* sStr);
	bool operator ==(const IFStringW& o) const;
	bool operator ==(const IFWCHAR* o) const;
	bool operator <(const IFStringW& o) const;
	bool operator !=(const IFStringW& o) const
	{
		return !((*this)==o);
	}

	bool operator !=(const IFWCHAR* s) const
	{
		return !((*this)==s);
	}

	IFStringW operator +(const IFStringW& o)const;
	IFStringW operator +(const  IFWCHAR* sStr)const;
	IFStringW& operator +=(const IFStringW& o);
	IFStringW& operator +=(const  IFWCHAR* sStr);
	IFStringW& operator +=( IFWCHAR sStr);

	inline IFWCHAR& operator[](int nIndex)
	{
		IFWCHAR* pPtr = makeSureSelfBuffer(m_nSize);
		return  pPtr[nIndex];
	}
	inline const IFWCHAR& operator[](int nIndex)const
	{
		return  c_str()[nIndex];
	}

	inline operator const IFWCHAR*()
	{
		return c_str();
	}

	inline const IFWCHAR* c_str()const
	{
		return (m_spBuffer)?m_spBuffer->m:m_SmallBuff;
	} ;

	inline IFUI32 size()const
	{
		return (IFUI32)m_nSize;
	}

	inline IFUI32 length()const
	{
		return (IFUI32)m_nSize;
	}

	void reserve(int nSize);

	void resize(int nSize);

	void clear();

	void push_back(IFWCHAR c);

	void erase(int nPos,int nSize = 1);
	IFStringW& insert(int nPos, const IFStringW& s);

	int find(const IFStringW& other, int noffset = 0, bool nocase = false) const ;
	int find_first_of(IFWCHAR c,int noffset = 0) const;
	int find_last_of(IFWCHAR c,int noffset = 0) const;

	void replace(IFWCHAR oldVal, IFWCHAR newVal, IFUI32 nBegin = 0, IFUI32 nEnd=-1);
	void replace(const IFStringW& oldVal, const IFStringW& newVal, IFUI32 nBegin = 0, IFUI32 nEnd=-1);


	void upper();
	IFStringW toUpper() const;

	IFString toUTF8String() const;
#ifndef IF_STRING_NO_ANSI
	IFString toANSIString() const;
#endif

	bool isEmpty() const
	{
		return size() == 0;
	}

	IFStringW sub(int index, int size) const;

	IFStringW& format(const IFWCHAR* sFormat, ... );

	IFI32 toInt32(int nRadix = 10) const;
	IFUI32 toUint32(int nRadix = 10) const;
	IFUI64 toUint64(int nRadix = 10) const;
	IFI64 toInt64(int nRadix = 10) const;
	inline IFUI32 toRSHash() const
	{
		if (m_nRSHash)
			return m_nRSHash;

		m_nRSHash = RSHash(c_str(), (int)length());
		return m_nRSHash;
	}
	static IFUI32 RSHash(const IFWCHAR* s, int nLen = -1);

	static const IFStringW Empty;

private:
	IFWCHAR* makeSureSelfBuffer(int nBufInitialSize);
	//friend IFWCHAR* IFString_makeSureSelfBuffer<IFWCHAR>(IFStringW& s, int nBufInitialSize);
	inline bool isUseSmallBuff() const
	{
		return m_spBuffer==NULL;
	}
	inline IFWCHAR* selfptr()
	{
		return (IFWCHAR*)c_str();
	}
	DEFINE_FRIEND_FUN();

	typedef IFStringBuffer<IFWCHAR> RefBuffer;
	typedef IFRefPtr<RefBuffer> BufferPtr;

	BufferPtr m_spBuffer;
	IFWCHAR m_SmallBuff[IFSTRING_SMALL_LEN + 1];
	int m_nSize;
	int m_nCap;
	mutable IFUI32 m_nRSHash;

	//friend class IFString;
};
inline IFStringW operator+(const IFWCHAR* a, const IFStringW& b)
{
	IFStringW s = a;
	return s + b;
}
#ifdef WIN32
#define toLocalString toANSIString
#else
#define toLocalString toUTF8String
#endif

#endif //__IF_STRING_H__