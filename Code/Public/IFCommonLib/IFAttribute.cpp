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
#include "IFAttribute.h"
#include "IFAttributeSet.h"
#include "IFSuperAttrMgr.h"
#include "IFLogSystem.h"
#include "IFUtility.h"
#include "IFAttributeMgr.h"
#include "IFJSON.h"
#include <stdio.h>
#include <wchar.h>




IF_DEFINERTTI( IFAttribute, IFRefObj );

IF_DEFINERTTI( IFAttrINT, IFAttribute );

IF_DEFINERTTI( IFAttrFLOAT, IFAttribute );

IF_DEFINERTTI( IFAttrSTR, IFAttribute );

IF_DEFINERTTI( IFAttrSTRFileName, IFAttrSTR );
IF_DEFINERTTI( IFAttrLongStr, IFAttrSTR );

IF_DEFINERTTI( IFAttrBOOL, IFAttrENUM );

//IF_DEFINERTTI( IFUIAttrUV, IFAttribute );

IF_DEFINERTTI( IFAttrCOLOR, IFAttrSubAttr );


IF_DEFINERTTI( IFAttrENUM, IFAttribute );
IF_DEFINERTTI( IFAttrCombine, IFAttrENUM );

IF_DEFINERTTI( IFAttrSubAttr, IFAttribute);
IF_DEFINERTTI( IFAttrRECT, IFAttrSubAttr );



IF_DEFINERTTI( IFAttributeList, IFObj)

IF_DEFINERTTI( IFAttrSTRRefObj, IFAttrSTR);

IF_DEFINERTTI(IFAttrENUMSTR,IFAttrENUM);

IF_DEFINERTTI(IFAttrFixNumber,IFAttribute);






IF_DEFINECREATEOBJ(IFAttrINT			)
IF_DEFINECREATEOBJ(IFAttrFLOAT			)
IF_DEFINECREATEOBJ(IFAttrSTR			)
IF_DEFINECREATEOBJ(IFAttrSTRFileName	)
IF_DEFINECREATEOBJ(IFAttrLongStr	)
IF_DEFINECREATEOBJ(IFAttrBOOL			)
IF_DEFINECREATEOBJ(IFAttrCOLOR			)
IF_DEFINECREATEOBJ(IFAttrENUM			)
IF_DEFINECREATEOBJ(IFAttrCombine		)
IF_DEFINECREATEOBJ(IFAttrSubAttr		)
IF_DEFINECREATEOBJ(IFAttrRECT			)


IF_DEFINECREATEOBJ(IFAttrSTRRefObj			)


IF_DEFINECREATEOBJ(IFAttrENUMSTR)
IF_DEFINECREATEOBJ(IFAttrFixNumber)


IFObj* IFAttribute::CreateObjStatic()
{
	return NULL;
}




IFAttributePtr IFAttributeList::getAttribute(const IFString& attrName)
{
	AttributeList::iterator it = m_AttrList.find( attrName );
	if( it != m_AttrList.end() )
		return it->second;
	return NULL;
}

void IFAttributeList::removeAttribute(const IFString& attrName )
{
	AttributeList::iterator it = m_AttrList.find( attrName );
	if( it != m_AttrList.end() )
	{
		m_AttrList.erase(it);
	}

}

void IFAttributeList::setAttribute(const IFString& attrName,IFAttributePtr pAttribute )
{
	m_AttrList[attrName] = pAttribute;
	pAttribute->setIndex(m_nCurIndex);
	m_nCurIndex ++;
}

IFI32 IFAttributeList::getAttrCount() const
{

	return m_AttrList.size();
}

void IFAttributeList::clear()
{
	m_AttrList.clear();
}


bool IFAttributeList::loadFromBinary(const char* pBuf, IFI32 nSize)
{


	return true;
}

IFI32 IFAttributeList::saveToBinary(char* pBuf, IFI32 nSize ) const
{

	return 0;
}


const IFAttributeList::AttributeList& IFAttributeList::getAttrList() const
{

	return m_AttrList;
}


