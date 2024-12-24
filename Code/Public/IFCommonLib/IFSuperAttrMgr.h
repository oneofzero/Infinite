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
#include "IFCommonLib_API.h"
#include "ifsingleton.h"
#include "IFAttribute.h"
#include "IFRefObj.h"
#include "IFEventSlot.h"


class IFCOMMON_API IFSuperAttrInfo : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFSuperAttrInfo();

	IFAttributeList m_AttributeList;
	IFString m_sName;
	IFString m_ClassName;
	IFString m_sDir;


	IFString getPath();

	bool isLoaded()
	{
		return m_loaded;
	}

	bool load();

protected:
	~IFSuperAttrInfo();
	
	bool m_loaded;

	friend class IFSuperAttrMgr;

};

class IFAsyncResultBool;

class IFCOMMON_API IFSuperAttrMgr : public IFSingleton<IFSuperAttrMgr>, public IFObj
{
	IF_DECLARERTTI;
public:

	IFEventSlot<void(IFSuperAttrInfo* pInfo)> event_SuperAttrAdd;
	IFEventSlot<void(IFSuperAttrInfo* pInfo)> event_SuperAttrRemove;


	typedef IFHashMap<IFString, IFRefPtr<IFSuperAttrInfo> > SuperAttList;

	static const IFString BUILTIN_DIR;

public:
	IFSuperAttrMgr(void);
	~IFSuperAttrMgr(void);

	
	
	//bool loadSuperAttrsFromDir(const IFString& sDir); //sdir must is absolute
	bool loadSuperAttrsFromFileList(const IFString& sDirPath);
	bool loadSuperAttrsFromStream(const IFString& sDir, IFStream* pStream);

	IFRefPtr<IFAsyncResultBool> loadSuperAttrsFromFileListAsync(const IFString& sDirPath, bool loadAttrs);


	bool saveSuperAttrs(const IFString& sSuperAttrFileName );
	bool saveSuperAttr(const IFString& sSuperAttrName);
	bool saveSuperAttr(IFSuperAttrInfo* pInfo);
	bool saveSuperAttrs();

	IFSuperAttrInfo* getSuperAttr(const IFString& sName);

	void addSuperAttr(IFSuperAttrInfo* pInfo);
	void removeSuperAttr(const IFString& sName, bool needSave = true);
	void renameSuperAttr(const IFString& sOldName, const IFString& sNewName);

	SuperAttList::iterator getFirstSuperAttr();
	SuperAttList::iterator getEndSuperAttr();

	//IFString getAttFilePath(const IFString& sSuperAttrName);

	bool saveSuperAttriNameList(const IFString& sDir);

private:

	bool loadSuperAttr(IFSuperAttrInfo* info);

	SuperAttList m_SuperAttList;
	//IFHashSet<IFString> m_Loaded;
	IFString m_sCurSuperAttrFileName;
	//IFString m_sSuperAttrDir;

	friend class IFSuperAttrInfo;
};

