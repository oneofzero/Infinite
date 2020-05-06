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
#undef IF_FUNCTION_PARAM_NUM
#define IF_FUNCTION_PARAM_NUM IF_REPEAT_INCLUDE_INDEX
#include "IFParamsMacro.h"

template<IF_TEMPALTE_PARAM_DEFINE(P)>
class IFEventSlot<void(IF_FUNCTION_PARAM_CALL(P))> : public IFEventSlotBase<void(IF_FUNCTION_PARAM_CALL(P))>
{
public:
	using IFEventSlotBase<void(IF_FUNCTION_PARAM_CALL(P))>::m_FunctionList;

	IFEventSlot(bool bThreadSafe = false)
		:IFEventSlotBase<void(IF_FUNCTION_PARAM_CALL(P))>(bThreadSafe)
	{
	}

	void operator()(IF_FUNCTION_PARAM_DEFINE(P,p))
	{
		IFCSLockHelper locker(IFEventSlotBase<void(IF_FUNCTION_PARAM_CALL(P))>::m_FunctionListLock);

		if(m_FunctionList)
		{
#ifndef WIN32
			typename IFEventSlotBase<void(IF_FUNCTION_PARAM_CALL(P))>::EventHandleListHolder holder(m_FunctionList);
#else
			EventHandleListHolder holder(m_FunctionList);

#endif
			holder.m_pList->m_CurIter = m_FunctionList->begin();
			auto lastit = m_FunctionList->end();
			--lastit;
			if (m_FunctionList->size())
			{
				do
				{
					IFEventHandle<typename IFEventSlotBase<void(IF_FUNCTION_PARAM_CALL(P))>::functionType>* pFun = (*holder.m_pList->m_CurIter);

					if (holder.m_pList->m_CurIter==lastit)
					{
						(*pFun->m_pFunctor)(IF_FUNCTION_PARAM_CALL(p));
						return;
					}
					else
					{
						++holder.m_pList->m_CurIter;
						(*pFun->m_pFunctor)(IF_FUNCTION_PARAM_CALL(p));
					}
		
				}
				while (holder.m_pList->m_CurIter!=holder.m_pList->end());
				
			}
			
		}	
	}

};
