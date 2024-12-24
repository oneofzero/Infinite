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
#include "IFRefObj.h"
#include "IFRefPtr.h"
#include "IFMap.h"
#include "IFString.h"
#include "ifsingleton.h"
#include "IFArray.h"
#include "IFFixNumber.h"//#include "IFUITypes.h"
#include "IFHashMap.h"

class IFAttributeSet;

#define STRTRUEFALSE(v) (v)?"TRUE":"FALSE"
#define TRUEFALSE(v) strcmp( v, "TRUE")==0? TRUE:FALSE



#define IF_REGISTER_ATTRIBTE(attname) \
	IFObjectFactory::getSingleton().registerObject<attname>(); \
	IFAttributeMgr::getSingleton().registerAttribute(&attname::m_Type );\

typedef IFHashSet<IFString> IFAttributeNameList;



class IFAttribute;

class IFJSONNode;

typedef IFRefPtr<IFAttribute> IFAttributePtr;
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
#define IF_ATTRIBUTE_SET_HELP_STRING(pAttr, str) pAttr->setHelpString(str)
#define IF_ATTRIBUTE_SET_ALIAS_NAME(pAttr, str) pAttr->setAliasName(str)
#define IF_ATTRIBUTE_SET_READ_ONLY(pAttr, b) pAttr->setReadOnly(b)
#else
#define IF_ATTRIBUTE_SET_HELP_STRING(pAttr, str)
#define IF_ATTRIBUTE_SET_ALIAS_NAME(pAttr, str)
#define IF_ATTRIBUTE_SET_READ_ONLY(pAttr, str)
#endif

class IFCOMMON_API IFAttribute : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFAttribute()
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
	:m_bReadOnly(false)
#endif
	{

	}
	virtual ~IFAttribute()
	{

	}

	virtual bool loadFromJSON(IFJSONNode* pNode) = 0;
	virtual bool saveToJSON(IFJSONNode* pNode) = 0;

	virtual IFString getDisplayString() = 0;

#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT
	void setGroup(const IFString& sGroupName);
	const IFString& getGroup();

	int getIndex()
	{
		return m_nIndex;
	}
	void setIndex(int n)
	{
		m_nIndex = n;
	}

	void setReadOnly(bool bReadOnly)
	{
		m_bReadOnly = bReadOnly;
	}

	bool isReadOnly()
	{
		return m_bReadOnly;
	}

	void setHelpString(const IFString& s)
	{
		m_sHelpString = s;
	}
	
	void setAliasName(const IFString& s)
	{
		m_sAliasName = s;
	}

	const IFString& getAliasName()
	{
		return m_sAliasName;
	}

	const IFString& getHelpString()
	{
		return m_sHelpString;
	}
#else

#endif

	virtual bool isEqual(IFAttribute* pOther) = 0;

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) = 0;

	IFAttributePtr clone();

	//IFRefPtr<IFUIAttrUserObj> m_spRefObj;//可以挂个自定义的对象

protected:

	virtual void assignTo(IFAttribute* pAttribute);
#ifndef IF_ATTRIBUTE_NO_EDITOR_SUPPORT

	int m_nIndex;

	IFString m_sHelpString;
	IFString m_sAliasName;

	bool m_bReadOnly;
#endif
	
};

template<typename FT, typename VT>
inline VT IFAttributeGenericLerp(const VT& fromV, const VT& toV, FT f)
{
	return fromV + (VT)((toV - fromV) * f);
}



template<>
inline RECT IFAttributeGenericLerp(const RECT& fromV, const RECT& toV, float f)
{

	RECT rc = {
		IFAttributeGenericLerp(fromV.left, toV.left, f),
		IFAttributeGenericLerp(fromV.top, toV.top, f),
		IFAttributeGenericLerp(fromV.right, toV.right, f),
		IFAttributeGenericLerp(fromV.bottom, toV.bottom, f),
	};
	return rc;
}

template<typename FT = float, typename AttrT>
inline void IFAttributeLerp(AttrT* pFrom, IFAttribute* pTo, IFAttribute* pOut, FT f)
{
	auto pTTo = IFDynamicCast<AttrT>(pFrom);
	auto pTOut = IFDynamicCast< AttrT>(pTo);
	if (!pTTo || !pTOut)
		return;
	
	pTOut->set(IFAttributeGenericLerp(pFrom->get(), pTTo->get(), f));
}

