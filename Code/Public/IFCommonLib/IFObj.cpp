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
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include "IFObj.h"
#include "IFAlloc.h"
#include "IFCSLockHelper.h"
#include "IFStream.h"
#include <wchar.h>
#ifdef WIN32
#include <tchar.h>
#endif
/*
* debug_new.cpp  1.11 2003/07/03
*
* Implementation of debug versions of new and delete to check leakage
*
* By Wu Yongwei
*
*/
IF_DEFINERTTIROOT(IFObj);


#ifdef _MSC_VER
#pragma warning(disable: 4073)
#pragma init_seg(lib) 
#endif

#ifndef DEBUG_NEW_HASHTABLESIZE
#define DEBUG_NEW_HASHTABLESIZE 16384
#endif

#ifndef DEBUG_NEW_HASH
#define DEBUG_NEW_HASH(p) (((unsigned int)((size_t)(p)) >> 8) % DEBUG_NEW_HASHTABLESIZE)
#endif

// The default behaviour now is to copy the file name, because we found
// that the exit leakage check cannot access the address of the file
// name sometimes (in our case, a core dump will occur when trying to
// access the file name in a shared library after a SIGINT).
#ifndef DEBUG_NEW_FILENAME_LEN
#define DEBUG_NEW_FILENAME_LEN	0
#endif
#if DEBUG_NEW_FILENAME_LEN == 0 && !defined(DEBUG_NEW_NO_FILENAME_COPY)
#define DEBUG_NEW_NO_FILENAME_COPY
#endif
#ifndef DEBUG_NEW_NO_FILENAME_COPY
#include <string.h>
#endif

struct new_ptr_list_t
{
	new_ptr_list_t*		next;
#ifdef DEBUG_NEW_NO_FILENAME_COPY
	const char*			file;
#else
	char				file[DEBUG_NEW_FILENAME_LEN];
#endif
	int					line;
	unsigned long		size;
	unsigned int		index;
};

extern IFCSLock g_ptrlistlock;

static new_ptr_list_t* new_ptr_list[DEBUG_NEW_HASHTABLESIZE];

bool if_new_verbose_flag = false;
bool if_new_autocheck_flag = true;

static unsigned int sg_NewIndex = 0;

bool if_check_leaks()
{
	bool fLeaked = false;
	for (int i = 0; i < DEBUG_NEW_HASHTABLESIZE; ++i)
	{
		new_ptr_list_t* ptr = new_ptr_list[i];
		if (ptr == NULL)
			continue;
		fLeaked = true;
#ifdef WIN32
		WCHAR buf[256];
#else
		char buf[256];
#endif
		while (ptr)
		{
#ifdef WIN32
			_sntprintf_s(buf,_TRUNCATE,_T("%S(%d) : Leaked object at %p size %u, %d \n"),

				ptr->file,
				ptr->line,
				(char*)ptr + sizeof(new_ptr_list_t),
				ptr->size,
				ptr->index)
				;
			ptr = ptr->next;
#ifdef _DEBUG
			OutputDebugString( buf );
#endif
#else
//#	ifdef ANDROID
			sprintf(buf,"%s(%d) : Leaked object at %p size %u, %d \n",

				ptr->file,
				ptr->line,
				(char*)ptr + sizeof(new_ptr_list_t),
				ptr->size,
				ptr->index)
				;
			ptr = ptr->next;

			//OutputDebugString( buf );
//#	endif

#endif

		}
	}
	if (fLeaked)
		return true;
	else
		return false;
}

