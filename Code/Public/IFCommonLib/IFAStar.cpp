#include "stdafx.h"
#include "IFAStar.h"
#include "IFTypes.h"
#include <math.h>
#include <vector>
#include <algorithm>





IFAStarState::IFAStarState()
	:m_nCurFindID(0)
{


}


bool IFAStarState::step()
{
	if (m_OpenList.size())
	{
		auto curIt = m_OpenList.begin();
		m_nCurNodeID = curIt->second;
		if (m_nCurNodeID==1)
		{
			return false;
		}
		IFAStarNodeInfo* info = &m_NodeInfoList[m_nCurNodeID];
		info->state = IFAStarNodeInfo::S_CLOSED;
		m_OpenList.erase(info->openit);
		int nNum = info->pNode->getSurroundNum(m_pGraph);
		for (int i = 0; i <nNum; i ++)
		{
			if(IFAStarNode* pNode = info->pNode->getSurround(i,m_pGraph))
			{
				IFAStarNodeInfo& ci = getNodeInfo(pNode);
				info = &m_NodeInfoList[m_nCurNodeID];
				if (ci.state == IFAStarNodeInfo::S_CLOSED /*|| (!ci.pNode->canPass(info->pNode)&& ci.pNode!=m_pDest)*/)
				{
					continue;
				}
				else if (ci.state == IFAStarNodeInfo::S_OPENED)
				{
					if (ci.f < info->f)
					{
						ci.nPrevID = m_nCurNodeID;
						ci.refreshValue(info,&m_NodeInfoList[1],m_pGraph);
						m_OpenList.erase(ci.openit);
						ci.openit = m_OpenList.insert(makeIFPair(ci.f,ci.nID));
						if (ci.h<m_NodeInfoList[m_nNearstID].h)
							m_nNearstID = ci.nID;
					}
				}
				else
				{
					ci.state = IFAStarNodeInfo::S_OPENED;
					ci.refreshValue(info,&m_NodeInfoList[1],m_pGraph);
					ci.openit = m_OpenList.insert(makeIFPair(ci.f,ci.nID));
					ci.nPrevID = m_nCurNodeID;
					if (ci.h<m_NodeInfoList[m_nNearstID].h)
						m_nNearstID = ci.nID;
				}

			}
			
		}

		return true;
	}
	else
	{
		m_nCurNodeID = -1;
		return false;
	}
}

IFAStarNode* IFAStarState::getCurNode()
{
	if (m_nCurNodeID>=0&&m_nCurNodeID<m_NodeInfoList.size())
		return m_NodeInfoList[m_nCurNodeID].pNode;
	else if ( m_nNearstID >=0 && m_nNearstID< m_NodeInfoList.size())
		return m_NodeInfoList[m_nNearstID].pNode;
	else
		return NULL;
}

IFArray<IFAStarNode*> IFAStarState::getPath()
{
	IFArray<IFAStarNode*> wps;
	IFAStarNode* pCurNode = getCurNode();
	while(pCurNode)
	{
		wps.push_back(pCurNode);
		pCurNode = getPrevNode(pCurNode);
	}

	return wps;
}

IFAStarState::~IFAStarState()
{

}

IFAStarNodeInfo& IFAStarState::getNodeInfo(IFAStarNode* pNode)
{
	auto it = m_NodeInfoMap.find(pNode);
	if (it!=m_NodeInfoMap.end())
	{
		return m_NodeInfoList[it->second];
	}
	else
	{
		int nId = m_NodeInfoList.size();
		m_NodeInfoList.push_back(IFAStarNodeInfo(pNode));
		m_NodeInfoMap[pNode] = nId;
		m_NodeInfoList.back().nID = nId;
		return m_NodeInfoList.back();
	}
}

IFAStarNode* IFAStarState::getPrevNode(IFAStarNode* pNode)
{
	IFAStarNodeInfo& info = getNodeInfo(pNode);
	if (info.nPrevID!=-1)
		return m_NodeInfoList[info.nPrevID].pNode;
	else
		return NULL;
}