bool IFAttributeList::loadFromJSON( IFJSONNode* pNode )
{
	if (IFJSONNode* pSA = pNode->getSubNode(IFAttributeMgr::xml_superattname))
	{
		setSuperAttrList(pSA->getValue().getString());
	}
	IFJSONNode* pA = pNode->getSubNode(IFAttributeMgr::xml_attnodename);
	if (pA)
	{
		pA->for_each_map([&](const IFString& k, IFJSONNode* pValue)
		{
			int nAttID = -1;

			if ( IFJSONNode* pVN = pValue->getSubNode(IFAttributeMgr::xml_atttypindexename))
			{
				nAttID = pVN->getValue().getInt();
			}
			else
				return false;
			IFRefPtr<IFAttribute> pAtt = IFAttributeMgr::getSingleton().createAttributeByIndex(nAttID);
			if (pAtt)
			{
				pAtt->loadFromJSON(pValue);
				m_AttrList[k] = pAtt;
				return true;
			}
			else
				return false;


		});
	}
	return true;

}


void IFAttributeList::simplify()
{
	if (IFAttributeList* pSuperAttList = getSuperAttrList())
		removeSameAttr(	*pSuperAttList);
}


bool IFAttributeList::saveToJSON( IFJSONNode* pNode ) const
{
	auto it = m_AttrList.begin();
	if(m_sSuperAttrName.size())
	{
		IFRefPtr<IFJSONNode> spSA = IFNew IFJSONNode;
		spSA->setValue(m_sSuperAttrName.toUTF8String());
		pNode->insert(IFAttributeMgr::xml_superattname, spSA);
	}

	IFRefPtr<IFJSONNode> spAtts = IFNew IFJSONNode;
	pNode->insert(IFAttributeMgr::xml_attnodename, spAtts);
	while( it != m_AttrList.end() )
	{
		if (isSameAsSuperAttr(it->first, it->second))
		{
			++it;
			continue;
		}

		IFStringW sValue;

		int nAttID = IFAttributeMgr::getSingleton().getIndexByName(it->second->GetTypeName());

		IFRefPtr<IFJSONNode> spAtt = IFNew IFJSONNode;
		IFRefPtr<IFJSONNode> spAttID = IFNew IFJSONNode;
		spAttID->setValue(nAttID);
		spAtt->insert(IFAttributeMgr::xml_atttypindexename, spAttID);

		spAtts->insert(it->first, spAtt);

		it->second->saveToJSON(spAtt);

		++ it;
	}

	return true;
}

void IFAttributeList::queryAttsByThisNameList(IFAttributeSet* pAttSet, IFAttributeList& attlist )
{
	AttributeList::iterator it = m_AttrList.begin();
	IFAttributeSet::AttibuteNameList nameList;
	while(it!=m_AttrList.end())
	{
		nameList.insert(it->first);
		++it;
	}

	pAttSet->queryAttribute(attlist, &nameList);
}

bool IFAttributeList::setSuperAttrList( const IFStringW& sSuperAttrName )
{
	m_sSuperAttrName = sSuperAttrName;
	return true;
}
//
IFAttributeList* IFAttributeList::getSuperAttrList()
{
	if(IFSuperAttrMgr::getSingletonPtr())
	{
		IFSuperAttrInfo* pSuperInfo = IFSuperAttrMgr::getSingletonPtr()->getSuperAttr(m_sSuperAttrName);
		if(pSuperInfo)
			return &pSuperInfo->m_AttributeList;
	}
	return NULL;
}

bool IFAttributeList::isEqual(const IFAttributeList& Other ) const
{
	if( Other.m_AttrList.size() == m_AttrList.size() )
	{
		auto myIter = m_AttrList.begin();
		auto otherIter = Other.m_AttrList.begin();
		while(myIter!=m_AttrList.end())
		{
			if(!myIter->second->isEqual(otherIter->second))
			{
				return false;
			}

			++myIter;
			++otherIter;
		}
		return true;
	}

	return false;
}

