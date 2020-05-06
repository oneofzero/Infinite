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
	IFObjectFactory::getSingleton().registerObject(#attname, &attname::CreateObjStatic); \
	IFAttributeMgr::getSingleton().registerAttribute(makeIFFunctor((IFAttribute*(*)())attname::CreateObjStatic), #attname );\




class IFAttribute;

class IFJSONNode;

typedef IFRefPtr<IFAttribute> IFAttributePtr;

class IFCOMMON_API IFAttribute : public IFRefObj
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;
public:
	IFAttribute():m_bReadOnly(false)
	{

	}
	virtual ~IFAttribute()
	{

	}

	virtual bool loadFromJSON(IFJSONNode* pNode) = 0;
	virtual bool saveToJSON(IFJSONNode* pNode) = 0;

	virtual IFStringW getDisplayString() = 0;

	void setGroup(const IFStringW& sGroupName);
	const IFStringW& getGroup();

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

	void setHelpString(const IFStringW& s)
	{
		m_sHelpString = s;
	}
	
	void setAliasName(const IFStringW& s)
	{
		m_sAliasName = s;
	}

	const IFStringW& getAliasName()
	{
		return m_sAliasName;
	}

	const IFStringW& getHelpString()
	{
		return m_sHelpString;
	}

	virtual bool isEqual(IFAttribute* pOther) = 0;

	IFAttributePtr clone();

	//IFRefPtr<IFUIAttrUserObj> m_spRefObj;//可以挂个自定义的对象

protected:

	virtual void assignTo(IFAttribute* pAttribute);

	int m_nIndex;

	IFStringW m_sHelpString;
	IFStringW m_sAliasName;

	bool m_bReadOnly;
	
};




class IFCOMMON_API IFAttributeList : public IFObj
{
	IF_DECLARERTTI;
public:
	typedef IFHashMap<IFString, IFAttributePtr> AttributeList;

public:
	IFAttributeList():m_nCurIndex(0){};

	IFAttributeList(const IFAttributeList& ot);

	IFAttributeList& operator = (const IFAttributeList& ot);

	IFAttributePtr getAttribute(const IFString& attrName);
	void setAttribute(const IFString& attrName,IFAttributePtr pAttribute );

	void removeAttribute(const IFString& attrName );

	void merge(const IFAttributeList& o);

	IFI32 getAttrCount() const;

	void clear();

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode) const;

	bool loadFromBinary(const char* pBuf, IFI32 nSize);
	IFI32 saveToBinary(char* pBuf, IFI32 nSize ) const;


	const AttributeList& getAttrList() const;

	void queryAttsByThisNameList(IFAttributeSet* pAttSet, IFAttributeList& attlist);

	IFAttributeList* getSuperAttrList();

	bool setSuperAttrList(const IFStringW& sSuperAttrName);
	const IFStringW& getSuperAttrListName()
	{
		return m_sSuperAttrName;
	}

	virtual bool isEqual(const IFAttributeList& Other) const;

	bool isSameAsSuperAttr(const IFString& sAttName, IFAttribute* pAtt) const;

	void removeSameAttr(const IFAttributeList& other);

	void simplify();



protected:



	AttributeList m_AttrList;
	int m_nCurIndex;
	IFStringW m_sSuperAttrName;


};

class IFCOMMON_API IFAttrSubAttr : public IFAttribute
{
public:
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;


	IFAttrSubAttr(){};
	IFAttrSubAttr(IFAttributeList& attrList);
	virtual ~IFAttrSubAttr();


	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFStringW getDisplayString();
	virtual bool isEqual(IFAttribute* pOther);
	IFAttributeList m_AttList;

protected:
	virtual void assignTo(IFAttribute* pAttribute);

};

class IFCOMMON_API IFAttrINT : public IFAttribute
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

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

	virtual IFStringW getDisplayString();
	virtual void set( const int& n);

	virtual const int& get();


	virtual bool isEqual(IFAttribute* pOther);

protected:
	virtual void assignTo(IFAttribute* pAttribute);

	int m_nN;
};

class IFCOMMON_API IFAttrFixNumber : public IFAttribute
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

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

	virtual IFStringW getDisplayString();
	virtual void set( const IFFixNumber& n);

	virtual const IFFixNumber& get();


	virtual bool isEqual(IFAttribute* pOther);

protected:
	virtual void assignTo(IFAttribute* pAttribute);

	IFFixNumber m_nN;
};

