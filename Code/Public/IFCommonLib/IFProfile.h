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
#include "IFUtility.h"
#include "IFObj.h"
#include "IFMap.h"
#include "IFHashMap.h"
#include "IFCommonLib_API.h"

struct IFProfileName : public IFMemObj
{
	IFProfileName()
	{

	}
	IFProfileName(const char* name, int line)
		:_name(name), _line(line)
	{

	}
	const char* _name;
	int _line;

	inline bool operator < (const IFProfileName& o) const
	{
		if ( _name < o._name )
			return true;
		else if (_name == o._name)
			return _line < o._line;
		return false;
	}
	inline bool operator == (const IFProfileName& o) const
	{
		return _name == o._name && _line == o._line;
	}
};

inline IFUI32 IFHashFunc(const IFProfileName& o)
{
	return IFHashFunc(o._name) ^ o._line;
}

class IFProfileInfo;
typedef IFHashMap<IFProfileName,IFProfileInfo> ProfileInfoList;

class IFCOMMON_API IFProfileInfo : public IFMemObj
{
public:
	IFProfileInfo()
		:_useTime(0)
		,_callNum(0)
	{
	}

	IFUI64 _useTime;
	IFUI64 _callNum;

	ProfileInfoList _subInfos;

	void dump(const IFString& tb, IFString& s);
};

class  IFCOMMON_API IFProfileMgr : public IFSingleton<IFProfileMgr>
{
public:

	IFProfileMgr();
	~IFProfileMgr();

	void resetProfileInfo();

	void dumpProfileInfo(IFString& sinfo);

private:

	IFProfileInfo* getBackProfile();

	inline void setBackProfile(IFProfileInfo* pInfo)
	{
		ThreadProfileInfo* tpi = getCurrentThreadProfileInfo();

		tpi->_backProfile = pInfo;
	}

private:

	struct ThreadProfileInfo : public IFMemObj
	{
		ThreadProfileInfo()
			: _backProfile (NULL)
		{

		}
		IFProfileInfo* _backProfile;
		IFProfileInfo _profileRoot;
	};

	ThreadProfileInfo* getCurrentThreadProfileInfo();
	

	IFHashMap<IFUI32, ThreadProfileInfo*> _threadprofiles;
	//TODO 一般windows 线程id 不会大于9999吧
	//enum {MAX_THREAD_ID = 65536};

	//ThreadProfileInfo* _threadprofiles[MAX_THREAD_ID+1];

	friend class IFProfile;
};

class IFCOMMON_API IFProfile 
{
public:
	IFProfile(const char* sFunctionName, int nLine);
	IFProfile(const char* sName);
	~IFProfile();

private:
	IFUI64 _beginTime;
	IFProfileInfo* _pinfo;
	IFProfileInfo* _parentInfo;
};


//#define IF_ENABLE_PROFILE TRUE

#ifdef IF_ENABLE_PROFILE

//重置性能分析信息
#define IF_PROFILE_RESET()	if(IFProfileMgr::getSingletonPtr())IFProfileMgr::getSingleton().resetProfileInfo();

//在输出显示分析信息
#define IF_PROFILE_DUMP(s)		if(IFProfileMgr::getSingletonPtr())IFProfileMgr::getSingleton().dumpProfileInfo(s);

#define IF_PROFILE_POINT()	IFProfile __IFProfile(__FUNCTION__, __LINE__);
#define IF_PROFILE_POINT_BEGIN()	{IFProfile __IFProfile(__FUNCTION__, __LINE__);
#define IF_PROFILE_POINT_END()	}

#define IF_PROFILE_INIT() IFNew IFProfileMgr()

#else

#define IF_PROFILE_RESET()

#define IF_PROFILE_DUMP(s) 

#define IF_PROFILE_POINT()
#define IF_PROFILE_POINT_BEGIN()	
#define IF_PROFILE_POINT_END()
#define IF_PROFILE_INIT()
#endif
