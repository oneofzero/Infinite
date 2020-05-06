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
//这里是放全局变量的
#include "stdafx.h"

#include "IFMemPool.h"
#include <vector>
#include "IFMap.h"
#include "IFHashMap.h"

#ifdef _MSC_VER
#pragma warning(disable: 4073)
#pragma init_seg(lib) 
#endif

IFCSLock g_AllocLock;

IFCSLock g_ptrlistlock;

IFCSLock g_WeakPtrListLock;

IFMap<void*,IFRBTree<void*> > g_WeakPtrList;

#if defined(_DEBUG) && (defined(IFPLATFORM_WINDOWS) || defined(IFPLATFORM_LINUX))
#define DUMP_REF_STACK
#endif

#if defined(DUMP_REF_STACK)
#include "IFStackDumper.h"
IFCSLock g_RefObjAllocStackInfoListLock;
IFHashMap<IFStackDumper, IFUI32> g_RefObjAllocStackInfoList(1024*64);
IFHashMap<IFRefObj*, IFStackDumper> g_RefObjStackInfoMap(1024*64);
#endif
//int  IFMemPollAllocID;
////= 0;
//
//#ifdef USEIFMEMPOOLALLOC
//
//#define memheadersize 8
//
//IFMemPool<char[4+memheadersize]	,IFSystemAllocSA<PoolData<char[4+memheadersize]>*	> >		g_ifPool4		(256*10); //4096*32
//IFMemPool<char[8+memheadersize]	,IFSystemAllocSA<PoolData<char[8+memheadersize]>*	> >		g_ifPool8		(256*10); //4096*32
//IFMemPool<char[16+memheadersize]	,IFSystemAllocSA<PoolData<char[16+memheadersize]>*	> >		g_ifPool16		(256*10); //4096*32
//IFMemPool<char[32+memheadersize]	,IFSystemAllocSA<PoolData<char[32+memheadersize]>*	> >		g_ifPool32		(128*10); //4096*32
//IFMemPool<char[64+memheadersize]	,IFSystemAllocSA<PoolData<char[64+memheadersize]>*	> >		g_ifPool64		(64*10)	; //2048*64
//IFMemPool<char[96+memheadersize]	,IFSystemAllocSA<PoolData<char[96+memheadersize]>*	> >		g_ifPool96		(64*10)	; //2048*64
//IFMemPool<char[128+memheadersize]	,IFSystemAllocSA<PoolData<char[128+memheadersize]>*	> >		g_ifPool128		(32*10)	;//1024*128;
//IFMemPool<char[256+memheadersize]	,IFSystemAllocSA<PoolData<char[256+memheadersize]>*	> >		g_ifPool256		(16*10)	;//256*1024;
//IFMemPool<char[384+memheadersize]	,IFSystemAllocSA<PoolData<char[384+memheadersize]>*	> >		g_ifPool384		(16*10)	;//256*1024;
//IFMemPool<char[512+memheadersize]	,IFSystemAllocSA<PoolData<char[512+memheadersize]>*	> >		g_ifPool512		(8*10)	;//512K
//IFMemPool<char[768+memheadersize]	,IFSystemAllocSA<PoolData<char[768+memheadersize]>*	> >		g_ifPool768		(8*10)	;//512K;
//IFMemPool<char[1024+memheadersize]	,IFSystemAllocSA<PoolData<char[1024+memheadersize]>* > > g_ifPool1024	(4*10)	;//256K;
//IFMemPool<char[1280+memheadersize]	,IFSystemAllocSA<PoolData<char[1280+memheadersize]>* > > g_ifPool1280	(4*10)	;//256K;
//IFMemPool<char[1536+memheadersize]	,IFSystemAllocSA<PoolData<char[1536+memheadersize]>* > > g_ifPool1536	(4*10)	;//256K;
//IFMemPool<char[2048+memheadersize]	,IFSystemAllocSA<PoolData<char[2048+memheadersize]>* > > g_ifPool2048	(2*10)	;//256K;
//#endif								