void* IFMemObj::operator new(size_t size, const char* file, int line)
{
#ifdef _DEBUG
	size_t s = size + sizeof(new_ptr_list_t);
	//new_ptr_list_t* ptr = (new_ptr_list_t*)malloc(s);
	new_ptr_list_t* ptr = (new_ptr_list_t*)IFAlloc::Alloc((int)s);
	if (ptr == NULL)
	{
		fprintf(stderr, "new:  out of memory when allocating %u bytes\n",
			size);
		abort();
	}
	IFCSLockHelper lh(g_ptrlistlock);

	void* pointer = (char*)ptr + sizeof(new_ptr_list_t);
	size_t hash_index = DEBUG_NEW_HASH(pointer);
	ptr->next = new_ptr_list[hash_index];
#ifdef DEBUG_NEW_NO_FILENAME_COPY
	ptr->file = file;
#else
	strncpy_s(ptr->file, DEBUG_NEW_FILENAME_LEN, file, DEBUG_NEW_FILENAME_LEN - 1);
	ptr->file[DEBUG_NEW_FILENAME_LEN - 1] = '\0';
#endif
	ptr->line = line;
	ptr->size = size;
	ptr->index = sg_NewIndex;
	sg_NewIndex ++;
	new_ptr_list[hash_index] = ptr;
	if (if_new_verbose_flag)
		printf("new:  allocated  %p (size %u, %s:%d)\n",
		pointer, size, file, line);
	return pointer;
#else
	return IFAlloc::Alloc((int)size);
#endif
}

void* IFMemObj::operator new[](size_t size, const char* file, int line)
{
	return operator new(size, file, line);
}

void* IFMemObj::operator new(size_t size)
{
	return operator new(size, "<Unknown>", 0);
}

//void* IFMemObj::operator new(size_t size, void* pMemory)
//{
//	return pMemory;
//}

void* IFMemObj::operator new[](size_t size)
{
	return operator new(size);
}
//
//void* IFObj::operator new(size_t size, const std::nothrow_t&) throw()
//{
//	return operator new(size);
//}

//void* IFObj::operator new[](size_t size, const std::nothrow_t&) throw()
//{
//	return operator new[](size);
//}

void IFMemObj::operator delete(void* pointer)
{

	if (pointer == NULL)
		return;
#ifdef _DEBUG

	//IFCSLockHelper lh(g_ptrlistlock);
	g_ptrlistlock.lock();
	size_t hash_index = DEBUG_NEW_HASH(pointer);
	new_ptr_list_t* ptr = new_ptr_list[hash_index];
	new_ptr_list_t* ptr_last = NULL;
	while (ptr)
	{
		if ((char*)ptr + sizeof(new_ptr_list_t) == pointer)
		{
			if (if_new_verbose_flag)
				printf("delete: freeing  %p (size %u)\n", pointer, ptr->size);
			if (ptr_last == NULL)
				new_ptr_list[hash_index] = ptr->next;
			else
				ptr_last->next = ptr->next;
			//free(ptr);
			g_ptrlistlock.unlock();
			IFAlloc::Dealloc(ptr);
			return;
		}
		ptr_last = ptr;
		ptr = ptr->next;
	}
	g_ptrlistlock.unlock();
	fprintf(stderr, "delete: invalid pointer %p\n", pointer);
	abort();
#else
	IFAlloc::Dealloc(pointer);
#endif
}

void IFMemObj::operator delete[](void* pointer)
{
	operator delete(pointer);
}

// Some older compilers like Borland C++ Compiler 5.5.1 and Digital Mars
// Compiler 8.29 do not support placement delete operators.
// NO_PLACEMENT_DELETE needs to be defined when using such compilers.
// Also note that in that case memory leakage will occur if an exception
// is thrown in the initialization (constructor) of a dynamically
// created object.
#ifndef NO_PLACEMENT_DELETE
void IFMemObj::operator delete(void* pointer, const char* file, int line)
{
	if (if_new_verbose_flag)
		printf("info: exception thrown on initializing object at %p (%s:%d)\n",
		pointer, file, line);
	operator delete(pointer);
}

void IFMemObj::operator delete(void* pointer, void* p2)
{
	//operator delete(pointer, file, line);
}

void IFMemObj::operator delete[](void* pointer, const char* file, int line)
{
	operator delete(pointer, file, line);
}

