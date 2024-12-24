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
#ifndef __IF_ATOMIC_OPERATION_H__
#define __IF_ATOMIC_OPERATION_H__
#include "IFPlatformDefine.h"
#if defined(IFPLATFORM_WINDOWS) || defined(IFPLATFORM_WP) || defined(IFPLATFORM_WINDOWS_SHOP)
//#	ifdef _M_IX86
#if WINVER >= 0x0600 && !defined(_USING_V110_SDK71_)
#		define ATOMIC_CAS_PTR(a,c,s) (InterlockedCompareExchangePointer(( PVOID  volatile*)a, (PVOID )s, (PVOID )c) == c)
#		define ATOMIC_CAS_INT32(a,c,s) (InterlockedCompareExchange(( long  volatile*)a, (long )s, (long )c) == c)

#		define ATOMIC_INC_INT32(a) InterlockedIncrement((volatile long*)a)
#		define ATOMIC_INC_INT64(a) InterlockedIncrement64((volatile LONG64*)a)
#		define ATOMIC_DEC_INT32(a) InterlockedDecrement((volatile long*)a)
#		define ATOMIC_DEC_INT64(a) InterlockedDecrement64((volatile LONG64*)a)

#		define ATOMIC_ADD_INT32(a,b) InterlockedExchangeAdd((volatile long*)a, b)
#		define ATOMIC_ADD_INT64(a,b) InterlockedExchangeAdd64((volatile LONG64*)a, b)
#else
#		define ATOMIC_CAS_PTR(a,c,s) (InterlockedCompareExchangePointer(( PVOID  volatile*)a, (PVOID )s, (PVOID )c) == c)
#		define ATOMIC_CAS_INT32(a,c,s) (InterlockedCompareExchange(( long  volatile*)a, (long )s, (long )c) == c)

#		define ATOMIC_INC_INT32(a) InterlockedIncrement((volatile long*)a)
#		define ATOMIC_INC_INT64(a) ((*a)++)//InterlockedIncrement64((volatile LONG64*)a)
#		define ATOMIC_DEC_INT32(a) InterlockedDecrement((volatile long*)a)
#		define ATOMIC_DEC_INT64(a) ((*a)--)//InterlockedDecrement64((volatile LONG64*)a)

#		define ATOMIC_ADD_INT32(a,b) InterlockedExchangeAdd((volatile long*)a, b)
#		define ATOMIC_ADD_INT64(a,b) ((*a)+=b)// InterlockedExchangeAdd64((volatile LONG64*)a, b)

#endif
#elif defined(IFPLATFORM_ANDROID) || defined(IFPLATFORM_LINUX) || defined(IFPLATFORM_WEB)
#		define ATOMIC_CAS_PTR(a,c,s) (__sync_bool_compare_and_swap(( PVOID  volatile*)a, (PVOID )c, (PVOID )s))
#		define ATOMIC_CAS_INT32(a,c,s) (__sync_bool_compare_and_swap(( int  volatile*)a, (int )c, (int )s))
#		define ATOMIC_CAS_INT64(a,c,s) (__sync_bool_compare_and_swap(( long long  volatile*)a, (long long )c, (long long )s))

#		define ATOMIC_INC_INT32(a) __sync_add_and_fetch((volatile int*)a,1)
#		define ATOMIC_INC_INT64(a) __sync_add_and_fetch((volatile long long*)a,1)

#		define ATOMIC_DEC_INT32(a) __sync_sub_and_fetch((volatile int*)a,1)
#		define ATOMIC_DEC_INT64(a) __sync_sub_and_fetch((volatile long long*)a,1)

#		define ATOMIC_ADD_INT32(a,b) __sync_add_and_fetch((volatile int*)a, b)
#		define ATOMIC_ADD_INT64(a,b) __sync_add_and_fetch((volatile long long*)a, b)

#elif defined(IFPLATFORM_MAC) || defined(IFPLATFORM_IOS)
#       include <libkern/OSAtomic.h>
#		ifdef IFPLATFORM_IOS
#		define ATOMIC_CAS_PTR(a,c,s) (OSAtomicCompareAndSwapPtr((PVOID )c, (PVOID )s,( PVOID  volatile*)a))
#		define ATOMIC_CAS_INT32(a,c,s) (OSAtomicCompareAndSwapInt((int )c, (int )s,( int  volatile*)a))