class IFCOMMON_API IFAttrFLOAT : public IFAttribute
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

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

	virtual IFStringW getDisplayString();

	virtual void set( const float f)
	{
		m_fF = f;
	}
	virtual float  get( )
	{
		return m_fF;
	}


	virtual bool isEqual(IFAttribute* pOther);
protected:
	virtual void assignTo(IFAttribute* pAttribute);

	float m_fF;
};

class IFCOMMON_API IFAttrSTR : public IFAttribute
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

public:
	IFAttrSTR(){}
	IFAttrSTR( const IFStringW& s):m_sS(s)
	{

	}
	virtual ~IFAttrSTR()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFStringW getDisplayString();

	virtual void set( const IFStringW& s)
	{		
		m_sS = s;
	}
	virtual const IFStringW& get( )
	{	
		return m_sS;
	}

	virtual bool isEqual(IFAttribute* pOther);
protected:

	virtual void assignTo(IFAttribute* pAttribute);

	IFStringW m_sS;
};

class IFCOMMON_API IFAttrLongStr : public IFAttrSTR
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

public:
	virtual IFStringW getDisplayString()
	{
		return L"[RAWDATA]";
	}

	virtual bool isEqual(IFAttribute* pOther);
};

//这个用来挂其他的对象
class IFCOMMON_API IFAttrSTRRefObj : public IFAttrSTR
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

public:

};

class IFCOMMON_API IFAttrSTRFileName : public IFAttrSTR
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

public:
	IFAttrSTRFileName():m_bIsDirectory(false){}
	IFAttrSTRFileName( const IFStringW& s):IFAttrSTR(s)
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
	IF_DECLARECREATEABLE;

public:
	IFAttrCOLOR(){}
	IFAttrCOLOR(IFUI32 dwColor);
	virtual ~IFAttrCOLOR()
	{

	}

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFStringW getDisplayString();
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
	IF_DECLARECREATEABLE;

public:
	IFAttrRECT();
	IFAttrRECT(const RECT& rect);
	virtual ~IFAttrRECT();

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	virtual IFStringW getDisplayString();
	virtual void set(const RECT&  rect);
	virtual const RECT& get();
	void toSubAttList();
	void fromSubAttList();
	virtual bool isEqual(IFAttribute* pOther);

protected:
	virtual void assignTo(IFAttribute* pAttribute);

	RECT m_RECT;
};



class IFCOMMON_API IFAttrENUM : public IFAttribute
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

	struct IFCOMMON_API IFENUM
	{
	public:
		IFENUM(const IFStringW& s, int nV):sName(s),nValue(nV)
		{

		}
		IFStringW sName;
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

	virtual IFStringW getDisplayString()
	{
		IFStringW str;
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

protected:

	virtual void assignTo(IFAttribute* pAttribute);

	int	 m_nValue;
};
class IFCOMMON_API IFAttrBOOL : public IFAttrENUM
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

public:
	IFAttrBOOL()
	{
		m_EnumList.push_back( IFENUM(L"FALSE",0) );
		m_EnumList.push_back( IFENUM(L"TRUE",1) );
	}
	IFAttrBOOL( bool b)
	{
		m_EnumList.push_back( IFENUM(L"FALSE",0) );
		m_EnumList.push_back( IFENUM(L"TRUE",1) );
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
	IF_DECLARECREATEABLE;
public:

	bool loadFromJSON(IFJSONNode* pNode);
	bool saveToJSON(IFJSONNode* pNode);

	IFStringW get();
	virtual bool isEqual(IFAttribute* pOther);

};

class IFCOMMON_API IFAttrCombine: public IFAttrENUM
{
	IF_DECLARERTTI;
	IF_DECLARECREATEABLE;

	
public:
	IFAttrCombine(){}
	IFAttrCombine(const IFENUMLIST& enums, int nVal):IFAttrENUM(enums,nVal)
	{

	}
	virtual ~IFAttrCombine()
	{

	}

	virtual IFStringW getDisplayString()
	{
		IFStringW str;
		bool bFirst = true;
		for( int i = 0; i < (int)m_EnumList.size(); i ++ )
		{
			if(m_EnumList[i].nValue & m_nValue )
			{
				if( !bFirst )
				{
					str += L"|";
				}
				str += m_EnumList[i].sName;

				bFirst = false;
				
			}
		}

		return str;
	}
	virtual bool isEqual(IFAttribute* pOther);

};

