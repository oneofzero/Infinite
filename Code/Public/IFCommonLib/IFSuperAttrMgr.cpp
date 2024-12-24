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

bool IFSuperAttrMgr::loadSuperAttr(IFSuperAttrInfo* pInfo)
{
	//auto sPath = getAttFilePath(sSuperAttrName);

	IFRefPtr<IFStream> pStream = IFFileSystem::getSingletonPtr()->openStream(pInfo->getPath(), "rb");
	if (!pStream || pStream->size() == 0)
	{
		IFLogError("loadSuperAttr error file not exist!%s\r\n", pInfo->getPath().c_str());
		return false;
	}

	{
		IFString err;
		IFRefPtr<IFJSONNode> spRoot = IFJSONParser::parse(pStream,&err);
		if (spRoot)
		{
			IFAttributeRemap::Guard guard;
			IFRefPtr<IFAttributeRemap> spRemap = IFNew IFAttributeRemap;
			
			if(spRemap->loadFromJSON(spRoot))
			{
				IFAttributeMgr::getSingleton().setIndexRemap(spRemap);
			}
			//IFRefPtr<IFSuperAttrInfo> pInfo = IFNew IFSuperAttrInfo;
			//auto pInfo = &info;
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
			pInfo->m_loaded = true;
			//addSuperAttr(pInfo);
			return true;
		}
		else
		{
			IFLOG(IFLL_ERROR, "super attr %s load error:%s\r\n", pInfo->getPath().c_str(), err.c_str());
			pStream->seek(0, IFStream::ISSF_BEGIN);
		}
	}




	return false;
}



bool IFSuperAttrMgr::saveSuperAttrs( const IFString& sSuperAttrFileName )
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


bool IFSuperAttrMgr::saveSuperAttr( const IFString& sSuperAttrName )
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
		spRoot->setSubValue("attname", pInfo->m_sName);

		if (pInfo->m_AttributeList.getSuperAttrListName() == pInfo->m_sName)
		{
			pInfo->m_AttributeList.setSuperAttrList(IFString::Empty);
		}

		if(pInfo->m_AttributeList.getSuperAttrListName().size())
			spRoot->setSubValue("super", pInfo->m_AttributeList.getSuperAttrListName());


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
		IFString sUTF8 = spRoot->toString(true,true);
		IFString sFullName = pInfo->getPath(); //getAttFilePath(pInfo->m_sName);

		IFRefPtr<IFStream> spStream = IFFileSystem::getSingleton().openStream(sFullName,"wb");
		if (!spStream)
			return false;
		spStream->write(IFString::UTF8Flag, IFArraySize(IFString::UTF8Flag));
		spStream->write(sUTF8.c_str(), sUTF8.size());

		saveSuperAttriNameList(pInfo->m_sDir);
	}

	return true;

}


IFSuperAttrInfo* IFSuperAttrMgr::getSuperAttr( const IFString& sName )
{
	SuperAttList::iterator it = m_SuperAttList.find(sName);
	if(it!=m_SuperAttList.end())
	{
		if (!it->second->isLoaded())
		{
			//m_Loaded.insert(sName);
			loadSuperAttr(it->second);
		}

		return it->second;
	}
	return NULL;
}

void IFSuperAttrMgr::addSuperAttr(IFSuperAttrInfo* pInfo )
{
	m_SuperAttList[pInfo->m_sName] = pInfo;
	saveSuperAttr(pInfo);

	event_SuperAttrAdd(pInfo);
}

void IFSuperAttrMgr::removeSuperAttr( const IFString& sName, bool needSave)
{
	SuperAttList::iterator it = m_SuperAttList.find(sName);
	if(it!=m_SuperAttList.end())
	{
		if (needSave)
		{
			IFFileSystem::getSingleton().removeFile(it->second->getPath());
		}		

		event_SuperAttrRemove(it->second);
		auto sDir = it->second->m_sDir;
		m_SuperAttList.erase(it);
		if (needSave)
		{
			saveSuperAttriNameList(sDir);
		}

	}

	//if(bNeedSave)
	//	saveSuperAttrs( );
}

