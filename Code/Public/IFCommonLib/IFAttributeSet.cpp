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
#include "IFAttributeSet.h"
#include "IFAttribute.h"
#include "IFObjectFactory.h"
#include "IFLogSystem.h"

IF_DEFINERTTI(IFAttributeSet, IFRefObj)

IFAttributeSet::IFAttributeSet(void)
{
}

IFAttributeSet::~IFAttributeSet(void)
{
}

void IFAttributeSet::setAttribute( IFAttributeList& attlist,bool bEventNeedOldAttr )
{
	//IFRefPtr<IFRefObj> holdThis(this);
	IFREFHOLDTHIS();
	if(attlist.getSuperAttrList() && m_sSuperAttrName != attlist.getSuperAttrListName())
	{
		m_sSuperAttrName = attlist.getSuperAttrListName();

		setAttribute(*attlist.getSuperAttrList(), false);

		m_sSuperAttrName = attlist.getSuperAttrListName();

	}

	if(bEventNeedOldAttr && eventInstance(event_AttributeChange).hasHandle())
	{


		IFAttributeList oldAttrlist;

		attlist.queryAttsByThisNameList(this, oldAttrlist);

		changeAttribute(attlist);

		/*IFAttributeList::AttributeList& changeAttList = attlist.getAttrList();
		IFAttributeList::AttributeList::iterator it = changeAttList.begin();
		while(it!=changeAttList.end())
		{
			if()

			++ it;
		}*/


		if(oldAttrlist.getAttrCount())
		{
			IFREFHOLDTHISCALL(fireEvent(event_AttributeChange,this,&attlist, &oldAttrlist));
		}
	}
	else
	{
		changeAttribute(attlist);
		IFREFHOLDTHISCALL(fireEvent(event_AttributeChange, this,&attlist, (IFAttributeList*)NULL))
	}

}



void IFAttributeSet::queryAttribute( IFAttributeList& attlist,const IFAttributeNameList* pNameList /*= NULL*/ )
{
	attlist.setSuperAttrList(m_sSuperAttrName);
	attlist.queryAttributeFromObject(GetType(), this, pNameList);

}

IFAttributeSetPtr IFAttributeSet::clone()
{
	//IFRefPtr<IFAttributeSet> pObj = (IFAttributeSet*)CreateObj();
	auto pObj = IFObjectFactory::getSingleton().createIFRefObj<IFAttributeSet>(GetType());
	if(pObj)
	{
		assignTo(pObj);
		return pObj;
	}

	else
	{
		IFLogError("can't create object:%s\r\n", GetType()->GetTypeName());
		return NULL;
	}
}

void IFAttributeSet::assignTo( IFAttributeSet* pObj )
{
	pObj->m_sSuperAttrName = m_sSuperAttrName;
}

void IFAttributeSet::changeAttribute( IFAttributeList& newAttribute )
{
	newAttribute.setAttributeToObject(GetType(), this);
}



const IFEventSlot<void(IFAttributeSet* pObj, IFAttributeList* pNewAttList, IFAttributeList* pOldAttList)>	IFAttributeSet::event_AttributeChange;
