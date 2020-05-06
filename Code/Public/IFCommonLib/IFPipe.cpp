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
#include "IFPipe.h"
#include "IFPipeCore.h"
#include "IFFIFOStream.h"

IF_DEFINERTTI(IFPipe, IFRefObj)
#ifdef WIN32



IFPipe::IFPipe(bool bServerPipe,IFPipeCore* pCore, bool bPackageMode)
	:m_hPipe(INVALID_HANDLE_VALUE)
	,m_bPackageMode(bPackageMode)
	,m_bServerPipe(bServerPipe)
	, m_FIFO(IFNew IFFIFOStream )
	,m_nNotCompletePackLen(0)
	,m_pCore(pCore)
{

}

IFPipe::~IFPipe()
{

}

void IFPipe::disconnect()
{
	if(m_hPipe != INVALID_HANDLE_VALUE)
	{
		if(m_bServerPipe)
			DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);

		m_hPipe = INVALID_HANDLE_VALUE;

		m_pCore->removeConnection(this);
	}
}

bool IFPipe::sendData( const void* pData, IFUI32 nLen )
{

	if(m_hPipe == INVALID_HANDLE_VALUE )
		return false;


	int nRemainLen = m_bPackageMode?nLen+2:nLen;

	const char* pSendData = (const char*)pData;

	bool bFirstPack = true && m_bPackageMode;

	while(nRemainLen)
	{
		IFPipeIOData* pIoData = IFNew IFPipeIOData(IFPipeIOData::WRITE, this);


		int nSendLen = min(nRemainLen, IFPipeIOData::BUFSIZE);
		if(bFirstPack)
		{
			(*(IFUI16*)pIoData->m_Buf) = nLen + 2;
			memcpy(pIoData->m_Buf + 2, pSendData, nSendLen-2);
			pIoData->m_DataLen = nSendLen;
			bFirstPack = false;

			nRemainLen -= nSendLen;
			pSendData += nSendLen-2;
		}
		else
		{
			memcpy(pIoData->m_Buf, pSendData, nSendLen);
			pIoData->m_DataLen = nSendLen;

			nRemainLen -= nSendLen;
			pSendData += nSendLen;
		}





		DWORD dwSendSize=0;
		if( !WriteFile(m_hPipe, pIoData->m_Buf, pIoData->m_DataLen, &dwSendSize,pIoData ))
		{
			if( GetLastError() != ERROR_IO_PENDING )
			{
				printf("send error!\r\n");
				//m_pNetCore->FreeIOData(pIoData);
				//delete pIoData;
				delete pIoData;
				disconnect();
				return false;
			};
		}

	}

	return true;

}

bool IFPipe::sendString( const IFString& s )
{
	return sendData(s.c_str(), s.length()+1);

}

bool IFPipe::sendString( const IFStringW& ws )
{
	return sendData(ws.c_str(), ws.length()*2+2);

}

bool IFPipe::sendPack( IFMemStream* pStream )
{
	return sendData(pStream->getBuffer(),(IFUI32) pStream->size() );

}

void IFPipe::recvData( IFPipeIOData* pData, int nLen )
{
	if(m_bPackageMode )
	{
		m_FIFO->write(pData->m_Buf,nLen);

		char buf[64*1024];
		if(m_nNotCompletePackLen)
		{

			if(m_FIFO->read(buf, m_nNotCompletePackLen-2))
			{
				event_RecvData(this,buf, m_nNotCompletePackLen-2 );

				m_nNotCompletePackLen = 0;
			}



		}

		while(m_FIFO->read(&m_nNotCompletePackLen, 2 ))
		{
			if(m_FIFO->read(buf, m_nNotCompletePackLen-2))
			{
				event_RecvData(this,buf, m_nNotCompletePackLen-2 );

				m_nNotCompletePackLen = 0;
			}
			else
			{
				break;
			}
		}

	}
	else
		event_RecvData(this, pData->m_Buf,nLen);
}

bool IFPipe::requestRead()
{
	if(m_hPipe==INVALID_HANDLE_VALUE)
		return false;
	IFPipeIOData* pPipeData = IFNew IFPipeIOData(IFPipeIOData::READ, this);
	DWORD dwReadSize = 0;
	if(!ReadFile(m_hPipe, &pPipeData->m_Buf, IFPipeIOData::BUFSIZE, &dwReadSize,pPipeData))
	{
		DWORD dwLastError = GetLastError();
		if(dwLastError != ERROR_IO_PENDING)
		{
			delete pPipeData;
			return false;
		}
	}
	//else
	//{
	//	PostQueuedCompletionStatus(m_pCore->m_hCompletionPort, dwReadSize, (ULONG_PTR)m_hPipe, pPipeData);
	//}

	return true;
}
#else
IFPipe::IFPipe(bool bServerPipe,IFPipeCore* pCore, bool bPackageMode)
	:m_bPackageMode(bPackageMode)
	,m_bServerPipe(bServerPipe)
	, m_FIFO(IFNew IFFIFOStream )
	,m_nNotCompletePackLen(0)
	,m_pCore(pCore)
{

}

IFPipe::~IFPipe()
{

}

void IFPipe::disconnect()
{

}

bool IFPipe::sendData( const void* pData, IFUI32 nLen )
{



	return true;

}

bool IFPipe::sendString( const IFString& s )
{
	return sendData(s.c_str(), s.length()+1);

}

bool IFPipe::sendString( const IFStringW& ws )
{
	return sendData(ws.c_str(), ws.length()*2+2);

}

bool IFPipe::sendPack( IFMemStream* pStream )
{
	return sendData(pStream->getBuffer(),(IFUI32) pStream->size() );

}

void IFPipe::recvData( IFPipeIOData* pData, int nLen )
{
	if(m_bPackageMode )
	{
		m_FIFO->write(pData->m_Buf,nLen);

		char buf[64*1024];
		if(m_nNotCompletePackLen)
		{

			if(m_FIFO->read(buf, m_nNotCompletePackLen-2))
			{
				event_RecvData(this,buf, m_nNotCompletePackLen-2 );

				m_nNotCompletePackLen = 0;
			}



		}

		while(m_FIFO->read(&m_nNotCompletePackLen, 2 ))
		{
			if(m_FIFO->read(buf, m_nNotCompletePackLen-2))
			{
				event_RecvData(this,buf, m_nNotCompletePackLen-2 );

				m_nNotCompletePackLen = 0;
			}
			else
			{
				break;
			}
		}

	}
	else
		event_RecvData(this, pData->m_Buf,nLen);
}

bool IFPipe::requestRead()
{

	return true;
}
#endif