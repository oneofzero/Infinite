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
#ifndef __IF_NET_MSG_FACTORY_H__
#define __IF_NET_MSG_FACTORY_H__
#include "IFCommonLib_API.h"
#include "ifsingleton.h"
#include "IFRefObj.h"
#include "IFStream.h"
#include "IFNetConnection.h"
#include "IFTypes.h"
#include "IFFunctor.h"
#include "IFHashMap.h"

class IFCOMMON_API IFNet_Message : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum {
		FLAG_NONE = 0,
		FLAG_COMPRESS = 1,
		FLAG_ENCRYPT =  1 << 1,
	};
	IFNet_Message()
		:IFRefObj(true)
	{

	}
public:
	virtual void deserialize(IFStream* pStream)
	{
	}
	virtual void serialize(IFStream* pStream)
	{
	}

	//virtual bool process(IFNetConnection* pConnection) = 0;

	IFRefPtr<IFMemStream> toPackage(bool bCompress = false, IFNetEncryptFunctor* pFun = NULL);


	virtual IFI32 getMsgID() = 0;

protected:
	virtual ~IFNet_Message()
	{

	}

};

template<typename T>
class IFNet_MessageGen : public IFNet_Message
{
public:
	typedef T MSGEntity;

	static IFRefPtr<IFFunctor<void(IFNetConnection* pConnection,MSGEntity* pMsg)> > MsgProc;
	static IFI32 MsgID;
	static inline IFRefPtr<IFNet_Message> createMsg()
	{
		IFRefPtr<IFNet_Message> spMsg = IFNew MSGEntity;
		return spMsg;
	}

	static IFI32 GetMsgID()
	{
		return MsgID;
	}

	virtual IFI32 getMsgID()
	{
		return MsgID;
	}

	static inline void setMsgProc(IFRefPtr<IFFunctor<void(IFNetConnection* pConnection,MSGEntity* pMsg)> > p)
	{
		MsgProc = p;
	}
	static inline IFFunctor<void(IFNetConnection* pConnection,MSGEntity* pMsg)>* getMsgProc()
	{
		return MsgProc;
	}

	virtual void deserialize(IFStream* pStream)
	{
		//int nMsgID;
		//*pStream >> nMsgID;
		//assert(nMsgID == MsgID);
	}
	virtual void serialize(IFStream* pStream)
	{
		*pStream << MsgID;
	}
	inline bool process(IFNetConnection* pConnection)
	{
		if (MsgProc)
		{
			(*MsgProc)(pConnection, (MSGEntity*)this);
			return true;
		}

		return false;
	}

};

#define IF_NET_DEFINEMSG(MSGSTRUCT, MSGID) \
template <> IFRefPtr<IFFunctor<void(IFNetConnection* ,MSGSTRUCT* )> > IFNet_MessageGen<MSGSTRUCT>::MsgProc = NULL;\
template <> IFI32 IFNet_MessageGen<MSGSTRUCT>::MsgID = MSGID;

#define IF_NET_REGISTERMSG(MSGSTRUCT, processfun, netcore) \
	netcore->getMsgFactory()->registerMsg(processfun);
	//assert(MSGSTRUCT::getMsgProc()==NULL);
	//MSGSTRUCT::setMsgProc(processfun);


class IFCOMMON_API IFNetMsgFactory: public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFNetMsgFactory(void);

	typedef IFRefPtr<IFFunctor<IFRefPtr<IFNet_Message>()> > MsgCreateFunPtr;
	typedef IFRefPtr<IFFunctor<void(IFNetConnection*, IFNet_Message*)>> MsgProcFunPtr;
public:

	struct MsgInfo
	{
		MsgCreateFunPtr spCreator;
		MsgProcFunPtr spProcess;
	};

	MsgInfo* getMsgInfo(IFI32 nMsgID);

	template<typename MSGT>
	void registerMsg(IFRefPtr<IFFunctor<void(IFNetConnection*, MSGT*)>> spProcessFun)
	{
		MsgInfo& info = m_MsgCreatorList[MSGT::MsgID];
		info.spCreator = makeIFFunctor<IFRefPtr<IFNet_Message>()>([]()
		{
			return IFNew MSGT();
		});
		info.spProcess = makeIFFunctor<void(IFNetConnection*, IFNet_Message*)>([=](IFNetConnection* pCon, IFNet_Message* pMsg)
		{
			(*spProcessFun)(pCon, (MSGT*)pMsg);
		});
	}

	template<typename MSGT>
	void registerMsg()
	{
		MsgInfo& info = m_MsgCreatorList[MSGT::MsgID];
		info.spCreator = makeIFFunctor<IFRefPtr<IFNet_Message>()>([]()
		{
			return IFNew MSGT();
		});
		info.spProcess = NULL;
	}

protected:
	~IFNetMsgFactory(void);



	typedef IFHashMap<IFI32, MsgInfo> MsgCreatorList;
	MsgCreatorList m_MsgCreatorList;
};


class IFNet_Message_EstablishEncryption_Req : public IFNet_MessageGen<IFNet_Message_EstablishEncryption_Req>
{
	IF_DECLARERTTI;
	IFString m_sPublicKeyMD5;
	IFSimpleArray<char> m_sEncryptAESKey;
	IFSimpleArray<char> m_sTestDataEncrypt;

	virtual void deserialize(IFStream* pStream);
	virtual void serialize(IFStream* pStream);
};

class IFNet_Message_EstablishEncryption_Res : public IFNet_MessageGen<IFNet_Message_EstablishEncryption_Res>
{
	IF_DECLARERTTI;
	IFSimpleArray<char> m_sTestData;
	virtual void deserialize(IFStream* pStream);
	virtual void serialize(IFStream* pStream);
};

class IFCOMMON_API IFNet_Message_Generic_Encrypt_Pack : public IFNet_MessageGen<IFNet_Message_Generic_Encrypt_Pack>
{
	IF_DECLARERTTI;
	IFRefPtr<IFMemStream> m_spStream;
	IFMemStream* getRawPack()
	{
		return m_spStream;
	}
	void setRawPack(IFMemStream* pRawPack)
	{
		m_spStream = pRawPack;
	}

	virtual void deserialize(IFStream* pStream);
	virtual void serialize(IFStream* pStream);
};


class IFCOMMON_API IFNet_Message_Sproto : public IFNet_MessageGen<IFNet_Message_Sproto>
{
	IF_DECLARERTTI;
public:
	IFNet_Message_Sproto()
		:m_sSProtoBuf(IFString::EC_UTF8)
	{

	}
	IFNet_Message_Sproto(const IFString& s)
	{
		m_sSProtoBuf = s;
	}
	IFNet_Message_Sproto(const char* p, int len)
	{
		m_sSProtoBuf = IFString(p, len);
	}



	static void sendSprotoPack(IFNetConnection* pConnection, const char* p, int len);

	IFString m_sSProtoBuf;

	IFString getSprotoBuf();
	virtual void deserialize(IFStream* pStream);
	virtual void serialize(IFStream* pStream);

protected:
	~IFNet_Message_Sproto()
	{

	}
};
#endif //__IF_NET_MSG_FACTORY_H__