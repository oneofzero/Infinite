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
#ifndef __IF_JSON_H__
#define __IF_JSON_H__
#include "IFCommonLib_API.h"
#include "IFRefObj.h"
#include "IFString.h"
#include "IFAnyBasic.h"
#include "IFList.h"


class FastStr;

class IFCOMMON_API IFJSONNode : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum NodeType : unsigned char
	{
		NT_NIL,
		NT_VALUE,
		NT_MAP,
		NT_ARRAY,

	};
public:
	IFJSONNode(void);


	const char* parse(const char*& sUTF8, bool bSortedmap = true);

	inline bool isMap()
	{
		return m_NT == NT_MAP;
	}

	inline bool isArray()
	{
		return m_NT == NT_ARRAY;
	}

	inline bool isValue()
	{
		return m_NT == NT_VALUE;
	}
	inline bool isNil()
	{
		return m_NT == NT_NIL;
	}
	
	const IFAnyBasic& getValue();
	const IFAnyBasic& getSubValue(const IFString& sName);
	const IFAnyBasic& getSubValue(int nIndex);
	const IFString& getSubKey(int nIndex);
	void setSubValue(const IFString& sName, const IFAnyBasic& val);
	bool setSubValue(int idx, const IFAnyBasic& val);

	IFJSONNode* getSubNode(const IFString& sName);
	IFJSONNode* getSubNode(int nIndex);
	void setSubNode(int nIndex, IFRefPtr<IFJSONNode> pNode);
	void setSubNode(const IFString& sName, IFRefPtr<IFJSONNode> pNode);


	int getSubNodeNum();

	bool removeSubNode(const IFString& sName);
	bool removeSubNode(int nIndex);

	void removeSameNode(IFJSONNode* pNode);

	void merge(IFJSONNode* pNode); //pNode overwrite this node

	IFRefPtr<IFJSONNode> clone();

	bool isEqual(IFJSONNode* pNode);

	NodeType getNodeType()
	{
		return m_NT;
	}

	template<typename F>
	bool for_each_map(F f)
	{
		if (m_NT == NT_MAP)
		{
			if (m_pMapInfo)
				for (auto it = m_pMapInfo->m_UnsortedMap.begin();it != m_pMapInfo->m_UnsortedMap.end(); ++it)
				{
					if (!f(it->first,it->second))
						return false;
				}
			return true;
		} 
		return false;
	}

	template<typename F>
	bool for_each_array(F f)
	{
		if (m_NT == NT_ARRAY)
		{
			if (m_pChildArray)
			{	
				for (int i = 0; i < m_pChildArray->size(); i ++  )
				{
					if(!f(i,(*m_pChildArray)[i]))
						return false;
				}
				return true;
			}
		}
		return false;
	}

	IFString toString(bool bVisibleFormat = true, bool bKeywordQM = true, const IFString& tb = IFString::Empty );
	void toString(IFString& s, IFString& tb, bool bVisibleFormat = true, bool bKeywordQM = true);
	void saveToStream(IFStream* pStream, bool bVisibleFormat = true, bool bKeywordQM = true, const IFString& tb = IFString::Empty);
	void saveToStream(IFStream* pStream, IFString& tb, bool bVisibleFormat = true, bool bKeywordQM = true);
	void setValue(const IFAnyBasic& basic);
	void push_back(IFJSONNode* pNode);
	void push_back(const IFAnyBasic& basic);
	void insert(const IFString& k, IFRefPtr<IFJSONNode> pNode, bool sortmap = true);

	void clear();


protected:
	~IFJSONNode(void);


	const char* parseMap(const char*& sUTF8, bool sortmap = true );
	const char* parseArray(const char*& sUTF8, bool sortmap = true);
	const char* parseString(const char*& sUTF8);
	const char* parseNumber(const char*& sUTF8);


	NodeType m_NT;


	//IFAnyBasic m_Value;
	typedef IFMap<IFString,IFRefPtr<IFJSONNode>> ChildMap;
	//ChildMap m_ChildMap;

	typedef IFArray<IFRefPtr<IFJSONNode>> ChildArray;

	typedef IFArray<IFPair<IFString, IFRefPtr<IFJSONNode>>> ChildMapUnsorted;

	//ChildArray m_ChildArray;


	struct MapInfo : public IFMemObj
	{
		ChildMap m_ChildMap;
		ChildMapUnsorted m_UnsortedMap;

	};
	union 
	{
		IFAnyBasic* m_Value;
		ChildArray* m_pChildArray;
		MapInfo* m_pMapInfo;
	};
};

class IFCOMMON_API IFJSONParser : public IFMemObj
{
public:
	static IFRefPtr<IFJSONNode> parse(const IFString& s, IFString* pErrorDesc = NULL, bool bSortedmap = true);
	static IFRefPtr<IFJSONNode> parse(const char* s, IFString* pErrorDesc = NULL, bool bSortedmap = true);
	static IFRefPtr<IFJSONNode> parse(IFStream* pStram, IFString* pErrorDesc = NULL, bool bSortedmap = true);
private:

};
#endif //__IF_JSON_H__