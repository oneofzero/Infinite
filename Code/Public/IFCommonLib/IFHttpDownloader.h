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
#ifndef __IF_HTTP_DOWNLOADER_H__
#define __IF_HTTP_DOWNLOADER_H__
#include "IFCommonLib_API.h"
#include "IFRefPtr.h"
#include "IFRefObj.h"
#include "IFEventSlot.h"
#include "IFTimer.h"

class IFNetCore;
class IFStream;
class IFStringW;
class IFNetConnection;

#ifdef IFPLATFORM_WEB
struct emscripten_fetch_t;
#endif


class IFCOMMON_API IFHttpDownloader : public IFRefObj
{
	IF_DECLARERTTI;
public:
	enum ERROCODE
	{
		ER_OK,
		ER_CONNECTTION,
		ER_HTTP_ERROR,
		ER_CANCEL,
	};
public:
	IFEventSlot<void(IFHttpDownloader* pDownload, const IFString& surl, IFUI64 nCur, IFUI64 nTotal)> event_DownloadProgress;
	IFEventSlot<void(IFHttpDownloader* pDownload, const IFString& surl, ERROCODE bOK, IFStream* pStream, IFUI32 nStatusCode)> event_DownloadResult;
public:
	IFHttpDownloader(IFNetCore* pNetCore);
	void setTimeOut(int nMS);

	void setParam(const IFString& name, const IFString& value);

	void download(const IFString& surl, IFRefPtr<IFStream> spStream, IFUI64 nByesOffset = 0);
	void downloadByPost(const IFString& surl, IFRefPtr<IFStream> spStream, IFRefPtr<IFStream> spPostStream);
	bool hasTask();
	void cancelTask();

	void retry();


	IFUI32 getLastReceiveDataTime()
	{
		return m_nLastReceiveDataTime;
	}

	const IFMap<IFString, IFString>& getResponseHead()
	{
		return m_ResponseHead;
	}

	void setDisconnectAfterDone(bool b)
	{
		m_disconnectAfterDone = b;
	}
	bool isDisconnectAfterDone()
	{
		return m_disconnectAfterDone;
	}
private:
#ifdef IFPLATFORM_WEB
	static void FetchDownloadSucceeded(emscripten_fetch_t* fetch);
	static void FetchDownloadFailed(emscripten_fetch_t* fetch);
	static void FetchDownloadProgress(emscripten_fetch_t* fetch);
	emscripten_fetch_t* m_pFetch;
#endif
protected:
	~IFHttpDownloader();
	IFEventHandle<void(IFNetConnection* pConnection, bool bSuccess)> m_handleConnectResult;
	void procConnectResult(IFNetConnection* pConnection, bool bSuccess);

	IFEventHandle<void(IFNetConnection* pConnection)> m_handleDisConnect;
	void procDisconnect(IFNetConnection* pConnection);

	IFEventHandle<void(IFNetConnection* pConnection, const void* pData, IFUI32 nLen)> m_handleRecvData;
	void procDataReceive(IFNetConnection* pConnection, const void* pData, IFUI32 nLen);

	IFEventHandle<void(IFNetConnection* pConnection, bool bOK)> m_handleSSLEstablished;
	void procSSLEstablished(IFNetConnection* pConnection, bool bOK);

	void fillData(const void* pdata, IFUI32 nLen);
	void notifyDownloadResult(ERROCODE err);


	void fillChunkData(const IFI8* pData, IFUI32 nLen);

	static bool parseURL(const IFString& sURL, bool& bHTTPS, IFString& sServer, int& nPort, IFString& sFileName);

#ifndef IFPLATFORM_WEB
	IFRefPtr<IFNetCore> m_spNetCore;
	IFRefPtr<IFNetConnection> m_spConnection;

	IFRefPtr<IFTimer::TimerFunInfo> m_spCheckTimeOut;
	IFRefPtr<IFMemStream> m_HTTPRequest;

#endif
	IFRefPtr<IFStream> m_spOutPutStream;

	int m_nTimeOutMS;

	IFString m_sURL;

	enum CONSTATE
	{
		HS_NOT_CONNECT,
		HS_CONNECTED,
		HS_DONE,
	};
	CONSTATE m_State;
	IFUI64 m_nContentLength;
	IFUI64 m_nRecvLength;
	IFUI32 m_nLastReceiveDataTime;
	IFMap<IFString, IFString> m_Params;
	IFMap<IFString, IFString> m_ResponseHead;

	IFUI32 m_nChunkDataLeft;
	IFString m_ChunkSizeData;
	IFString m_sHTTPHead;
	
	IFString m_ServerAddress;
	int m_ServerPort;
	int m_httpState;
	bool m_bHTTPS;
	bool m_disconnectAfterDone;

};

class IFCOMMON_API IFHttpDowloadQueue : public IFRefObj
{
	IF_DECLARERTTI;
public:
	IFHttpDowloadQueue(IFNetCore* pNetCore);

	IFRefPtr<IFHttpDownloader> download(const IFString& surl, IFRefPtr<IFStream> spStream, IFUI64 nByesOffset = 0);

	int getDownloadingNum();


protected:
	~IFHttpDowloadQueue();

	typedef IFArray<IFRefPtr<IFHttpDownloader>> DownloaderList;
	DownloaderList m_FreeList;
	DownloaderList m_DownloadingList;
	
	IFNetCore* m_pNetCore;
	int m_nTimeOutMs;
	IFEventHandle<void(IFHttpDownloader* pDownload, const IFString& surl, IFHttpDownloader::ERROCODE bOK, IFStream* pStream, IFUI32 nStatusCode)> m_handleDownloadResult;
	void procDownloadResult(IFHttpDownloader* pDownload, const IFString& surl, IFHttpDownloader::ERROCODE bOK, IFStream* pStream, IFUI32 nStatusCode);
};

#endif //__IF_HTTP_DOWNLOADER_H__