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
#include "IFNetConnection.h"
#include "IFNetIOData.h"
#include "IFNetCore.h"
#include "IFLogSystem.h"
#include "IFNetMsgFactory.h"
#include "IFTimer.h"
#include "IFSystemAPI.h"
#include "IFCompress.h"
#include "IFRSA.h"
#include "IFAES.h"
#include "IFMD5.h"
#include "IFAtomicOperation.h"
#ifdef IFCOMMON_UNITY_SUPPORT
#define DONT_USE_SSL
#endif

#ifndef DONT_USE_SSL
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#endif

class PackageMemStream : public IFMemStream
{
public:

	PackageMemStream( IFNetRecvPackage* pPackage)
		:IFMemStream(pPackage->m_Buf,pPackage->m_Buf.size())
		,spPackage(pPackage)
	{

	}
	IFRefPtr<IFNetRecvPackage> spPackage;
};

IF_DEFINERTTI(IFNetConnection, IFRefObj)

//void initSSL()
//{
//
//}
#ifndef DONT_USE_SSL
SSL_CTX * GetSSLContext()
{
	static SSL_CTX *ssl_ctx = NULL;
	if (ssl_ctx)
		return ssl_ctx;
	SSL_library_init();        //init libraries
	SSLeay_add_ssl_algorithms();;
	OPENSSL_add_all_algorithms_noconf(); //支持所有算法
	SSL_load_error_strings();  //提供将错误号解析为字符串的功能

							   //创建SSL上下文环境 每个进程只需维护一个SSL_CTX结构体
	ssl_ctx = SSL_CTX_new(SSLv23_method());
	SSL_CTX_set_verify(ssl_ctx, SSL_VERIFY_NONE, NULL);
    return ssl_ctx;
}
#endif

IFNetConnection::IFNetConnection( IFNetCore* pNetCore,bool bSyncEvent, bool bPackageMode ):
IFRefObj(true),
m_pNetCore(pNetCore),m_eConnectionState(IFNCS_UNKNOWN),
	m_nNotCompletePackLen(0),
	m_nSendPendingBytes(0),
	m_nMaxSendBufferSize(64*1024),
	m_nAutoKeepAliveTimeOutMS(0),
	m_bSyncEvent(bSyncEvent),
	m_bPackageMode(bPackageMode),
	m_nSendID(0),
	m_bEncryption(false),
	m_ssl(0),m_rbio(0),m_wbio(0), ssl_state(SSL_NONE), ssl_decrpyted(false)


{
	m_nLocalPort = 0;
	if (bPackageMode)
		m_spDepacker = IFNew IFNetDepacker;
	m_LastCommunicateTime = IFNativeSystemAPI::getTickCount();
	//m_pRecvDataBuf = pNetCore->AllocIOData(this, IFNIO_RECV, true);
	//if (!ssl_ctx)
	//{
	//	initSSL();
	//}
}

IFNetConnection::~IFNetConnection(void)
{
#ifndef DONT_USE_SSL
	if (m_ssl)
	{
		SSL_free(m_ssl);
		m_ssl = NULL;
	}
#endif
}