template<typename FT = float, typename AttrT>
inline void IFAttributeCantLerp(AttrT* pFrom, IFAttribute* pTo, IFAttribute* pOut, FT f)
{
	auto pTTo = IFDynamicCast<AttrT>(pFrom);
	auto pTOut = IFDynamicCast< AttrT>(pTo);
	if (!pTTo || !pTOut)
		return;
	if (f == 1)
		pTOut->set(pTTo->get());
	else
		pTOut->set(pFrom->get());
}


class IFCOMMON_API IFAttributeList : public IFObj
{
	IF_DECLARERTTI;
public:
	typedef IFHashMap<IFString, IFAttributePtr> AttributeList;

public:
	IFAttributeList():m_nCurIndex(0){};

	IFAttributeList(const IFAttributeList& ot);

	IFAttributeList& operator = (const IFAttributeList& ot);

	IFAttributePtr getAttribute(const IFString& attrName) const;
	void setAttribute(const IFString& attrName,IFAttributePtr pAttribute );

	void removeAttribute(const IFString& attrName );

	void merge(const IFAttributeList& o);

	IFI32 getAttrCount() const;

	void clear();

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode) const;

	bool loadFromBinary(const char* pBuf, IFI32 nSize);
	IFI32 saveToBinary(char* pBuf, IFI32 nSize ) const;


	const AttributeList& getAttrList() const
	{
		return m_AttrList;
	}

	void queryAttsByThisNameList(IFAttributeSet* pAttSet, IFAttributeList& attlist) const;

	IFAttributeList* getSuperAttrList() const;

	bool setSuperAttrList(const IFString& sSuperAttrName);
	const IFString& getSuperAttrListName()
	{
		return m_sSuperAttrName;
	}

	virtual bool isEqual(const IFAttributeList& Other) const;

	bool isSameAsSuperAttr(const IFString& sAttName, IFAttribute* pAtt) const;

	void removeSameAttr(const IFAttributeList& other);

	void simplify();





	void queryAttributeFromObject(const IFRTTI* pType, IFObj* pObj, const IFAttributeNameList* pNameList);
	void setAttributeToObject(const IFRTTI* pType, IFObj* pObj);

	void lerp(const IFAttributeList& dest, IFAttributeList& out, float f) const;

protected:



	AttributeList m_AttrList;
	int m_nCurIndex;
	IFString m_sSuperAttrName;


};

class IFCOMMON_API IFAttrSubAttr : public IFAttribute
{
public:
	IF_DECLARERTTI;


	IFAttrSubAttr(){};
	IFAttrSubAttr(IFAttributeList& attrList);
	

	virtual bool isWhole() { return false; }

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();
	virtual bool isEqual(IFAttribute* pOther);
	IFAttributeList m_AttList;

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override;

protected:
	virtual ~IFAttrSubAttr();
	virtual void assignTo(IFAttribute* pAttribute);

};

class IFCOMMON_API IFAttrArrayAttr : public IFAttribute
{
public:
	IF_DECLARERTTI;


	IFAttrArrayAttr() {};
	IFAttrArrayAttr(const IFArray<IFAttributePtr>& attrList);
	virtual ~IFAttrArrayAttr();

	
	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();
	virtual bool isEqual(IFAttribute* pOther);

	IFAttributePtr m_spElementTemplate;
	IFArray<IFAttributePtr> m_AttributeArray;

	void set(const IFArray<IFAttributePtr>& o)
	{
		m_AttributeArray = o;
	}

	const IFArray<IFAttributePtr>& get()
	{
		return m_AttributeArray;
	}
	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override;

protected:
	virtual void assignTo(IFAttribute* pAttribute);

};

class IFCOMMON_API IFAttrINT : public IFAttribute
{
	IF_DECLARERTTI;

public:
	IFAttrINT():m_nN(0){};
	IFAttrINT(int n):m_nN(n)
	{

	}
	virtual ~IFAttrINT()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();
	virtual void set( const int& n);

	virtual const int& get();


	virtual bool isEqual(IFAttribute* pOther);

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override;
protected:
	virtual void assignTo(IFAttribute* pAttribute);

