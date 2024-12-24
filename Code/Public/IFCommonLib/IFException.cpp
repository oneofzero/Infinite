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
#include "stdafx.h"
#include "IFException.h"
#include "IFLogSystem.h"


#ifdef IF_DONT_USE_CXX_EXCEPTION
thread_local static IFArray<IFExceptionInfo*>* s_exception_info = NULL;
#endif
//thread_local static jmp_buf s_jump_buf = {0};
IFExceptionInfo::IFExceptionInfo()
	:errorCode(0)
{
#ifdef IF_DONT_USE_CXX_EXCEPTION
	if (s_exception_info == NULL)
		s_exception_info = IFNew IFArray<IFExceptionInfo*>();
	//IFLOG(IFLL_FATAL, "EXCEPTION CATCH BEGIN!%d\r\n", s_exception_info.size());
	s_exception_info->push_back(this);
#endif
}

IFExceptionInfo::~IFExceptionInfo()
{
#ifdef IF_DONT_USE_CXX_EXCEPTION
	if (s_exception_info->back() != this)
	{
		IFLOG(IFLL_FATAL, "Exception FATAL ERROR!\r\n");
		assert(0);
	}
	s_exception_info->pop_back();
#endif
	//IFLOG(IFLL_FATAL, "EXCEPTION CATCH END!%d\r\n", s_exception_info.size());
}
IFExceptionInfo::IFExceptionInfo(int code, const IFString& err)
	:errorCode(code), errorString(err)
{
}
#ifdef IF_DONT_USE_CXX_EXCEPTION
void IFTryThrow(int err, const IFString& errS)
{

	if (!s_exception_info || s_exception_info->size() == 0)
	{
		IFLOG(IFLL_FATAL, "EXCEPTION NOT CATCHED!%s\r\n", errS.c_str());
		assert(!"EXCEPTION NOT CATCHED!");
		return;
	}
	IFLOG(IFLL_INFO, "THROW EXCEPTION:%s\r\n", errS.c_str());

	s_exception_info->back()->errorCode = err;
	s_exception_info->back()->errorString = errS;
	longjmp(s_exception_info->back()->jumpBuff, 1);



}
#else
void IFTryThrow(int err, const IFString& errS) 
{
	throw IFExceptionInfo(err, errS);
}
#endif