void IFSuperAttrMgr::renameSuperAttr( const IFString& sOldName, const IFString& sNewName )
{
	if(sOldName == sNewName )
		return;
	SuperAttList::iterator it = m_SuperAttList.find(sOldName);
	if(it!=m_SuperAttList.end())
	{
		auto sOldName = it->second->getPath();

	
		IFRefPtr<IFSuperAttrInfo> spSA = it->second;
		m_SuperAttList.erase(it);
		spSA->m_sName = sNewName;
		m_SuperAttList[sNewName] = spSA;
		IFFileSystem::getSingleton().rename(sOldName, it->second->getPath());


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

bool IFSuperAttrMgr::saveSuperAttriNameList(const IFString& sDir)
{
	if (sDir.isEmpty())
		return false;

	IFRefPtr<IFJSONNode> spRoot = IFNew IFJSONNode;
	for (auto& pr:m_SuperAttList)
	{
		IFRefPtr<IFJSONNode> spValue = IFNew IFJSONNode;
		//IFString sFullName = UGetRelativePath(getAttFilePath(pr.first), sDir);
		if (pr.second->m_sDir != sDir)
			continue;
		spValue->setValue(pr.second->getPath());
		spRoot->push_back(spValue);
	}

	IFString s = spRoot->toString();

	IFRefPtr<IFStream> spStream = IFFileSystem::getSingleton().openStream(UCombinePath(sDir,"attlist.json"), "wb");
	if (!spStream)
		return false;
	spStream->write(IFString::UTF8Flag, IFArraySize(IFString::UTF8Flag));
	spStream->write(s.c_str(), s.size());
	return true;

}

bool IFSuperAttrMgr::loadSuperAttrsFromFileList(const IFString& sDir)
{
	//m_sSuperAttrDir = sDir;
	auto loadPath = UCombinePath(sDir, "attlist.json");
	IFRefPtr<IFStream> spStream = IFFileSystem::getSingleton().openStream(loadPath, "rb");
	if (!spStream)
	{
		IFLogError("loadSuperAttrsFromFileList error!can't open %s\r\n", loadPath.c_str());
		return false;
	}
	
	return loadSuperAttrsFromStream(sDir, spStream);
}

bool IFSuperAttrMgr::loadSuperAttrsFromStream(const IFString& sDir, IFStream* pStream)
{
	IFRefPtr<IFJSONNode> spNode = IFJSONParser::parse(pStream);
	if (!spNode)
		return false;

	spNode->for_each_array([&](int i, IFJSONNode* pValue)
		{
			if (pValue->isValue() && pValue->getValue().isString())
			{
				auto attName = UGetFileMainName(pValue->getValue().getString());
				auto spInfo = NewIFRefObj< IFSuperAttrInfo>();
				spInfo->m_sDir = sDir;
				spInfo->m_sName = attName;
				m_SuperAttList[attName] = spInfo;

				event_SuperAttrAdd(spInfo);

			}
			else
			{

			}

			//IFString sFullPath = UCombinePath(m_sSuperAttrDir, pValue->getValue().getString());
			//loadSuperAttrs(sFullPath);
			return true;
		});

	return true;
}

IFRefPtr<IFAsyncResultBool> IFSuperAttrMgr::loadSuperAttrsFromFileListAsync(const IFString& sDirPath, bool loadAttrs)
{
	auto spResult = NewIFRefObj<IFAsyncResultBool>();
	auto loadPath = UCombinePath(sDirPath, "attlist.json");

	IFFileSystem::getSingleton().openStreamAsync(loadPath, "rb")->onResult(
		[=](auto spStream) {
			if (!loadSuperAttrsFromStream(sDirPath, spStream))
			{
				spResult->setResult(false);
				return;
			}
			if (loadAttrs)
			{
				for (auto it = m_SuperAttList.begin(); it != m_SuperAttList.end(); ++it)
				{
					if (!it->second->isLoaded())
					{
						//m_Loaded.insert(sName);
						loadSuperAttr(it->second);
					}
				}
			}
			spResult->setResult(true);
		}
	);


	return spResult;
}



const IFString IFSuperAttrMgr::BUILTIN_DIR = "ui/superattr";


//////////////////////////////////////////////////////////////////////////

IF_DEFINERTTI(IFSuperAttrInfo, IFRefObj);

IFSuperAttrInfo::IFSuperAttrInfo()
	:m_loaded(false)
{

}

bool IFSuperAttrInfo::load()
{
	return IFSuperAttrMgr::getSingleton().loadSuperAttr(this);
}


IFString IFSuperAttrInfo::getPath()
{
	return UCombinePath(m_sDir, m_sName) + ".attrxml";
}

IFSuperAttrInfo::~IFSuperAttrInfo()
{

}