bool IFAttributeList::isSameAsSuperAttr( const IFString& sAttName, IFAttribute* pAtt ) const
{
	if(m_sSuperAttrName.size())
	{
		IFSuperAttrInfo* pSuperAttrInfo = IFSuperAttrMgr::getSingletonPtr()->getSuperAttr(m_sSuperAttrName );
		if(pSuperAttrInfo && &pSuperAttrInfo->m_AttributeList != this )
		{
			IFAttribute* pSuperAtt = pSuperAttrInfo->m_AttributeList.getAttribute(sAttName);
			if(pSuperAtt )
			{
				return pSuperAtt->isEqual(pAtt);
			}
			else
			{
				return pSuperAttrInfo->m_AttributeList.isSameAsSuperAttr(sAttName, pAtt);
			}
		}

	}

	return false;
}

IFAttributeList& IFAttributeList::operator=( const IFAttributeList& ot )
{
	m_nCurIndex = 0;
	m_sSuperAttrName = ot.m_sSuperAttrName;
	auto it = ot.m_AttrList.begin();
	m_AttrList.clear();
	while(it!=ot.m_AttrList.end())
	{
		setAttribute(it->first, it->second->clone());

		++ it;
	}

	return *this;
}

void IFAttributeList::removeSameAttr(const IFAttributeList& other)
{
	const AttributeList& attsother = other.getAttrList();
	for (auto pr:attsother)
	{
		if (IFAttribute* pAtt = getAttribute(pr.first))
		{
			IFAttrSubAttr* pSubA = IFDynamicCast<IFAttrSubAttr>(pAtt);
			IFAttrSubAttr* pSubB = IFDynamicCast<IFAttrSubAttr>(pr.second);
			if (pSubA&&pSubB)
			{
				pSubA->m_AttList.removeSameAttr(pSubB->m_AttList);
				if (pSubA->m_AttList.m_AttrList.size()==0)
				{
					removeAttribute(pr.first);
				}
				continue;
			}

			if (pAtt->isEqual(pr.second))
			{
				removeAttribute(pr.first);
			}
		}
	}
}

void IFAttributeList::merge(const IFAttributeList& o)
{
	auto it = o.m_AttrList.begin();
	while(it!=o.m_AttrList.end())
	{
		if (auto pAtt = getAttribute(it->first))
		{
			if (pAtt->GetType() == it->second->GetType())
			{
				if (IFAttrSubAttr* pSubAttr = IFDynamicCast<IFAttrSubAttr>(pAtt))
				{
					pSubAttr->m_AttList.merge(((IFAttrSubAttr*)(void*)(it->second))->m_AttList);
				}
				else
				{
					setAttribute(it->first, it->second->clone());
				}
			}
		}
		else
		{
			setAttribute(it->first, it->second->clone());
		}

		++ it;
	}
}

IFAttributeList::IFAttributeList(const IFAttributeList& ot)
{
	*this = ot;
}



//IFAttributeList::IFAttributeList( const IFAttributeList& attlist )
//{
//	operator=(attlist);
//}
//
//IFAttributeList& IFAttributeList::operator=( const IFAttributeList& ot )
//{
//	m_AttrList.clear();
//
//	AttributeList::const_iterator it = ot.m_AttrList.begin();
//	while(it!=ot.m_AttrList.end())
//	{
//		m_AttrList[it->first] = it->second->clone();
//		++it;
//	}
//
//	return *this;
//}

//////////////////////////////////////////////////////////////////////////
IFAttrSubAttr::IFAttrSubAttr(IFAttributeList& attrList)
{
	m_AttList = attrList;
}

IFAttrSubAttr::~IFAttrSubAttr()
{

}


IFStringW IFAttrSubAttr::getDisplayString()
{
	return L"[...]";
}


bool IFAttrSubAttr::loadFromJSON( IFJSONNode* pNode )
{
	return m_AttList.loadFromJSON(pNode);
}

bool IFAttrSubAttr::saveToJSON( IFJSONNode* pNode )
{
	return m_AttList.saveToJSON(pNode);
}


void IFAttrSubAttr::assignTo(IFAttribute* pAttribute)
{
	IFAttribute::assignTo(pAttribute);
	//IFAttributeList::AttributeList ls = m_AttList.getAttrList()
	IFAttrSubAttr* pAttr = (IFAttrSubAttr*)pAttribute;
	pAttr->m_AttList = m_AttList;
}

