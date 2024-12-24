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
#include "IFHttpDownloader.h"
#include "IFString.h"
#include "IFStream.h"
#include "IFNetCore.h"
#include "IFNetConnection.h"
#include "IFLogSystem.h"
#include "IFDateTime.h"
#include "IFUtility.h"
#include "IFSystemAPI.h"

IF_DEFINERTTI(IFHttpDownloader, IFRefObj)
IF_DEFINERTTI(IFHttpDowloadQueue, IFRefObj)

IFHttpDownloader::IFHttpDownloader( IFNetCore* pNetCore )
	:IFRefObj(true)
	, event_DownloadProgress()
	, event_DownloadResult()

	,m_spNetCore(pNetCore)
	, m_nTimeOutMS(0)
	
	, m_ServerPort(0)
	, m_httpState(0)
	, m_bHTTPS(false)
	, m_disconnectAfterDone(true)
{
	m_State = HS_NOT_CONNECT;

	m_handleConnectResult = makeIFFunctor(this, &IFHttpDownloader::procConnectResult);
	m_handleRecvData = makeIFFunctor(this, &IFHttpDownloader::procDataReceive);
	m_handleSSLEstablished = makeIFFunctor(this, &IFHttpDownloader::procSSLEstablished);
}

void IFHttpDownloader::setTimeOut(int nMS)
{
	if (m_spCheckTimeOut)
	{
		m_spNetCore->getTimer()->removeFunctor(m_spCheckTimeOut);
	}

	m_nTimeOutMS = nMS;
	if (nMS > 0)
	{
		m_spCheckTimeOut = m_spNetCore->getTimer()->addFunctor([=](int deltaTime)
		{
			if (m_spConnection && m_spConnection->getState() == IFNCS_CONNECTED)
			{
				if (IFNativeSystemAPI::getTickCount() - m_spConnection->getLastCommunicateTime() > nMS)
				{
					m_spConnection->disconnect();
				}
			}

		}, nMS, -1);
	}
	
}

void IFHttpDownloader::setParam(const IFString& name, const IFString& value)
{
	m_Params[name] = value;
}

IFHttpDownloader::~IFHttpDownloader()
{
	setTimeOut(0);
	if (m_spConnection)
	{
		m_handleRecvData.disconnectAllSlot();
		m_handleDisConnect.disconnectAllSlot();
		m_spConnection->disconnect();
		m_spConnection = NULL;
	}
}