	int m_nN;
};

class IFCOMMON_API IFAttrFixNumber : public IFAttribute
{
	IF_DECLARERTTI;

public:
	IFAttrFixNumber():m_nN(0){};
	IFAttrFixNumber(const IFFixNumber& n):m_nN(n)
	{

	}
	virtual ~IFAttrFixNumber()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();
	virtual void set( const IFFixNumber& n);

	virtual const IFFixNumber& get();


	virtual bool isEqual(IFAttribute* pOther);

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override;

protected:
	virtual void assignTo(IFAttribute* pAttribute);

	IFFixNumber m_nN;
};

class IFCOMMON_API IFAttrFLOAT : public IFAttribute
{
	IF_DECLARERTTI;

public:
	IFAttrFLOAT(){};
	IFAttrFLOAT(float f):m_fF(f)
	{

	}
	virtual ~IFAttrFLOAT()
	{

	}


	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();

	virtual void set( const float f)
	{
		m_fF = f;
	}
	virtual float  get( )
	{
		return m_fF;
	}


	virtual bool isEqual(IFAttribute* pOther);

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override;
protected:
	virtual void assignTo(IFAttribute* pAttribute);

	float m_fF;
};

class IFCOMMON_API IFAttrSTR : public IFAttribute
{
	IF_DECLARERTTI;

public:
	IFAttrSTR(){}
	IFAttrSTR( const IFString& s):m_sS(s)
	{

	}
	virtual ~IFAttrSTR()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();

	virtual void set( const IFString& s)
	{		
		m_sS = s;
	}
	virtual const IFString& get( )
	{	
		return m_sS;
	}

	virtual bool isEqual(IFAttribute* pOther);

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override;
protected:

	virtual void assignTo(IFAttribute* pAttribute);

	IFString m_sS;
};

class IFCOMMON_API IFAttrLongStr : public IFAttrSTR
{
	IF_DECLARERTTI;

public:
	virtual IFString getDisplayString()
	{
		return "[RAWDATA]";
	}

	virtual bool isEqual(IFAttribute* pOther);
};

//这个用来挂其他的对象
class IFCOMMON_API IFAttrSTRRefObj : public IFAttrSTR
{
	IF_DECLARERTTI;

public:

};

class IFCOMMON_API IFAttrSTRFileName : public IFAttrSTR
{
	IF_DECLARERTTI;

public:
	IFAttrSTRFileName():m_bIsDirectory(false){}
	IFAttrSTRFileName( const IFString& s):IFAttrSTR(s), m_bIsDirectory(false)
	{

	}
	virtual ~IFAttrSTRFileName()
	{

	}

	bool m_bIsDirectory;
	virtual bool isEqual(IFAttribute* pOther);
};
 
class IFCOMMON_API IFAttrCOLOR : public IFAttrSubAttr
{
	IF_DECLARERTTI;

public:
	IFAttrCOLOR(){}
	IFAttrCOLOR(IFUI32 dwColor);
	virtual ~IFAttrCOLOR()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();
	virtual void set( const IFUI32& color);
	virtual IFUI32 get();
	void toSubAttList();
	void fromSubAttList();
	virtual bool isEqual(IFAttribute* pOther);

protected:
	virtual void assignTo(IFAttribute* pAttribute);

	IFUI32 m_dwColor;
};


class IFCOMMON_API IFAttrRECT:public IFAttrSubAttr
{
	IF_DECLARERTTI;

public:
	IFAttrRECT();
	IFAttrRECT(const RECT& rect);
	virtual ~IFAttrRECT();

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString();
	virtual void set(const RECT&  rect);
	virtual const RECT& get();
	void toSubAttList();
	void fromSubAttList();
	virtual bool isEqual(IFAttribute* pOther);

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override
	{
		IFAttributeLerp(this, pDest, pOut, f);
	}

protected:
	virtual void assignTo(IFAttribute* pAttribute);

	RECT m_RECT;
};



class IFCOMMON_API IFAttrENUM : public IFAttribute
{
	IF_DECLARERTTI;