bool IFNetConnection::recvData( const char* pData, IFUI32 nLen )
{
#ifndef DONT_USE_SSL
	if (m_ssl&&!ssl_decrpyted)
	{
		IFLOG(IFLL_DEBUG, "process ssl data%d\r\n", nLen);
		BIO_write(m_rbio, pData, nLen);

		if (!SSL_is_init_finished(m_ssl))
		{

			int n = SSL_do_handshake(m_ssl);
			int sslerr = SSL_get_error(m_ssl, n);
			if (sslerr == SSL_ERROR_SSL)
			{
				//event_SSLEstablishResult(this, false);
				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_SSL_ESTABLISH_RESULT,false);
				m_pNetCore->pushEvent(pEvent);

				return true;
			}
			IFSimpleArray<char> buf;
			buf.resize(4096);
			int rl = BIO_read(m_wbio, buf, buf.size()); // Read from BIO, put data in buffer
			if (rl > 0)
			{
				sendData(buf, rl);

			}
			if (SSL_is_init_finished(m_ssl))
			{
				ssl_state = SSL_ESTABLISED;
				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_SSL_ESTABLISH_RESULT, true);
				m_pNetCore->pushEvent(pEvent);

				//event_SSLEstablishResult(this,true);
			}
			else
			{
				return true;
			}
		}
		if (ssl_state == SSL_ESTABLISED)
		{
			char buf[1024 * 8];
			ssl_decrpyted = true;
			int nL = SSL_read(m_ssl, buf, sizeof(buf));
			while (nL>0)
			{
				recvData(buf, nL);
				nL = SSL_read(m_ssl, buf, sizeof(buf));
			}
			ssl_decrpyted = false;
			return true;
		}
		else
		{
			return true;
		}

	}
#endif
	//IFLOG(IFLL_DEBUG, "process normal data\r\n");

	if(m_bPackageMode )
	{
		//m_FIFO->write(pData->getBuffer(),nLen);
		if (!m_spDepacker->write(pData,nLen))
		{
			if (m_bSyncEvent)
			{
				IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_ERROR_PACK);
				m_pNetCore->pushEvent(pEvent);

			}
			else
				event_ErrorPack(this);
		}

		while (IFRefPtr<IFNetRecvPackage> spPackage = m_spDepacker->getPackage())
		{
			int nMsgID = 0;
			try
			{
				IFRefPtr<IFMemStream> spStream = IFNew PackageMemStream(spPackage);
				if (m_spPackPreProcessFun)
				{
					if ((*m_spPackPreProcessFun)(spStream) == false)
						continue;
				}
				spStream->seek(IFNetRecvPackage::PACKAGE_HEAD_SIZE,IFStream::ISSF_BEGIN);
				IFI8 flag = spStream->readI8();

				if (flag&IFNet_Message::FLAG_ENCRYPT)
				{
					if (!m_spDecryptFun)
					{
						IFLOG(IFLL_ERROR,"recv a encrypt package, but decrypt fun not set!\r\n");
						continue;
					}
					spStream = (*m_spDecryptFun)(spStream);
					spStream->seek(0,IFStream::ISSF_BEGIN);

				}

				if (flag&IFNet_Message::FLAG_COMPRESS)
				{
					IFRefPtr<IFMemStream> spDecS = IFNew IFMemStream;
					IFCompress::decompress(spStream, spDecS);
					spStream = spDecS;
					spStream->seek(0,IFStream::ISSF_BEGIN);
				}


	
				//IFUI16 msgLen = spStream->readUI16();
				nMsgID = spStream->readI32();
				if (nMsgID != 0)
				{
					auto msginfo = m_pNetCore->getMsgFactory()->getMsgInfo(nMsgID);
					if(msginfo)
					{
						auto spMessage = (*msginfo->spCreator)();
						spMessage->deserialize(spStream);

			
						if (m_pNetCore->m_bSyncEvent)
						{
							IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, spMessage);
							m_pNetCore->pushEvent(pEvent);
							//m_PackageList.push_back(spMessage);
						}
						else
						{
							(*msginfo->spProcess)(this, spMessage);
							//spMessage->process(this);
						}
					}
					else
					{
						event_RecvUnKnownPackage(this,spPackage->m_Buf, spPackage->m_Buf.size());
						//IFLOG(IFLL_WARNING, "无法解析的错误消息\r\n");
					}
				}
				else
				{
					//KeepAlive Msg
				}
			}
			catch(IFStreamReadException& )
			{
				if (m_pNetCore->m_bSyncEvent)
				{
					IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_ERROR_PACK);
					m_pNetCore->pushEvent(pEvent);

				}
				else
					event_ErrorPack(this);
				IFLOG(IFLL_WARNING, "无法解析的错误消息[%d]\r\n", nMsgID);
			}
		}
		

	}
	else
	{
		if (m_bSyncEvent)
		{
			IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNew IFSimpleArray<char>(nLen,pData));
			m_pNetCore->pushEvent(pEvent);
		}
		else
			event_RecvData(this, pData,nLen);
	}

	return true;
}


