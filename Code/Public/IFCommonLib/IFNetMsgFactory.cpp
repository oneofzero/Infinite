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
#include "IFNetMsgFactory.h"
#include "IFCompress.h"
#include "IFNetConnection.h"

IF_DEFINERTTIROOT(IFNet_Message)

IF_DEFINERTTI(IFNetMsgFactory, IFRefObj)

IFNetMsgFactory::IFNetMsgFactory(void)
{
}


IFNetMsgFactory::~IFNetMsgFactory(void)
{
}



IFNetMsgFactory::MsgInfo* IFNetMsgFactory::getMsgInfo(IFI32 nMsgID)
{
	auto it = m_MsgCreatorList.find(nMsgID);
	if (it != m_MsgCreatorList.end())
		return &it->second;
	return NULL;
}

//void IFNetMsgFactory::registerMsg( IFI32 nMsgID, MsgCreateFunPtr pCreator )
//{
//	if (pCreator)
//		m_MsgCreatorList[nMsgID] = pCreator;
//}


IFRefPtr<IFMemStream> IFNet_Message::toPackage(bool bCompress, IFNetEncryptFunctor* pFun )
{
	IFRefPtr<IFMemStream> spStream = IFNew IFMemStream;
	IFUI32 packageLen = 0;
	*spStream << packageLen;
	IFI8 nFlag = 0;
	if (bCompress)
		nFlag |= FLAG_COMPRESS;
	if (pFun)
	{
		nFlag |= FLAG_ENCRYPT;

	}
	spStream->writeI8(nFlag);

	if (bCompress||pFun)
	{
		IFRefPtr<IFMemStream> spX = IFNew IFMemStream;
		serialize(spX);
		spX->seek(0,IFStream::ISSF_BEGIN);
		if (bCompress)
		{
			IFRefPtr<IFMemStream> spXX = IFNew IFMemStream;
			IFCompress::compress(spX, spXX);
			spXX->seek(0,IFStream::ISSF_BEGIN);
			spX = spXX;
		}
		if (pFun)
		{
			spX = (*pFun)(spX);
		}
		spStream->write(spX->getBuffer(), (int)spX->size());

	}
	else
	{
		serialize(spStream);

	}



	IFUI32* pLen = (IFUI32*)spStream->getBuffer();
	*pLen = (IFUI32)spStream->size();
	return spStream;
}

void IFNet_Message_EstablishEncryption_Req::deserialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_EstablishEncryption_Req>::deserialize(pStream);
	*pStream >> m_sPublicKeyMD5 >> m_sEncryptAESKey >> m_sTestDataEncrypt;


}

void IFNet_Message_EstablishEncryption_Req::serialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_EstablishEncryption_Req>::serialize(pStream);
	*pStream << m_sPublicKeyMD5 << m_sEncryptAESKey << m_sTestDataEncrypt;

}

void IFNet_Message_EstablishEncryption_Res::deserialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_EstablishEncryption_Res>::deserialize(pStream);
	*pStream >> m_sTestData;
}

void IFNet_Message_EstablishEncryption_Res::serialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_EstablishEncryption_Res>::serialize(pStream);
	*pStream << m_sTestData;

}

IF_DEFINERTTI(IFNet_Message_EstablishEncryption_Req, IFNet_Message);
IF_NET_DEFINEMSG(IFNet_Message_EstablishEncryption_Req, 0xFFFF0001);
IF_DEFINERTTI(IFNet_Message_EstablishEncryption_Res, IFNet_Message);
IF_NET_DEFINEMSG(IFNet_Message_EstablishEncryption_Res, 0xFFFF0002);
IF_DEFINERTTI(IFNet_Message_Generic_Encrypt_Pack, IFNet_Message);
IF_NET_DEFINEMSG(IFNet_Message_Generic_Encrypt_Pack, 0xFFFF0003);

void IFNet_Message_Generic_Encrypt_Pack::deserialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_Generic_Encrypt_Pack>::deserialize(pStream);
	auto size = pStream->readI32();
	m_spStream = IFNew IFMemStream(0, 0, size);
	
	m_spStream->seek(size, IFStream::ISSF_BEGIN);

	pStream->read((char*)m_spStream->getBuffer(), size);

	m_spStream->seek(0, IFStream::ISSF_BEGIN);

}

void IFNet_Message_Generic_Encrypt_Pack::serialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_Generic_Encrypt_Pack>::serialize(pStream);
	
	pStream->writeI32(m_spStream->size());
	pStream->write(m_spStream->getBuffer(), m_spStream->size());


}

IFString IFNet_Message_Sproto::getSprotoBuf()
{
	return m_sSProtoBuf;
}

void IFNet_Message_Sproto::deserialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_Sproto>::deserialize(pStream);
	IFI32 nLen = pStream->readI32();
	m_sSProtoBuf.resize(nLen);
	pStream->read(&m_sSProtoBuf[0], nLen);
}

void IFNet_Message_Sproto::serialize(IFStream* pStream)
{
	IFNet_MessageGen<IFNet_Message_Sproto>::serialize(pStream);
	*pStream << m_sSProtoBuf.size();
	pStream->write(m_sSProtoBuf.c_str(), m_sSProtoBuf.length());
}

void IFNet_Message_Sproto::sendSprotoPack(IFNetConnection* pCore, const char* p, int len)
{
	IFRefPtr<IFNet_Message_Sproto> spMsg = IFNew IFNet_Message_Sproto(p, len);
	pCore->sendPack(spMsg->toPackage(), 0);
}

IF_NET_DEFINEMSG(IFNet_Message_Sproto, 1397772879);

IF_DEFINERTTI(IFNet_Message_Sproto, IFNet_Message)