bool IFAttrSubAttr::isEqual( IFAttribute* pOther )
{
	if( pOther->GetType() == &IFAttrSubAttr::m_Type )
	{
		IFAttrSubAttr* pSubAttr = (IFAttrSubAttr*)pOther;
		return pSubAttr->m_AttList.isEqual(m_AttList);
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////

IFAttrCOLOR::IFAttrCOLOR(IFUI32 dwColor):m_dwColor(dwColor)
{

	toSubAttList();

}


IFStringW IFAttrCOLOR::getDisplayString()
{

	return IFStringW().format(L"%08X", m_dwColor );

}

void IFAttrCOLOR::set( const IFUI32& color)
{
	m_dwColor = color;
	toSubAttList();
}
IFUI32 IFAttrCOLOR::get()
{
	fromSubAttList();
	return m_dwColor;
}
void IFAttrCOLOR::toSubAttList()
{
	IFAttrINT* pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("A");
	if( pAtt )
		pAtt->set(m_dwColor >> 24 );
	else
		m_AttList.setAttribute("A" , IFNew IFAttrINT(m_dwColor >> 24));

	pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("R");
	if( pAtt )
		pAtt->set( (m_dwColor >> 16 )&0x000000ff);
	else
		m_AttList.setAttribute("R", IFNew IFAttrINT((m_dwColor >> 16 )&0x000000ff));

	pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("G");
	if( pAtt )
		pAtt->set((m_dwColor >> 8 )&0x000000ff );
	else
		m_AttList.setAttribute("G", IFNew IFAttrINT((m_dwColor >> 8 )&0x000000ff));

	pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("B");
	if( pAtt )
		pAtt->set((m_dwColor  )&0x000000ff);
	else
		m_AttList.setAttribute("B",	IFNew  IFAttrINT((m_dwColor  )&0x000000ff));
}
void IFAttrCOLOR::fromSubAttList()
{
	//m_dwColor = 0;
	IFAttribute* pAtt = m_AttList.getAttribute("A");
	if (pAtt)
	{
		m_dwColor &= 0x00FFFFFF;
		m_dwColor |= ((IFAttrINT*)pAtt)->get()<<24;
	}
	pAtt = m_AttList.getAttribute("R");
	if (pAtt)
	{
		m_dwColor &= 0xFF00FFFF;
		m_dwColor |= ((IFAttrINT*)pAtt)->get()<<16;
	}
	pAtt = m_AttList.getAttribute("G");
	if (pAtt)
	{
		m_dwColor &= 0xFFFF00FF;

		m_dwColor |= ((IFAttrINT*)pAtt)->get()<<8;

	}
	pAtt = m_AttList.getAttribute("B");
	if (pAtt)
	{
		m_dwColor &= 0xFFFFFF00;

		m_dwColor |= ((IFAttrINT*)pAtt)->get();

	}

}


void IFAttrCOLOR::assignTo(IFAttribute* pAttribute)
{
	IFAttrSubAttr::assignTo(pAttribute);
	IFAttrCOLOR* pCLR = (IFAttrCOLOR*)pAttribute;
	pCLR->m_dwColor = m_dwColor;
}

bool IFAttrCOLOR::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrCOLOR::m_Type )
	{
		return ((IFAttrCOLOR*)pOther)->m_dwColor == m_dwColor;
	}

	return false;
}

bool IFAttrCOLOR::loadFromJSON( IFJSONNode* pNode )
{
	if (IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_dwColor = (IFUI32)pVal->getValue().getInt();
		//return true;
	}
	else
		m_dwColor = -1;
	toSubAttList();

	return true;
}

bool IFAttrCOLOR::saveToJSON( IFJSONNode* pNode )
{
	fromSubAttList();
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue((IFI32)m_dwColor);
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}


//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

IFAttrRECT::IFAttrRECT()
{
	ZeroMemory(&m_RECT,sizeof(m_RECT));
}

IFAttrRECT::IFAttrRECT(const RECT& rect):
m_RECT(rect)
{
	toSubAttList();
}

IFAttrRECT::~IFAttrRECT()
{

}