IFNetConnectionState IFNetConnection::getState() const
{
	return m_eConnectionState;
}

const IFString& IFNetConnection::getRemoteIP() const
{
	return m_sRemoteIP;
}

int IFNetConnection::getRemotePort() const
{
	return m_nRemotePort;
}

IFNetCore* IFNetConnection::getNetCore()
{
	return m_pNetCore;
}



IFUI64 IFNetConnection::sendString( const char* sString )
{
	return sendData(sString, (IFUI32)strlen(sString)+1);
}

IFUI64 IFNetConnection::sendMsg(IFNet_Message* pMsg, bool bCompress /*= false*/, IFNetEncryptFunctor* pFun /*= NULL*/)
{
	//IFLOG(IFLL_TRACE, "IFNetConnection::sendMsg 1\r\n");
	IFAsyncSendMsgInfo info;
	info.spConnection = this;
	//IFLOG(IFLL_TRACE, "IFNetConnection::sendMsg 1.5\r\n");
	info.nSendID = getSendID();
	//IFLOG(IFLL_TRACE, "IFNetConnection::sendMsg 2\r\n");

	info.bCompress = bCompress;
	info.pFun = pFun?pFun:m_spDefaultEncryptFun.getPtr();
	//IFLOG(IFLL_TRACE, "IFNetConnection::sendMsg 3\r\n");

	info.spMsg = pMsg;
	//IFLOG(IFLL_TRACE, "IFNetConnection::sendMsg 4\r\n");

	m_pNetCore->sendAsyncMsg(info);
	//IFLOG(IFLL_TRACE, "IFNetConnection::sendMsg 5\r\n");

	return info.nSendID;
}

void IFNetConnection::setPackageMode(bool b)
{
	m_bPackageMode = b;
}

bool IFNetConnection::isPackageMode()
{
	return m_bPackageMode;
}

IFUI64 IFNetConnection::sendData(const void* pData, IFUI32 nLen)
{
#ifndef DONT_USE_SSL
	if (m_ssl && ssl_state == SSL_ESTABLISED)
	{
		int nwritesize = SSL_write(m_ssl, pData, nLen);
		char buf[8 * 1024];
		IFUI64 nSendSize = 0;
		int nSZ = BIO_read(m_wbio, buf, sizeof(buf));
		while (nSZ>0)
		{
			IFRefPtr<IFMemStream> spPack = IFNew IFMemStream;
			spPack->write(buf, nSZ);
			nSendSize += sendPack(spPack);
			nSZ = BIO_read(m_wbio, buf, sizeof(buf));
		}
		return nSendSize;

	}
#endif
	IFRefPtr<IFMemStream> spPack = IFNew IFMemStream;
	spPack->write(pData,nLen);
	return sendPack(spPack);
}

/*
bool IFNetConnection::requestRecvData(IFNetIOData* pData)
{
#ifdef WIN32
	DWORD dwRecvSize = 0, dwFlag = 0;
	if (pData == NULL)
		pData = m_pNetCore->AllocIOData(this, IFNIO_RECV, true);
	else
	{
		pData->reset();
	}
	//m_pRecvDataBuf->reset();
	HRESULT hr  = WSARecv( m_Socket, &(pData->wsaBuf), 1, &dwRecvSize, &dwFlag, (LPWSAOVERLAPPED)(pData),NULL  );
	if( hr == SOCKET_ERROR )
	{
		int nLastError = WSAGetLastError();
		if( nLastError != ERROR_IO_PENDING )
		{
			IFLOG(IFLL_ERROR, "recv error!\r\n");
			return false;
		}
	}
#endif
	return true;
}
*/
void IFNetConnection::setUserData( IFRefPtr<IFRefObj> spUserData )
{
	m_spUserData = spUserData;
}

