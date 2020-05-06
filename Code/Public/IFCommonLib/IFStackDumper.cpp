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
#include "IFStackDumper.h"
#include "IFHashMap.h"
#include "IFUtility.h"

#include <stdlib.h>
#ifdef WIN32
#include "DbgHelp.h"
#elif defined(LINUX) 
#ifndef ANDROID
#include <execinfo.h>
#endif
#include <unistd.h>
#endif
//#pragma comment(lib,"dbghelp.lib")

IFStackDumper::IFStackDumper(void)
	:m_Stack(32)
{
	m_nHash = 0;
}

IFStackDumper::IFStackDumper(const IFStackDumper& dmp)
{
	m_Stack = dmp.m_Stack;
	m_nHash = dmp.m_nHash;
}

IFStackDumper::IFStackDumper(IFStackDumper&& dmp)
	:m_nHash(dmp.m_nHash)
	,m_Stack(static_cast<IFArray<void*>&&>(dmp.m_Stack))
{

}

IFStackDumper::~IFStackDumper(void)
{
}
#ifdef WIN32
typedef BOOL
	IMAGEAPI
	StackWalk64Fun(
	__in DWORD MachineType,
	__in HANDLE hProcess,
	__in HANDLE hThread,
	__inout LPSTACKFRAME64 StackFrame,
	__inout PVOID ContextRecord,
	__in_opt PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
	__in_opt PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
	__in_opt PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
	__in_opt PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
	);

typedef BOOL
	IMAGEAPI
	SymInitializeFun(
	__in HANDLE hProcess,
	__in_opt PCSTR UserSearchPath,
	__in BOOL fInvadeProcess
	);

typedef BOOL IMAGEAPI
	SymFromAddrFun(
	__in HANDLE hProcess,
	__in DWORD64 Address,
	__out_opt PDWORD64 Displacement,
	__inout PSYMBOL_INFO Symbol
	);

typedef BOOL
	IMAGEAPI
	SymGetLineFromAddr64Fun(
	__in HANDLE hProcess,
	__in DWORD64 qwAddr,
	__out PDWORD pdwDisplacement,
	__out PIMAGEHLP_LINE64 Line64
	);

typedef BOOL
	IMAGEAPI
	SymCleanupFun(
	__in HANDLE hProcess
	);

typedef PVOID
	IMAGEAPI
	SymFunctionTableAccess64Fun(
	__in HANDLE hProcess,
	__in DWORD64 AddrBase
	);

typedef DWORD64
	IMAGEAPI
	SymGetModuleBase64Fun(
	__in HANDLE hProcess,
	__in DWORD64 qwAddr
	);