IFStringW IFAttrRECT::getDisplayString()
{
	WCHAR buf[128];
	fromSubAttList();
	//sprintf_s(buf,128,"%d,%d,%d,%d", m_RECT.left, m_RECT.top, m_RECT.right, m_RECT.bottom );
#ifdef WIN32
	_snwprintf_s(buf, 128, _TRUNCATE, L"%d,%d,%d,%d", m_RECT.left, m_RECT.top, m_RECT.right, m_RECT.bottom );
#else
	swprintf(buf,sizeof(buf)/2, L"%d,%d,%d,%d", m_RECT.left, m_RECT.top, m_RECT.right, m_RECT.bottom );
#endif
	return buf;
}

void IFAttrRECT::set(const RECT&  rect)
{
	m_RECT = rect;
	toSubAttList();
}

const RECT& IFAttrRECT::get()
{
	fromSubAttList();
	return m_RECT;
}

void IFAttrRECT::toSubAttList()
{
	IFAttrINT* pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("Left");
	if( pAtt )
		pAtt->set( m_RECT.left );
	else
		m_AttList.setAttribute("Left" , IFNew IFAttrINT(m_RECT.left));

	pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("Top");
	if( pAtt )
		pAtt->set( m_RECT.top);
	else
		m_AttList.setAttribute("Top", IFNew IFAttrINT(m_RECT.top));

	pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("Right");
	if( pAtt )
		pAtt->set( m_RECT.right );
	else
		m_AttList.setAttribute("Right", IFNew IFAttrINT(m_RECT.right));

	pAtt = (IFAttrINT*)(IFAttribute*)m_AttList.getAttribute("Bottom");
	if( pAtt )
		pAtt->set( m_RECT.bottom);
	else
		m_AttList.setAttribute("Bottom",	IFNew  IFAttrINT(m_RECT.bottom));
}

void IFAttrRECT::fromSubAttList()
{
	IFAttribute* pAtt = m_AttList.getAttribute("Left");
	if (pAtt)
		m_RECT.left = ((IFAttrINT*)pAtt)->get();
	pAtt = m_AttList.getAttribute("Top");
	if (pAtt)
		m_RECT.top = ((IFAttrINT*)pAtt)->get();
	pAtt = m_AttList.getAttribute("Right");
	if (pAtt)
		m_RECT.right = ((IFAttrINT*)pAtt)->get();
	pAtt = m_AttList.getAttribute("Bottom");
	if (pAtt)
		m_RECT.bottom = ((IFAttrINT*)pAtt)->get();

}


void IFAttrRECT::assignTo(IFAttribute* pAttribute)
{
	IFAttrSubAttr::assignTo(pAttribute);
	IFAttrRECT* pRECT = (IFAttrRECT*)pAttribute;
	pRECT->m_RECT = m_RECT;
}

bool IFAttrRECT::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrRECT::m_Type )
	{
		return memcmp(&((IFAttrRECT*)pOther)->m_RECT, &m_RECT, sizeof(m_RECT)) == 0;
	}

	return false;
}

bool IFAttrRECT::loadFromJSON( IFJSONNode* pNode )
{
	if(IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		StringList sl;
		USplitStrings( &sl, pVal->getValue().getString().c_str(), "," );
		if( sl.size() != 4)
			return false;
		m_RECT.left = atoi(sl[0].c_str());
		m_RECT.top = atoi(sl[1].c_str());
		m_RECT.right = atoi(sl[2].c_str());
		m_RECT.bottom = atoi(sl[3].c_str());
	}
	else
	{
		ZeroMemory(&m_RECT,sizeof(m_RECT));
	}

	toSubAttList();
	return true;
}

bool IFAttrRECT::saveToJSON( IFJSONNode* pNode )
{
	fromSubAttList();
	IFString s;
	s.format("%d,%d,%d,%d", m_RECT.left, m_RECT.top, m_RECT.right, m_RECT.bottom);
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(s);
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}


//////////////////////////////////////////////////////////////////////////




bool IFAttrINT::loadFromJSON( IFJSONNode* pNode )
{
	if ( IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_nN = pVal->getValue().getInt();
		return true;
	}
	return false;
}

bool IFAttrINT::saveToJSON( IFJSONNode* pNode )
{
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(m_nN);
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}


