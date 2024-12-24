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
#include "IFProfile.h"
#include "IFSystemAPI.h"
#include "IFLogSystem.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include "IFThread.h"

thread_local IFProfileMgr::ThreadProfileInfo* s_profileinfo = NULL;
IFProfileMgr::ThreadProfileInfo* IFProfileMgr::getCurrentThreadProfileInfo()
{
	if (!s_profileinfo)
	{

		s_profileinfo = new ThreadProfileInfo();
#ifndef IFTHREAD_NOT_ENABLE

		IFCSLockHelper lh(m_lock);
		_threadprofiles.insert( makeIFPair((IFUI32)IFThread::getCurrentThreadID(), s_profileinfo));
#endif
	}
	return s_profileinfo;
}



IF_DEFINESINGLETON(IFProfileMgr);

//IFProfileMgr g_ProfileMgr;
IFUI64 g_frequency;


IFProfileMgr::IFProfileMgr()
{
	//QueryPerformanceFrequency((LARGE_INTEGER*)&g_frequency);

	//memset(_threadprofiles,0, sizeof(_threadprofiles));
}


void IFProfileMgr::dumpProfileInfo(IFString& sinfo)
{
	
	IFCSLockHelper lh(m_lock);
	for (auto& info:_threadprofiles)
	{
		if (info.second)
		{
			if (info.second->_profileRoot._subInfos.size())
			{
				if (sinfo.size() == 0)
				{
					sinfo += "profile info :\r\n";
				}

				//char buf[32];
				//_snprintf_s(buf, _TRUNCATE,"Thread:%d\r\n", i);
				sinfo += IFString().format("Thread:%d\r\n", info.first);
				//OutputDebugStringA(buf);
				info.second->_profileRoot.dump("", sinfo);
			}
			
			//delete _threadprofiles[i];
		}
	}
    
}

void IFProfileMgr::resetProfileInfo()
{
	IFCSLockHelper lh(m_lock);
	for(auto& pr: _threadprofiles)
		pr.second->_profileRoot._subInfos.clear();
	//ThreadProfileInfo* tpi = getCurrentThreadProfileInfo();

	//tpi->_profileRoot._subInfos.clear();
}

IFProfileMgr::~IFProfileMgr()
{
	IFString info;
	dumpProfileInfo(info);

	for (auto& info : _threadprofiles)
	{
		if (info.second)
			delete info.second;
	}
	IFLOG(IFLL_WARNING,"%s", info.c_str());
}

IFProfileInfo* IFProfileMgr::getBackProfile()
{
	ThreadProfileInfo* tpi = getCurrentThreadProfileInfo();

	return tpi->_backProfile?tpi->_backProfile:&tpi->_profileRoot;
}



IFProfile::IFProfile( const char* sFunctionName, int nLine )
{
	if (!IFProfileMgr::getSingletonPtr())
		return;
	IFProfileName name(sFunctionName,nLine);
	_parentInfo = IFProfileMgr::getSingleton().getBackProfile(); 
	_pinfo = &_parentInfo->_subInfos[name];
	_pinfo->_callNum ++;

	_beginTime = IFNativeSystemAPI::getMicrosSec();
	//QueryPerformanceCounter((LARGE_INTEGER*)&_beginTime);

	IFProfileMgr::getSingleton().setBackProfile(_pinfo);
}

IFProfile::IFProfile( const char* sName )
{
	IFProfileName name(sName,0);
	if (!IFProfileMgr::getSingletonPtr())
		return;
	_pinfo = &IFProfileMgr::getSingleton().getBackProfile()->_subInfos[name];
	_pinfo->_callNum ++;
	//QueryPerformanceCounter((LARGE_INTEGER*)&_beginTime);
	_beginTime = IFNativeSystemAPI::getMicrosSec();

}

IFProfile::~IFProfile()
{
	if (!IFProfileMgr::getSingletonPtr())
		return;
	IFUI64 curtime;
	//QueryPerformanceCounter((LARGE_INTEGER*)&curtime);
	curtime = IFNativeSystemAPI::getMicrosSec();

	_pinfo->_useTime += curtime - _beginTime;
	IFProfileMgr::getSingleton().setBackProfile(_parentInfo);

}



void IFProfileInfo::dump(const IFString& tb,IFString& s)
{
	//char buf[512];
	for (ProfileInfoList::iterator it = _subInfos.begin();
		it != _subInfos.end(); ++it
		)
	{

		s += IFString().format( "%s %s(%d) time=%llu callnum=%d\r\n", &tb[0], it->first._name,it->first._line, it->second._useTime, it->second._callNum);
		//_snprintf_s(buf, _TRUNCATE, "%s %s(%d) time=%llu callnum=%d\r\n", &tb[0], it->first._name,it->first._line, it->second._useTime, it->second._callNum);
		//OutputDebugStringA(buf);
		it->second.dump(tb + "\t", s);
	}
     
}