void IFHttpDownloader::download( const IFString& surl, IFRefPtr<IFStream> spStream,IFUI64 nByesOffset )
{

	m_ResponseHead.clear();
	m_spOutPutStream = spStream;
	m_nContentLength = 0;
	m_State = HS_NOT_CONNECT;

	m_nRecvLength = 0;
	m_bHTTPS = false;
	IFString sServer;
	IFString sFileName;

	int nPort;
	if (!parseURL(surl, m_bHTTPS, sServer, nPort, sFileName))
	{
		return;
	}

	bool serverchange = m_ServerAddress != sServer || m_ServerPort != nPort;
	m_ServerAddress = sServer;
	m_ServerPort = nPort;

	m_sURL = surl;
	IFString request;
	request += "GET ";
	request += sFileName;
	request += " HTTP/1.1\r\n";
	request += "User-Agent: Mozialla/4.0\r\n";

	if (nByesOffset)
	{
		request += IFString().format("RANGE: bytes=%lld-\r\n", nByesOffset);
	}


	request += "Host: ";
	request += sServer;
	request += "\r\n";

	for (auto& pr: m_Params)
	{
		request += pr.first;
		request += ": ";
		request += pr.second;
		request += "\r\n";
	}


	request += "Accept:*/*\r\n";
	request += "\r\n";

	m_HTTPRequest = IFNew IFMemStream();
	IFString s = request;// .toUTF8String();
	if (!s.isUTF8Codeing())
	{
		s = IFStringW(request).toUTF8String();
	}

	m_HTTPRequest->write(s.c_str(), s.length());
	m_sHTTPHead.clear();

	if (serverchange || m_State != HS_DONE || !m_spConnection || m_spConnection->getState() != IFNCS_CONNECTED)
	{
		m_handleConnectResult.disconnectAllSlot();
		m_handleRecvData.disconnectAllSlot();
		m_handleSSLEstablished.disconnectAllSlot();
		IFLOG(IFLL_DEBUG, "connect to %s:%d\r\n", sServer.c_str(), nPort);
		if (!sServer.size())
			return;
		m_spConnection = m_spNetCore->createConnection(sServer.c_str(), nPort, false, false);
		if (!m_spConnection)
		{
			event_DownloadResult(this, surl, ER_CONNECTTION, spStream, 0);
			return;
		}
		//m_spConnection->event_ConnectResult.AddSelfHoldHandle(makeIFFunctor(this, &IFHttpDownloader::procConnectResult));
		m_handleConnectResult.connectSlot(m_spConnection->event_ConnectResult);
	}
	else
	{
		m_State = HS_NOT_CONNECT;
		m_spConnection->sendData(m_HTTPRequest->getBuffer(), m_HTTPRequest->size());

	}


	m_nLastReceiveDataTime = IFDateTime::now().toIntTime();

}
void IFHttpDownloader::downloadByPost(const IFString& surl, IFRefPtr<IFStream> spStream, IFRefPtr<IFStream> spPostStream)
{
	m_ResponseHead.clear();
	m_spOutPutStream = spStream;
	m_State = HS_NOT_CONNECT;
	m_httpState = 0;
	m_nContentLength = 0;
	m_nRecvLength = 0;
	m_bHTTPS = false;
	IFString sServer;
	IFString sFileName;
	m_sHTTPHead.clear();

	int nPort;
	if (!parseURL(surl, m_bHTTPS, sServer, nPort, sFileName))
	{
		return;
	}
	bool serverchange = m_ServerAddress != sServer || m_ServerPort != nPort;
	m_ServerAddress = sServer;
	m_ServerPort = nPort;

	m_sURL = surl;
	IFStringW postrequest;

	postrequest += L"POST ";
	postrequest += sFileName;
	postrequest += L" HTTP/1.1\r\n";
	postrequest += L"User-Agent: Mozialla/4.0\r\n";

	if (m_Params.find("Host") == m_Params.end())
		m_Params["Host"] = sServer;
	m_Params["Content-Length"] = IFString().format("%d", spPostStream->size());
	m_Params["Accept"] = "*/*";


	for (auto& pr : m_Params)
	{
		postrequest += pr.first;
		postrequest += L": ";
		postrequest += pr.second;
		postrequest += L"\r\n";
	}

	postrequest += L"\r\n";

	//postrequest.format(
	//	L"POST %s HTTP/1.1\r\n"
	//	L"User-Agent: Mozialla/4.0\r\n"
	//	L"Host:%s\r\n"
	//	L"Content-Length:%d\r\n"
	//	L"Accept:*/*\r\n"
	//	L"\r\n"
	//	, sFileName.c_str()
	//	, sServer.c_str()
	//	, (int)spPostStream->size());
	m_HTTPRequest = IFNew IFMemStream();
	IFString s = postrequest.toUTF8String();
	m_HTTPRequest->write(s.c_str(), s.length());
	char buf[1024];
	spPostStream->seek(0, IFStream::ISSF_BEGIN);
	while (int nReadLen = spPostStream->read(buf,1024))
	{
		m_HTTPRequest->write(buf, nReadLen);
	}
	m_handleConnectResult.disconnectAllSlot();
	m_handleRecvData.disconnectAllSlot();
	m_handleSSLEstablished.disconnectAllSlot();
	IFLOG(IFLL_DEBUG, "connect to %s:%d\r\n", sServer.c_str(), nPort);
	if (!sServer.size())
		return;
	m_spConnection = m_spNetCore->createConnection(sServer.c_str(), nPort, false, false);
	if (!m_spConnection)
	{
		event_DownloadResult(this, surl, ER_CONNECTTION, spStream, 0);
		return;
	}
	//m_spConnection->event_ConnectResult.AddSelfHoldHandle(makeIFFunctor(this, &IFHttpDownloader::procConnectResult));
	m_handleConnectResult.connectSlot(m_spConnection->event_ConnectResult);
	m_nLastReceiveDataTime = IFDateTime::now().toIntTime();
}