void IFNetConnection::fireRecvPackageEvent( IFNetRecvPackage* pPackage )
{

}

void IFNetConnection::fireSendDoneEvent(IFUI64 nSendID, bool bOK)
{
	if (m_bSyncEvent)
	{
		IFNetCoreEvent* pEvent = IFNew IFNetCoreEvent(this, IFNetCoreEvent::ET_DATA_SENDED,nSendID, bOK);
		m_pNetCore->pushEvent(pEvent);
	}
	else
	{
		event_SendResult(this, nSendID, bOK);
	}
}

void IFNetConnection::procEstablishEncryptRes(IFNet_Message_EstablishEncryption_Res* pRes)
{
	if (m_spAES&& m_EncryptTestData.size() > 0&& m_EncryptTestData.size()== pRes->m_sTestData.size())
	{
		if (memcmp(m_EncryptTestData, pRes->m_sTestData, pRes->m_sTestData.size()) == 0)
		{
			setDecryptFun(makeIFFunctor<IFRefPtr<IFMemStream>(IFStream*)>([=](IFStream* pIn)
			{
				IFRefPtr<IFMemStream> spOUT = IFNew IFMemStream;
				m_spAES->decrypt(pIn, spOUT);
				return spOUT;
			}));
			setDefalutEncryptFun(makeIFFunctor<IFRefPtr<IFMemStream>(IFStream*)>([=](IFStream* pIn)
			{
				IFRefPtr<IFMemStream> spOUT = IFNew IFMemStream;
				m_spAES->encrypt(pIn, spOUT);
				return spOUT;
			}));
			m_bEncryption = true;
			event_EncryptionEstablished(this);
		}
	}
}

void IFNetConnection::ProcEstablishEncryptRes(IFNetConnection* pCon, IFNet_Message_EstablishEncryption_Res* pRes)
{
	pCon->procEstablishEncryptRes(pRes);
}

IFUI64 IFNetConnection::getSendID()
{
	//IFLOG(IFLL_TRACE, "send id 0x%p %p\r\n", &m_nSendID, this);
	return ATOMIC_INC_INT64(&m_nSendID);
}

IFUI32 IFNetConnection::getLastCommunicateTime()
{
	return m_LastCommunicateTime;
}

void IFNetConnection::setPackPreProcessFun( PackProcessFunPtr spFun )
{
	m_spPackPreProcessFun = spFun;
}

void IFNetConnection::setMaxSendBufferSize( IFUI32 nMaxNum /*= 32*/ )
{
	m_nMaxSendBufferSize = nMaxNum;
}

IFUI32 IFNetConnection::getMaxSendBufferSize()
{
	return m_nMaxSendBufferSize;
}

void IFNetConnection::setLastCommunicateTime()
{
	m_LastCommunicateTime = IFNativeSystemAPI::getTickCount();
}

void IFNetConnection::setAutoKeepAliveTime(IFUI32 timeoutMS, IFRefPtr<IFMemStream> spKeepAlivePackage)
{
	m_nAutoKeepAliveTimeOutMS = timeoutMS;
	if (m_nAutoKeepAliveTimeOutMS == 0)
	{
		m_pNetCore->removeAutoKeepAliveConnection(this);
	}
	else
	{
		m_spKeepAlivePackage = spKeepAlivePackage;
		m_pNetCore->addAutoKeepAliveConnection(this);
	}
}

IFUI32 IFNetConnection::getAutoKeepAliveTime()
{
	return m_nAutoKeepAliveTimeOutMS;
}