static StackWalk64Fun* pStackWalk64 = NULL;
static SymInitializeFun* pSymInitialize = NULL;
static SymFromAddrFun* pSymFromAddr = NULL;
static SymGetLineFromAddr64Fun* pSymGetLineFromAddr64 = NULL;
static SymCleanupFun* pSymCleanup = NULL;
static SymFunctionTableAccess64Fun* pSymFunctionTableAccess64 = NULL;
static SymGetModuleBase64Fun* pSymGetModuleBase64 = NULL;
static bool binited = false;
static bool binitedfailed = false;
static HMODULE hDbgHelp = NULL;
static HANDLE process = NULL;
IFStackDumper IFStackDumper::Dump()
{
	IFStackDumper dmper;

	if (!binited)
	{

		process = GetCurrentProcess();  // Get current process & thread.   

		if (binitedfailed)
			return dmper;
		binitedfailed = true;
		hDbgHelp = LoadLibrary(L"dbghelp.dll");
		if (!hDbgHelp)
			return dmper;
		pStackWalk64 = (StackWalk64Fun*)GetProcAddress(hDbgHelp, "StackWalk64");
		if (!pStackWalk64)
			return dmper;
		pSymInitialize = (SymInitializeFun*)GetProcAddress(hDbgHelp, "SymInitialize");
		if (!pSymInitialize)
			return dmper;
		pSymFromAddr = (SymFromAddrFun*)GetProcAddress(hDbgHelp, "SymFromAddr");
		if (!pSymFromAddr)
			return dmper;
		pSymGetLineFromAddr64 = (SymGetLineFromAddr64Fun*)GetProcAddress(hDbgHelp, "SymGetLineFromAddr64");
		if (!pSymGetLineFromAddr64)
			return dmper;
		pSymCleanup = (SymCleanupFun*)GetProcAddress(hDbgHelp, "SymCleanup");
		if (!pSymCleanup)
			return dmper;

		pSymFunctionTableAccess64 = (SymFunctionTableAccess64Fun*)GetProcAddress(hDbgHelp, "SymFunctionTableAccess64");
		if (!pSymFunctionTableAccess64)
			return dmper;

		pSymGetModuleBase64 = (SymGetModuleBase64Fun*)GetProcAddress(hDbgHelp, "SymGetModuleBase64");
		if (!pSymGetModuleBase64)
			return dmper;



		binited = true;
		binitedfailed = false;

	}

	PVOID address[256];
	DWORD hash;
	int ncount = CaptureStackBackTrace(1, 256,address, &hash);
	

	dmper.m_Stack.resize(ncount);
	for (int i = 0; i < ncount; i++)
	{
		dmper.m_Stack[i] = address[i];
	}
	dmper.m_nHash = hash;
#ifdef USE_STACKWALKER


	CONTEXT context;            // Store register addresses.   
	STACKFRAME64 stackframe;        // Call stack.   
	HANDLE  thread;         // Handle to current process & thread.   
	// Generally it can be subsitituted with 0xFFFFFFFF & 0xFFFFFFFE.   
	

	// Initialize STACKFRAME64 structure.   
	RtlCaptureContext(&context);            // Get context.   
	memset(&stackframe, 0, sizeof(STACKFRAME64));  
#ifdef _M_IX86
	stackframe.AddrPC.Offset = context.Eip;     // Fill in register addresses (EIP, ESP, EBP).   
	stackframe.AddrPC.Mode = AddrModeFlat;  
	stackframe.AddrStack.Offset = context.Esp;  
	stackframe.AddrStack.Mode = AddrModeFlat;  
	stackframe.AddrFrame.Offset = context.Ebp;  
	stackframe.AddrFrame.Mode = AddrModeFlat;  
	DWORD machine = IMAGE_FILE_MACHINE_I386;
#else
	stackframe.AddrPC.Offset = context.Rip;     // Fill in register addresses (EIP, ESP, EBP).   
	stackframe.AddrPC.Mode = AddrModeFlat;  
	stackframe.AddrStack.Offset = context.Rsp;  
	stackframe.AddrStack.Mode = AddrModeFlat;  
	stackframe.AddrFrame.Offset = context.Rsp;
	stackframe.AddrFrame.Mode = AddrModeFlat;  
	DWORD machine = IMAGE_FILE_MACHINE_AMD64;


#endif
	thread = GetCurrentThread();  

	const UINT max_name_length = 256;   // Max length of symbols' name.   

	PSYMBOL_INFO symbol;            // Debugging symbol's information.   
	IMAGEHLP_LINE64 source_info;        // Source information (file name & line number)   
	DWORD displacement;         // Source line displacement.   

								// Initialize PSYMBOL_INFO structure.   
								// Allocate a properly-sized block.   
	//symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
	//memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
	//symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).   
	//symbol->MaxNameLen = max_name_length;

	//// Initialize IMAGEHLP_LINE64 structure.   
	//memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
	//source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	// Initialize dbghelp library.   
	
	//_putts(__T("Call stack: \n\n"));  

	// Enumerate call stack frame.   
	
	IFString sDmpInfo;
	IFUI32 b = 378551;
	IFUI32 a = 63689;
	while((pStackWalk64)(machine, process, thread, &stackframe,   
		&context, NULL, pSymFunctionTableAccess64, pSymGetModuleBase64, NULL))  
	{  
		if(stackframe.AddrFrame.Offset == 0)    // End reaches.   
			break;  

		dmper.m_Stack.push_back(stackframe.AddrPC.Offset);

		dmper.m_nHash = dmper.m_nHash * a + (stackframe.AddrPC.Offset);
		a = a * b;
		

		 
	}  

#endif

	return dmper;
}
bool bSymboleInited = false;
const UINT max_name_length = 256;   // Max length of symbols' name.   

