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
#include "IFAttributeMgr.h"
#include "IFJSON.h"
#include "IFAttributeSet.h"
#include "IFLogSystem.h"
#include "IFAttribute.h"

IF_DEFINESINGLETON(IFAttributeMgr);

const IFString IFAttributeMgr::xml_attnodename = "a";
const IFString IFAttributeMgr::xml_atttypindexename = "t";
const IFString IFAttributeMgr::xml_attname = "n";
const IFString IFAttributeMgr::xml_attvaluename = "v";
const IFString IFAttributeMgr::xml_superattname = "sa";


IFAttributeMgr::IFAttributeMgr():m_nAttrCount(0)
{
	IF_REGISTER_ATTRIBTE(IFAttrSubAttr);
	IF_REGISTER_ATTRIBTE(IFAttrArrayAttr);
	IF_REGISTER_ATTRIBTE(IFAttrINT);
	IF_REGISTER_ATTRIBTE(IFAttrFixNumber);
	IF_REGISTER_ATTRIBTE(IFAttrFLOAT);
	IF_REGISTER_ATTRIBTE(IFAttrSTR);
	IF_REGISTER_ATTRIBTE(IFAttrLongStr);
	IF_REGISTER_ATTRIBTE(IFAttrSTRFileName);
	IF_REGISTER_ATTRIBTE(IFAttrCOLOR);
	IF_REGISTER_ATTRIBTE(IFAttrRECT);
	IF_REGISTER_ATTRIBTE(IFAttrArrayAttr);
	IF_REGISTER_ATTRIBTE(IFAttrENUM);
	IF_REGISTER_ATTRIBTE(IFAttrBOOL);
	IF_REGISTER_ATTRIBTE(IFAttrENUMSTR);
	IF_REGISTER_ATTRIBTE(IFAttrCombine);
	IF_REGISTER_ATTRIBTE(IFAttrENUMSTR);



}

bool IFAttributeMgr::registerAttribute(const IFRTTI* pAttrRTTI)
{
	if (!pAttrRTTI->IsKindOf(&IFAttribute::m_Type))
	{
		IFLogError("IFAttributeMgr::registerAttribute error!%s is not kind of IFAttribute\r\n", pAttrRTTI->GetTypeName());
		return false;
	}

	
	AttriRegisterList::iterator it = m_RegisterList.find(pAttrRTTI->GetTypeName());
	if (it != m_RegisterList.end())
	{
		IFLogError("IFAttributeMgr::registerAttribute error!%s is registerd\r\n", pAttrRTTI->GetTypeName());

		return false;
	}

	m_RegisterList[pAttrRTTI->GetTypeName()] = m_nAttrCount;
	m_AttrCreateInfo.push_back(pAttrRTTI);
	//m_AttrCreateInfo.push_back(AtrrCreateInfo(sAttributeName,pCreateFun));
	//	m_AttrIndexRemap[m_nAttrCount] = m_nAttrCount;
	m_nAttrCount ++;

	return true;
}

IFAttributePtr IFAttributeMgr::createAttributeByIndex( int nIndex )
{
	if(m_spRemap)
	{
		nIndex = m_spRemap->getIndex(nIndex);
	}
	
	if (nIndex > -1 && nIndex < m_nAttrCount)
	{
		return IFObjectFactory::getSingleton().createIFRefObj<IFAttribute>(m_AttrCreateInfo[nIndex].pRTTI);
		//return (*m_AttrCreateInfo[nIndex].pCreateFun)();
	}
	return NULL;
}

int IFAttributeMgr::getIndexByName( const IFString& sAttTypeName )
{
	AttriRegisterList::iterator it = m_RegisterList.find(sAttTypeName);
	if(it!=m_RegisterList.end())
		return it->second;
	return -1;
}

const IFString& IFAttributeMgr::getNameByIndex( int nIndex )
{
	if(nIndex>-1 && nIndex<m_nAttrCount)
	{
		return m_AttrCreateInfo[nIndex].sName;
	}
	return IFString::Empty;
}

int IFAttributeMgr::getAttributeTypeCount()
{
	return m_nAttrCount;
}

void IFAttributeMgr::setIndexRemap( IFAttributeRemap* pRemap )
{
	m_spRemap = pRemap;
}

IFAttributeRemap* IFAttributeMgr::getIndexRemap()
{
	return m_spRemap;
}

const IFAttributeList& IFAttributeMgr::getDefaultAttribute(const IFString& sClassName)
{
	auto it = m_DefaultControlAttri.find(sClassName);
	if (it!=m_DefaultControlAttri.end())
		return it->second;

	IFRefPtr<IFAttributeSet> spSet = IFObjectFactory::getSingleton().createObject(sClassName);
	if (spSet)
	{
		IFAttributeList& attlist = m_DefaultControlAttri[sClassName];
		spSet->queryAttribute(attlist);
		return attlist;
	}
	static IFAttributeList al;
	return al;
}

void IFAttributeMgr::clearDefaultAttribute(const IFString& sClassName)
{
	auto it = m_DefaultControlAttri.find(sClassName);
	if (it!=m_DefaultControlAttri.end())
		m_DefaultControlAttri.erase(it);

}

bool IFAttributeRemap::loadFromJSON( IFJSONNode* pNode )
{
	IFJSONNode* pVal = pNode->getSubNode("attidmap");
	if (!pVal)
	{
		return false;
	}

	pVal->for_each_map([&](const IFString& k, IFJSONNode* pVal)
	{
		int nOldIndex = pVal->getValue().getInt();
		int nCurIndex = IFAttributeMgr::getSingleton().getIndexByName(k);
		if(nCurIndex<0xffff && nCurIndex>=0 )
		{
			if(nOldIndex<m_RemapIndex.size())
				m_RemapIndex[nOldIndex] = nCurIndex;
			else
			{
				m_RemapIndex.resize(nOldIndex+64);
				m_RemapIndex[nOldIndex] = nCurIndex;
			}
		}
		return true;
	});
	
	return true;
}

void IFAttributeRemap::saveTOJSON( IFJSONNode* pNode )
{
	//TiXmlElement element("attidmap");

	IFRefPtr<IFJSONNode> spAtts =IFNew IFJSONNode;
	for (int i = 0; i < IFAttributeMgr::getSingleton().getAttributeTypeCount(); i ++ )
	{
		const IFString& sName = IFAttributeMgr::getSingleton().getNameByIndex(i);
		//element.SetAttribute(sName.c_str(), i);
		IFRefPtr<IFJSONNode> spVal = IFNew IFJSONNode;
		spVal->setValue(i);
		spAtts->insert(sName,spVal);
	}
	pNode->insert("attidmap", spAtts);
}



IFAttributeRemap::IFAttributeRemap()
{
	m_RemapIndex.resize(IFAttributeMgr::getSingleton().getAttributeTypeCount());
	for (int i = 0; i < m_RemapIndex.size(); i ++ )
	{
		m_RemapIndex[i] = i;
	}
}

IFAttributeRemap::~IFAttributeRemap()
{

}

