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
#include "IFNaviPath.h"
#include "IFNaviCell.h"

IFNaviPath::IFNaviPath(void)
{
}


IFNaviPath::~IFNaviPath(void)
{
}

void IFNaviPath::simplePath( IFArray<IFVector2D>& path )
{
	path.push_back(IFVector2D(m_Start.x, m_Start.y));

	int nCurNode = 0;
	do 
	{
		nCurNode = findTurnNode(path, nCurNode);

	} while (nCurNode!=-1);
	/*
	if (m_WayPointList.size()>1)
	{
		const IFLine2D* pLine = m_WayPointList[m_WayPointList.size()-2]->findEdge(m_WayPointList.back());

		IFLine2D l(path.back(),pLine->a );
		IFLine2D r(path.back(),pLine->b );
		getLeftRight(l, r);

		IFLine2D d(path.back(), IFVector2D(m_Dest.x,m_Dest.y));	
		if (l.dot(d)<0.0f)
			path.push_back(l.b);
		else if (r.dot(d)>0.0f)
			path.push_back(r.b);
	}
*/

	path.push_back(IFVector2D(m_Dest.x, m_Dest.y));

}


void IFNaviPath::getLeftRight(  IFLine2D& l, IFLine2D& r )
{
	if (l.dot(r) < 0)
	{
		IFLine2D tp = r;
		r = l;
		l = tp;

	}

}

int IFNaviPath::findTurnNode(IFArray<IFVector2D>& path, int nStartNode)
{
	IFLine2D Left;
	IFLine2D Right;
	bool updateV = true;
	//<0 左
	int curlNode = 0;
	int currNode = 0;
	IFLine2D lastEdge(m_Dest.x,m_Dest.y, m_Dest. x+ 1, m_Dest.y);
	for (int i = nStartNode; i < m_WayPointList.size(); i ++ )
	{
		const IFLine2D* pLine;
		if (i == m_WayPointList.size()-1)
			pLine = &lastEdge;
		else
			pLine = m_WayPointList[i]->findEdge(m_WayPointList[i+1]);

		if (pLine->a == path.back() ||pLine->b == path.back())
			return i + 1;

		IFLine2D l(path.back(), pLine->a);
		IFLine2D r(path.back(), pLine->b);
		
		getLeftRight(l,r);


		if (updateV)
		{
			Left = l;
			Right = r;
			updateV =false;
			curlNode = i;
			currNode = i;
		}
		else
		{
			if (Right.dot(l) > 0.0f)
			{
				path.push_back(Right.b);

				Right = IFLine2D(path.back(), pLine->b);
				Left = IFLine2D(path.back(), pLine->a);
				getLeftRight(Left,Right);
				return currNode+1;


			}
			else if (Left.dot(r) < 0.0f)
			{
				path.push_back(Left.b);
				Right = IFLine2D(path.back(), pLine->b);
				Left = IFLine2D(path.back(), pLine->a);
				getLeftRight(Left,Right);

				return curlNode+1;

			}

			else
			{

				if (Left.dot(l)>=0.0f)
				{
					Left = l;
					curlNode = i;
				}


				if (Right.dot(r)<=0.0f)
				{
					Right = r;
					currNode = i;
				}
			}

		}

	}
	return -1;
}
