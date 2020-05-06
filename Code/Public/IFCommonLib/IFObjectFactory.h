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
#include "ifsingleton.h"
#include "IFString.h"
#include "IFMap.h"
#include "IFObj.h"
#include "IFHashMap.h"
class IFObj;
class IFCOMMON_API IFObjectFactory : public IFSingleton<IFObjectFactory>, public IFObj
{
public:
	typedef IFObj* (CREATEOBJECTFUN)();
public:
	IFObjectFactory(void);
	virtual ~IFObjectFactory(void);


	IFObj* createObject(const IFString& sObjectName);

	bool registerObject(const IFString& sObjectName, CREATEOBJECTFUN* pCreateFunction );

private:

	typedef IFHashMap<IFString,CREATEOBJECTFUN*> ObjectCreateFunList;
	ObjectCreateFunList m_FunList;

};

//helper macro
//#define  IF_DECLARECREATEOBJ static IFObj* createObject();
#define  IF_DEFINECREATEOBJ(classname) IFObj* classname::CreateObjStatic(){return IFNew classname();} 

#define IF_REGISTER2OBJFACTOR(classname) IFObjectFactory::getSingleton().registerObject(#classname, &classname::CreateObjStatic)
