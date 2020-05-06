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
#include "IFCommonLib_API.h"
#include "IFObj.h"
#include "IFAlloc.h"

template <class KeyType,bool EnableMultiInsert = false>
class  IFRBTree : public IFMemObj
{
	typedef IFRBTree<KeyType,EnableMultiInsert> MyType;
	//! red/black tree for map
	//template <class KeyType>
	class TreeNode : public IFMemObj
	{
	public:

		TreeNode(const KeyType& k)
			: left(0), right(0), parent(0), key(k),
			mIsRed(true) {}

		TreeNode(KeyType&& k)
			: left(0), right(0), parent(0), key(k),
			mIsRed(true) {}

		~TreeNode(){}

		inline void setLeftChild(TreeNode* p)	{ left=p; if (p) p->setParent(this); }
		inline void setRightChild(TreeNode* p)	{ right=p;if (p) p->setParent(this); }
		inline void setParent(TreeNode* p)		{ parent=p; }

		//void setValue(const ValueTypeRB& v)	{ mValue = v; }

		inline void setRed()					{ mIsRed = true; }
		inline void setBlack()					{ mIsRed = false; }

		inline TreeNode* getLeftChild() const	{ return left; }
		inline TreeNode* getRightChild() const	{ return right; }
		inline TreeNode* getParent() const		{ return parent; }


		inline const KeyType& getKey() const
		{
			return key;
		}

		inline KeyType& getKey()
		{
			return key;
		}


		inline bool isRoot() const
		{
			return parent==0;
		}

		inline bool isLeftChild() const
		{
			return parent!=0 && parent->getLeftChild()==this;
		}

		inline bool isRightChild() const
		{
			return parent!=0 && parent->getRightChild()==this;
		}

		inline bool isLeaf() const
		{
			return left==0 && right==0;
		}

		inline unsigned int getLevel() const
		{
			if (isRoot())
				return 1;
			else
				return getParent()->getLevel() + 1;
		}


		inline bool isRed()	const
		{
			return  mIsRed;
		}

		inline bool isBlack()	const
		{
			return !mIsRed;
		}




	//private:
		//TreeNode();

		TreeNode*		left;
		TreeNode*		right;

		TreeNode*		parent;

		KeyType	key;
		bool mIsRed;
	};

	template<class NodeType, class TreeType>
	static NodeType* moveRight(NodeType* pNode ,TreeType* pTree)
	{
		if( !pNode )
		{
			return pTree->minnum(pTree->mRoot);
		}

		if( pNode->right)
		{
			return pTree->minnum(pNode->right);
		}
		else if( pNode->parent )
		{


			if (pNode->isLeftChild())
				return pNode->parent;
			else
			{
				pNode = pNode->parent;
				while (pNode && pNode->isRightChild())
				{
					pNode = pNode->parent;
				}
				if (pNode)
					return pNode->parent;;
			}


		}
		return NULL;
	}

	template<class NodeType, class TreeType>
	static NodeType* moveLeft(NodeType* pNode ,TreeType* pTree)
	{
		if( !pNode )
		{
			return pTree->maxnum(pTree->mRoot);
		}

		if( pNode->left /*&& m_pNode->left != m_pTree->NIL*/ )
		{
			return pTree->maxnum(pNode->left);
		}
		else if( pNode->parent /*&& m_pNode->parent != m_pTree->NIL*/ )
		{
			if (pNode->isRightChild())
				return pNode->parent;
			else
			{
				pNode = pNode->parent;
				while (pNode && pNode->isLeftChild())
				{
					pNode = pNode->parent;
				}
				if (pNode)
					return pNode->parent;;
			}



		}

		return NULL;
	}
public:

	//typedef TreeNode<KeyType> Node;



	// Constructor.
	IFRBTree() : mRoot(0),mSize(0)
	{

	}
	IFRBTree(const MyType& ot) : mRoot(0),mSize(0)
	{
		operator=(ot);
	}


	// Destructor
	~IFRBTree()
	{
		clear();
	}


	class iterator
	{
	public:
		inline iterator():m_pNode(NULL),m_pTree(NULL){}
		inline iterator(MyType* pTree,TreeNode* pNode ):m_pNode(pNode),m_pTree(pTree)
		{

		}

		iterator& operator =(const iterator& other)
		{
			m_pNode = other.m_pNode;
			m_pTree = other.m_pTree;
			return *this;
		};

		inline bool operator == (const iterator& other )
		{
			return m_pNode == other.m_pNode;
		}