	struct IFCOMMON_API IFENUM
	{
	public:
		IFENUM(const IFString& s, int nV):sName(s),nValue(nV)
		{

		}
		IFString sName;
		int nValue;
	};
	typedef IFArray<IFENUM> IFENUMLIST;
public:
	IFAttrENUM(){}
	IFAttrENUM(const IFENUMLIST& enums, int nVal):
	  m_EnumList(enums),m_nValue(nVal)
	{

	}

	virtual ~IFAttrENUM()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFString getDisplayString()
	{
		IFString str;
		for( int i = 0; i < (int)m_EnumList.size(); i ++ )
		{
			if(m_EnumList[i].nValue == m_nValue )
			{
				str = m_EnumList[i].sName;
				return str;
			}
		}

		return str;
	}

	virtual void set( int n)
	{
		m_nValue = n;
	}
	int get()
	{
		return m_nValue;
	}
	virtual bool isEqual(IFAttribute* pOther);
	IFENUMLIST m_EnumList;

	virtual void lerp(IFAttribute* pDest, IFAttribute* pOut, float f) override
	{
		IFAttributeLerp(this, pDest, pOut, f);
	}

protected:

	virtual void assignTo(IFAttribute* pAttribute);

	int	 m_nValue;
};
class IFCOMMON_API IFAttrBOOL : public IFAttrENUM
{
	IF_DECLARERTTI;

public:
	IFAttrBOOL()
	{
		m_EnumList.push_back( IFENUM("FALSE",0) );
		m_EnumList.push_back( IFENUM("TRUE",1) );
	}
	IFAttrBOOL( bool b)
	{
		m_EnumList.push_back( IFENUM("FALSE",0) );
		m_EnumList.push_back( IFENUM("TRUE",1) );
		m_nValue = b?1:0;

	}
	virtual ~IFAttrBOOL()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual void set( bool b)
	{
		m_nValue = b?1:0;
	}
	bool get()
	{
		return m_nValue?true:false;
	}
	virtual bool isEqual(IFAttribute* pOther);


	//BOOL m_bB;
};

class IFCOMMON_API IFAttrENUMSTR: public IFAttrENUM
{
	IF_DECLARERTTI;
public:

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	IFString get();
	virtual bool isEqual(IFAttribute* pOther);

};

class IFCOMMON_API IFAttrCombine: public IFAttrENUM
{
	IF_DECLARERTTI;

	
public:
	IFAttrCombine(){}
	IFAttrCombine(const IFENUMLIST& enums, int nVal):IFAttrENUM(enums,nVal)
	{

	}
	virtual ~IFAttrCombine()
	{

	}

	virtual IFString getDisplayString()
	{
		IFString str;
		bool bFirst = true;
		for( int i = 0; i < (int)m_EnumList.size(); i ++ )
		{
			if(m_EnumList[i].nValue & m_nValue )
			{
				if( !bFirst )
				{
					str += "|";
				}
				str += m_EnumList[i].sName;

				bFirst = false;
				
			}
		}

		return str;
	}
	virtual bool isEqual(IFAttribute* pOther);

};

//template<>
//inline void IFAttributeList::bindAttributeChange(const IFString& name, IFString& attr)
//{
//	auto p = IFDynamicCast<IFAttrSTR>(getAttribute(name));
//	if (p)
//		attr = p->get();
//}
//
//template<typename T>
//inline void IFAttributeList::bindAttributeChange(const IFString& name, IFArray<T>& attr)
//{
//	IFAttribute* pAttr = getAttribute(name);
//	if (IFAttrSubAttr* pSubAttr = IFDynamicCast<IFAttrSubAttr>(pAttr))
//	{
//		int ncount = IFDynamicCast<IFAttrINT>(pSubAttr->m_AttList.getAttribute("Count"))->get();
//		if (ncount < 0)
//			ncount = 0;
//
//		attr.resize(ncount);
//		for (int i = 0; i < ncount; i++)
//		{
//			pSubAttr->m_AttList.bindAttributeChange(IFString().format("ITEM%03d", i), attr[i]);
//			//auto p = IFDynamicCast<IFAttrSTR>(pSubAttr->m_AttList.getAttribute(IFString().format("ITEM%03d", i)));
//			//if (p)
//			//	m_ResourceDir[i] = p->get();
//		}
//		//setName(pSTR->get());
//	}
//}
//
