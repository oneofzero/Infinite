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
#include "IFAlloc.h"
template<class T>
class IFSet
{
	struct Node
	{

		Node(){};
		Node(const T& v):Val(v){}
		T Val;
		char isBlack;
		Node* pParent;
		Node* pLeft;
		Node* pRight;
	};

public:
	IFSet(void):m_pRoot(NULL),m_nSize(0),m_pNill(NULL){}
	virtual ~IFSet(void){}

	class iterator
	{
	public:
		iterator(Node* pNode, IFSet<T>* pSet):m_pNode(pNode),m_pSet(pSet)
		{

		}


		Node* m_pNode;
		IFSet<T>* m_pSet;

	};

	inline iterator insert(const T& o)
	{
		if( m_pRoot == NULL )
		{
			m_pRoot = new (IFAlloc::Alloc(sizeof(Node))) Node(o);
			m_pRoot->isBlack = 1;
			m_pRoot->pParent = NULL;
			m_pRoot->pLeft = NULL;
			m_pRoot->pRight = NULL;
			m_nSize = 1;
			return iterator(m_pRoot,this);
		}
		Node* pCurNode = m_pRoot;
		Node* pInsertNode = m_pRoot;
		bool bInsertLeft = true;
		while( pCurNode )
		{
			if( pCurNode->Val < o )
			{
				pInsertNode = pCurNode;
				pCurNode = pCurNode->pRight;
				bInsertLeft = false;
			}
			else if( o < pCurNode->Val )
			{
				pInsertNode = pCurNode;
				pCurNode = pCurNode->pLeft;
				bInsertLeft = true;
			}
			else	//find
				return iterator(pCurNode,this);
		}

		Node* pNewNode = new(IFAlloc::Alloc(sizeof(Node))) Node(o);
		pNewNode->isBlack = false;
		pNewNode->pLeft = NULL;
		pNewNode->pRight = NULL;
		m_nSize++;

		if( pInsertNode->isBlack )
		{
			if( bInsertLeft )
			{
				//pNewNode->pLeft = pInsertNode->pLeft;
				pInsertNode->pLeft = pNewNode;
				pNewNode->pParent = pInsertNode;

				

				return iterator(pNewNode,this);
			}
			else
			{
				pInsertNode->pRight = pNewNode;
				pNewNode->pParent = pInsertNode;

				return iterator(pNewNode,this);
			}

		}
		else
		{
			if( bInsertLeft )
			{
				pInsertNode->pLeft = pNewNode;
				pNewNode->pParent = pInsertNode;
				
				//if( pInsertNode->pRight)

				_fixInsert(pNewNode);
				return iterator(pNewNode,this);
			}
			else
			{
				pInsertNode->pRight = pNewNode;
				pNewNode->pParent = pInsertNode;
				_fixInsert(pNewNode);
				return iterator(pNewNode,this);
			}
		}



	}



	inline iterator find(const T& o)
	{
		Node* pCurNode = m_pRoot;
		while( pCurNode )
		{
			if( pCurNode->Val < o )
			{
				pCurNode = pCurNode->pRight;
			}
			else if( o < pCurNode->Val )
			{
				pCurNode = pCurNode->pLeft;
			}
			else	//find
				break;
		}
		return iterator(pCurNode,this);
	}

private:

	inline void _rotateLeft(Node* pNode)//uncle 是黑色的 红红，左树调整
	{

		Node* pFather	 = pNode->pParent;
		Node* pGrandpa	 = pFather->pParent;
		Node* pGrandpapa = pGrandpa->pParent;
		
		if( pNode == pNode->pParent->pLeft )//
		{
			pFather->isBlack = true;
			pGrandpa->isBlack = false;

			pGrandpa->pLeft = pFather->pRight;
			
			pFather->pRight = pGrandpa;
			pGrandpa->pParent = pFather;

			pFather->pParent = pGrandpapa;
			if( pGrandpapa )
			{
				if( pGrandpapa->pLeft == pGrandpa )
				{
					pGrandpapa->pLeft = pFather;
				}
				else
				{
					pGrandpapa->pRight = pFather;
				}	
			}

			if( pGrandpa == m_pRoot )
			{
				m_pRoot = pFather;
			}
		
			return;
		}
		else
		{
			pNode->isBlack = true;
			pGrandpa->isBlack = false;

			pFather->pRight = pNode->pLeft;
			pNode->pLeft = pFather;
			pFather->pParent = pNode;
			


			pGrandpa->pLeft = pNode->pRight;
			pNode->pRight = pGrandpa;
			pGrandpa->pParent = pNode;
			
			pNode->pParent = pGrandpapa;
			if( pGrandpapa )
			{
				if( pGrandpapa->pLeft == pGrandpa )
				{
					pGrandpapa->pLeft = pNode;
				}
				else
				{
					pGrandpapa->pRight = pNode;
				}
			}
			if( pGrandpa == m_pRoot )
			{
				m_pRoot = pNode;
			}


			return;	
		}
	}

