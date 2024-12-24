#include "stdafx.h"
#include "IFAttributeAccessor.h"
#include "IFAttribute.h"


IFAttributeAccessor::IFAttributeAccessor(const IFString& name, const IFRTTI* pRTTI)
	:m_sName(name)
	,m_pRTTI(pRTTI)
{

}

IFAttributeAccessor::~IFAttributeAccessor()
{
	//IFStaticArray<int, int, int> aa(1, 2, 3);
	//std::tuple
	//MakeIFStaticArray(2,2,2);
}


void IFAttributeAccessor::setAttribute(IFObj* pTarget, IFAttributeList& attrlist) const
{
	auto spAttr = attrlist.getAttribute(m_sName);
	if (spAttr)
	{
		setAttribute(pTarget, spAttr);
	}
}

void IFAttributeAccessor::queryAttribute(IFObj* pTarget, IFAttributeList& attrlist, const IFAttributeNameList* pNameList) const
{
	if (!pNameList || pNameList->find(m_sName) != pNameList->end())
	{
		auto spAttribute = getAttribute(pTarget);
		if (!spAttribute)
			return;
		attrlist.setAttribute(m_sName, spAttribute);
	}
}



//IFAttributeAccessorSubAttr::IFAttributeAccessorSubAttr(const IFString& name, const IFRTTI* pRTTI, const std::initializer_list<IFAttributeAccessor*>& list)
//	:IFAttributeAccessor(name, pRTTI)
//{
//	m_nCount = (int)list.size();
//	m_pAccessor = new IFAttributeAccessor*[m_nCount];
//	auto pBegin = list.begin();
//	for (int i = 0; i < m_nCount; i++)
//	{
//		m_pAccessor[i] = *(pBegin+i);
//	}
//}



//IFAttributePtr IFAttributeAccessorSubAttr::getAttribute(IFObj* pTarget) const
//{
//	auto spSubAttr = NewIFRefObj<IFAttrSubAttr>();
//	for (int i = 0; i < m_nCount; i++)
//	{
//		m_pAccessor[i]->queryAttribute(pTarget, spSubAttr->m_AttList, NULL);
//	}
//
//	return spSubAttr;
//}
//
//void IFAttributeAccessorSubAttr::setAttribute(IFObj* pTarget, IFAttribute* pAttribute) const
//{
//	auto spSubAttr = IFDynamicCast<IFAttrSubAttr>(pAttribute);
//	if (!spSubAttr)
//		return;
//	for (int i = 0; i < m_nCount; i++)
//	{
//		m_pAccessor[i]->setAttribute(pTarget, spSubAttr->m_AttList);
//	}
//}
