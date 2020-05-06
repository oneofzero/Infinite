#pragma once
#include "IFJSON.h"

template<typename T>
inline IFRefPtr<IFJSONNode> makeIFJsonNode(const T& t)
{
	return t.toJson();
}


#define IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(TT)\
template<>\
inline IFRefPtr<IFJSONNode> makeIFJsonNode(TT n)\
{\
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;\
	spNode->setValue(n);\
	return spNode;\
}

IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFI8&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFI16&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFI32&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFI64&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFUI8&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFUI16&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFUI32&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFUI64&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const bool&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const float&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const double&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFString&);
IF_IMPLEMENT_IFANYBASIC_MAKE_JSON_NODE(const IFFixNumber&);


#define IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(N)\
inline bool loadFromIFJsonNode(N& n, IFJSONNode* pNode)\
{\
	if (!pNode)\
		return false;\
	n = pNode->getValue().toInt();\
	return true;\
}

inline bool loadFromIFJsonNode(IFI64& n, IFJSONNode* pNode)
{
	if (!pNode)
		return false;
	n = pNode->getValue().getInt64();
	return true;
}
inline bool loadFromIFJsonNode(IFUI64& n, IFJSONNode* pNode)
{
	if (!pNode)
		return false;
	n = pNode->getValue().getUint64();
	return true;
}

inline bool loadFromIFJsonNode(IFString& n, IFJSONNode* pNode)
{
	if (!pNode)
		return false;
	n = pNode->getValue().toString();
	return true;
}

inline bool loadFromIFJsonNode(IFFixNumber& n, IFJSONNode* pNode)
{
	if (!pNode)
		return false;
	n = pNode->getValue().toFixNumber();
	return true;
}

IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(IFI8);
IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(IFI16);
IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(IFI32);
IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(IFUI8);
IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(IFUI16);
IF_IMPLEMENT_IFANYBASIC_LOAD_JSON_NODE_INT(IFUI32);

template<typename T, int nNum>
inline IFRefPtr<IFJSONNode> makeIFJsonNode(T(&arr)[nNum])
{
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
	for (int i = 0; i < nNum; i ++)
	{
		IFRefPtr<IFJSONNode> spValue = makeIFJsonNode(arr[i]);
		spNode->push_back(spValue);
	}
	return spNode;
}

template<typename T, int nNum>
inline bool loadFromIFJsonNode(T(&arr)[nNum],IFJSONNode* pNode )
{
	if (!pNode ||pNode->getNodeType() != IFJSONNode::NT_ARRAY)
		return false;
	
	pNode->for_each_array([&](int nIdx, IFJSONNode* pValue)
	{
		if (nIdx >= nNum)
			return false;

		if (!loadFromIFJsonNode(arr[nIdx], pValue))
			return false;
		
		return true;
	});

	return true;
}

template<typename T>
inline bool loadFromIFJsonNode( IFArray<T>& arr,IFJSONNode* pNode )
{
	if (!pNode || pNode->getNodeType() != IFJSONNode::NT_ARRAY)
		return false;
	arr.resize(pNode->getSubNodeNum());

	pNode->for_each_array([&](int nIdx, IFJSONNode* pValue)
	{
		

		if (!loadFromIFJsonNode(arr[nIdx], pValue))
			return false;

		return true;
	});

	return true;
}

template<typename K, typename V>
inline IFRefPtr<IFJSONNode> makeIFJsonNode(const IFMap<K,V>& m)
{
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
	for (auto& pr:m)
	{
		IFRefPtr<IFJSONNode> spKey = makeIFJsonNode(m.first);
		IFRefPtr<IFJSONNode> spVal = makeIFJsonNode(m.second);

		spNode->insert(spKey->getValue().toString(), spVal);
	}
	return spNode;
}

template<typename T>
inline IFRefPtr<IFJSONNode> makeIFJsonNode(const IFArray<T>& arr)
{
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
	for (int i = 0; i < arr.size(); i ++)
	{
		IFRefPtr<IFJSONNode> spValue = makeIFJsonNode(arr[i]);
		spNode->push_back(spValue);
	}
	return spNode;
}

template<typename T>
inline IFRefPtr<IFJSONNode> makeIFJsonNode(const IFVector2DT<T>&v)
{
	IFRefPtr<IFJSONNode> spNode = IFNew IFJSONNode;
	spNode->insert("x", makeIFJsonNode(v.x));
	spNode->insert("y", makeIFJsonNode(v.y));
	return spNode;
}

template<typename T>
inline bool loadFromIFJsonNode(IFVector2DT<T>&v, IFJSONNode* pNode)
{
	return loadFromIFJsonNode(v.x, pNode->getSubNode("x")) &&
		loadFromIFJsonNode(v.y, pNode->getSubNode("y"));
}

template<typename T>
inline T loadFromIFJsonNode(IFJSONNode* pNode)
{
	T o;
	loadFromIFJsonNode(o, pNode);
	return o;
}