struct MYSYMBOINFO : SYMBOL_INFO
{
	MYSYMBOINFO()
	{
		ZeroMemory(this, sizeof(*this));
		SizeOfStruct = sizeof(SYMBOL_INFO);
		MaxNameLen = max_name_length;
	}
	char buf[max_name_length];
};



IFString IFStackDumper::toString() const
{
	static IFHashMap<void*, IFString> StackString(1024*16);
	MYSYMBOINFO m;
	MYSYMBOINFO* symbol = &m;            // Debugging symbol's information.   
	IMAGEHLP_LINE64 source_info;        // Source information (file name & line number)   
	DWORD displacement;         // Source line displacement.   

								// Initialize PSYMBOL_INFO structure.   
								// Allocate a properly-sized block.   
	

	if (!bSymboleInited)
	{
		if (!(pSymInitialize)(process, NULL, TRUE))
			return IFString::Empty;
		bSymboleInited = true;
	}
	//symbol = (PSYMBOL_INFO)malloc(sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
	//memset(symbol, 0, sizeof(SYMBOL_INFO) + (max_name_length - 1) * sizeof(TCHAR));
	//symbol->SizeOfStruct = sizeof(SYMBOL_INFO);  // SizeOfStruct *MUST BE* set to sizeof(SYMBOL_INFO).   
	//symbol->MaxNameLen = max_name_length;

	// Initialize IMAGEHLP_LINE64 structure.   
	memset(&source_info, 0, sizeof(IMAGEHLP_LINE64));
	source_info.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
	IFString sDmpInfo;
	sDmpInfo.reserve(1024);
	for (auto offset : m_Stack)
	{
		auto it = StackString.find(offset);
		if (it != StackString.end())
		{
			sDmpInfo += it->second;
		}
		else
		{
			IFString sCurInfo;


			if ((pSymGetLineFromAddr64)(process, (IFUI64)offset,
				&displacement, &source_info)) {     // Get source information.   
													//_tprintf(__T("\t[%s:%d] at addr 0x%08LX\n"),   
													//source_info.FileName,   
													//source_info.LineNumber,  
													//stackframe.AddrPC.Offset);  
				sCurInfo += IFString().format("%s(%d):",	source_info.FileName,	source_info.LineNumber			);
			}
			else
			{
				if (GetLastError() == 0x1E7) {       // If err_code == 0x1e7, no symbol was found.   
													 //_tprintf(__T("\tNo debug symbol loaded for this function.\n"));  
				}
			}

			if ((pSymFromAddr)(process, (IFUI64)offset, NULL, symbol))// Get symbol.   
			{

				//_tprintf(__T(" > %s\n"), symbol->Name); 
				sCurInfo += ">";
				sCurInfo += symbol->Name;
				sCurInfo += "\r\n";
			}
			else
			{
				sCurInfo += IFString().format("> UnkownAddress 0x%llx\r\n", offset);
			}

			sDmpInfo += sCurInfo;
			StackString[offset] = sCurInfo;

		}
		
	}



	//(pSymCleanup)(process);    // Clean up and exit.   
	//free(symbol);
	return sDmpInfo;
}


#elif defined(LINUX) 
#if defined(ANDROID)
IFStackDumper IFStackDumper::Dump()
{
	return IFStackDumper();
}
IFString IFStackDumper::toString() const
{
	return IFString::Empty;
}
#else
IFStackDumper IFStackDumper::Dump()
{
	IFStackDumper dmper;
	void* address[256];
	int sz = backtrace(address, 256);
	dmper.m_Stack.resize(sz);
	IFUI32 b = 378551;
	IFUI32 a = 63689;
	for (int i = 0; i < sz; i++)
	{
		dmper.m_Stack[i] = address[i];
		dmper.m_nHash = dmper.m_nHash * a + (IFUI32)(IFUI64)dmper.m_Stack[i];
		a *= b;
	}
	return dmper;
}


