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
#include "IFPipeCore.h"
#include "IFPipe.h"
#include "IFThread.h"

IF_DEFINERTTI(IFPipeCore, IFRefObj)

IFPipeCore::IFPipeCore()
:m_nMaxClientNum(0)
,m_bPackageMode(true)
,m_bExitListenThread(false)
#ifdef WIN32
,m_hCompletionPort(INVALID_HANDLE_VALUE)
#endif

{
#ifdef WIN32
	m_hCompletionPort= CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, NULL, NULL );
#endif
}

IFPipeCore::~IFPipeCore()
{
#ifdef WIN32
	CloseHandle(m_hCompletionPort);
#endif
}

bool IFPipeCore::startServer( const IFString& sPipeName, int nMaxConnection/*=1*/, bool bPackageMode  )
{
	m_sServerPipeName = "\\\\.\\pipe\\";
	m_sServerPipeName += sPipeName;

	m_nMaxClientNum = nMaxConnection;

	m_bPackageMode = bPackageMode;
#ifdef WIN32
	ZeroMemory(&m_ListenOverLapped, sizeof(m_ListenOverLapped));
	m_ListenOverLapped.hEvent = CreateEvent(0,0,0,0);
#endif
	m_spListenThread = IFNew IFThread;
	m_spListenThread->start(makeIFFunctor(this, &IFPipeCore::pipeListen));
	//DWORD dwThread;

	//if(m_hListenThread!=INVALID_HANDLE_VALUE)
	//{
	//	CloseHandle(m_hListenThread);
	//}

	//m_hListenThread = CreateThread(0,0, ListenThread, this, 0, &dwThread );
	IFThread::sleep(100);
	

	return true;
}


void IFPipeCore::stopServer()
{
	while (m_Connections.size())
	{

		removeConnection(m_Connections.front());
	}
	if (m_spListenThread)
	{
		m_bExitListenThread = true;
#ifdef WIN32
		ResetEvent(m_ListenOverLapped.hEvent);
#endif
		m_spListenThread->waitExit();
#ifdef WIN32
		CloseHandle(m_ListenOverLapped.hEvent);
#endif
	}
}

IFPipe* IFPipeCore::createClientPipe( const IFString& sPipeName,bool bPackageMode  )
{
#ifdef WIN32

	IFString sPPName = "\\\\.\\pipe\\";
	sPPName+=sPipeName;

	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int nTryNum = 10;
	while (true)
	{
		hPipe = CreateFile( 
			IFStringW(sPPName).c_str(),   // pipe name 
			GENERIC_READ |  // read and write access 
			GENERIC_WRITE, 
			0,              // no sharing 
			NULL,           // default security attributes
			OPEN_EXISTING,  // opens existing pipe 
			FILE_FLAG_OVERLAPPED,              // default attributes 
			NULL);          // no template file 

		// Break if the pipe handle is valid. 

		if (hPipe != INVALID_HANDLE_VALUE) 
			break;

		// Exit if an error other than ERROR_PIPE_BUSY occurs. 

		if (GetLastError() != ERROR_PIPE_BUSY) 
		{
			break;
		}

		if (!WaitNamedPipe(IFStringW(sPPName).c_str(), 500)) 
		{
			return NULL;
		}
		
		nTryNum -- ;
		if(nTryNum == 0 )
			break;


	}
	if(hPipe == INVALID_HANDLE_VALUE)
		return NULL;

	CreateIoCompletionPort(hPipe, m_hCompletionPort, (ULONG_PTR)hPipe, 0);
	IFRefPtr<IFPipe> spPipe = IFNew IFPipe(false,this,bPackageMode);
	spPipe->m_hPipe = hPipe;
	spPipe->m_sPipeName = sPPName;
	m_Connections.push_back(spPipe);
	

	if(!spPipe->requestRead())
	{
		m_Connections.erase(m_Connections.rbegin().toNormal());

	}
	return spPipe;
#else
	return NULL;
#endif
}

