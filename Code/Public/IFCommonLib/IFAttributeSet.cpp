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

	if(bEventNeedOldAttr && event_AttributeChange.hasHandle())
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
			IFREFHOLDTHISCALL(event_AttributeChange(this,&attlist, &oldAttrlist));
		}
	}
	else
	{
		changeAttribute(attlist);
		IFREFHOLDTHISCALL(event_AttributeChange(this,&attlist, (IFAttributeList*)NULL))
	}

}

void IFAttributeSet::queryAttribute( IFAttributeList& attlist,const AttibuteNameList* pNameList /*= NULL*/ )
{
	attlist.setSuperAttrList(m_sSuperAttrName);
}

IFAttributeSetPtr IFAttributeSet::clone()
{
	IFRefPtr<IFAttributeSet> pObj = (IFAttributeSet*)CreateObj();
	if(pObj)
	{
		assignTo(pObj);
		return pObj;
	}

	else
	{
		return NULL;
	}
}

void IFAttributeSet::assignTo( IFAttributeSet* pObj )
{
	pObj->m_sSuperAttrName = m_sSuperAttrName;
}

void IFAttributeSet::changeAttribute( IFAttributeList& newAttribute )
{

}


IF_DEFINECREATEOBJ(IFAttributeSet)