void* GetSOBaseAddr(const IFString& basename)
{
	static IFMap<IFString, void*> soMap;
	if (soMap.size() == 0)
	{
		IFString popencmd;
		popencmd.format("/proc/%d/maps", getpid());
		printf("%s\n", popencmd.c_str());
		FILE* fp = fopen(popencmd.c_str(), "r");
		if (fp)
		{
			IFSimpleArray<char> buf;
			buf.resize(1024 * 1024);
			int sz = fread(buf, 1, buf.size(), fp);
			buf[sz] = 0;
			fclose(fp);
			StringList lines;
			USplitStrings(&lines, buf, "\n");
			for (int l = 0; l < lines.size(); l++)
			{
				//printf("read ok!\n");
				//7ff729661000-7ff72996a000 r-xp 00000000 fc:15 32593                      /mnt/webserver/bin/libIFLuaScript.so
				StringList sl;
				USplitStrings(&sl, lines[l].c_str(), " ");
				/*	for (int i = 0; i < sl.size(); i++)
					{
						printf("%d:%s\n", i, sl[i].c_str());
					}*/
				if (sl.size() == 6 && sl[2].toInt64(16) == 0)
				{
					int sonamestart = sl[5].find_last_of('/');
					if (sonamestart != -1)
					{

						IFString soname = sl[5].sub(sonamestart + 1, sl[5].length() - sonamestart - 1);

						StringList addressrange;
						USplitStrings(&addressrange, sl[0].c_str(), "-");
						if (addressrange.size() == 2)
						{
							soMap[soname] = (void*)addressrange[0].toUint64(16);
							//printf("%s:%p\n", soname.c_str(), soMap[soname]);
						}
					}
				}
			}

		}
	}
	auto it = soMap.find(basename);
	if (it != soMap.end())
		return it->second;
	return nullptr;
}

void Addr2Line(const IFString& fn, void* addr, IFString& stack)
{
	IFString cmdline = "addr2line -e ";

	int sp = fn.find_last_of('/');

	IFString soname = fn;
	if (sp != -1)
		soname = fn.sub(sp + 1, fn.length() - sp-1);
	//else
	cmdline += soname;
	cmdline += " ";
	void* pbase = GetSOBaseAddr(soname);
	IFUI64 diff = (IFUI64)addr - (IFUI64)pbase;
	//printf("%s base:%p %p %lld\n", fn.c_str(), pbase ,addr, diff);
	cmdline += IFString().format("%llX", diff) ;
	//printf("%s\n", cmdline.c_str());
	FILE* f = popen(cmdline, "r");
	char buf[256];
	while (int sz = fread(buf, 1, 256, f))
	{
		stack += IFString(buf, sz);
	}
	pclose(f);
}

IFString IFStackDumper::toString() const
{
	//./test(+0x9db) [0x55738ee4a9db]
	//IFString addr2linecmd = "addr2line "
	//void** stackptr = 
	char** stacks = backtrace_symbols(&m_Stack[0], m_Stack.size());
	//IFString lastfn;
	IFString sstack;
	IFString curaddrs;
	for (int i = 0; i < m_Stack.size(); i++)
	{
		printf("stack:%s\n", stacks[i]);
		IFString s = stacks[i];
		int fl = s.find_first_of('(');
		int fr = s.find_first_of(')');
		if (fl == -1 || fr == -1)
			continue;
		IFString fn = s.sub(0, fl);
		int addr = s.find("[0x", fl);
		int addrend = s.find("]", fl);
		if (addr == -1 || addrend ==-1)
			continue;
		addr += 3;
		IFString addrs = s.sub(addr , addrend - addr);


		//if (lastfn != fn)
		//{
		//	if (lastfn.size() > 0)
		Addr2Line(fn, (void*)addrs.toUint64(16), sstack);
		//	lastfn = fn;
		//	curaddrs.clear();
		//}
		//curaddrs += addrs;
		//curaddrs += " ";
	}
	//if (curaddrs.size() > 0)
	//{
	//	Addr2Line(lastfn, curaddrs, sstack);

	//}
	free(stacks);
	return sstack;
}

#endif
#endif