IFStringW IFAttrINT::getDisplayString()
{
	WCHAR buf[16];
#ifdef WIN32
	_snwprintf_s(buf,_TRUNCATE, L"%d", m_nN );
#else
	swprintf(buf, 16, L"%d", m_nN );
#endif
	return buf;
}

void IFAttrINT::set( const int& n )
{
	m_nN = n;
}

const int& IFAttrINT::get()
{
	return m_nN;
}

void IFAttrINT::assignTo(IFAttribute* pAttribute)
{
	IFAttribute::assignTo(pAttribute);
	IFAttrINT* pINT =(IFAttrINT*)pAttribute;
	pINT->m_nN = m_nN;
}

bool IFAttrINT::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrINT::m_Type)
	{

		return ((IFAttrINT*)pOther)->m_nN == m_nN;
	}

	return false;
}



bool IFAttrFLOAT::loadFromJSON( IFJSONNode* pNode )
{
	if ( IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_fF = pVal->getValue().getFloat();
		return true;
	}
	return false;
}

bool IFAttrFLOAT::saveToJSON( IFJSONNode* pNode )
{
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(m_fF);
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}

IFStringW IFAttrFLOAT::getDisplayString()
{
	return IFStringW().format(L"%f", m_fF);
}

void IFAttrFLOAT::assignTo(IFAttribute* pAttribute)
{
	IFAttribute::assignTo(pAttribute);
	IFAttrFLOAT* pFLOAT = (IFAttrFLOAT*)pAttribute;
	pFLOAT->m_fF = m_fF;
}

bool IFAttrFLOAT::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrFLOAT::m_Type)
	{
		return ((IFAttrFLOAT*)pOther)->m_fF == m_fF;
	}
	return false;
}

void IFAttrSTR::assignTo(IFAttribute* pAttribute)
{
	IFAttribute::assignTo(pAttribute);
	IFAttrSTR* pSTR = (IFAttrSTR*)pAttribute;
	pSTR->m_sS = m_sS;
}

bool IFAttrSTR::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrSTR::m_Type )
	{
		return ((IFAttrSTR*)pOther)->m_sS == m_sS;
	}

	return false;
}

IFStringW IFAttrSTR::getDisplayString()
{
	return m_sS;
}

bool IFAttrSTR::loadFromJSON( IFJSONNode* pNode )
{
	if ( IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_sS = pVal->getValue().getString();
		return true;
	}
	return false;
}

bool IFAttrSTR::saveToJSON( IFJSONNode* pNode )
{
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(m_sS.toUTF8String());
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}

void IFAttrENUM::assignTo(IFAttribute* pAttribute)
{
	IFAttribute::assignTo(pAttribute);
	IFAttrENUM* pENUM = (IFAttrENUM*)pAttribute;
	pENUM->m_EnumList = m_EnumList;
	pENUM->m_nValue = m_nValue;
}

bool IFAttrENUM::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrENUM::m_Type )
	{
		return ((IFAttrENUM*)pOther)->m_nValue == m_nValue;
	}

	return false;

}

bool IFAttrENUM::loadFromJSON( IFJSONNode* pNode )
{
	//const char* sVal = pElement->Attribute(IFAttributeMgr::xml_attvaluename.c_str());
	if(IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_nValue =  pVal->getValue().getInt();
		return true;
	}

	return true;
}

bool IFAttrENUM::saveToJSON( IFJSONNode* pNode )
{
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(m_nValue);
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}



bool IFAttrBOOL::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrBOOL::m_Type )
	{
		return ((IFAttrBOOL*)pOther)->m_nValue == m_nValue;
	}

	return false;
}

bool IFAttrBOOL::loadFromJSON( IFJSONNode* pNode )
{
	if(IFJSONNode* pVal = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_nValue =  pVal->getValue().getBool()?1:0;
		return true;
	}

	return false;
}

bool IFAttrBOOL::saveToJSON( IFJSONNode* pNode )
{
	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(bool(m_nValue?true:false));
	pNode->insert(IFAttributeMgr::xml_attvaluename, spVal);
	return true;
}