		inline bool operator !=(const iterator& other )
		{
			return !operator==(other);
		}

		inline iterator& operator ++()
		{
			m_pNode = moveRight(m_pNode,m_pTree);
			return *this;
			//if( m_pNode->right/* && m_pNode->right != m_pTree->NIL */)
			//{
			//	//
			//	m_pNode = m_pTree->minnum(m_pNode->right);

			//	return *this;
			//}
			//else if( m_pNode->parent /*&& m_pNode->parent != m_pTree->NIL*/ )
			//{

			//	TreeNode* pParent=m_pNode->parent;
			//	while(pParent /* && pParent != m_pTree->NIL*/)
			//	{
			//		if( pParent->key < m_pNode->key )
			//			pParent = pParent->parent;
			//		else
			//		{
			//			m_pNode = pParent;
			//			return *this;
			//		}
			//	}

			//}

			//m_pNode = NULL;
			//return *this;
		}

		inline iterator& operator --()
		{
			m_pNode=  moveLeft(m_pNode,m_pTree);
			return *this;
			//if( !m_pNode )
			//{
			//	m_pNode = m_pTree->maxnum(m_pTree->mRoot);
			//	return *this;
			//}

			//if( m_pNode->left /*&& m_pNode->left != m_pTree->NIL*/ )
			//{
			//	m_pNode = m_pTree->maxnum(m_pNode->left);
			//	return *this;
			//}
			//else if( m_pNode->parent /*&& m_pNode->parent != m_pTree->NIL*/ )
			//{

			//	TreeNode* pParent=m_pNode->parent;
			//	while(pParent  /*&& pParent != m_pTree->NIL*/)
			//	{
			//		if( m_pNode->key < pParent->key   )
			//			pParent = pParent->parent;
			//		else
			//		{
			//			m_pNode = pParent;
			//			return *this;
			//		}
			//	}

			//}

			//m_pNode = NULL;
			//return *this;
		}
		inline KeyType& operator*()
		{
			return m_pNode->key;
		}

		inline KeyType* operator->() const
		{
			return &m_pNode->key;
		}

		TreeNode* m_pNode;
		MyType* m_pTree;
	};

	class reverse_iterator
	{
	public:
		reverse_iterator():m_pNode(NULL),m_pTree(NULL){}
		reverse_iterator(MyType* pTree,TreeNode* pNode ):m_pNode(pNode),m_pTree(pTree)
		{

		}

		inline reverse_iterator& operator =(const reverse_iterator& other)
		{
			m_pNode = other.m_pNode;
			m_pTree = other.m_pTree;
			return *this;
		};

		inline bool operator == (const reverse_iterator& other )
		{
			return m_pNode == other.m_pNode;
		}

		inline bool operator !=(const reverse_iterator& other )
		{
			return !operator==(other);
		}

		inline reverse_iterator& operator ++()
		{
			m_pNode = moveLeft(m_pNode,m_pTree);
			return *this;

		}

		inline reverse_iterator& operator --()
		{
			m_pNode = moveRight(m_pNode,m_pTree);
			return *this;

		}
		inline KeyType& operator*()
		{
			return m_pNode->key;
		}

		inline KeyType* operator->()
		{
			return &m_pNode->key;
		}

		operator iterator()
		{
			return iterator(m_pTree,m_pNode);
		}

		TreeNode* m_pNode;
		MyType* m_pTree;
	};

	class const_iterator
	{
	public:
		const_iterator():m_pNode(NULL),m_pTree(NULL){}
		const_iterator(const MyType* pTree,const TreeNode* pNode ):m_pNode(pNode),m_pTree(pTree)
		{

		}

		const_iterator& operator =(const const_iterator& other)
		{
			m_pNode = other.m_pNode;
			m_pTree = other.m_pTree;
			return *this;
		};

		inline bool operator == (const const_iterator& other )
		{
			return m_pNode == other.m_pNode;
		}

		inline bool operator !=(const const_iterator& other )
		{
			return !operator==(other);
		}

		inline const_iterator& operator ++()
		{
			m_pNode = moveRight(m_pNode,m_pTree);
			return *this;

		}

		inline const_iterator& operator --()
		{
			m_pNode = moveLeft(m_pNode,m_pTree);
			return *this;
		}
		inline const KeyType& operator*() const
		{
			return m_pNode->key;
		}

		inline const KeyType* operator->() const
		{
			return &m_pNode->key;
		}

