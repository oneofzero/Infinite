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
#ifndef __IF_STACK_DUMPER_H__
#define __IF_STACK_DUMPER_H__
#include "IFCommonLib_API.h"
#include "IFString.h"

class IFCOMMON_API IFStackDumper
{
private:
	IFStackDumper(void);

public:
	IFStackDumper(const IFStackDumper& dmp);
	IFStackDumper(IFStackDumper&& dmp);

	~IFStackDumper(void);

	static IFStackDumper Dump( );
	
	IFString toString() const;

	bool operator == (const IFStackDumper& dmp) const
#ifdef IFPLATFORM_WEB
		;
#else
	{
		return m_nHash == dmp.m_nHash && m_Stack == dmp.m_Stack;
	}
#endif

	inline IFUI32 getHash() const
#ifdef IFPLATFORM_WEB
		;
#else
	{
		return m_nHash;
	}
#endif

private:

#ifdef IFPLATFORM_WEB
	IFString m_CallStackString;
#else
	IFArray<void*> m_Stack;
	int m_nHash;
#endif
};
inline IFUI32 IFHashFunc(const IFStackDumper& p)
{
	return p.getHash();
}
#endif //__IF_STACK_DUMPER_H__