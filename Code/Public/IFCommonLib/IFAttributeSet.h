﻿/*
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
#include "IFRBTree.h"
#include "IFString.h"
#include "IFRefObj.h"
#include "IFEventSlot.h"

class IFAttributeList;
class IFAttribute;
class IFAttributeSet;

typedef IFRefPtr<IFAttributeSet> IFAttributeSetPtr;

class IFCOMMON_API IFAttributeSet :public  IFRefObj
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;
public:
	typedef IFRBTree<IFString> AttibuteNameList;

	IFEventSlot<void(IFAttributeSet* pObj,IFAttributeList* pNewAttList, IFAttributeList* pOldAttList)>	event_AttributeChange;
public:
	IFAttributeSet(void);



	virtual void queryAttribute(IFAttributeList& attlist,const AttibuteNameList* pNameList = NULL) ;	//属性查询
	void setAttribute(IFAttributeList& attlist, bool bEventNeedOldAttr = true);	//属性设置

	virtual IFAttributeSetPtr clone();
	virtual void assignTo(IFAttributeSet* pObj);

protected:
	virtual void changeAttribute(IFAttributeList& newAttribute );


	virtual ~IFAttributeSet(void);
	IFStringW m_sSuperAttrName;
};
#define IF_CAN_PUT_ATTRIBUTE(name) (!pNameList ||(pNameList&& pNameList->find(name) != pNameList->end()))
#define IF_GET_ATTRIBUTE(atttype,attptr, attname) atttype* attptr = IFDynamicCast<atttype>(attlist.getAttribute( attname ))