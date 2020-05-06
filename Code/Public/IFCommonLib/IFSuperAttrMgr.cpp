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
#include "IFSuperAttrMgr.h"
#include "IFFileSystem.h"
#include "IFAttributeMgr.h"
#include "IFUtility.h"
#include "IFLogSystem.h"
#include "IFJSON.h"
#include "IFAttributeMgr.h"

IF_DEFINESINGLETON(IFSuperAttrMgr);
IF_DEFINERTTI(IFSuperAttrMgr, IFObj);
IFSuperAttrMgr::IFSuperAttrMgr(void)
{
}


IFSuperAttrMgr::~IFSuperAttrMgr(void)
{
}

bool IFSuperAttrMgr::loadSuperAttrs( const IFStringW& sSuperAttrFileName )
{
	m_sCurSuperAttrFileName = sSuperAttrFileName;

	IFRefPtr<IFStream> pStream = IFFileSystem::getSingletonPtr()->openStream(sSuperAttrFileName,"rb");
	if(!pStream)
		return false;

	{
		IFRefPtr<IFJSONNode> spRoot = IFJSONParser::parse(pStream);
		if (spRoot)
		{
			IFRefPtr<IFAttributeRemap> spRemap = IFNew IFAttributeRemap;
			if(spRemap->loadFromJSON(spRoot))
			{
				IFAttributeMgr::getSingleton().setIndexRemap(spRemap);
			}
			IFRefPtr<IFSuperAttrInfo> pInfo = IFNew IFSuperAttrInfo;
			pInfo->m_ClassName = spRoot->getSubValue("attclassname").getString();
			pInfo->m_sName = spRoot->getSubValue("attname").getString();
			if (IFJSONNode* pSuper = spRoot->getSubNode("super"))
			{
				pInfo->m_AttributeList.setSuperAttrList(pSuper->getValue().getString());

			}

			if (IFJSONNode* pAtts = spRoot->getSubNode("atts"))
			{
				pInfo->m_AttributeList.loadFromJSON(pAtts);
			}

			addSuperAttr(pInfo);
			return true;
		}
		else
		{
			pStream->seek(0, IFStream::ISSF_BEGIN);
		}
	}




	return false;
}



bool IFSuperAttrMgr::saveSuperAttrs( const IFStringW& sSuperAttrFileName )
{	
	
	return true;
}

bool IFSuperAttrMgr::saveSuperAttrs()
{

	for (SuperAttList::iterator it = m_SuperAttList.begin(); it != m_SuperAttList.end(); ++it)
	{
		saveSuperAttr(it->second);
	}
	return true;
}


bool IFSuperAttrMgr::saveSuperAttr( const IFStringW& sSuperAttrName )
{

	SuperAttList::iterator it = m_SuperAttList.find(sSuperAttrName);
	if (it != m_SuperAttList.end())
		return saveSuperAttr(it->second);
	return false;
}

bool IFSuperAttrMgr::saveSuperAttr( IFSuperAttrInfo* pInfo )
{
	if (0)
	{

	
	}
	else
	{
		IFRefPtr<IFJSONNode> spRoot = IFNew IFJSONNode;
		IFRefPtr<IFAttributeRemap> spRemap = IFNew IFAttributeRemap;
		spRemap->saveTOJSON(spRoot);

		spRoot->setSubValue("attclassname", pInfo->m_ClassName);
		spRoot->setSubValue("attname", pInfo->m_sName.toUTF8String());

		if (pInfo->m_AttributeList.getSuperAttrListName() == pInfo->m_sName)
		{
			pInfo->m_AttributeList.setSuperAttrList(IFStringW::Empty);
		}

		if(pInfo->m_AttributeList.getSuperAttrListName().size())
			spRoot->setSubValue("super", pInfo->m_AttributeList.getSuperAttrListName().toUTF8String());


		IFRefPtr<IFJSONNode> spAtts = IFNew IFJSONNode;
		IFAttributeList tempatt = pInfo->m_AttributeList;
		if (tempatt.getSuperAttrList())
		{
			auto al = IFAttributeMgr::getSingleton().getDefaultAttribute(pInfo->m_ClassName);

			al.merge(*tempatt.getSuperAttrList());

			tempatt.removeSameAttr(al);

		}
		else
		{
			const auto& da = IFAttributeMgr::getSingleton().getDefaultAttribute(pInfo->m_ClassName);

			tempatt.removeSameAttr(da);
		}

		//tempatt.simplify();
		//tempatt.removeSameAttr(IFAttributeMgr::getSingleton().getDefaultAttribute(pInfo->m_ClassName));
		tempatt.saveToJSON(spAtts);
		spRoot->insert("atts", spAtts);
		IFString sUTF8 = spRoot->toString(true,false);
		IFStringW sFullName = getAttFilePath(pInfo->m_sName);

		IFRefPtr<IFStream> spStream = IFFileSystem::getSingleton().openStream(sFullName,"wb");
		if (!spStream)
			return false;
		spStream->write(IFString::UTF8Flag, IFArraySize(IFString::UTF8Flag));
		spStream->write(sUTF8.c_str(), sUTF8.size());

		saveSuperAttriNameList();
	}

	return true;

}
IFStringW IFSuperAttrMgr::getAttFilePath( const IFStringW& sSuperAttrName )
{
	IFStringW sFullName = UCombinePathW(m_sSuperAttrDir,  sSuperAttrName);
	sFullName += L".attrxml";
	return sFullName;
}

