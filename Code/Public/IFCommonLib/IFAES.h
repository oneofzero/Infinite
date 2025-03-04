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
#pragma once
#ifndef __IF_AES_H__
#define __IF_AES_H__
#include "IFRefPtr.h"
#include "ifsingleton.h"
#include "IFString.h"
#include "IFCommonLib_API.h"

class IFStream;
class IFMemStream;


class IFCOMMON_API IFAES : public IFRefObj
{
	IF_DECLARERTTI;
public:
	
	IFAES(const IFString& sUserKey);


	void encrypt(IFStream* pInStream, IFStream* pOutStream) const;
	void decrypt(IFStream* pInStream, IFStream* pOutStream) const;
protected:
	~IFAES();
private:
	IFAES (const IFAES& o){};
	IFAES& operator=(const IFAES& o){return *this;};
	void* m_pEnKey;
	void* m_pDeKey;

};

class IFCOMMON_API IFDES : public IFMemObj
{
	IF_DECLARERTTI_STATIC;
public:
	IFDES(const IFString& sUserKey);
	~IFDES();
	void encrypt(IFStream* pInStream, IFStream* pOutStream) const;
	void decrypt(IFStream* pInStream, IFStream* pOutStream) const;

	void encrypt(char* in_block, char* out_block) const; //block size = 8
	void decrypt(char* in_block, char* out_block) const; //block size = 8

protected:

	IFDES(const IFDES& o) {};
	IFDES& operator=(const IFDES& o) { return *this; };

	IFSimpleArray<char> m_des;
};

/*
class IFAES :  public IFSingleton<IFAES>, public IFMemObj
{
public:
	struct Key
	{
		char enckey[64];
		char deckey[64];
	};

	Key generateKey(const IFString& sUserKey);
};
*/

#endif //__IF_AES_H__