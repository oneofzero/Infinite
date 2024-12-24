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
#ifndef __IF_FLOAT_NUMBER_H__
#define __IF_FLOAT_NUMBER_H__
#include "IFObj.h"
#include "IFString.h"
#include "IFCommonLib_API.h"

class IFCOMMON_API IFFloatNumber
{
public:
	int m_num;
	int m_exp; //10^exp

public:
	IFFloatNumber(int num);
	IFFloatNumber(int num, int exp);
	IFFloatNumber(float f);
	IFFloatNumber(const IFFloatNumber& o);

	IFFloatNumber operator -() const
	{
		return IFFloatNumber(-m_num, m_exp);
	}

	IFFloatNumber& operator +=(const IFFloatNumber& o);

	IFFloatNumber operator +(const IFFloatNumber& o) const
	{
		IFFloatNumber temp(*this);
		temp += o;
		return temp;
	}

	IFFloatNumber& operator -=(const IFFloatNumber& o)
	{
		return *this += -o;		
	}

	IFFloatNumber operator -(const IFFloatNumber& o) const
	{
		IFFloatNumber temp(*this);
		temp -= o;
		return temp;
	}

	IFFloatNumber& operator *=(const IFFloatNumber& o);

	IFFloatNumber operator *(const IFFloatNumber& o) const
	{
		IFFloatNumber temp(*this);
		temp *= o;
		return temp;
	}
	IFFloatNumber& operator /=(const IFFloatNumber& o);

	IFFloatNumber operator /(const IFFloatNumber& o) const
	{
		IFFloatNumber temp(*this);
		temp /= o;
		return temp;
	}

	IFString toString() const;

	float toFloat() const;
};
#endif //__IF_FLOAT_NUMBER_H__