#		define ATOMIC_INC_INT32(a) OSAtomicIncrement32((volatile int*)a)
#		define ATOMIC_DEC_INT32(a) OSAtomicDecrement32((volatile int*)a)
#		define ATOMIC_ADD_INT32(a,b) OSAtomicAdd32(b,(volatile int*)a)
#		define ATOMIC_INC_INT64(a) OSAtomicIncrement64((volatile long long*)a)
#		define ATOMIC_DEC_INT64(a) OSAtomicDecrement64((volatile long long*)a)
#		define ATOMIC_ADD_INT64(a,b) OSAtomicAdd64(b,(volatile long long*)a)
#		else

#		define ATOMIC_CAS_PTR(a,c,s) (OSAtomicCompareAndSwapPtr((PVOID )c, (PVOID )s,( PVOID  volatile*)a))
#		define ATOMIC_CAS_INT32(a,c,s) (OSAtomicCompareAndSwapInt((int )c, (int )s,( int  volatile*)a))

#		define ATOMIC_INC_INT32(a) OSAtomicIncrement32((volatile int*)a)
#		define ATOMIC_DEC_INT32(a) OSAtomicDecrement32((volatile int*)a)
#		define ATOMIC_ADD_INT32(a,b) OSAtomicAdd32(b,(volatile int*)a)
#		define ATOMIC_INC_INT64(a) OSAtomicIncrement64((volatile long long*)a)
#		define ATOMIC_DEC_INT64(a) OSAtomicDecrement64((volatile long long*)a)
#		define ATOMIC_ADD_INT64(a,b) OSAtomicAdd64(b,(volatile long long*)a)
#		endif
#elif defined(FREE_RTOS)
#		include "freertos/FreeRTOS.h"
#		include "freertos/atomic.h"
#		define ATOMIC_CAS_PTR(a,c,s) (Atomic_SwapPointers_p32(( PVOID  volatile*)a, (PVOID )c, (PVOID )s))
#		define ATOMIC_CAS_INT32(a,c,s) (Atomic_CompareAndSwap_u32 (( uint32_t   volatile*)a, (uint32_t )c, (uint32_t )s))
#		define ATOMIC_CAS_INT64  ATOMIC_CAS_INT32

#		define ATOMIC_INC_INT32(a) (Atomic_Increment_u32((volatile uint32_t *)a)+1)
#		define ATOMIC_INC_INT64 ATOMIC_INC_INT32

#		define ATOMIC_DEC_INT32(a) (Atomic_Decrement_u32((volatile uint32_t*)a)-1)
#		define ATOMIC_DEC_INT64 ATOMIC_DEC_INT32

#		define ATOMIC_ADD_INT32(a,b) Atomic_Add_u32((volatile uint32_t*)a, b)
#		define ATOMIC_ADD_INT64 ATOMIC_ADD_INT32
#else
inline bool ATOMIC_CAS_PTR(volatile void** a, void* c, void* s)
{
	if (*a == c)
	{
		*a = s;
		return true;
	}
	else
		return false;
}
inline bool ATOMIC_CAS_INT32(volatile int* a, int c, int s)
{
	if (*a == c)
	{
		*a = s;
		return true;
	}
	else
		return false;
}
#		define ATOMIC_CAS_INT64  ATOMIC_CAS_INT32

#		define ATOMIC_INC_INT32(a) (*(volatile int32_t*)a)++
#		define ATOMIC_INC_INT64 ATOMIC_INC_INT32

#		define ATOMIC_DEC_INT32(a) --(*(volatile int32_t*)a)
#		define ATOMIC_DEC_INT64 ATOMIC_DEC_INT32

#		define ATOMIC_ADD_INT32(a, b)* (volatile int32_t*)a = (*(volatile int32_t*)a) + b
#		define ATOMIC_ADD_INT64 ATOMIC_ADD_INT32
#endif
#endif //IFPH_IF_ATOMIC_OPERATION_H