#pragma once
#include "IFArray.h"
#include "IFHashMap.h"

class IFAStarGraph
{
public:
};

class IFCOMMON_API IFAStarNode 
{
public:


	IFAStarNode(void);
	~IFAStarNode(void);


	//IFArray<IFAStarNode*> m_Surround;
	virtual IFAStarNode* getSurround(int idx, IFAStarGraph* pGraph) = 0;
	virtual int getSurroundNum(IFAStarGraph* pGraph) = 0;
	
	virtual float evalCost(IFAStarNode* pFrom,IFAStarGraph* pGraph) = 0;
};



class IFCOMMON_API IFAStarNodeInfo : public IFMemObj
{
public:
	enum State
	{
		S_NULL,
		S_CLOSED,
		S_OPENED,
	};

	IFAStarNodeInfo(IFAStarNode* node)
		:g(0),h(0),f(0),state(S_NULL),pNode(node),nPrevID(-1)
	{

	}

	IFAStarNode* pNode;
	int nID;
	int nPrevID;
	//IFAStarNode* pParent;
	float g;
	float h;
	float f;
	int state;
	IFMultiMap<float,int>::iterator openit;

	void refreshValue(IFAStarNodeInfo* pPrev, IFAStarNodeInfo* pDest, IFAStarGraph* pGraph);


};


class IFCOMMON_API IFAStarState : public IFRefObj
{
public:

	IFAStarState();


	bool init(IFAStarNode* pFrom, IFAStarNode* pTo, IFAStarGraph* pGraph);

	bool step();

	bool isPathFound();

	IFAStarNode* getCurNode();
	IFAStarNode* getPrevNode(IFAStarNode* pNode);
	IFArray<IFAStarNode*> getPath();
protected:
	~IFAStarState();
	IFAStarNodeInfo& getNodeInfo(IFAStarNode* pNode);

	IFMultiMap<float,int> m_OpenList;
	IFArray<IFAStarNodeInfo> m_NodeInfoList;
	IFAStarNode* m_pDest;
	int m_nNearstID;
	int m_nCurNodeID;
	int m_nCurFindID;
	IFAStarGraph* m_pGraph;
	IFHashMap<IFAStarNode*,int> m_NodeInfoMap;
};

class IFCOMMON_API IFAStarGridScene : public IFRefObj, public IFAStarGraph
{
public:

	IFAStarGridScene();
	class Node : public IFAStarNode
	{
	public:
		char val;
		int x,y;
		//IFArray<IFAStarNode*> m_Surround;
		//virtual bool canPass(IFAStarNode* pFrom);

		virtual float evalCost(IFAStarNode* pFrom,IFAStarGraph* pGraph);

		virtual IFAStarNode* getSurround(int idx, IFAStarGraph* pUserData);
		virtual int getSurroundNum(IFAStarGraph* pGraph);
	};

	void setWH(int nW, int nH);
	void setBlock(int nX, int nY, bool bBlock);
	bool isBlock(int nX, int nY);
	Node* getNode(int nX, int nY);


	inline int getWidth()
	{
		return m_nW;
	}

	inline int getHeight()
	{
		return m_nH;
	}

	IFArray<Node> m_Buff;
	int m_nW, m_nH;
	int m_nLinkNum;
};