		const TreeNode* m_pNode;
		const MyType* m_pTree;
	};

	inline iterator begin()
	{
		return iterator(this,minnum(mRoot));
	}

	inline iterator end()
	{
		return iterator(this,0);
	}

	inline const_iterator begin() const
	{
		return const_iterator(this,minnum(mRoot));
	}

	inline const_iterator end() const
	{
		return const_iterator(this,0);
	}

	inline reverse_iterator rbegin()
	{
		return reverse_iterator(this,maxnum(mRoot));
	}

	inline reverse_iterator rend()
	{
		return reverse_iterator(this,0);
	}

	iterator insert(const KeyType& keyNew)
	{
		return insert((KeyType&&)keyNew);
	}
	iterator insert(KeyType&& keyNew)
	{
		TreeNode* newNode = IFNew/*(IFAlloc::Alloc(sizeof(TreeNode)))*/ TreeNode(keyNew);
		if (!insert(newNode))
		{
			delete newNode;

			return iterator(this,NULL);
		}

		TreeNode* pInsertNode = newNode;
		while (!newNode->isRoot() && (newNode->getParent()->isRed()))
		{
			if (newNode->getParent()->isLeftChild())
			{
				TreeNode* newNodesUncle = newNode->getParent()->getParent()->getRightChild();
				if ( newNodesUncle!=0 && newNodesUncle->isRed())
				{
					newNode->getParent()->setBlack();
					newNodesUncle->setBlack();
					newNode->getParent()->getParent()->setRed();
					newNode = newNode->getParent()->getParent();
				}
				else
				{
					if ( newNode->isRightChild())
					{
						newNode = newNode->getParent();
						rotateLeft(newNode);
					}
					newNode->getParent()->setBlack();
					newNode->getParent()->getParent()->setRed();
					rotateRight(newNode->getParent()->getParent());
				}
			}
			else
			{
				TreeNode* newNodesUncle = newNode->getParent()->getParent()->getLeftChild();
				if ( newNodesUncle!=0 && newNodesUncle->isRed())
				{
					newNode->getParent()->setBlack();
					newNodesUncle->setBlack();
					newNode->getParent()->getParent()->setRed();
					newNode = newNode->getParent()->getParent();
				}
				else
				{
					if (newNode->isLeftChild())
					{
						newNode = newNode->getParent();
						rotateRight(newNode);
					}
					newNode->getParent()->setBlack();
					newNode->getParent()->getParent()->setRed();
					rotateLeft(newNode->getParent()->getParent());
				}

			}
		}
		mRoot->setBlack();
		return iterator(this,pInsertNode);
	}

	iterator erase(iterator it)
	{
		iterator itnext = it;
		++ itnext ;
		TreeNode* p = it.m_pNode;
		while(p->getRightChild())
		{
			rotateLeft(p);
		}
		TreeNode* left = p->getLeftChild();

		if (p->isLeftChild())
			p->getParent()->setLeftChild(left);

		else if (p->isRightChild())
			p->getParent()->setRightChild(left);

		else
		{
			setRoot(left);
		}

		delete p;

		mSize--;
		return itnext;
	}


	bool remove(const KeyType& k)
	{
		iterator it = find(k);
		if (it == end())
		{
			return false;
		}
		erase(it);


		return true;
	}

	void clear()
	{
		while(mSize)
		{
			erase( iterator(this,mRoot));
			//it=nexit;

			//mSize--;
		}

		mRoot = 0;
		//mSize= 0;
	}

	inline bool isEmpty() const
	{
		return mRoot == 0;
	}

	inline iterator find(const KeyType& keyToFind)
	{
		TreeNode* pNode = mRoot;
		TreeNode* pLastNode = NULL;
		while(pNode)
		{
			//const KeyType& key(pNode->getKey());


			if (keyToFind < pNode->key)
				pNode = pNode->left;
			else
			{
				pLastNode = pNode;
				pNode = pNode->right;
			}

		}
		if (pLastNode && pLastNode->key  < keyToFind )
			return iterator(this, NULL);
		//	return iterator(this,pLastNode);
		return iterator(this, pLastNode);
	}

	const_iterator find(const KeyType& keyToFind) const
	{
		TreeNode* pNode = mRoot;

		while(pNode!=0)
		{
			const KeyType& key(pNode->getKey());


			if (keyToFind < key)
				pNode = pNode->getLeftChild();
			else if( key < keyToFind   ) 
				pNode = pNode->getRightChild();
			else
				return const_iterator(this,pNode);
		}

		return const_iterator(this,NULL);
	}

