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
#include "IFTypes.h"
#include "IFCommonLib_API.h"
#include "IFStream.h"

struct IFFixNumber : public IFMemObj
{
	IFCOMMON_API static const IFFixNumber N_0;
	IFCOMMON_API static const IFFixNumber N_1;
	IFCOMMON_API static const IFFixNumber N_2;

	enum {PRECISION=10000};
	IFFixNumber(IFI64 n = 0)
		:nNum(n)
	{

	}
	IFFixNumber(int big,int s = 0)
	{
		nNum = ((IFI64)big) * PRECISION + s;
	}

	IFFixNumber(float f)
	{
		int b = (int)f;
		int s = int((f - (float)b)*(float(PRECISION)));
		nNum = b*PRECISION+s;

	}

	inline IFFixNumber operator + (const IFFixNumber& o) const
	{
		IFI64 s = nNum + o.nNum;
		return s;
	}

	inline IFFixNumber operator - (const IFFixNumber& o) const
	{
		IFI64 s = nNum - o.nNum;
		return s;
	}

	inline IFFixNumber operator * (const IFFixNumber& o) const
	{
		IFFixNumber s;
		s.nNum = (nNum * o.nNum)/PRECISION;
		return s;
	}

	inline IFFixNumber operator / (const IFFixNumber& o) const
	{
		IFI64 s = (nNum*PRECISION / o.nNum);
		return s;
	}

	inline IFFixNumber& operator+=(const IFFixNumber& o)
	{
		nNum += o.nNum;
		return *this;
	}

	inline IFFixNumber& operator-=(const IFFixNumber& o)
	{
		nNum -= o.nNum;
		return *this;
	}

	inline IFFixNumber& operator*=(const IFFixNumber& o)
	{
		nNum *= o.nNum;
		nNum/=PRECISION;
		return *this;
	}

	inline IFFixNumber& operator/=(const IFFixNumber& o)
	{
		nNum *= PRECISION;
		nNum /= o.nNum;
		return *this;
	}

	inline operator int() const
	{
		return int(nNum/PRECISION);
	}
	inline operator float() const
	{
		return nNum/float(PRECISION);
	}


	inline bool operator < (const IFFixNumber& o) const
	{
		return nNum < o.nNum;
	}

	inline bool operator <= (const IFFixNumber& o) const
	{
		return nNum <= o.nNum;
	}

	inline bool operator > (const IFFixNumber& o) const
	{
		return nNum > o.nNum;
	}
	inline bool operator >= (const IFFixNumber& o) const
	{
		return nNum >= o.nNum;
	}

	inline bool operator == (const IFFixNumber& o) const
	{
		return nNum == o.nNum;
	}

	inline operator bool() const
	{
		return nNum!=0;
	}

	bool operator != (const IFFixNumber& o) const
	{
		return nNum != o.nNum;
	}

	inline IFFixNumber operator -() const
	{
		return IFFixNumber(-nNum);
	}

	IFCOMMON_API IFString toString() const;

	IFCOMMON_API void  loadFromString(const IFString& s);

	IFI64 nNum;
};

inline IFStream& operator << (IFStream& s, const IFFixNumber& b)
{
	s.writeI64(b.nNum);
	return s;
}

inline IFStream& operator >> (IFStream& s, IFFixNumber& b)
{
	b.nNum = s.readI64();
	return s;
}

inline IFFixNumber IFSqrt(const IFFixNumber& t)
{
	return IFFixNumber(float(t));
}


template<>
inline IFFixNumber IFVector2DLength(const IFVector2DT<IFFixNumber>& v)
{
	IFI64 xx = v.x.nNum * v.x.nNum + v.y.nNum * v.y.nNum;
	return IFSqrt(xx);
}

typedef IFVector2DT<IFFixNumber> IFFixVec2;
typedef IFLine2DT<IFFixNumber> IFFixLine2D;

IFCOMMON_API extern const IFFixVec2 IFFIXVEC2_AXIS_UNIT_X;
IFCOMMON_API extern const IFFixVec2 IFFIXVEC2_AXIS_UNIT_Y;
IFCOMMON_API extern const IFFixVec2 IFFIXVEC2_ZERO;
//typedef :public IFVector2DT<IFFixNumber>
//
//struct IFFixVec2 :public IFVector2DT<IFFixNumber>
//{
//	IFCOMMON_API static const IFFixVec2 AXIS_UNIT_X;
//	IFCOMMON_API static const IFFixVec2 AXIS_UNIT_Y;
//	IFCOMMON_API static const IFFixVec2 ZERO;
//
//	IFFixVec2() 
//	{
//		x = y = 0.0f;
//	}
//	IFFixVec2(const IFFixNumber& x, const IFFixNumber& y)
//	{
//		this->x = x;
//		this->y = y;
//	}
//};

inline IFStream& operator << (IFStream& s, const IFFixVec2& b)
{
	s << b.x << b.y;
	return s;
}

inline IFStream& operator >> (IFStream& s, IFFixVec2& b)
{
	s >> b.x >> b.y;
	return s;
}