static const char* getfilename(const char* spath)
{
	int nIdx = strlen(spath) - 1;
	while (nIdx>=0)
	{
		if (spath[nIdx]=='\\'||spath[nIdx]=='/')
		{
			break;
		}

		nIdx--;
	}

	return spath + nIdx+1;
}

void IFMemObj::dumpMemoryleak( IFStream* pStream )
{
	bool fLeaked = false;
	char buf[512]={0};
	IFUI64 nSize = 0;

	struct FLInfo
	{
		const char* file;
		IFUI32 nline;
		inline bool operator <(const FLInfo& o) const
		{
			int r = strcmp(file,o.file);
			if (r < 0)
				return true;
			else if (r>0)
				return false;
			else
				return nline < o.nline;
		}
	};

	struct DetInfo
	{
		DetInfo()
			:nSize(0),nCount(0)
		{

		}
		IFUI32 nSize;
		IFUI32 nCount;
	};
	typedef 
	IFMap<FLInfo,DetInfo> AllocInfoMap;
	AllocInfoMap allocinfo;
	for (int i = 0; i < DEBUG_NEW_HASHTABLESIZE; ++i)
	{
		new_ptr_list_t* ptr = new_ptr_list[i];
		if (ptr == NULL)
			continue;
		fLeaked = true;
		while (ptr)
		{
			FLInfo fl;
			fl.file = getfilename(ptr->file);
			fl.nline = ptr->line;
			DetInfo& info = allocinfo[fl];//.first += ptr->size;
			info.nSize += ptr->size;
			info.nCount ++;

			//int l = sprintf(buf,"%s(%d) : Leaked object at %p size %u, %d \r\n",
			//	ptr->file,
			//	ptr->line,
			//	(char*)ptr + sizeof(new_ptr_list_t),
			//	ptr->size,
			//	ptr->index)
			//	;
			nSize += ptr->size;
			ptr = ptr->next;
			//pStream->write(buf, l);

			//OutputDebugString( buf );

		}
	}
	int l =sprintf(buf,"total memory size:%llu bytes\r\n", nSize);
	pStream->write(buf,l);

	for (auto& pr:allocinfo)
	{
		int l = sprintf(buf, "%s(%d) size:%d count:%d \r\n", pr.first.file, pr.first.nline, pr.second.nSize, pr.second.nCount);
		pStream->write(buf, l);
	}

}

//void IFObj::operator delete(void* pointer, const std::nothrow_t&)
//{
//	operator delete(pointer, "<Unknown>", 0);
//}
//
//void IFObj::operator delete[](void* pointer, const std::nothrow_t&)
//{
//	operator delete(pointer, std::nothrow);
//}
#endif // NO_PLACEMENT_DELETE

// Proxy class to automatically call check_leaks if new_autocheck_flag is set

#if defined(_DEBUG)&& defined(IFPLATFORM_WINDOWS)
#include "IFStackDumper.h"
#include "IFHashMap.h"
extern IFHashMap<IFStackDumper, IFUI32> g_RefObjAllocStackInfoList;
extern IFHashMap<IFRefObj*, IFStackDumper> g_RefObjStackInfoMap;
#endif
class ifobj_check_t
{
public:
	ifobj_check_t() 
	{
		//InitializeCriticalSection(&g_ptrlistlock);

	}
	~ifobj_check_t()
	{
		//DeleteCriticalSection(&g_ptrlistlock);
		if (if_new_autocheck_flag)
		{
#if defined(_DEBUG)&& defined(IFPLATFORM_WINDOWS)
			g_RefObjAllocStackInfoList.clear();
			g_RefObjStackInfoMap.clear();
#endif
			// Check for leakage.
			// If any leaks are found, set new_verbose_flag so that any
			// delete operations in the destruction of global/static
			// objects will display information to compensate for
			// possible false leakage reports.
			if (if_check_leaks())
				if_new_verbose_flag = true;
		}
	}
};
static ifobj_check_t ifobj_check_object;

IFObj::~IFObj()
{

}
