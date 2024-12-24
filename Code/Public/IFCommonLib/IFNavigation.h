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
#ifndef __IF_NAVIGATION_H__
#define __IF_NAVIGATION_H__
#include "IFRefObj.h"
#include "IFArray.h"
#include "IFTypes.h"
#include "IFAStar.h"

class IFNaviCell;
class IFNaviPath;

class IFCOMMON_API IFNavigation : public IFAStarGraph, public IFRefObj
{
public:
	IFNavigation();

	void addCell(IFNaviCell* pCell);

	void clear();

	IFRefPtr<IFNaviPath> navigate(const IFVector3D& start, const IFVector3D& dest, float fSize = 0.0f);

	IFNaviCell* findCellByPoint(const IFVector3D& pt);

	const IFArray<IFRefPtr<IFNaviCell>>& getCells()
	{
		return m_Cells;
	}

protected:
	~IFNavigation();

	IFArray<IFRefPtr<IFNaviCell>> m_Cells;
	IFRefPtr<IFAStarState> m_spAStarState;
};

#endif //__IF_NAVIGATION_H__