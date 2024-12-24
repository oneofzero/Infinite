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
#ifndef __IF_OBJ_H__
#define __IF_OBJ_H__
//#include "IFObjectFactory.h"
#include "IFRTTI.h"
#include <new>
#include "stddef.h"

class IFStream;

class IFCOMMON_API IFMemObj
{
	//IF_DECLARERTTI;
public:
	inline IFMemObj(void)
	{

	}
	inline ~IFMemObj(void)
	{

	}


public:
	static void* operator new(size_t size);

	static void* operator new[](size_t size);

	static inline void* operator new(size_t size,void* pData)
	{
		return pData;
	}


	//static void* operator new(size_t size, const std::nothrow_t&) throw();
	//static void* operator new[](size_t size, const std::nothrow_t&) throw();


	static void* operator new(size_t size, const char* file, int line);
	static void* operator new[](size_t size, const char* file, int line);

	static void operator delete(void*);
	static void operator delete(void*,void*);

	static void operator delete(void* pointer, const char* file, int line);
	static void operator delete[](void* pointer, const char* file, int line);
	static void operator delete[](void*);	// MSVC 6 requires this declaration

	static void dumpMemoryleak(IFStream* pStream);

	//static void operator delete(void* pointer, const std::nothrow_t&);
	//static void operator delete[](void* pointer, const std::nothrow_t&);


};

class IFCOMMON_API IFObj : public IFMemObj
{
	IF_DECLARERTTI;
public:
	virtual ~IFObj();
};

#if defined(_DEBUG) || defined(IF_DEBUG_NEW)
#define IFNew new(__FILE__,__LINE__)
#else
#define IFNew new
#endif



//template<class T>
//class IFSTLAllocator
//{
//public:
//
//	typedef   T   value_type;   
//	typedef   size_t   size_type;   
//	typedef   ptrdiff_t   difference_type;   
//	typedef   T*   pointer;   
//	typedef   const   T*   const_pointer;   
//	typedef   T&   reference;         
//	typedef   const   T&   const_reference;     
//
//	pointer   address(reference   r)   const   {   return   &r;   }   
//	const_pointer   address(const_reference   r)   const   {   return   &r;   }   
//
//	IFSTLAllocator()   throw()   ;   
//	template   <class   U>   IFSTLAllocator(const   IFSTLAllocator<U>&)   throw()   ;   
//	~IFSTLAllocator()   throw()   ;   
//
//	pointer   allocate(size_type   n,   IFSTLAllocator<void>::const_pointer   hint   =   0)
//	{
//		return IFObj::new( n * sizeof(T) );
//	}
//	void   deallocate(pointer   p,   size_type   n) 
//	{
//		IFObj::delete(p);
//	}
//	void   construct(pointer   p,   const   T&   val)   {   new(p)   T(val)   ;   }     
//	void   destroy(pointer   p){   p->~T()   ;   }   
//	size_type   max_size()   const   throw()   ;   
//	template   <class   U>   
//	struct   rebind   {   typedef   IFSTLAllocator<U>   other;   };    
//	
//};

#endif //__IF_OBJ_H__