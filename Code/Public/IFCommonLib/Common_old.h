#pragma once
#ifdef _INFINTEENGINE_EXPORTS_
#define  _INFINITEAPI_ __declspec(dllexport)
#else
#define  _INFINITEAPI_ __declspec(dllimport)
#endif
#pragma warning(disable:4251)
#pragma warning(disable:4661)
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <string>
#include "IFSingleton.h"
#include "IFTypes.h"


#define SAFE_release(p) if( p){p->release(); p = NULL;}
#define SAFE_Release(p) if( p){p->Release(); p = NULL;}
#define SAFE_DELETE(p) if(p){delete p;p = NULL;}
#define SAFE_DELETE_EX(p) if(p){delete []p;p = NULL;}

class CLASSTYPE
{
	const char* pTypeName;
	CLASSTYPE* pSuper;
public:
	CLASSTYPE( const char* sTypeName, CLASSTYPE* pSuperType )
	{
		pTypeName = sTypeName;
		pSuper = pSuperType;
	}
	const char* GetTypeName()const {return pTypeName;};
	const CLASSTYPE* GetSuperType()const {return pSuper;};
};
#define IF_DECLARERTTI \
public:\
	virtual const CLASSTYPE* GetType(){return &m_Type;};\
	virtual const char* GetTypeName(){return m_Type.GetTypeName();}\
	static CLASSTYPE m_Type;

#define IF_DEFINERTTI(className, superType)\
	CLASSTYPE className::m_Type( #className, &superType::m_Type );

#define IF_DEFINERTTIROOT(classname)\
CLASSTYPE classname::m_Type( #classname, NULL );

