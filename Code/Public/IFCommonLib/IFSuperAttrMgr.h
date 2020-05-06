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
	IFStringW m_sName;
	IFString m_ClassName;

protected:
	~IFSuperAttrInfo();

};

class IFCOMMON_API IFSuperAttrMgr : public IFSingleton<IFSuperAttrMgr>, public IFObj
{
	IF_DECLARERTTI;
public:

	IFEventSlot<void(IFSuperAttrInfo* pInfo)> event_SuperAttrAdd;
	IFEventSlot<void(IFSuperAttrInfo* pInfo)> event_SuperAttrRemove;


	typedef IFMap<IFStringW, IFRefPtr<IFSuperAttrInfo> > SuperAttList;
public:
	IFSuperAttrMgr(void);
	~IFSuperAttrMgr(void);

	bool loadSuperAttrs(const IFStringW& sSuperAttrFileName );
	
	bool loadSuperAttrsFromDir(const IFStringW& sDir); //sdir must is absolute
	bool loadSuperAttrsFromFileList(const IFStringW& sDir);

	bool saveSuperAttrs(const IFStringW& sSuperAttrFileName );
	bool saveSuperAttr(const IFStringW& sSuperAttrName);
	bool saveSuperAttr(IFSuperAttrInfo* pInfo);
	bool saveSuperAttrs();

	IFSuperAttrInfo* getSuperAttr(const IFStringW& sName);

	void addSuperAttr(IFSuperAttrInfo* pInfo, bool bNeedSave = false);
	void removeSuperAttr(const IFStringW& sName, bool bNeedSave = false);
	void renameSuperAttr(const IFStringW& sOldName, const IFStringW& sNewName, bool bNeedSave = false);

	SuperAttList::iterator getFirstSuperAttr();
	SuperAttList::iterator getEndSuperAttr();

	IFStringW getAttFilePath(const IFStringW& sSuperAttrName);

	bool saveSuperAttriNameList();

private:

	SuperAttList m_SuperAttList;
	IFStringW m_sCurSuperAttrFileName;
	IFStringW m_sSuperAttrDir;
};