bool IFHttpDownloader::parseURL(const IFString& surl, bool& bHTTPS, IFString& sServer, int& nPort, IFString& sFileName)
{
	if (surl.size() == 0)
		return false;

	bHTTPS = false;

	int nFindStart = 0;
	if (surl.size() > 7 && surl.sub(0, 7).toUpper() == "HTTP://")
		nFindStart = 7;
	if (surl.size() > 8 && surl.sub(0, 8).toUpper() == "HTTPS://")
	{
		nFindStart = 8;
		bHTTPS = true;
	}

	int nFileSplit = surl.find_first_of(L'/', nFindStart);
	if (nFileSplit == -1)
		nFileSplit = surl.length();

	sServer = surl.sub(nFindStart, nFileSplit - nFindStart);;
	sFileName = surl.sub(nFileSplit, surl.size() - nFileSplit);
	if (sFileName.size() == 0)
		sFileName = L"/";

	nPort = bHTTPS ? 443 : 80;
	int nPortPos = sServer.find_first_of(':');
	IFLOG(IFLL_DEBUG, "IFHttpDownloader download %d %d %d\r\n", nFindStart, nFileSplit, nPortPos);

	if (nPortPos != -1)
	{
		nPort = sServer.sub(nPortPos + 1, sServer.size() - nPortPos).toInt32();
		sServer = sServer.sub(0, nPortPos);
	}
    return true;
}




void IFHttpDownloader::procSSLEstablished(IFNetConnection* pConnection, bool bOK)
{
	if (bOK)
	{
		pConnection->sendData(m_HTTPRequest->getBuffer(),m_HTTPRequest->size());
		m_handleRecvData.connectSlot(pConnection->event_RecvData);

	}
	else
	{
		IFLOG(IFLL_ERROR, "http download connect to %s failed! ssl error\r\n", m_sURL.c_str());

		//event_DownloadResult(this, m_sURL, ER_CONNECTTION, m_spOutPutStream, 0);
		m_spOutPutStream = NULL;

	}
}


void IFHttpDownloader::procConnectResult( IFNetConnection* pConnection, bool bSuccess )
{
	IFLOG(IFLL_DEBUG, "IFHttpDownloader::procConnectResult %s\r\n", bSuccess ? "true" : "false");
	if (bSuccess)
	{
		if (m_bHTTPS)
		{
			IFLOG(IFLL_DEBUG, "use https\r\n");

			pConnection->establishSSL(m_ServerAddress);
			m_handleSSLEstablished.connectSlot(pConnection->event_SSLEstablishResult);
		}
		else
		{
			IFLOG(IFLL_DEBUG, "http download connect to %s ok!\r\n", m_sURL.c_str());
			pConnection->sendData(m_HTTPRequest->getBuffer(), m_HTTPRequest->size());
			m_handleRecvData.connectSlot(pConnection->event_RecvData);
			//pConnection->event_RecvData.AddSelfHoldHandle(makeIFFunctor(this, &IFHttpDownloader::procDataReceive)	);
		
		}
		m_handleDisConnect = [=](IFNetConnection* pConnection)
		{
			IFLOG(IFLL_ERROR, "http download error disconnect!\r\n");
			event_DownloadResult(this, m_sURL, ER_CONNECTTION, m_spOutPutStream, 0);
		};
		m_handleDisConnect.connectSlot(pConnection->event_Disconnect);
		
	}
	else
	{
		IFLOG(IFLL_ERROR,"http download connect to %s failed!\r\n", m_sURL.c_str());

		event_DownloadResult(this,m_sURL, ER_CONNECTTION, m_spOutPutStream, 0);
		m_spOutPutStream = NULL;
	}
}