bool IFAttrLongStr::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrLongStr::m_Type )
	{
		return ((IFAttrLongStr*)pOther)->m_sS == m_sS;
	}

	return false;
}

bool IFAttrSTRFileName::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrSTRFileName::m_Type )
	{
		return ((IFAttrSTRFileName*)pOther)->m_sS == m_sS;
	}

	return false;
}

bool IFAttrCombine::isEqual( IFAttribute* pOther )
{
	if(pOther->GetType() == &IFAttrCombine::m_Type )
	{
		return ((IFAttrCombine*)pOther)->m_nValue == m_nValue;
	}

	return false;
}

void IFAttribute::setGroup(const IFStringW& sGroupName)
{

}

const IFStringW& IFAttribute::getGroup()
{
	return IFStringW::Empty;
}

IFAttributePtr IFAttribute::clone()
{
	IFObj* pObj = CreateObj();
	if(IFRefPtr<IFAttribute> pAttr = IFDynamicCast<IFAttribute>(pObj) )
	{
		assignTo(pAttr);
		return pAttr;
	}
	delete pObj;
	return NULL;
}

void IFAttribute::assignTo( IFAttribute* pAttribute )
{
	pAttribute->m_nIndex = m_nIndex;
	pAttribute->m_bReadOnly = m_bReadOnly;
	pAttribute->m_sHelpString = m_sHelpString;
	pAttribute->m_sAliasName = m_sAliasName;
}



//////////////////////////////////////////////////////////////////////////


IFStringW IFAttrENUMSTR::get()
{
	IFStringW s;
	for(int i = 0; i < m_EnumList.size(); i ++ )
	{
		if(m_EnumList[i].nValue == m_nValue)
		{
			s = m_EnumList[i].sName;
			return s;
		}
	}


	return IFStringW::Empty;
}

bool IFAttrENUMSTR::isEqual(IFAttribute* pOther)
{
	if(pOther->GetType() == &IFAttrENUMSTR::m_Type )
	{
		return ((IFAttrENUMSTR*)pOther)->m_nValue == m_nValue;
	}

	return false;
}

bool IFAttrENUMSTR::loadFromJSON( IFJSONNode* pNode )
{
	IFAttrENUM::loadFromJSON(pNode);
	//const char* sVal = pElement->Attribute("str");
	if(IFJSONNode* pVal = pNode->getSubNode("str"))
	{
		//IFString s = sVal;
		//s.setUTF8Codeing(true);
		m_EnumList.push_back(IFENUM(pVal->getValue().getString(),m_nValue));
		return true;
	}
	return false;
}

bool IFAttrENUMSTR::saveToJSON( IFJSONNode* pNode )
{
	IFAttrENUM::saveToJSON(pNode);


	IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
	spVal->setValue(get().toUTF8String());
	pNode->insert("str", spVal);
	return true;
}


bool IFAttrFixNumber::loadFromJSON(IFJSONNode* pNode)
{
	if (IFJSONNode* pValNode = pNode->getSubNode(IFAttributeMgr::xml_attvaluename))
	{
		m_nN = pValNode->getValue().toFixNumber();
	}
	else
		m_nN = IFFixNumber::N_0;
	return true;
}

bool IFAttrFixNumber::saveToJSON(IFJSONNode* pNode)
{
	pNode->setSubValue(IFAttributeMgr::xml_attvaluename, m_nN);
	return true;
}

IFStringW IFAttrFixNumber::getDisplayString()
{
	return m_nN.toString();
}

void IFAttrFixNumber::set(const IFFixNumber& n)
{
	m_nN = n;
}

const IFFixNumber& IFAttrFixNumber::get()
{
	return m_nN;
}

bool IFAttrFixNumber::isEqual(IFAttribute* pOther)
{
	IFAttrFixNumber* pF = IFDynamicCast<IFAttrFixNumber>(pOther);
	return pF && pF->m_nN == m_nN;
}

void IFAttrFixNumber::assignTo(IFAttribute* pAttribute)
{
	IFAttrFixNumber* pF = IFDynamicCast<IFAttrFixNumber>(pAttribute);
	if (pF)
	{
		pF->m_nN = m_nN;
	}
}
