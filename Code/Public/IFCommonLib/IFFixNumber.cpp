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


const IFFixNumber IFFixNumber::N_0(0);

const IFFixNumber IFFixNumber::N_1(1);

const IFFixNumber IFFixNumber::N_2(2);

const IFFixVec2 IFFIXVEC2_AXIS_UNIT_X(1,0);

const IFFixVec2 IFFIXVEC2_AXIS_UNIT_Y(0,1);

const IFFixVec2 IFFIXVEC2_ZERO(0,0);

IFCOMMON_API void IFFixNumber::loadFromString( const IFString& s )
{
	StringList sl;
	USplitStrings(&sl,s.c_str(), ".");
	if (sl.size()>1)
	{
		if (sl[1].size()>4)
			sl[1].erase(4, sl[1].size()-4);
		else while (sl[1].size()<4)
		{
			sl[1].push_back('0');
		}
		int nBig = sl[0].toInt32();
		int nSmall = sl[1].toInt32();
		if (nBig<0)
			nSmall=-nSmall;
		*this = IFFixNumber(nBig, nSmall);
	}
	else
	{
		*this = IFFixNumber(sl[0].toInt32());
	}
}
IFString IFFixNumber::toString() const
{
	int b = int(nNum/PRECISION);
	int s = int(nNum - b*PRECISION);
	
	if (s%10==0)
		s/=10;
	if (s%10==0)
		s/=10;
	if (s%10==0)
		s/=10;

	IFString ss;
	if (s)
		ss.format("%d.%d", b, s);
	else
		ss.format("%d",b);
	return ss;
}
