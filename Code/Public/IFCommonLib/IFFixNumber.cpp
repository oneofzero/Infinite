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
#include "IFFixNumber.h"
#include "IFUtility.h"
#include "IFNumParse.h"

const IFFixNumber IFFixNumber::N_0(0);

const IFFixNumber IFFixNumber::N_1(1);

const IFFixNumber IFFixNumber::N_2(2);

const IFFixVec2 IFFIXVEC2_AXIS_UNIT_X(1,0);

const IFFixVec2 IFFIXVEC2_AXIS_UNIT_Y(0,1);

const IFFixVec2 IFFIXVEC2_ZERO(0,0);


IFCOMMON_API void IFFixNumber::loadFromString( const IFString& s )
{
	StringList sl;
	double df;
	float f;
	int i;
	IFI64 l;
	auto p = s.c_str();
	switch (IFNumParse::parse(p, f, df, i, l))
	{
	case 1:		FromFloat1(f);		break;
	case 2:		FromFloat1((float)df);		break;
	case 3:		FromInt(i);		break;
	case 4:		FromInt64(l);		break;
	}
}
IFString IFFixNumber::toString() const
{
	int b = int(nNum>>PRECISION);
	

	IFString ss;
	if (nNum&((1<< PRECISION) -1))
		ss.format("%g", (double)nNum/(double)(1<< PRECISION));
	else
		ss.format("%d", nNum >> PRECISION);
	return ss;
}