void IFHttpDownloader::procDataReceive( IFNetConnection* pConnection, const void* pData, IFUI32 nLen )
{
	m_nLastReceiveDataTime = IFDateTime::now().toIntTime();
	//IFLOG(IFLL_DEBUG,"http recv:%d bytes\r\n", nLen);

	if (m_State == HS_NOT_CONNECT)
	{
		m_sHTTPHead.append((const char*)pData, nLen);

		int nDataPos = m_sHTTPHead.find("\r\n\r\n");
		if (nDataPos != -1)
		{



			//IFString header((const char*)pData,nLen);

			int HeadLineEndPos = m_sHTTPHead.find("\r\n");
			if (HeadLineEndPos == -1)
				HeadLineEndPos = m_sHTTPHead.size();
			IFString sHeadLine = m_sHTTPHead.sub(0, HeadLineEndPos);
			IFArray<IFString> headinfos;
			USplitStrings(&headinfos, sHeadLine, " ");

			IFString sResponseParams = m_sHTTPHead.sub(HeadLineEndPos + 2, nDataPos);
			IFArray<IFString> responseParams;
			USplitStrings(&responseParams, sResponseParams, "\r\n");
			IFArray<IFString> kv;
			m_ResponseHead.clear();
			for (auto& p: responseParams)
			{
				USplitStrings(&kv, p, ": ");
				if (kv.size() == 2)
				{
					auto exits = m_ResponseHead.find(kv[0]);
					if (exits != m_ResponseHead.end())
						m_ResponseHead.erase(exits);
					m_ResponseHead.insert(makeIFPair(kv[0], kv[1]));
				}
				kv.clear();
			}
			//IFString okHeader11 = "HTTP/1.1 200 OK";
			//IFString okHeader10 = "HTTP/1.0 200 OK";
			int nhttpstate = headinfos.size() >= 2 ? headinfos[1].toInt32() : 0;
			m_httpState = nhttpstate;
			if (nhttpstate >= 200 &&
				nhttpstate < 300)
			{
				
				IFLOG(IFLL_DEBUG, "http is ok!\r\n");

				IFString clfalg = "Content-Length:";
				int clfalgPos = m_sHTTPHead.find(clfalg);
				if (clfalgPos == -1)
					clfalgPos = m_sHTTPHead.find("content-length:");

				if (clfalgPos == -1)
				{
					IFString tcflag = "Transfer-Encoding:";
					clfalgPos = m_sHTTPHead.find(tcflag);
					if (clfalgPos == -1)
					{
						notifyDownloadResult(ER_HTTP_ERROR);
						return;
					}
					else
					{
						m_nContentLength = 0xFFFFFFFFFFFFFFF;
						m_nChunkDataLeft = 0;
						int nDataPos = m_sHTTPHead.find("\r\n\r\n");
						if (nDataPos == -1)
						{
							notifyDownloadResult(ER_HTTP_ERROR);
							return;
						}

						nDataPos += 2;
						int nCurConSize = nLen - nDataPos;

						fillChunkData((const char*)pData + nDataPos, nCurConSize);
						m_State = HS_CONNECTED;

					}

				}
				else
				{
					int nSizeEndPos = m_sHTTPHead.find_first_of('\r', clfalgPos);
					IFString ctlen = m_sHTTPHead.sub(clfalgPos + clfalg.length(), nSizeEndPos - clfalgPos - clfalg.length());
					m_nContentLength = ctlen.toUint64();//_strtoui64(ctlen.c_str(), NULL, 10);
					IFLOG(IFLL_DEBUG, "http content length=%lld\r\n", m_nContentLength);
					int nDataPos = m_sHTTPHead.find("\r\n\r\n");
					if (nDataPos == -1)
					{
						notifyDownloadResult(ER_HTTP_ERROR);
						return;
					}
					nDataPos += 4;
					int nCurConSize = m_sHTTPHead.length() - nDataPos;
					if (nCurConSize)
					{
						fillData((const char*)m_sHTTPHead.c_str() + nDataPos, nCurConSize);
					}

				}
				m_State = HS_CONNECTED;
			}
			else
			{
				IFLOG(IFLL_DEBUG, "http is error%s!\r\n", m_sHTTPHead.c_str());
				/*if (m_nContentLength == 0)
				{
					auto contentlength = m_ResponseHead.find("Content-Length");
					if (contentlength != m_ResponseHead.end())
					{
						m_nContentLength = contentlength->second.toUint64();
					}
				}
				int nDataPos = m_sHTTPHead.find("\r\n\r\n");
				if (nDataPos != -1)
				{
					nDataPos += 4;
					int nCurConSize = m_sHTTPHead.length() - nDataPos;
					fillData((const char*)m_sHTTPHead.c_str() + nDataPos, nCurConSize);
				}*/
				

				
				notifyDownloadResult(ER_HTTP_ERROR);

			}
		}
	}
	else
	{
		//chunk data;
		if (m_nContentLength == 0xFFFFFFFFFFFFFFF)
		{
			fillChunkData((const char*)pData, nLen);

			return;
		}

		fillData(pData, nLen);
	}
}

void IFHttpDownloader::retry()
{
	download(m_sURL, m_spOutPutStream, m_spOutPutStream->size());
}

