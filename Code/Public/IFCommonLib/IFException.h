/*
The MIT License (MIT)
Copyright © 2023 Huang Cong

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
#ifndef __IF_EXCEPTION_H__
#define __IF_EXCEPTION_H__
#include "IFString.h"

#ifdef IF_DONT_USE_CXX_EXCEPTION
#include <setjmp.h>
#endif

struct IFCOMMON_API IFExceptionInfo
{
	IFExceptionInfo();
	~IFExceptionInfo();
	IFExceptionInfo(int code, const IFString& err);

	int errorCode;		//==0 OK
	IFString errorString;

#ifdef IF_DONT_USE_CXX_EXCEPTION
	jmp_buf jumpBuff;
#endif
};

IFCOMMON_API void IFTryThrow(int err, const IFString& errS);

#ifdef IF_DONT_USE_CXX_EXCEPTION



#define IF_TRY_BEGIN {IFExceptionInfo exceptionInfo; if (setjmp(exceptionInfo.jumpBuff)==0){

#define IF_CATCH }else{


#define IF_THROW( err,  errS) IFTryThrow(err, errS)
		
#define IF_TRY_END } }
#else
#define IF_TRY_BEGIN try{

#define IF_CATCH }catch(IFExceptionInfo exceptionInfo){

#define IF_THROW( err,  errS) IFTryThrow(err,errS)

#define IF_TRY_END }
#endif
#endif //__IF_EXCEPTION_H__