void IFNetConnection::establishEncryption(IFMemStream* pPublicKey)
{
	IFString sAesKey;
	srand(IFNativeSystemAPI::getTickCount());
	for (int i = 0; i < 16; i++)
	{
		sAesKey.push_back(rand()%93+33);
	}
	IFRefPtr<IFMemStream> spEncryptAESKey = IFNew IFMemStream;
	IFRefPtr<IFMemStream> spInputAESKey = IFNew IFMemStream(sAesKey.c_str(),sAesKey.length());
	pPublicKey->seek(0, IFStream::ISSF_BEGIN);
	IFLOG(IFLL_DEBUG, "rsa encrypt 1!\r\n");
	if (!IFRSA::encryptPublic(spEncryptAESKey, spInputAESKey, pPublicKey))
	{
		return;
	}
	IFLOG(IFLL_DEBUG, "rsa encrypt 2\r\n");



	IFRefPtr<IFNet_Message_EstablishEncryption_Req> spReq = IFNew IFNet_Message_EstablishEncryption_Req;
	pPublicKey->seek(0, IFStream::ISSF_BEGIN);
	IFMD5Result md5(pPublicKey);
	spReq->m_sPublicKeyMD5 =  md5.toString();
	spReq->m_sEncryptAESKey.resize(spEncryptAESKey->size());
	memcpy(spReq->m_sEncryptAESKey, spEncryptAESKey->getBuffer(), spEncryptAESKey->size());
	IFLOG(IFLL_DEBUG, "rsa encrypt 3\r\n");
	m_EncryptTestData.resize(32);
	for (int i = 0; i < 32; i ++)
	{
		m_EncryptTestData[i] = rand() % 256;
	}
	IFLOG(IFLL_DEBUG, "rsa encrypt 4\r\n");

	IFRefPtr<IFMemStream> spEncryptTestData = IFNew IFMemStream;
	IFRefPtr<IFMemStream> spTestData = IFNew IFMemStream(m_EncryptTestData, m_EncryptTestData.size());
	m_spAES = IFNew IFAES(sAesKey);
	IFLOG(IFLL_DEBUG, "rsa encrypt 5\r\n");

	m_spAES->encrypt(spTestData, spEncryptTestData);
	IFLOG(IFLL_DEBUG, "rsa encrypt 6\r\n");

	spReq->m_sTestDataEncrypt.resize(spEncryptTestData->size());
	memcpy(spReq->m_sTestDataEncrypt, spEncryptTestData->getBuffer(), spEncryptTestData->size());
	IFLOG(IFLL_DEBUG, "rsa encrypt 7\r\n");

	sendMsg(spReq);
	IFLOG(IFLL_DEBUG, "rsa encrypt 8\r\n");

}

bool IFNetConnection::isEncryption()
{
	return m_bEncryption;
}

bool IFNetConnection::establishSSL()
{
#ifndef DONT_USE_SSL
	m_ssl = SSL_new(GetSSLContext());
	m_rbio = BIO_new(BIO_s_mem());
	m_wbio = BIO_new(BIO_s_mem());

	SSL_set_fd(m_ssl, getSocket());
	SSL_set_bio(m_ssl, m_rbio, m_wbio);

	SSL_set_connect_state(m_ssl);

	int n = SSL_do_handshake(m_ssl);
	int sslerr = SSL_get_error(m_ssl, n);

	IFSimpleArray<char> buf;
	buf.resize(4096);
	int nLen = BIO_read(m_wbio, buf, buf.size()); // Read from BIO, put data in buffer
												  //nLen = BIO_read(wbio, buf, buf.size());
	if (nLen > 0)
		sendData(buf, nLen);
	else
		return false;
	return true;
#else
	return false;
#endif

}

bool IFNetConnection::waitEstablishSSL()
{
#ifndef DONT_USE_SSL
	m_ssl = SSL_new(GetSSLContext());
	m_rbio = BIO_new(BIO_s_mem());
	m_wbio = BIO_new(BIO_s_mem());

	SSL_set_fd(m_ssl, getSocket());
	SSL_set_bio(m_ssl, m_rbio, m_wbio);

	//SSL_set_connect_state(m_ssl);
	SSL_set_accept_state(m_ssl);

	//BIO_write(m_rbio, pData, nLen);

	//SSL_accept(m_ssl);


	//IFSimpleArray<char> buf;
	//buf.resize(4096);
	//int nLen = BIO_read(m_wbio, buf, buf.size()); // Read from BIO, put data in buffer
	//											  //nLen = BIO_read(wbio, buf, buf.size());
	//if (nLen > 0)
	//	sendData(buf, nLen);
	//else
	//	return false;
	return true;
#else
	return false;
#endif
}

