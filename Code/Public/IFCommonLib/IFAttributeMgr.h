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
#include "ifsingleton.h"
#include "IFObj.h"
#include "IFString.h"
#include "IFArray.h"
#include "IFFunctor.h"
#include "IFObjectFactory.h"
#include "IFAttribute.h"


class IFAttributeRemap;
class IFJSONNode;


class IFCOMMON_API IFAttributeMgr: public IFSingleton<IFAttributeMgr> , public IFMemObj
{
public:

	typedef IFRefPtr<IFFunctor<IFAttribute*()> > AttributeCreateFunPtr;
	typedef IFMap<IFString,IFUI32> AttriRegisterList;

	struct AtrrCreateInfo
	{
		AtrrCreateInfo(const char* name,AttributeCreateFunPtr pFun):
			sName(name),pCreateFun(pFun)
		{

		}
		IFString sName;
		AttributeCreateFunPtr pCreateFun;
	};
	static const IFString xml_attnodename;
	static const IFString xml_atttypindexename;
	static const IFString xml_attname;
	static const IFString xml_attvaluename;
	static const IFString xml_superattname;



public:

	IFAttributeMgr();
	void registerAttribute(AttributeCreateFunPtr pCreateFun, const char* sAttributeName);

	int getIndexByName(const IFString& sAttTypeName);
	IFAttribute* createAttributeByIndex(int nIndex);
	const IFString& getNameByIndex(int nIndex);
	int getAttributeTypeCount();

	void setIndexRemap(IFAttributeRemap* pRemap);
	IFAttributeRemap* getIndexRemap();

	const IFAttributeList& getDefaultAttribute(const IFString& sClassName);
	void clearDefaultAttribute(const IFString& sClassName);
private:
	int m_nAttrCount;

	IFArray<AtrrCreateInfo> m_AttrCreateInfo;

	IFRefPtr<IFAttributeRemap> m_spRemap;

	AttriRegisterList m_RegisterList;

	IFMap<IFString,IFAttributeList> m_DefaultControlAttri;

};

//class IFAttributeCreateFunTrans : public IFFunctor<IFAttribute*()>
//{
//public:
//	IFAttributeCreateFunTrans( IFObjectFactory::CREATEOBJECTFUN* pCreateFun)
//		:m_pCreateFun(pCreateFun)
//	{
//
//	}
//
//	IFAttribute* operator()()
//	{
//		return (IFAttribute*)(*m_pCreateFun)();
//	}
//
//	 IFObjectFactory::CREATEOBJECTFUN* m_pCreateFun;
//};

class IFCOMMON_API IFAttributeRemap : public IFRefObj
{
public:
	IFAttributeRemap();

	bool loadFromJSON(IFJSONNode* pNode);
	void saveTOJSON(IFJSONNode* pNode);

	inline int getIndex(int nIndex)
	{
		if(nIndex>=0&&nIndex<m_RemapIndex.size())
			return m_RemapIndex[nIndex];
		return -1;
	}

protected:
	~IFAttributeRemap();
	IFArray<int> m_RemapIndex;

};