void IFHttpDownloader::notifyDownloadResult( ERROCODE err)
{
	IFREFHOLDTHIS();
	if (m_spConnection&&m_disconnectAfterDone)
	{
		m_handleRecvData.disconnectAllSlot();
		m_handleDisConnect.disconnectAllSlot();
		m_spConnection->disconnect();
		m_spConnection = NULL;
	}
	m_State = HS_DONE;
	IFRefPtr<IFStream> spStream = m_spOutPutStream;
	m_spOutPutStream = NULL;


	event_DownloadResult(this, m_sURL, err,spStream, m_httpState);


}

void IFHttpDownloader::fillData(const void* pData, IFUI32 nLen)
{
	m_nLastReceiveDataTime = IFDateTime::now().toIntTime();
	if (!m_spOutPutStream)
		return;
	IFUI32 writesize = IFMin(nLen, IFUI32(m_nContentLength-m_nRecvLength));
	m_spOutPutStream->write(pData, writesize);
	m_nRecvLength += writesize;
	//IFLOG(IFLL_DEBUG,"http filldata %lld/%lld\r\n", m_nRecvLength, m_nContentLength);

	IFREFHOLDTHIS();

	event_DownloadProgress(this, m_sURL, m_nRecvLength, m_nContentLength);
	if (m_nRecvLength == m_nContentLength)
	{
		notifyDownloadResult(ER_OK);
	}
}

bool IFHttpDownloader::hasTask()
{
	return !!m_spConnection && m_State!= HS_DONE;
}

void IFHttpDownloader::cancelTask()
{
	notifyDownloadResult(ER_CANCEL);
}



void IFHttpDownloader::fillChunkData(const IFI8* pData, IFUI32 nLen)
{
	const IFI8* pEndData = pData+nLen;
	while (pData!=pEndData)
	{
		if (m_nChunkDataLeft == 0)
		{
			while (pData!=pEndData)
			{
				if (m_ChunkSizeData.size()>2)
				{
					if (*pData=='\n')
					{
						pData++;
						m_nChunkDataLeft = m_ChunkSizeData.sub(2,m_ChunkSizeData.size()-2).toUint32(16);
						m_ChunkSizeData.clear();
						if (m_nChunkDataLeft==0)
						{
							notifyDownloadResult(ER_OK);
							return;
						}
						break;
					}
				}
			
				m_ChunkSizeData.push_back(*pData);
				++pData;
			}
		}
		else
		{
			IFUI32 nWriteSize = IFMin(m_nChunkDataLeft,IFUI32(pEndData-pData));
			m_nRecvLength += nWriteSize;
			m_spOutPutStream->write(pData,nWriteSize);
			pData+= nWriteSize;
			m_nChunkDataLeft -= nWriteSize;
			event_DownloadProgress(this, m_sURL, m_nRecvLength, m_nContentLength);

		}

	}
}

IFRefPtr<IFHttpDownloader> IFHttpDowloadQueue::download(const IFString& surl, IFRefPtr<IFStream> spStream, IFUI64 nByesOffset /*= 0*/)
{
	IFRefPtr<IFHttpDownloader> spDownloader;
	if (m_FreeList.size())
	{
		spDownloader = m_FreeList.back();
		m_FreeList.pop_back();
		spDownloader->event_DownloadResult.removeAllHandle();
	}
	else
	{
		spDownloader = IFNew IFHttpDownloader(m_pNetCore);
	}
	
	m_DownloadingList.push_back(spDownloader);

	m_handleDownloadResult.connectSlot(spDownloader->event_DownloadResult);

	spDownloader->download(surl,spStream,nByesOffset);


	return spDownloader;
}

IFHttpDowloadQueue::IFHttpDowloadQueue(IFNetCore* pNetCore)
{
	m_pNetCore = pNetCore;
	m_handleDownloadResult = makeIFFunctor(this, &IFHttpDowloadQueue::procDownloadResult);
}

IFHttpDowloadQueue::~IFHttpDowloadQueue()
{

}

void IFHttpDowloadQueue::procDownloadResult(IFHttpDownloader* pDownload, const IFString& surl, IFHttpDownloader::ERROCODE bOK, IFStream* pStream, IFUI32 nStatusCode)
{
	if (m_FreeList.find(pDownload) == m_FreeList.end())
		m_FreeList.push_back(pDownload);

	auto it = m_DownloadingList.find(pDownload);
	if (it!=m_DownloadingList.end())
		m_DownloadingList.erase(it);	
}

int IFHttpDowloadQueue::getDownloadingNum()
{
	return m_DownloadingList.size();
}