void IFNetConnection::setDecryptFun(IFNetEncryptFunctor* pDecryptFun)
{
	m_spDecryptFun = pDecryptFun;
}





void IFNetConnection::setDefalutEncryptFun(IFNetEncryptFunctor* pEncryptFun)
{
	m_spDefaultEncryptFun = pEncryptFun;
}

bool IFNetDepacker::write(const void* pData, int nLen)
{
	//IFCSLockHelper lh(m_Lock);
	IFNetRecvPackage* pLastPackage=NULL;
	if (m_Packages.size())
	{
		pLastPackage = m_Packages.back();
		if (pLastPackage->isFull())
			pLastPackage = NULL;

	}


	
	const char* pCurPtr = (const char*)pData;

	int nRemainSize = nLen;
	while (nRemainSize>0)
	{
		if (!pLastPackage)
		{
			pLastPackage = IFNew IFNetRecvPackage;
			m_Packages.push_back(pLastPackage);
		}
		int nWriteSize = pLastPackage->write(pCurPtr,nRemainSize);
		if (nWriteSize==0)
			return false;
		nRemainSize -= nWriteSize;
		pCurPtr += nWriteSize;

		if (pLastPackage->isFull())
		{
			pLastPackage = NULL;
		}
		

	}
	return true;
}

IFRefPtr<IFNetRecvPackage> IFNetDepacker::getPackage()
{
	//IFCSLockHelper lh(m_Lock);

	if (m_Packages.size())
	{
		IFRefPtr<IFNetRecvPackage> spPack = m_Packages.front();
		if (spPack->isFull())
		{
			m_Packages.pop_front();
			return spPack;
		}
	}
	return NULL;

}

bool IFNetDepacker::hasPackage()
{

	IFCSLockHelper lh(m_Lock);

	if (m_Packages.size())
	{	
		return m_Packages.front()->isFull();	
	}
	return false;
}

IFNetDepacker::IFNetDepacker()
	:m_nPackLenBufSize(0)

{
	//m_spSizeBuf = IFNew IFMemStream();
}

int IFNetRecvPackage::write(const void* p, IFUI32 nSize)
{
	const char* pData =(const char*)p;
	int nRemain = nSize;
	if (m_nCurPackLenBufSize!=PACKAGE_HEAD_SIZE)
	{
		while (nRemain)
		{
			m_LenBuf[m_nCurPackLenBufSize] = *pData;
			pData ++;
			nRemain --;
			m_nCurPackLenBufSize ++;
			if (m_nCurPackLenBufSize==PACKAGE_HEAD_SIZE)
			{
				if (m_PackLen<PACKAGE_HEAD_SIZE)
					return 0;
				if (m_PackLen > 1024*1024*64)
					return 0;
				m_Buf.resize(m_PackLen);
				m_Buf[0] = m_LenBuf[0];
				m_Buf[1] = m_LenBuf[1];
				m_Buf[2] = m_LenBuf[2];
				m_Buf[3] = m_LenBuf[3];

				m_nCurBufSize += PACKAGE_HEAD_SIZE;
				break;
			}

		}
	}

	int nReadSize = IFMin(nRemain,m_Buf.size()-m_nCurBufSize);
	if(nReadSize)
	{
		memcpy(&m_Buf[m_nCurBufSize],pData, nReadSize);
		m_nCurBufSize += nReadSize;
		nRemain -= nReadSize;
	}

	return nSize - nRemain;
}

IF_DEFINERTTI(IFNetDepacker, IFRefObj);
IF_DEFINERTTI(IFNetRecvPackage, IFRefObj);