bool IFSuperAttrMgr::loadSuperAttrsFromDir( const IFStringW& sDir )
{
	m_sSuperAttrDir = sDir;
	//IFStringW sCurDir = IFFileSystem::getSingleton().getCurrentDirectory();
	//if (IFFileSystem::getSingleton().setCurrentDirectory(m_sSuperAttrDir))
	{
		IFFileInfoList filelist = IFFileSystem::getSingleton().listDirectory(m_sSuperAttrDir, L"*.attrxml");
		if (filelist.size() == 0)
			return false;
		//IFFileSystem::getSingleton().setCurrentDirectory(sCurDir);
		for (int i = 0; i < filelist.size(); i ++ )
		{
			loadSuperAttrs(filelist[i]->getPath());
		}
	}
	return true;
}


IFSuperAttrInfo* IFSuperAttrMgr::getSuperAttr( const IFStringW& sName )
{
	SuperAttList::iterator it = m_SuperAttList.find(sName);
	if(it!=m_SuperAttList.end())
	{
		return it->second;
	}
	return NULL;
}

void IFSuperAttrMgr::addSuperAttr( IFSuperAttrInfo* pInfo,bool bNeedSave )
{
	m_SuperAttList[pInfo->m_sName] = pInfo;
	if(bNeedSave)
		saveSuperAttr(pInfo);

	event_SuperAttrAdd(pInfo);
}

void IFSuperAttrMgr::removeSuperAttr( const IFStringW& sName,bool bNeedSave )
{
	SuperAttList::iterator it = m_SuperAttList.find(sName);
	if(it!=m_SuperAttList.end())
	{
		if (bNeedSave)
		{
			IFFileSystem::getSingleton().removeFile(getAttFilePath(sName));
		}

		event_SuperAttrRemove(it->second);
		m_SuperAttList.erase(it);
	}
	saveSuperAttriNameList();

	//if(bNeedSave)
	//	saveSuperAttrs( );
}

void IFSuperAttrMgr::renameSuperAttr( const IFStringW& sOldName, const IFStringW& sNewName, bool bNeedSave /*= false*/ )
{
	if(sOldName == sNewName )
		return;
	SuperAttList::iterator it = m_SuperAttList.find(sOldName);
	if(it!=m_SuperAttList.end())
	{
		if(bNeedSave)
		{
			IFFileSystem::getSingleton().rename(getAttFilePath(sOldName), getAttFilePath(sNewName));
			//saveSuperAttrs( );
		}
		IFRefPtr<IFSuperAttrInfo> spSA = it->second;
		m_SuperAttList.erase(it);
		spSA->m_sName = sNewName;
		m_SuperAttList[sNewName] = spSA;

	}

}

IFSuperAttrMgr::SuperAttList::iterator IFSuperAttrMgr::getFirstSuperAttr()
{
	return m_SuperAttList.begin();
}

IFSuperAttrMgr::SuperAttList::iterator IFSuperAttrMgr::getEndSuperAttr()
{
	return m_SuperAttList.end();
}

bool IFSuperAttrMgr::saveSuperAttriNameList()
{
	IFRefPtr<IFJSONNode> spRoot = IFNew IFJSONNode;
	for (auto& pr:m_SuperAttList)
	{
		IFRefPtr<IFJSONNode> spValue = IFNew IFJSONNode;
		IFStringW sFullName = UGetRelativePathW(getAttFilePath(pr.first), m_sSuperAttrDir);
		spValue->setValue(sFullName.toUTF8String());
		spRoot->push_back(spValue);
	}

	IFString s = spRoot->toString();

	IFRefPtr<IFStream> spStream = IFFileSystem::getSingleton().openStream(UCombinePathW(m_sSuperAttrDir,L"attlist.json"), "wb");
	if (!spStream)
		return false;
	spStream->write(IFString::UTF8Flag, IFArraySize(IFString::UTF8Flag));
	spStream->write(s.c_str(), s.size());
	return true;

}

bool IFSuperAttrMgr::loadSuperAttrsFromFileList(const IFStringW& sDir)
{
	m_sSuperAttrDir = sDir;
	IFRefPtr<IFStream> spStream = IFFileSystem::getSingleton().openStream(UCombinePathW(m_sSuperAttrDir, L"attlist.json"), "rb");
	if (!spStream)
		return false;
	IFRefPtr<IFJSONNode> spNode = IFJSONParser::parse(spStream);
	if (!spNode)
		return false;
	spNode->for_each_array([&](int i, IFJSONNode* pValue)
	{
		IFStringW sFullPath = UCombinePathW(m_sSuperAttrDir, pValue->getValue().getString());
		loadSuperAttrs(sFullPath);
		return true;
	});

	return true;
}






//////////////////////////////////////////////////////////////////////////

IF_DEFINERTTI(IFSuperAttrInfo, IFRefObj);

IFSuperAttrInfo::IFSuperAttrInfo()
{

}

IFSuperAttrInfo::~IFSuperAttrInfo()
{

}
