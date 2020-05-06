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
// IFCommonLib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "IFCommonLib.h"
#include "IFAttribute.h"
#include "IFAttributeSet.h"
#include "IFNetMsgFactory.h"
#include "IFLogSystem.h"
#include "androidwcs.h"
#include "IFRSA.h"
#include "IFComPort.h"

void registerDefaultAttribute();

void registerDefaultObjects();

void NativeSystemAPIInit( 
#ifdef IFPLATFORM_ANDROID
	JNIEnv* jni
#endif
);

bool IFCommonLibInit(
#ifdef IFPLATFORM_ANDROID
	JNIEnv* jni
#endif
)
{


	NativeSystemAPIInit(
#ifdef IFPLATFORM_ANDROID
		jni
#endif
	);
	WCHAR buf[512];
	IFI64 n = 32123123123123113;
	//android_wsprintf(buf,IFArraySize(buf), L"%I64d", n);
	//IFNew IFLogSystem;

	IFLOG(IFLL_INFO, "Begin Initial IFCommonLib\r\n");

	IFNew IFFileSystem;
	
	IFLOG(IFLL_INFO, "IFFileSystem Initial Done\r\n");

	IFNew IFObjectFactory;
	IFLOG(IFLL_INFO, "IFObjectFactory Initial Done\r\n");

	IFNew IFAttributeMgr;
	IFLOG(IFLL_INFO, "IFAttributeMgr Initial Done\r\n");

	IFNew IFSuperAttrMgr;
	IFLOG(IFLL_INFO, "IFSuperAttrMgr Initial Done\r\n");

	//IFNew IFNetMsgFactory;
	//IFLOG(IFLL_INFO, "IFNetMsgFactory Initial Done\r\n");

	/*IFString s = "asdasd";
	IFLOG(IFLL_INFO, "s=%s\r\n",s.c_str());*/

	registerDefaultObjects();
	IFLOG(IFLL_INFO, "registerDefaultObjects Done\r\n");

	registerDefaultAttribute();
	IFLOG(IFLL_INFO, "registerDefaultAttribute Done\r\n");

#ifdef IFPLATFORM_WINDOWS
	IFNew IFComPort();
#endif
//	IFNew IFRSA();

	return true;
}

bool IFCommonLibShutdown()
{
	if (IFObjectFactory::getSingletonPtr())
		delete IFObjectFactory::getSingletonPtr();
	

	if (IFAttributeMgr::getSingletonPtr())
		delete IFAttributeMgr::getSingletonPtr();

	if (IFSuperAttrMgr::getSingletonPtr())
		delete IFSuperAttrMgr::getSingletonPtr();

	if (IFFileSystem::getSingletonPtr())
		delete IFFileSystem::getSingletonPtr();

#ifdef IFPLATFORM_WINDOWS
	if (IFComPort::getSingletonPtr())
		delete IFComPort::getSingletonPtr();
#endif
	//if (IFRSA::getSingletonPtr())
	//	delete IFRSA::getSingletonPtr();

	//if (IFLogSystem::getSingletonPtr())
	//{
	//	delete IFLogSystem::getSingletonPtr();
	//}

	return true;

}

void registerDefaultObjects()
{
	IFLOG(IFLL_INFO, "registerDefaultObjects %s Done\r\n", IFAttributeSet::m_Type.GetTypeName());
	IFLOG(IFLL_INFO, "IFObjectFactory::getSingleton() = %p\r\n",  IFObjectFactory::getSingletonPtr());


	IFObjectFactory::getSingleton().registerObject(IFAttributeSet::m_Type.GetTypeName(), &IFAttributeSet::CreateObjStatic);

}

void registerDefaultAttribute()
{
	IF_REGISTER_ATTRIBTE(IFAttrINT			);
	IF_REGISTER_ATTRIBTE(IFAttrSTR			);
	IF_REGISTER_ATTRIBTE(IFAttrFLOAT			);
	IF_REGISTER_ATTRIBTE(IFAttrSTRFileName	);
	IF_REGISTER_ATTRIBTE(IFAttrLongStr	);
	IF_REGISTER_ATTRIBTE(IFAttrBOOL			);
	IF_REGISTER_ATTRIBTE(IFAttrCOLOR			);
	IF_REGISTER_ATTRIBTE(IFAttrENUM			);
	IF_REGISTER_ATTRIBTE(IFAttrENUMSTR		);
	IF_REGISTER_ATTRIBTE(IFAttrCombine		);
	IF_REGISTER_ATTRIBTE(IFAttrSubAttr		);
	IF_REGISTER_ATTRIBTE(IFAttrRECT			);
	IF_REGISTER_ATTRIBTE(IFAttrFixNumber	);

}