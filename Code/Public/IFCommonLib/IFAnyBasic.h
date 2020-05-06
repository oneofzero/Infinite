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
#include "IFBaseTypeDefine.h"
#include "IFString.h"
#include "IFFixNumber.h"
#include "IFCommonLib_API.h"
#include "IFMemStream.h"

class IFCOMMON_API IFAnyBasic : public IFMemObj
{
public:
	enum TYPE
	{
		T_I8,
		T_I16,
		T_I32,
		T_I64,
		T_UI8,
		T_UI16,
		T_UI32,
		T_UI64,
		T_BOOL,
		T_STRING,
		T_FLOAT,
		T_DOUBLE,
		T_FIX_NUMBER,
		T_MEMSTREAM,
		T_UNKOWN,
	};
public:
	IFAnyBasic(void);
	~IFAnyBasic(void);

	IFAnyBasic(const IFAnyBasic& o);

	IFAnyBasic(IFI8 n);
	IFAnyBasic(IFI16 n);
	IFAnyBasic(IFI32 n);
	IFAnyBasic(IFI64 n);
	IFAnyBasic(IFUI8 n);
	IFAnyBasic(IFUI16 n);
	IFAnyBasic(IFUI32 n);
	IFAnyBasic(IFUI64 n);
	IFAnyBasic(bool B);

	IFAnyBasic(float n);
	IFAnyBasic(double n);
	IFAnyBasic(const char* s);
	IFAnyBasic(const IFString& s);
	IFAnyBasic(const IFStringW& s);
	IFAnyBasic(const wchar_t* s);

	IFAnyBasic(const IFFixNumber& FixNum);
	IFAnyBasic(IFMemStream* pMemStream);


	static const IFAnyBasic NIL;

	inline bool isNil() const
	{
		return NIL==*this;
	}
	inline bool isString() const
	{
		return nType == T_STRING;
	}
	inline bool isNumber() const
	{
		return nType <= T_UI64 || (nType== T_FLOAT || nType == T_FIX_NUMBER || nType == T_DOUBLE);
	}
	inline bool isBool() const
	{
		return nType == T_BOOL;
	}

	void setNil();


	inline TYPE getType() const
	{
		return (TYPE)nType;
	}

	inline void set(IFI8 n)
	{
		setNil();
		n8 = n;
		nType = T_I8;
	}
	inline void set(IFI16 n)
	{
		setNil();
		n16 = n;
		nType = T_I16;
	}
	inline void set(IFI32 n)
	{
		setNil();
		n32 = n;
		nType = T_I32;
	}
	inline void set(IFI64 n)
	{
		setNil();
		n64 = n;
		nType = T_I64;
	}

	inline void set(IFUI8 n)
	{
		setNil();

		un8 = n;
		nType = T_UI8;
	}
	inline void set(IFUI16 n)
	{
		setNil();
		un16 = n;
		nType = T_UI16;
	}
	inline void set(IFUI32 n)
	{
		setNil();
		un32 = n;
		nType = T_UI32;
	}
	inline void set(IFUI64 n)
	{
		setNil();
		un64 = n;
		nType = T_UI64;
	}

	inline void set(bool B)
	{
		setNil();
		b = B;
		nType = T_BOOL;
	}

	inline void set(const IFString& s)
	{
		setNil();
		nType = T_STRING;
		sVal = IFNew IFString(s);
		//*sVal = s;
	}

	inline void set(const IFFixNumber& n)
	{
		setNil();
		nType = T_FIX_NUMBER;
		un64 = n.nNum;
	}

	inline void set(float n)
	{
		setNil();
		f = n;
		nType = T_FLOAT;
	}
	inline void set(double n)
	{
		setNil();

		df = n;
		nType = T_DOUBLE;
	}

	inline void set(IFMemStream* pMS)
	{
		setNil();
		pMemStream = pMS;
		pMemStream->addRef();
		nType = T_MEMSTREAM;
	}

	inline IFI32 getInt8() const
	{
		return n8;
	}
	inline IFUI32 getUInt8() const
	{
		return un8;
	}

	inline IFI32 getInt16() const
	{
		return n16;
	}
	inline IFUI32 getUInt16() const
	{
		return un16;
	}

	inline IFI32 getInt() const
	{
		return n32;
	}
	inline IFUI32 getUInt() const
	{
		return un32;
	}
	inline float getFloat() const
	{
		return f;
	}
	const IFString& getString() const
	{
		if(nType == T_STRING && sVal)
			return *sVal;
		return IFString::Empty;
	}

	const IFFixNumber& getFixNum() const
	{
		if (nType == T_FIX_NUMBER)
			return *(IFFixNumber*)&un64;
		return IFFixNumber::N_0;
	}

	IFMemStream* getMemStream() const
	{
		if (nType == T_MEMSTREAM)
			return pMemStream;
		return NULL;
	}

	const IFUI64 getUint64() const
	{
		return un64;
	}

	const IFI64 getInt64() const
	{
		return n64;
	}

	double getDouble() const
	{
		return df;
	}

	bool getBool() const
	{
		return b;
	}

	void serialize(IFStream* pStream) const;

	void deserialize(IFStream* pStream);

	const IFStringW toString() const;

	IFI32 toInt() const;
	float toFloat() const;
	IFFixNumber toFixNumber() const;

	bool operator == (const IFAnyBasic& o) const;

	bool operator != (const IFAnyBasic& o) const
	{
		return !operator==(o);
	}

	bool isEqual(const IFAnyBasic& o) const
	{
		return operator == (o);
	}

	IFAnyBasic& operator = (const IFAnyBasic& o);

	
	


private:
		IFI8 nType;

		union 
		{
			IFString* sVal;
			IFMemStream* pMemStream;

			IFI8 n8;
			IFI16 n16;
			IFI32 n32;
			IFI64 n64;
			IFUI8 un8;
			IFUI16 un16;
			IFUI32 un32;
			IFUI64 un64;
			float f;
			double df;
			bool b;
		};


};
inline IFStream& operator << (IFStream& stream, const IFAnyBasic& o)
{
	o.serialize(&stream);
	return stream;
}

inline IFStream& operator >> (IFStream& stream, IFAnyBasic& o)
{
	o.deserialize(&stream);
	return stream;
}
