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

struct  IFRect 
{
	int x,y,w,h;

	IFRect():x(0),y(0),w(0),h(0)
	{
	}
	IFRect( int x, int y, int w, int h )
	{
		this->x = x;
		this->y = y;
		this->w = w;
		this->h = h;
	}

	IFRect(const RECT& rc )
	{
		x = rc.left;
		y = rc.top;
		w = rc.right - rc.left;
		h = rc.bottom - rc.top;
	}

	inline int Right() const	//not contain 
	{
		return x + w;
	}
	inline int Bottom() const
	{
		return y + h;
	}
	inline bool PointInMe( int nX, int nY ) const
	{
		return !(nX < x || nX >= Right() || nY < y || nY >= Bottom()) ;
	}
	const RECT GetRect() const
	{
		RECT rc;
		rc.top = y;
		rc.left = x;
		rc.bottom = Bottom();
		rc.right = Right();
		return rc;
	}
	bool Intersect(const IFRect& rc ) const
	{
		//RECT rc0 = rc.GetRect();
		IFRect rect;
		rect.x = IFMax(x,rc.x);
		rect.y = IFMax(y,rc.y);
		int r = IFMin(Right(),rc.Right());
		int b = IFMin(Bottom(), rc.Bottom());
		return  r - rect.x > 0 &&b - rect.y>0;
		
		

		//return IntersectRect( &rc0, &rc0, &GetRect() );
	}

	IFRect And(const IFRect& rect)
	{
		IFRect rc;
		rc.x = IFMax(rect.x,x);
		rc.y = IFMax(rect.y,y);
		int right = IFMin(rect.Right(),Right());
		int bottom = IFMin(rect.Bottom(),Bottom());
		rc.w = right - rc.x ;
		rc.h = bottom - rc.y ;
		return rc;
	}

	void Include(int x, int y)
	{
		if( x < this->x )
			this->x = x;
		if( x > Right() )
			w = x - this->x;
		if( y < this->y )
			this->y = y;
		if( y > Bottom() )
			h = y - this->y;

	}

};