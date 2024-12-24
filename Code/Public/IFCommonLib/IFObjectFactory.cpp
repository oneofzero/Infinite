﻿/*
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
#include "IFObjectFactory.h"
IF_DEFINESINGLETON(IFObjectFactory);
//static IFObjectFactory objectfactory;
IFObjectFactory::IFObjectFactory(void)
{
}

IFObjectFactory::~IFObjectFactory(void)
{
}

bool IFObjectFactory::registerObject(const IFRTTI* pRTTI, CREATEOBJECTFUN* pCreateFunction )
{
	//m_FunList.insert(IFPair<IFString,CREATEOBJECTFUN*>(sObjectName));
	m_FunList[pRTTI] = pCreateFunction;
	m_STRFunList[pRTTI->GetTypeName()] = pCreateFunction;
	return true;
}

IFObj* IFObjectFactory::createObject(const IFString& sObjectName)
{
	auto it = m_STRFunList.find( sObjectName );
	if( it != m_STRFunList.end() )
	{
		return (*(it->second))();
	}
	return NULL;
}

IFObj* IFObjectFactory::createObject(const IFRTTI* pRTTI)
{
	auto it = m_FunList.find(pRTTI);
	if (it != m_FunList.end())
	{
		return (*(it->second))();
	}
	return NULL;
}
