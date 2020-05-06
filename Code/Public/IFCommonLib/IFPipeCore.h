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
#include "IFRefObj.h"
#include "IFEventSlot.h"
#include "IFString.h"
class IFThread;

class IFPipe;

class IFCOMMON_API IFPipeCore : public IFRefObj
{
	IF_DECLARERTTI;
public:

	IFEventSlot<void(IFPipeCore*, IFPipe* )> event_NewConnection;
public:
	IFPipeCore();

protected:
	~IFPipeCore();


public:
	bool startServer(const IFString& sPipeName, int nMaxConnection=1, bool bPackageMode = true);
	void stopServer();

	IFPipe* createClientPipe(const IFString& sPipeName, bool bPackageMode = true );

	void process();

private:
	void pipeListen();
	//static DWORD WINAPI ListenThread(LPVOID pParam);

	void removeConnection( IFPipe* pConnection );

	IFString m_sServerPipeName;

	int m_nMaxClientNum;
#ifdef WIN32
	HANDLE m_hCompletionPort;
#endif
	typedef IFList<IFRefPtr<IFPipe> > ConnectionList;
	
	IFCSLock m_ConnectionListLock;
	ConnectionList m_Connections;

	IFCSLock m_NewConnectionsListLock;
	ConnectionList m_NewConnectionsList;

	IFRefPtr<IFThread> m_spListenThread;
	//HANDLE m_hListenThread;

	//bool m_bListenThreadRunning;
#ifdef WIN32
	HANDLE _ServerPipeHandle;
	OVERLAPPED m_ListenOverLapped;
#endif
	bool m_bPackageMode;

	bool m_bExitListenThread;


	IFI64 m_nRecvPackage;
	IFI64 m_nRecvBytes;
	IFI64 m_nSendPackage;
	IFI64 m_nSendBytes;

	friend class IFPipe;
};