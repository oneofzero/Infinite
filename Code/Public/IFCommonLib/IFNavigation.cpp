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
#include "IFNavigation.h"
#include "IFNaviCell.h"
#include "IFNaviPath.h"

IFNavigation::IFNavigation()
{

}

IFNavigation::~IFNavigation()
{

}

void IFNavigation::addCell( IFNaviCell* pCell )
{
	for (int i = 0; i < m_Cells.size(); i ++ )
	{
		m_Cells[i]->requestLink(pCell);
	}
	m_Cells.push_back(pCell);
}

IFRefPtr<IFNaviPath> IFNavigation::navigate( const IFVector3D& start, const IFVector3D& dest, float fSize )
{
	IFNaviCell* pStart = findCellByPoint(start);
	IFNaviCell* pEnd = findCellByPoint(dest);
	if (!pStart || !pEnd)
		return NULL;

	m_spAStarState->init(pStart, pEnd, this);


	while (m_spAStarState->step());
	//m_spAStarState->getPath();
	if (IFAStarNode* pCurNode = m_spAStarState->getCurNode())
	{
		IFRefPtr<IFNaviPath> spPath = IFNew IFNaviPath;
		spPath->setStart(start);
		spPath->setDest(dest);
		while (pCurNode)
		{
			spPath->getWayPointList().push_back((IFNaviCell*)pCurNode);
			pCurNode = m_spAStarState->getPrevNode(pCurNode);
		}

		return spPath;
	}

	return NULL;
}

IFNaviCell* IFNavigation::findCellByPoint( const IFVector3D& pt )
{
	for (int i = 0; i < m_Cells.size(); i ++ )
	{
		if (m_Cells[i]->isPointInCell(pt))
			return m_Cells[i];
	}

	return NULL;
}

void IFNavigation::clear()
{
	m_Cells.clear();
}
