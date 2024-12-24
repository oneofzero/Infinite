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
#ifndef __IF_NAVI_PATH_H__
#define __IF_NAVI_PATH_H__
#include "IFRefObj.h"
#include "IFArray.h"
#include "IFTypes.h"

class IFNaviCell;
typedef IFArray<IFRefPtr<IFNaviCell>> IFNaviWayPointList;

class IFCOMMON_API IFNaviPath : public IFRefObj
{
public:
	IFNaviPath(void);

	IFNaviWayPointList& getWayPointList()
	{
		return m_WayPointList;
	}

	void simplePath(IFArray<IFVector2D>& path);

	void setStart(const IFVector3D& start)
	{
		m_Start = start;
	}
	void setDest(const IFVector3D& dest)
	{
		m_Dest = dest;
	}



protected:
	~IFNaviPath(void);

	void getLeftRight(IFLine2D& l, IFLine2D& r);

	int findTurnNode(IFArray<IFVector2D>& path, int nStartNode);

	IFNaviWayPointList m_WayPointList;
	IFVector3D m_Start;
	IFVector3D m_Dest;

};

#endif //__IF_NAVI_PATH_H__