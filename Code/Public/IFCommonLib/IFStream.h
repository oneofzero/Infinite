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
#ifndef __IF_STREAM_H___
#define __IF_STREAM_H___
#include "IFRefObj.h"


class IFString;
class IFStringW;

template<typename T>
class IFArray;

class IFStreamReadException
{
public:
	IFStreamReadException()
	{

	}
};


#define IF_FIXENDIAN(n) if (m_Endian != LOCAL)flipEndian(&n,sizeof(n))

class IFCOMMON_API IFStream : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum ENDIAN
	{
		LITTLE,
		BIG,

		LOCAL = LITTLE,
	};

protected:
	virtual ~IFStream(void);
public:
	IFStream(void);

	virtual const IFStringW& getName() = 0;

	virtual IFUI32 read(void* pDestData, IFUI32 nSize ) = 0  ; //读取指定大小一段数据
	virtual IFUI32 write(const void* pSourceData, IFUI32 nSize ) = 0;//写入一段大小数据
	IFUI32 write(const IFPair<const void*, IFUI32>& data)
	{
		return write(data.first, data.second);
	}


	void setEndian(ENDIAN edian);
	ENDIAN getEndian();

	
	IFString readS();
	IFStringW readWS();
	void writeS(const IFString& s);
	void writeWS(const IFStringW& s);
	void writeS(const char* s,int nLen);
	void writeS(const IFPair<const char*, IFUI32>& s)
	{
		writeS(s.first, s.second);
	}

	inline IFI8 readI8()
	{
		IFI8 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		return n;
	}
	inline IFUI8 readUI8()
	{
		IFUI8 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		return n;
	}

	inline IFI16 readI16()
	{
		IFI16 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		IF_FIXENDIAN(n);

		return n;
	}

	inline IFUI16 readUI16()
	{
		IFUI16 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		IF_FIXENDIAN(n);

		return n;
	}

	inline IFI32 readI32()
	{
		IFI32 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		IF_FIXENDIAN(n);

		return n;
	}

	inline IFUI32 readUI32()
	{
		IFUI32 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		IF_FIXENDIAN(n);

		return n;
	}

	inline IFI64 readI64()
	{
		IFI64 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		IF_FIXENDIAN(n);

		return n;
	}

	inline IFUI64 readUI64()
	{
		IFUI64 n;
		if(read(&n,sizeof(n)) != sizeof(n))
			throw IFStreamReadException();
		IF_FIXENDIAN(n);

		return n;
	}

	inline void writeI8(IFI8 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));
	}

	inline void writeUI8(IFUI8 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));
	}

	inline void writeI16(IFI16 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));

	}

	inline void writeUI16(IFUI16 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));
	}

	inline void writeI32(IFI32 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));

	}

	inline void writeUI32(IFUI32 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));
	}

	inline void writeI64(IFI64 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));
	}

	inline void writeUI64(IFUI64 n)
	{
		IF_FIXENDIAN(n);
		write(&n,sizeof(n));
	}

	inline void writeFloat(float f)
	{
		write(&f,sizeof(f));
	}

	inline float readFloat()
	{
		float f;
		if(read(&f,sizeof(f)) != sizeof(f))
			throw IFStreamReadException();
		return f;
	}

	inline void writeDouble(double f)
	{
		write(&f,sizeof(f));
	}

	inline double readDouble()
	{
		double f;
		if(read(&f,sizeof(f)) != sizeof(f))
			throw IFStreamReadException();
		return f;
	}

	static void flipEndian(void* pData, int nLen);


	enum IFSTREM_SEEK_FROM
	{
		ISSF_BEGIN,
		ISSF_CUR,
		ISSF_END,
	};

	virtual IFI64 seek( IFI64 nSeek, IFUI32 nFrom )=0 ;
	virtual IFI64 tell()const=0 ;
	virtual IFI64 size()const{return m_nSize;};
	virtual bool isEnd()const = 0 ;
	virtual void flush() = 0;
	virtual bool isVaild()const = 0 ;	
protected:
	IFI64 m_nSize;
	ENDIAN m_Endian;


};
#include "IFSerialize.h"

#endif