#include "stdafx.h"
#include "IFNodeObj.h"
#include "IFUtility.h"
#include "IFObjectFactory.h"


IF_DEFINERTTI(IFNodeObj, IFAttributeSet);

IFNodeObj::IFNodeObj()
	:m_pParent(NULL)
{
	m_nCurIterIndex = -1;

}


IFNodeObj::~IFNodeObj()
{
	auto it = m_Children.begin();
	while (it != m_Children.end())
	{
		(*it)->onRemoveFromParent();

		(*it)->setParent( NULL);
		++it;
	}
}

void IFNodeObj::onRemoveFromParent()
{
	event_RemoveFromParent(this);
	m_InParentIterator = m_pParent->m_Children.end();
	//m_pParent = NULL;
	setParent(NULL);
}

void IFNodeObj::setParent(IFNodeObj* pParent)
{
	m_pParent = pParent;
}

bool IFNodeObj::setName(const IFString& sName)
{
	if (m_sName == sName)
		return true;
	if (m_pParent)
	{

		auto it = m_pParent->m_sSubChildNameList.find(sName);
		if (it != m_pParent->m_sSubChildNameList.end())
			return false;
		it = m_pParent->m_sSubChildNameList.find(m_sName);
		assert(it != m_pParent->m_sSubChildNameList.end());
		m_pParent->m_sSubChildNameList.erase(it);

		m_pParent->m_sSubChildNameList.insert(makeIFPair(sName, this));
	}
	m_sName = sName;
	return true;
}

bool IFNodeObj::addObj(IFNodeObj* spObj, bool bAutoReName /*= true*/)
{
	return addObjBefore(spObj, NULL, bAutoReName);
}

bool IFNodeObj::addObjBefore(IFNodeObj* spObj, IFNodeObj* spBefore, bool bAutoReName /*= true*/)
{
	if (!spObj || spObj == this)
		return false;

	if (spObj->m_pParent)
		return false;

	if (m_Children.find(spObj) != m_Children.end())
	{
		return false;
	}
	auto it = m_sSubChildNameList.find(spObj->getName());
	if (it != m_sSubChildNameList.end() || spObj->getName().size() == 0)
	{

		if (bAutoReName)
		{
			IFString newName;// = spObj->GetTypeName();
			IFMap<IFString, IFNodeObj*>::iterator itRename;

			int nNum = 0;
			if (m_sSubChildNameList.size())
			{
				IFString& sName = m_sSubChildNameList.rbegin()->first;
				int i = sName.length() - 1;
				for (; i >= 0; i--)
				{
					if (sName[i] <'0' || sName[i] > '9')
					{
						i++;
						break;
					}
				}

				if (i < (int)sName.length())
					nNum = atoi(&sName[i]);
				nNum++;
			}
			do
			{
				newName.format("%s%d", spObj->GetTypeName(), nNum);
				nNum++;
				itRename = m_sSubChildNameList.find(newName);
			} while (itRename != m_sSubChildNameList.end());

			//if()
			//	return false;
			spObj->setName(newName);


		}
		else
			return false;

	}
	//while(it)





	spObj->setParent(this);
	//spObj->m_DrawRect.x = m_DrawRect.x + spObj->m_Rect.x;
	//spObj->m_DrawRect.y = m_DrawRect.y + spObj->m_Rect.y;
	//spObj->refreshAnchorDistance();


	m_sSubChildNameList.insert(IFPair<IFString, IFNodeObj*>(spObj->getName(), spObj));

	bool bInserted = false;
	if (spBefore)
	{
		for (auto it = m_Children.begin(); it != m_Children.end(); ++it)
		{
			if ((*it) == spBefore)
			{
				spObj->m_InParentIterator = m_Children.insert(it, spObj);
				bInserted = true;
				break;
			}
		}

	}
	if (!bInserted)
	{

		spObj->m_InParentIterator = m_Children.insert(m_Children.end(), spObj);
		
		bInserted = true;

	}

	spObj->event_AddToParent(spObj);

	event_SubObjAdd(this, spObj);



	return true;
}



