#pragma once
#include "IFAttributeSet.h"
class IFNodeObj;
typedef IFRefPtr<IFNodeObj> IFNodeObjPtr;
typedef IFArray<IFNodeObjPtr> IFNodeObjList;
class IFCOMMON_API IFNodeObj : public IFAttributeSet
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;
public:
	IFEventSlot<void(IFNodeObj* pObj, IFNodeObj* pSubObj)>							event_SubObjAdd;
	IFEventSlot<void(IFNodeObj* pObj, IFNodeObj* pSubObj)>							event_SubObjRemove;
	IFEventSlot<void(IFNodeObj* pObj)>											event_AddToParent;
	IFEventSlot<void(IFNodeObj* pObj)>											event_RemoveFromParent;
public:
	IFNodeObj();
	bool setName(const IFString& sName);
	const IFString& getName()
	{
		return m_sName;
	}
	bool addObj(IFNodeObj* spObj, bool bAutoReName = true);
	virtual bool addObjBefore(IFNodeObj* spObj, IFNodeObj* spBefore, bool bAutoReName = true);
	virtual bool removeObj(IFNodeObj* spObj);
	virtual bool removeFromParent();
	virtual void removeAllObj();

	IFNodeObj* getParent()
	{
		return m_pParent;
	}
	IFNodeObj* getNextSibling();
	IFNodeObj* getPrevSibling();
	//只支持从子对象中查找
	IFNodeObj* getSubObj(const IFString& sName, bool bSearchChildNode = false);
	//支持通过路径查找.  比如 window1.button1
	IFNodeObj* getObj(const IFString& sName);

	IFNodeObj* getSubObj(int nIndex);


	template< typename RT, typename FUN>
	RT for_each(const FUN& f)
	{
		for (m_nCurIterIndex = m_Children.size()-1; m_nCurIterIndex>=0; m_nCurIterIndex--)
		{
			if (auto p = f(m_Children[m_nCurIterIndex]))
				return p;
		}
		return NULL;
	}
	int getSubObjNum()
	{
		return m_Children.size();
	}


	IFAttributeSetPtr clone();

	virtual void assignTo(IFAttributeSet* pObj);
	virtual void copySubObjTo(IFNodeObj* pOther);

protected:
	virtual ~IFNodeObj();
	virtual void onRemoveFromParent();
	
	virtual void setParent(IFNodeObj* pParent);

protected:
	IFString m_sName;
	IFNodeObj* m_pParent;
	IFNodeObjList m_Children;

	int m_nCurIterIndex;

	IFMap<IFString, IFNodeObj*> m_sSubChildNameList;

	//IFNodeObjList::reverse_iterator m_CurIterateIt;

	IFNodeObjList::iterator m_InParentIterator;
};

