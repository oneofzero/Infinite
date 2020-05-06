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
#include "Windows.h"
#include "IFCommonLib.h"
#include <wchar.h>
#include "CrashDump.h"
#include <tchar.h>
#include <time.h>
#include <stdio.h>
//#include "WheatyExceptionReport.h"
#include <Windows.h>
#include <DbgHelp.h>

#pragma comment(lib,"dbghelp.lib")

static long WINAPI HandleCrash(PEXCEPTION_POINTERS pExceptPtrs);
static DWORD WINAPI WriteCrashDumpThread(LPVOID pParam);


// 禁用SetUnHandledExceptionFilter,让crt不能重新修改Filter,达到在crt下崩溃，也可以得到dmp的目的

#if defined _M_X64 || defined _M_IX86
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI 
	MyDummySetUnhandledExceptionFilter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}
#else
#error "This code works only for x86 and x64!"
#endif

BOOL DisableSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(_T("kernel32.dll"));
	if (hKernel32 == NULL) return FALSE;
	void *pOrgEntry = GetProcAddress(hKernel32, 
		"SetUnhandledExceptionFilter");
	if(pOrgEntry == NULL) return FALSE;

	DWORD dwOldProtect = 0;
	SIZE_T jmpSize = 5;
#ifdef _M_X64
	jmpSize = 13;
#endif
	BOOL bProt = VirtualProtect(pOrgEntry, jmpSize, 
		PAGE_EXECUTE_READWRITE, &dwOldProtect);
	BYTE newJump[20];
	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
#ifdef _M_IX86
	DWORD dwOrgEntryAddr = (DWORD) pOrgEntry;
	dwOrgEntryAddr += jmpSize; // add 5 for 5 op-codes for jmp rel32
	DWORD dwNewEntryAddr = (DWORD) pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
	// JMP rel32: Jump near, relative, displacement relative to next instruction.
	newJump[0] = 0xE9;  // JMP rel32
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
#elif _M_X64
	// We must use R10 or R11, because these are "scratch" registers 
	// which need not to be preserved accross function calls
	// For more info see: Register Usage for x64 64-Bit
	// http://msdn.microsoft.com/en-us/library/ms794547.aspx
	// Thanks to Matthew Smith!!!
	newJump[0] = 0x49;  // MOV R11, ...
	newJump[1] = 0xBB;  // ...
	memcpy(&newJump[2], &pNewFunc, sizeof (pNewFunc));
	//pCur += sizeof (ULONG_PTR);
	newJump[10] = 0x41;  // JMP R11, ...
	newJump[11] = 0xFF;  // ...
	newJump[12] = 0xE3;  // ...
#endif
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),
		pOrgEntry, newJump, jmpSize, &bytesWritten);

	if (bProt != FALSE)
	{
		DWORD dwBuf;
		VirtualProtect(pOrgEntry, jmpSize, dwOldProtect, &dwBuf);
	}
	return bProt;
}

static CrashDump::DumpType m_DT;
BOOL CrashDump::Install(DumpType dt)
{
	m_DT = dt;
	::SetUnhandledExceptionFilter(HandleCrash);
#ifdef NDEBUG
	return DisableSetUnhandledExceptionFilter();
#endif
	return TRUE;
}




DWORD WINAPI WriteCrashDumpThread(LPVOID pParam)
{
	//MessageBox(NULL,L"Crash!",L"Crash!", MB_OK);
	PEXCEPTION_POINTERS pExceptPtrs = (PEXCEPTION_POINTERS)pParam;
	//IFStringW sFileName;// = *(IFNew IFStringW);
	WCHAR buf[512];
	GetModuleFileName(NULL, buf, 512);

	SYSTEMTIME st;
	GetSystemTime(&st);

	WCHAR sFileName[512];
	wsprintf(sFileName,L"%s.%d(%d).%04d%02d%02d%02d%02d%02d.dmp", buf, GetCurrentProcessId(), GetCurrentThreadId(),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

//#ifdef _DEBUG
	MINIDUMP_TYPE nDumpType = m_DT == CrashDump::DT_STACK?MiniDumpNormal:((MINIDUMP_TYPE)(MiniDumpWithDataSegs| MiniDumpWithFullMemory | MiniDumpWithIndirectlyReferencedMemory |MiniDumpWithFullMemoryInfo));
//#else
//	MINIDUMP_TYPE nDumpType = (MINIDUMP_TYPE)(MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory ); //MiniDumpWithFullMemory;
//#endif

	HANDLE hDump = CreateFile(sFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);


	if(hDump == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, L"Could not open crash dump file.", L"Crash dump error.", MB_OK);
	}
	else
	{
		MINIDUMP_EXCEPTION_INFORMATION info;
		info.ClientPointers = FALSE;
		info.ExceptionPointers = pExceptPtrs;
		info.ThreadId = GetCurrentThreadId();

		// 这个地方的dmp信息，比较少。还要配置一个比较全的版本
		BOOL bRet = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hDump, nDumpType , &info, 0, 0);

		if (!bRet)
		{
			TCHAR szBuff[MAX_PATH+1];
			_sntprintf_s(szBuff, _TRUNCATE, L"无法生成Dump文件，错误号为%u.", GetLastError());
			MessageBox(0, L"无法生成Dump文件", szBuff, MB_OK);
		}
		CloseHandle(hDump);
	}
	MessageBox(NULL, L"程序发生了未知错误，即将退出。", L"ERROR!", MB_OK);
	return 0;
}

long WINAPI HandleCrash( PEXCEPTION_POINTERS pExceptPtrs )
{
	
	DWORD dwThreadID = 0;
	HANDLE hThread = CreateThread(NULL, 0, WriteCrashDumpThread, pExceptPtrs, 0, &dwThreadID );
	WaitForSingleObject(hThread, INFINITE );

	return EXCEPTION_EXECUTE_HANDLER;
}