	iterator find_nearest_smaller(const KeyType& keyToFind)
	{
		TreeNode* pNode = mRoot;
		TreeNode* pNearstSmallerNode = minnum(mRoot);

		while(pNode!=0)
		{
			const KeyType& key(pNode->getKey());

			if (keyToFind < key)
				pNode = pNode->getLeftChild();
			else if( key < keyToFind   ) 
			{
				pNearstSmallerNode = pNode;
				pNode = pNode->getRightChild();
			}
			else
				return iterator(this,pNode);
		}

		return iterator(this,pNearstSmallerNode);
	}

	iterator find_nearest(const KeyType& keyToFind)
	{
		iterator it = find_nearest_smaller(keyToFind);
		if( it.m_pNode )
		{
			iterator itNext = it;
			++itNext;
			if( itNext.m_pNode )
			{
				KeyType dA = keyToFind - (*it);
				KeyType dB = (*itNext) - keyToFind;
				if( dA < dB )
				{
					return it;
				}
				else
					return itNext;
			}
		}

		return it;
	}

	inline TreeNode* getRoot() const
	{
		return mRoot;
	}

	inline int size() const
	{
		return mSize;
	}


	MyType& operator = (const MyType& ot)
	{
		clear();
		MyType::const_iterator it = ot.begin();
		while(it!=ot.end())
		{

			insert(*it);
			++it;
		}
		return*this;
	}



private:


	inline TreeNode* maxnum(TreeNode* pCur)
	{
		while(pCur&&pCur->right)
		{
			pCur = pCur->right;
		}
		return pCur;
	}

	inline TreeNode* minnum(TreeNode* pCur)
	{

		while(pCur&&pCur->left)
		{
			pCur = pCur->left;
		}
		return pCur;
	}

	inline const TreeNode* maxnum(const TreeNode* pCur) const
	{
		while(pCur&&pCur->right)
		{
			pCur = pCur->right;
		}
		return pCur;
	}

	inline const TreeNode* minnum(const TreeNode* pCur) const
	{

		while(pCur&&pCur->left)
		{
			pCur = pCur->left;
		}
		return pCur;
	}

	inline void setRoot(TreeNode* newRoot)
	{
		mRoot = newRoot;
		if (mRoot!=0)
		{
			mRoot->setParent(0);
			mRoot->setBlack();
		}
	}

	bool insert(TreeNode* newNode)
	{
		bool result=true; // Assume success

		if (mRoot==0)
		{
			setRoot(newNode);
			mSize = 1;
		}
		else
		{
			TreeNode* pNode = mRoot;
			const KeyType& keyNew = newNode->getKey();
			while (pNode)
			{
				const KeyType& key(pNode->getKey());


				if (keyNew < key)
				{
					if (pNode->getLeftChild() == 0)
					{
						pNode->setLeftChild(newNode);
						pNode = 0;
					}
					else
						pNode = pNode->getLeftChild();
				}
				else if ( key < keyNew  )
				{
					if (pNode->getRightChild()==0)
					{
						pNode->setRightChild(newNode);
						pNode = 0;
					}
					else
					{
						pNode = pNode->getRightChild();
					}
				}
				else if( EnableMultiInsert )
				{
					if (pNode->getRightChild()==0)
					{
						pNode->setRightChild(newNode);
						pNode = 0;
					}
					else
					{
						pNode = pNode->getRightChild();
					}
				}
				else
				{
					result = false;
					pNode = 0;
				}
			}

			if (result)
				mSize++;
		}

		return result;
	}

	void rotateLeft(TreeNode* p)
	{
		TreeNode* right = p->getRightChild();

		p->setRightChild(right->getLeftChild());

		if (p->isLeftChild())
			p->getParent()->setLeftChild(right);
		else if (p->isRightChild())
			p->getParent()->setRightChild(right);
		else
			setRoot(right);

		right->setLeftChild(p);
	}

	void rotateRight(TreeNode* p)
	{

		TreeNode* left = p->getLeftChild();

		p->setLeftChild(left->getRightChild());

		if (p->isLeftChild())
			p->getParent()->setLeftChild(left);
		else if (p->isRightChild())
			p->getParent()->setRightChild(left);
		else
			setRoot(left);

		left->setRightChild(p);
	}

	TreeNode* mRoot; 
	int mSize; 

};

template<class T>
class IFMultiRBTree : public IFRBTree<T,true>
{

};
