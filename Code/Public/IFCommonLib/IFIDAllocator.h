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
#ifndef __IF_ID_ALLOCATOR_H__
#define __IF_ID_ALLOCATOR_H__
#include "IFCommonLib_API.h"
#include "IFTypes.h"
#include "IFObj.h"
#include "IFException.h"

class IFCOMMON_API IFIDAllocator : public IFRefObj
{
public:
	class IFIDAllocatorMaxIDException
	{

	};
protected:
	~IFIDAllocator(void);

public:
	IFIDAllocator(IFUI32 nMaxID = 64*1024);


	inline IFUI32 allocID()
	{
		if (m_FreeIDS.size())
		{
			IFUI32 nID = m_FreeIDS.back();
			m_FreeIDS.pop_back();
			return nID;
		}
		else
		{
			if (m_nCurMaxID > m_nMaxID)
			{
				IF_THROW(-1, "IFIDAllocatorMaxIDException");
				//throw IFIDAllocatorMaxIDException();
			}

			int nID = m_nCurMaxID;
			m_nCurMaxID ++;
			return nID;
		}
	}

	void freeID(int nID)
	{
		m_FreeIDS.push_back(nID);
	}

private:

	IFArray<IFUI32> m_FreeIDS;
	int m_nMaxID;
	int m_nCurMaxID;

};

#endif //__IF_ID_ALLOCATOR_H__