	inline void _rotateRight(Node* pNode)//uncle 是黑色的 红红，右树调整
	{

		Node* pFather	 = pNode->pParent;
		Node* pGrandpa	 = pFather->pParent;
		Node* pGrandpapa = pGrandpa->pParent;


		if( pNode == pNode->pParent->pLeft )//
		{
			pNode->isBlack = true;
			pGrandpa->isBlack = false;

			pGrandpa->pRight = pNode->pLeft;
			pNode->pLeft = pGrandpapa;
			pGrandpapa->pParent = pNode;

			pFather->pLeft = pNode->pRight;
			pNode->pRight = pFather;
			pFather->pParent = pNode;

			pNode->pParent = pGrandpapa;
			if( pGrandpapa )
			{
				if(pGrandpapa->pLeft == pGrandpa)
				{
					pGrandpapa->pLeft = pNode;
				}
				else
				{
					pGrandpapa->pRight = pNode;
				}
			}
			if( pGrandpa == m_pRoot )
			{
				m_pRoot = pNode;
			}

			return;
		}
		else
		{
			pFather->isBlack = true;
			pGrandpa->isBlack = false;

			pGrandpa->pRight = pFather->pLeft;
			pFather->pLeft = pGrandpa;
			pGrandpa->pParent = pFather;

			pFather->pParent = pGrandpapa;
			if( pGrandpapa )
			{
				if( pGrandpapa->pLeft == pGrandpa )
				{
					pGrandpapa->pLeft = pFather;
				}
				else
				{
					pGrandpapa->pRight = pFather;
				}
			}
			if( pGrandpa == m_pRoot )
			{
				m_pRoot = pFather;
			}
		}
	}

	inline void _redredFix(Node* pNode)
	{
		if( !pNode->isBlack && !pNode->pParent->isBlack )
		{
			pNode->pParent->isBlack = true;
			Node* pUncle;
			if( pNode->pParent->pParent->pLeft == pNode->pParent )
			{
				pUncle = pNode->pParent->pParent->pRight;
			}
			else
			{
				pUncle = pNode->pParent->pParent->pLeft;
			}
			pUncle->isBlack = true;
			if( pNode->pParent->pParent != m_pRoot)
			{
				pNode->pParent->pParent->isBlack =false;
				_fixInsert(pNode->pParent->pParent);

			}

		}
	}

	inline void _fixInsert(Node* pNode)
	{
		Node* pUncle;
		if( !pNode->pParent->pParent )
			return;
		if( pNode->pParent->pParent->pLeft == pNode->pParent )
		{
			pUncle = pNode->pParent->pParent->pRight;
		}
		else
		{
			pUncle = pNode->pParent->pParent->pLeft;
		}

		if( !pUncle || pUncle->isBlack )
		{
			if(pNode->pParent == pNode->pParent->pParent->pLeft )
			{
				_rotateLeft(pNode);
			}
			else
				_rotateRight(pNode);
		}
		else
		{
			_redredFix(pNode);
		}
	}

	Node* _minNode(Node* pNode)
	{
		while( pNode->pLeft )
		{
			pNode = pNode->pLeft;
		}
		return pNode;
	}
	Node* _maxNode(Node* pNode)
	{
		while( pNode->pRight )
		{
			pNode = pNode->pRight;
		}
		return pNode;
	}

private:

	Node* m_pRoot;
	int m_nSize;
	Node* m_pNill;

};