bool IFNodeObj::removeObj(IFNodeObj* pObj)
{
	IFREFHOLDTHIS();

	auto it = m_Children.find(pObj);

	if (it != m_Children.end())
	{

		IFNodeObjPtr spObj = pObj;



		auto itName = m_sSubChildNameList.find(spObj->getName());
		if (itName != m_sSubChildNameList.end())
			m_sSubChildNameList.erase(itName);


		event_SubObjRemove(this, spObj);
		//TODO:

		//if (m_CurIterateIt != m_Children.end() && (*m_CurIterateIt) == spObj)
		//{
		//	m_CurIterateIt = m_Children.erase(it);
		//}
		//else
		//	m_Children.erase(it);

		spObj->onRemoveFromParent();

		return true;
	}
	return false;
}

bool IFNodeObj::removeFromParent()
{
	if (m_pParent)
	{
		IFREFHOLDTHIS();

		m_pParent->removeObj(this);
		
		//m_pParent = NULL;

		return true;
	}
	return false;
}

void IFNodeObj::removeAllObj()
{
	IFREFHOLDTHIS();

	while (m_Children.size())
	{
		event_SubObjRemove(this, m_Children.back());
		m_Children.back()->onRemoveFromParent();
		m_Children.pop_back();
	}
	//m_CurIterateIt = m_Children.end();
	m_nCurIterIndex = 0;

	m_sSubChildNameList.clear();
	m_Children.clear();
}

IFAttributeSetPtr IFNodeObj::clone()
{
	IFNodeObjPtr pObj = (IFNodeObj*)(IFAttributeSet*)IFAttributeSet::clone();
	copySubObjTo(pObj);
	return pObj;
}

void IFNodeObj::assignTo(IFAttributeSet* pSet)
{
	IFAttributeSet::assignTo(pSet);
	IFNodeObj* pObj = (IFNodeObj*)pSet;
	pObj->m_sName = m_sName;

}

void IFNodeObj::copySubObjTo(IFNodeObj* pOther)
{
	auto it = m_Children.begin();
	while (it != m_Children.end())
	{
	
		IFNodeObjPtr pClone = (*it)->clone();
		pOther->addObj(pClone);
		

		++it;
	}
}


IFNodeObj* IFNodeObj::getNextSibling()
{
	if (m_pParent)
	{
		auto itNext = m_InParentIterator;
		++itNext;
		if (itNext != m_pParent->m_Children.end())
			return *itNext;
	}
	return NULL;
}


IFNodeObj* IFNodeObj::getPrevSibling()
{
	if (m_pParent)
	{
		auto itNext = m_InParentIterator;
		--itNext;
		if (itNext != m_pParent->m_Children.end())
			return *itNext;
	}
	return NULL;
}

IFNodeObj* IFNodeObj::getSubObj(const IFString& sName, bool bSearchChildNode)
{
	auto it = m_sSubChildNameList.find(sName);
	if (it != m_sSubChildNameList.end())
	{
		return it->second;
	}
	if (bSearchChildNode)
	{
		for (auto it = m_Children.begin(); it != m_Children.end(); ++it)
		{
			if (IFNodeObj* pObj = (*it)->getSubObj(sName, true))
			{
				return pObj;
			}
		}
	}
	return NULL;
}

IFNodeObj* IFNodeObj::getSubObj(int nIndex)
{
	if (nIndex >= 0 && nIndex < m_Children.size())
		return m_Children[nIndex];
	return NULL;
}

IFNodeObj* IFNodeObj::getObj(const IFString& sName)
{
	StringList sl;
	USplitStrings(&sl, sName.c_str(), ".");
	IFNodeObj* pObj = this;
	for (int i = 0; i < sl.size(); i++)
	{
		pObj = pObj->getSubObj(sl[i]);
		if (!pObj)
			return NULL;
	}

	return pObj;
}


IF_DEFINECREATEOBJ(IFNodeObj);