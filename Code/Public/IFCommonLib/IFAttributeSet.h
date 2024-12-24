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
#include "IFHashSet.h"
#include "IFString.h"
#include "IFRefObj.h"
#include "IFEventSlot.h"
#include "IFAttributeAccessor.h"

class IFAttributeList;
class IFAttribute;
class IFAttributeSet;

typedef IFRefPtr<IFAttributeSet> IFAttributeSetPtr;


class IFCOMMON_API IFAttributeSet :public  IFRefObj, public IFEventSlotStaticHost
{
	IF_DECLARERTTI;
public:

	static const IFEventSlot<void(IFAttributeSet* pObj,IFAttributeList* pNewAttList, IFAttributeList* pOldAttList)>	event_AttributeChange;
public:
	IFAttributeSet(void);



	void queryAttribute(IFAttributeList& attlist,const IFAttributeNameList* pNameList = NULL) ;	//属性查询
	void setAttribute(IFAttributeList& attlist, bool bEventNeedOldAttr = true);	//属性设置

	virtual IFAttributeSetPtr clone();
	virtual void assignTo(IFAttributeSet* pObj);

protected:
	void changeAttribute(IFAttributeList& newAttribute );


	virtual ~IFAttributeSet(void);
	IFString m_sSuperAttrName;
};
#define IF_CAN_PUT_ATTRIBUTE(name) (!pNameList ||(pNameList->find(name) != pNameList->end()))
#define IF_GET_ATTRIBUTE(atttype,attptr, attname) atttype* attptr = IFDynamicCast<atttype>(attlist.getAttribute( attname ))


template<typename TAttributeSetHolder, typename TSubAttributeSet>
class IFAttributeAccessorSubAttrForSubObject : public IFAttributeAccessor
{
public:
	typedef void (TAttributeSetHolder::* ActiveFunPtr)(bool active);
protected:

	IFAttributeNameList m_nameList;


	IFRefPtr< TSubAttributeSet> TAttributeSetHolder::* m_pSubPtr;
	ActiveFunPtr m_pActiveFun;
public:
	//IFAttributeAccessorSubAttr(IFAttributeAccessor* pAccessors, int count);
	IFAttributeAccessorSubAttrForSubObject(const IFString& name, const IFRTTI* pRTTI, const std::initializer_list<IFString>& subNameList,
		IFRefPtr< TSubAttributeSet> TAttributeSetHolder::* pSubPtr,
		ActiveFunPtr pActiveFun
		)
		:IFAttributeAccessor(name, pRTTI)
		, m_pSubPtr(pSubPtr)
		, m_pActiveFun(pActiveFun)
	{
		for (auto& n : subNameList)
		{
			m_nameList.insert(n);
		}
	}





	virtual IFAttributePtr getAttribute(IFObj* pTarget) const override
	{
		auto spSubAttrList = NewIFRefObj<IFAttrSubAttr>();
		
		auto pObj = (TAttributeSetHolder*)pTarget;
		auto spSubObj = pObj->*m_pSubPtr;

		if (spSubObj)
			spSubObj->queryAttribute(spSubAttrList->m_AttList, m_nameList.size()?&m_nameList:NULL);
		
		spSubAttrList->m_AttList.setAttribute(TAttributeSetHolder::attr_Use, NewIFRefObj<IFAttrBOOL>(spSubObj));
		
		return spSubAttrList;
	}
	virtual void setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const override
	{
		auto spSubAttrList = IFDynamicCast<IFAttrSubAttr>(pAttribute);
		if (!spSubAttrList)
			return;

		auto pUse = IFDynamicCast<IFAttrBOOL>(spSubAttrList->m_AttList.getAttribute(TAttributeSetHolder::attr_Use));
		
		auto pObj = (TAttributeSetHolder*)pTarget;

		if (m_pActiveFun && pUse)
		{
			(pObj->*m_pActiveFun)(pUse->get());
		}

		if (pObj->*m_pSubPtr)
		{			
			(pObj->*m_pSubPtr)->setAttribute(spSubAttrList->m_AttList);
		}
	}


};


template<typename TAttributeSetHolder, typename TSubAttributeSet>
inline  auto makeAttributeAccessor(const IFString& name, const std::initializer_list<IFString>& subNameList,
	IFRefPtr< TSubAttributeSet> TAttributeSetHolder::* pSubPtr,
	void (TAttributeSetHolder::* pActiveFun)(bool )
	)
{
	return IFAttributeAccessorSubAttrForSubObject<TAttributeSetHolder, TSubAttributeSet>(name, &TSubAttributeSet::m_Type, subNameList, pSubPtr, pActiveFun);
}

template<typename TAttributeSetHolder, typename TSubAttributeSet>
inline  auto makeAttributeAccessor(const IFString& name, const std::initializer_list<IFString>& subNameList,
	IFRefPtr< TSubAttributeSet> TAttributeSetHolder::* pSubPtr
)
{
	return IFAttributeAccessorSubAttrForSubObject<TAttributeSetHolder, TSubAttributeSet>(name, &TSubAttributeSet::m_Type, subNameList, pSubPtr, NULL);
}