bool IFAStarState::init(IFAStarNode* pS, IFAStarNode* pDest, IFAStarGraph* pGraph)
{
	if (pS==pDest || pDest == NULL || pS == NULL)
		return false;
	
	m_pGraph = pGraph;

	m_NodeInfoList.clear();
	m_NodeInfoList.reserve(256);
	m_OpenList.clear();
	m_NodeInfoMap.clear();
	getNodeInfo(pS);
	getNodeInfo(pDest);
	//m_NodeInfoList.push_back(IFAStarNodeInfo(pS));
	//m_NodeInfoList.push_back(IFAStarNodeInfo(pDest));
	m_NodeInfoList[0].state = IFAStarNodeInfo::S_OPENED;
	m_NodeInfoList[0].refreshValue(NULL, &m_NodeInfoList[1],m_pGraph);
	m_pDest = pDest;
	m_NodeInfoList[0].openit = m_OpenList.insert(makeIFPair(m_NodeInfoList[0].f, 0));
	m_nCurNodeID = -1;
	m_nNearstID = 0;
	return true;
}

bool IFAStarState::isPathFound()
{
	return m_nCurNodeID != -1;
}

void IFAStarNodeInfo::refreshValue(IFAStarNodeInfo* pPrev, IFAStarNodeInfo* pDest,IFAStarGraph* pGraph)
{
	if (pPrev)
		g = pPrev->g + pNode->evalCost(pPrev->pNode,pGraph);
	else
		g = 0;

	h =  pDest->pNode->evalCost(pNode,pGraph);

	f = h + g;
}

IFAStarGridScene::IFAStarGridScene()
{
	m_nLinkNum = 8;
}

void IFAStarGridScene::setWH(int nW, int nH)
{
	m_nW = nW;
	m_nH = nH;
	m_Buff.resize(nW*nH);
	for (int x = 0; x < nW; x ++)
	{
		for (int y = 0; y < nH; y ++ )
		{
			Node& n = m_Buff[x+y*m_nW];
			n.x = x;
			n.y = y;
			n.val = 0;	
		}
	}
}

void IFAStarGridScene::setBlock(int nX, int nY, bool bBlock)
{
	getNode(nX,nY)->val = bBlock;
}

bool IFAStarGridScene::isBlock(int nX, int nY)
{
	return getNode(nX,nY)->val;
}

IFAStarGridScene::Node* IFAStarGridScene::getNode(int nX, int nY)
{
	if(nX<0||nY<0||nX>=m_nW||nY>=m_nH)
		return NULL;
	int nIdx = nX+nY*m_nW;
	return &m_Buff[nIdx];

}


float IFAStarGridScene::Node::evalCost(IFAStarNode* pFrom,IFAStarGraph* pGraph)
{
	float dx = x-((Node*)pFrom)->x;
	float dy = y-((Node*)pFrom)->y;
	return sqrt(dx*dx+dy*dy);
}

static IFVector2DI sroffset[]=
{
	IFVector2DI(-1,0),

	IFVector2DI(1,0),

	IFVector2DI(0,-1),
	IFVector2DI(0,1),

	IFVector2DI(-1,-1),
	IFVector2DI(-1,1),
	IFVector2DI(1,1),
	IFVector2DI(1,-1),
};

static IFVector2DI ckOffset[]=
{
	IFVector2DI(0,2),
	IFVector2DI(0,3),
	IFVector2DI(1,3),
	IFVector2DI(2,1),
};


IFAStarNode* IFAStarGridScene::Node::getSurround(int idx, IFAStarGraph* pGraph)
{
	IFAStarGridScene* pScene = (IFAStarGridScene*)pGraph;


	
	if(Node* pNode = pScene->getNode(sroffset[idx].x+x,sroffset[idx].y+y))
	{
		if (pNode->val)
		{
			return NULL;
		}
		else
		{
			if (idx<4)
				return pNode;
			else
			{
				idx-=4;

				if (pScene->getNode(sroffset[ckOffset[idx].x].x+x,sroffset[ckOffset[idx].x].y+y)->val ||
					pScene->getNode(sroffset[ckOffset[idx].y].x+x,sroffset[ckOffset[idx].y].y+y)->val
					)
					return NULL;

				return pNode;
			}
		}


	}
	



	return NULL;
}

int IFAStarGridScene::Node::getSurroundNum(IFAStarGraph* pGraph)
{
	IFAStarGridScene* pGridScene = (IFAStarGridScene*)pGraph;
	return pGridScene->m_nLinkNum;
}



IFAStarNode::IFAStarNode(void)
{

}

IFAStarNode::~IFAStarNode(void)
{

}