void IFPipeCore::pipeListen()
{
#ifdef WIN32

	while (1)
	{
		while(m_Connections.size() + m_NewConnectionsList.size() >=  m_nMaxClientNum)
		{

			IFThread::sleep(10);
		}

		HANDLE hPipe = CreateNamedPipe( 
			IFStringW(m_sServerPipeName).c_str(),             // pipe name 
			PIPE_ACCESS_DUPLEX|FILE_FLAG_OVERLAPPED,       // read/write access 
			PIPE_TYPE_MESSAGE |       // message type pipe 
			PIPE_READMODE_MESSAGE |   // message-read mode 
			PIPE_WAIT,                // blocking mode 
			PIPE_UNLIMITED_INSTANCES, // max. instances  
			IFPipeIOData::BUFSIZE,                  // output buffer size 
			IFPipeIOData::BUFSIZE,                  // input buffer size 
			10,                        // client time-out 
			NULL);                    // default security attribute 

		if (hPipe == INVALID_HANDLE_VALUE) 
		{
			//printf("CreatePipe failed"); 
			return;
		}

		// Wait for the client to connect; if it succeeds, 
		// the function returns a nonzero value. If the function
		// returns zero, GetLastError returns ERROR_PIPE_CONNECTED. 
		BOOL bConnected = false;
		bConnected = ConnectNamedPipe(hPipe, &m_ListenOverLapped) ?TRUE : (GetLastError() == ERROR_PIPE_CONNECTED); 

		do 
		{

			if (!bConnected)
			{
				DWORD byteTransed = 0;
				bConnected = GetOverlappedResult( 
					hPipe,     // pipe handle 
					&m_ListenOverLapped, // OVERLAPPED structure 
					&byteTransed,    // bytes transferred 
					FALSE);    // does not wait 
			}



			if (bConnected) 
			{ 
				CreateIoCompletionPort(hPipe, m_hCompletionPort, (ULONG_PTR)hPipe, 0);

				IFRefPtr<IFPipe> spPipe = IFNew IFPipe(true, this, m_bPackageMode);
				spPipe->m_hPipe = hPipe;

				IFCSLockHelper lh(m_NewConnectionsListLock);
				m_NewConnectionsList.push_back(spPipe);

			}
			if (m_bExitListenThread)
				return;
			else
				Sleep(10);

		} while (!bConnected);
	

	}
#endif
}

void IFPipeCore::process()
{
#ifdef WIN32
	{
		IFCSLockHelper lh(m_NewConnectionsListLock);
		while(m_NewConnectionsList.size())
		{
			IFPipe* pPipe = *m_NewConnectionsList.begin();


			if(!pPipe->requestRead())
			{
				m_NewConnectionsList.erase(m_NewConnectionsList.begin());
				continue;
			}

			m_Connections.push_back( pPipe);
			event_NewConnection(this,pPipe );


			m_NewConnectionsList.erase(m_NewConnectionsList.begin());

		}

	}

	{
		//int hr;
		BOOL bIOFailed;
		DWORD dwWaitTIme = 10;//m_bMultiThread?INFINITE:0;
		int nMaxProcessCount = 1000;
		DWORD	dwTransBytesCount;
		HANDLE hPipe;
		IFPipeIOData* pIoData = NULL;
		while(nMaxProcessCount)
		{

			bIOFailed = GetQueuedCompletionStatus( m_hCompletionPort,&dwTransBytesCount,(PULONG_PTR) &hPipe, (LPOVERLAPPED*)&pIoData, dwWaitTIme )==0 || pIoData == 0;

			if( bIOFailed || dwTransBytesCount == 0 )
			{

				DWORD dwLastError = GetLastError();
				if(dwLastError == WAIT_TIMEOUT)
				{
					break;
				}

				pIoData->m_spPipe->disconnect();
				//delete pIoData;
				delete (pIoData);

			}
			else
			{

				if ( pIoData->m_OP == IFPipeIOData::READ )
				{
					m_nRecvPackage ++;
					m_nRecvBytes += dwTransBytesCount;
					IFRefPtr<IFPipe> pConnection = pIoData->m_spPipe;
					pIoData->m_DataLen = dwTransBytesCount;
					pConnection->recvData(pIoData, (int)dwTransBytesCount);

					if(pConnection->m_hPipe!=INVALID_HANDLE_VALUE)
					{
						if(!pConnection->requestRead())
						{
							pConnection->disconnect();
						}

					}
					else
					{
						pConnection->disconnect();						//delete pIoData;

					}

					delete (pIoData);

				}
				else
				{
					m_nSendPackage ++;
					m_nSendBytes += dwTransBytesCount;

					delete pIoData;
					//FreeIOData(pIoData);


				}
			}

			nMaxProcessCount --;

		}
		if(nMaxProcessCount == 0 )
		{
			//printf("全负荷!!!\r\n");
		}

	}
#endif
}



void IFPipeCore::removeConnection( IFPipe* pConnection )
{
	{
		IFCSLockHelper lh(m_ConnectionListLock);
		ConnectionList::iterator it = m_Connections.find(pConnection);
		if(it!=m_Connections.end())
		{
			pConnection->disconnect();
			m_Connections.erase(it);
			return;
		}
	}
	{
		IFCSLockHelper lh(m_NewConnectionsListLock);
		ConnectionList::iterator it = m_NewConnectionsList.find(pConnection);
		if(it!=m_Connections.end())
		{
			pConnection->disconnect();
			m_NewConnectionsList.erase(it);
			return;
		}
	}
}