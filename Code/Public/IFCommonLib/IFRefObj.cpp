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
#include "stdafx.h"
#include "IFRefObj.h"
#include "IFMap.h"
#include "IFList.h"
#include "IFHashMap.h"
#include "IFAtomicOperation.h"
#include "IFString.h"
#if defined(_DEBUG) && (defined(IFPLATFORM_WINDOWS) || defined(IFPLATFORM_LINUX))
#define DUMP_REF_STACK
#endif
IF_DEFINERTTI(IFRefObj, IFObj);

extern IFCSLock g_WeakPtrListLock;
extern IFMap<void*, IFSet<void*> > g_WeakPtrList;
#if defined(DUMP_REF_STACK)
#include "IFStackDumper.h"
#include "IFStream.h"
extern IFCSLock g_RefObjAllocStackInfoListLock;
extern IFHashMap<IFStackDumper, IFUI32> g_RefObjAllocStackInfoList;
extern IFHashMap<IFRefObj*, IFStackDumper> g_RefObjStackInfoMap;
#endif

static volatile bool stackdumping = false;

//#define NO_STACK_DUMP


IFRefObj::IFRefObj(bool bThreadSafe /*= false*/)
	:m_nRefCount(0)
	, m_bThreadSafe(bThreadSafe)
{
	//HoldPtr = this;


#ifndef NOREFOBJCOUNTER
	ATOMIC_INC_INT64(&ms_nInstanceCount);
#endif

#if defined(DUMP_REF_STACK)
	IFCSLockHelper lh(g_RefObjAllocStackInfoListLock);

	if (!stackdumping)
	{
		stackdumping = true;
		auto si = IFStackDumper::Dump();
		g_RefObjAllocStackInfoList[si] ++;
		g_RefObjStackInfoMap.insert(makeIFPair(this, si));
		stackdumping = false;
	}

#endif
}

IFRefObj::IFRefObj(const IFRefObj& o)
	:m_bThreadSafe(false)
{
	m_nRefCount = 0;
#ifndef NOREFOBJCOUNTER
	ATOMIC_INC_INT64(&ms_nInstanceCount);
#endif
#if defined(DUMP_REF_STACK)
	IFCSLockHelper lh(g_RefObjAllocStackInfoListLock);

	if (!stackdumping)
	{
		stackdumping = true;

		auto si = IFStackDumper::Dump();
		g_RefObjAllocStackInfoList[si] ++;
		g_RefObjStackInfoMap.insert(makeIFPair(this, si));
		stackdumping = false;
	}

#endif
}


IFRefObj::~IFRefObj()
{
#ifndef NOREFOBJCOUNTER
	ATOMIC_DEC_INT64(&ms_nInstanceCount);
#endif
#if defined(DUMP_REF_STACK)
	IFCSLockHelper lh(g_RefObjAllocStackInfoListLock);

	if (!stackdumping)
	{
		auto it = g_RefObjStackInfoMap.find(this);
		assert(it != g_RefObjStackInfoMap.end());
		g_RefObjAllocStackInfoList[it->second] --;
		g_RefObjStackInfoMap.erase(it);
	}
#endif
}

void IFRefObj::OutputAllocInfo(IFStream* pOutstream)
{
#if defined(DUMP_REF_STACK)
	IFCSLockHelper lh(g_RefObjAllocStackInfoListLock);
	IFString buf;
	stackdumping = true;

	for (auto& pr : g_RefObjAllocStackInfoList)
	{
		IFString s = pr.first.toString();
		s += "\r\n";
		s += buf.format("alloc num:%d\r\n", pr.second);
		pOutstream->write(s.c_str(), s.length());
	}
	stackdumping = false;
#endif
}

bool IFRefObj::GetAllocInfo(void* pOut)
{
#if defined(DUMP_REF_STACK)
	IFCSLockHelper lh(g_RefObjAllocStackInfoListLock);
	stackdumping = true;

	IFHashMap<IFStackDumper, IFUI32>* o = (IFHashMap<IFStackDumper, IFUI32>*)pOut;
	*o = g_RefObjAllocStackInfoList;
	stackdumping = false;

	return true;
#else

	return false;
#endif


}

void IFRefObj::release()
{
	if (g_WeakPtrList.size())
	{
		IFCSLockHelper lh(g_WeakPtrListLock);
		auto it = g_WeakPtrList.find(this);
		if(it!=g_WeakPtrList.end())
		{
			auto& ptrList = it->second;
			auto wkPtr = ptrList.begin();
			while(wkPtr!=ptrList.end())
			{
				void* pWKPtr = *wkPtr;
				*((IFUI32**)pWKPtr) = NULL;
				++wkPtr;
			}

			g_WeakPtrList.erase(it);

		}
	}


	delete this;

}

IFUI64 IFRefObj::ms_nInstanceCount = 0;
#ifdef IFREFOBJDEBUG
void IFRefObj::addRef(void* pHolder)
{
	//m_nOperateCount++;
	m_nRefCount++;
	RefPtrHolderList::iterator it = m_HolderList.find(pHolder);
	assert(it == m_HolderList.end());
	m_HolderList[pHolder] = new IFStackDumper(IFStackDumper::Dump());
}

void IFRefObj::decRef(void* pHolder)
{
	//m_nOperateCount++;
	m_nRefCount--;
	RefPtrHolderList::iterator it = m_HolderList.find(pHolder);
	assert(it != m_HolderList.end());
	delete ((IFStackDumper*)it->second);
	m_HolderList.erase(it);
	if (m_nRefCount == 0)
		release();
}

void IFRefObj::dumpRefInfo(IFString& s)
{
	for (auto& kv : m_HolderList)
	{
		s += IFString().format("%p:\r\n", kv.first);
		s += ((IFStackDumper*)kv.second)->toString();
		s += "\r\n";
	}
}
#endif
//IFRefObj::IFRefObj(bool bThreadSafe)
//
//
//IFRefObj::IFRefObj( const IFRefObj& o )
//
//IFRefObj::~IFRefObj()
//{
//
//}

