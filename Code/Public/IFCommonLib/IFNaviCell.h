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
#ifndef __IF_NAVI_CELL_H__
#define __IF_NAVI_CELL_H__
#include "IFRefObj.h"
#include "IFTypes.h"
#include "IFArray.h"
#include "IFAStar.h"

class IFCOMMON_API IFNaviCell: public IFRefObj, public IFAStarNode
{
public:
	IFNaviCell();

	void begin();
	bool checkVertex(const IFVector3D& vtx);
	bool addVertex(const IFVector3D& vtx);
	void end();

	bool isPointInCell(const IFVector3D& v);
	bool isValid();

	void requestLink(IFNaviCell* pCell);


	const IFVector3D& getCenter();

	const IFArray<IFLine2D>& getEdge()
	{
		return m_Edge;
	}

	const IFLine2D* findEdge(IFNaviCell* pOtherCell);

	const IFArray<IFVector3D>& getVertex()
	{
		return m_Vertex;
	}

	float caclCost(const IFVector3D& dest);


	virtual IFAStarNode* getSurround(int idx, IFAStarGraph* pGraph);
	virtual int getSurroundNum(IFAStarGraph* pGraph);

	virtual float evalCost(IFAStarNode* pFrom, IFAStarGraph* pGraph);
protected:
	~IFNaviCell();

	struct LinkInfo
	{
		IFNaviCell* pCell;
		int nEdgeIndex;
	};

	IFArray<IFVector3D> m_Vertex;
	IFArray<IFLine2D> m_Edge;
	IFArray<LinkInfo> m_Link;
	IFVector3D m_Center;
	bool m_bClosed;
	bool m_bIsCircled;

};


#endif //__IF_NAVI_CELL_H__