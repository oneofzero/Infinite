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
#ifndef __IF_LERP_H__
#define __IF_LERP_H__
#include <float.h>
#include "IFArray.h"
#include "IFFixNumber.h"


template<class T>
inline void IFLinearLerp(T* pOut,const T& a,const T& b, float step)
{
	*pOut = (a)*(1.0f-step) + (b)*(step);
}

inline void IFLinearLerp(IFFixNumber* pOut,const IFFixNumber& a,const IFFixNumber& b, const IFFixNumber& step)
{
	*pOut = (a)*(IFFixNumber::N_1-step) + (b)*(step);
}

inline void IFLinearLerp(IFFixVec2* pOut,const IFFixVec2& a,const IFFixVec2& b, const IFFixNumber& step)
{
	IFFixNumber f1 = IFFixNumber::N_1;
	f1-=step;
	*pOut = a;
	*pOut *= f1;

	IFFixVec2 bb=b;
	bb*=step;
	*pOut += bb;

	//*pOut = (a)*(IFFixNumber::N_1-step) + (b)*(step);
}

template<class T, class ST>
inline void IFBezierLerp(		T *pOut,
						   T* vtxs, int nCount,
						   const ST& step )
{
	//char buf[1024 * 32];
	if(nCount==2)
	{
		IFLinearLerp (pOut, vtxs[0], vtxs[1], step);
		return;
	}
	nCount = nCount-1;
	for(int i = 0; i < nCount; i ++ )
	{
		IFLinearLerp (&vtxs[i], vtxs[i], vtxs[i+1], step);
	}
	IFBezierLerp(pOut,vtxs, nCount, step);
}

template<class T, class ST>
inline void IFBezierLerpConst(		T *pOut,
					 const IFArray<T>& vtxs,
					 const ST& step ) 
{
	IFArray<T> vt = vtxs;
	IFBezierLerp(pOut, &vt[0], vt.size(), step);
}

template<class T, class ST, int nLen>
inline void IFBezierLerpConst(T *pOut,
	T(& vtxs)[nLen] ,
	const ST& step)
{
	T vt[nLen];
	for (int i = 0; i < nLen; i++)
		vt[i] = vtxs[i];
	//IFArray<T> vt = vtxs;
	IFBezierLerp(pOut, &vt[0], nLen, step);
}

#endif //__IF_LERP_H__