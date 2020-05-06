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
#include "IFNaviCell.h"

IFNaviCell::IFNaviCell()
	:m_bClosed(true)
	,m_bIsCircled(false)
{

}

IFNaviCell::~IFNaviCell()
{

}

bool IFNaviCell::isPointInCell( const IFVector3D& v )
{
	IFVector2D v2d(v.x,v.y);
	for (int i = 0; i < m_Edge.size(); i ++ )
	{
		IFLine2D l(m_Edge[i].a, v2d);
		if (m_Edge[i].dot(l)>0.0f)
			return false;
	}

	return true;
}

void IFNaviCell::begin()
{
	m_Vertex.clear();
	m_bIsCircled = false;
}

bool IFNaviCell::checkVertex( const IFVector3D& vtx )
{
	if (m_Vertex.size() < 2)
		return true;
	else
	{
		IFLine2D l(m_Vertex.back().x, m_Vertex.back().y, vtx.x, vtx.y);

		if ( m_Edge.back().dot(l) < 0.0f)
			return true;
	}
	return false;
}

bool IFNaviCell::addVertex( const IFVector3D& vtx )
{
	if (!checkVertex(vtx))
		return false;

	if (m_Vertex.size() > 2 && vtx == m_Vertex[0])
	{
		const IFVector3D& v0 = m_Vertex.back();
		IFVector3D& v1 = m_Vertex[0];

		IFLine2D l(v0.x, v0.y, v1.x, v1.y);

		if (l.dot(m_Edge[0]) >= 0.0f )
			return false;

		m_Edge.push_back(l);
		m_bIsCircled = true;
		return true;
	}

	m_Vertex.push_back(vtx);
	if (m_Vertex.size()>1)
	{
		IFVector3D& v0 = m_Vertex[m_Vertex.size()-2];
		IFVector3D& v1 = m_Vertex[m_Vertex.size()-1];

		IFLine2D l(v0.x, v0.y, v1.x, v1.y);

		m_Edge.push_back(l);
	}
	return true;
}

void IFNaviCell::end()
{
	if (m_Vertex.size()<3)
		return;

	IFVector3D v;
	for (int i = 0; i < m_Vertex.size(); i ++ )
	{
		v += m_Vertex[i];

	}
	//m_Center = v;
	m_Center = v / (float)m_Vertex.size();
}

bool IFNaviCell::isValid()
{
	return m_Vertex.size()>2 && m_bIsCircled;
}

void IFNaviCell::requestLink( IFNaviCell* pCell )
{
	for (int i = 0; i < m_Edge.size(); i ++ )
	{
		for (int j = 0; j < pCell->m_Edge.size();j ++ )
		{
			if (m_Edge[i] == pCell->m_Edge[j])
			{
				LinkInfo info;
				info.pCell = pCell;
				info.nEdgeIndex = i;
				m_Link.push_back(info);
				info.pCell = this;
				info.nEdgeIndex = j;
				pCell->m_Link.push_back(info);
			}
		}
	}
}


const IFVector3D& IFNaviCell::getCenter()
{
	return m_Center;
}

const IFLine2D* IFNaviCell::findEdge( IFNaviCell* pOtherCell )
{
	for (int i = 0; i < m_Link.size(); i ++ )
	{
		if (m_Link[i].pCell == pOtherCell)
		{
			return &m_Edge[m_Link[i].nEdgeIndex];
		}
	}
	return NULL;
}

float IFNaviCell::caclCost( const IFVector3D& dest )
{
	//float XDelta = fabs(dest.x - m_Center.x);
	//float YDelta = fabs(dest.y - m_Center.y);
	//float ZDelta = fabs(dest.z - m_Center.z);

	//return  IFMax(IFMax(XDelta,YDelta), ZDelta);


	return  (dest-m_Center).length();
}

IFAStarNode* IFNaviCell::getSurround(int idx, IFAStarGraph* pGraph)
{
	return m_Link[idx].pCell;
}

int IFNaviCell::getSurroundNum(IFAStarGraph* pGraph)
{
	return m_Link.size();
}

float IFNaviCell::evalCost(IFAStarNode* pFrom, IFAStarGraph* pGraph)
{
	IFNaviCell* pCell = (IFNaviCell*)pFrom;
	return  (pCell->m_Center - m_Center).length();
}

