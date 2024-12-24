#include "stdafx.h"
#include "IFMemoryDLLLoader.h"
#include "IFLogSystem.h"

#ifdef IFPLATFORM_WINDOWS
static int GetAlignedSize(int Origin, int Alignment)
{
	return (Origin + Alignment - 1) / Alignment * Alignment;	
}

IFMemoryDLLLoader::IFMemoryDLLLoader()
	:m_pDosHeader(NULL)
	, m_pNTHeader(NULL)
	, m_pSectionHeader(NULL)
	, m_pBase(NULL)
{
}

bool IFMemoryDLLLoader::LoadDLL(const IFString& sName, const char* pDLLData, int dataSize, bool loadOnly)
{
	if (!checkDLL(pDLLData, dataSize))
		return false;
	auto imageSize = calcImageSize();

	m_pBase = (char*)VirtualAlloc(NULL, imageSize,MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if(m_pBase == NULL)
		return false;

	copyToVirtualMemory(pDLLData, m_pBase);
	relocateFunction();
	if (loadOnly)
		return true;

	if (!setImportFunction())
		return false;

	 //修正基地址

	m_pNTHeader->OptionalHeader.ImageBase = (ULONGLONG)m_pBase;

	 unsigned long old;

 

	VirtualProtect(m_pBase, imageSize, PAGE_EXECUTE_READWRITE,&old);

	//接下来要调用一下dll的入口函数，做初始化工作。
	typedef BOOL (__stdcall *ProcDllMain)(HINSTANCE, DWORD, LPVOID );
	//typedef BOOL(APIENTRY *ProcDllMain)(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

	auto pDllMain = (ProcDllMain)( m_pBase + m_pNTHeader->OptionalHeader.AddressOfEntryPoint);

 

	BOOL InitResult = pDllMain((HINSTANCE)m_pBase,DLL_PROCESS_ATTACH,0);

 

	if (!InitResult) //初始化失败
	{

		pDllMain((HINSTANCE)m_pBase, DLL_PROCESS_DETACH, 0);



		VirtualFree(m_pBase, 0, MEM_RELEASE);



		pDllMain = NULL;
		return false;
	}


	return true;
}

void IFMemoryDLLLoader::SetExistModule(const IFString& sName, HMODULE hModule)
{
	m_LoadedModules[sName] = hModule;
}

void* IFMemoryDLLLoader::GetFunction(const IFString& sFunName)
{
	auto it = m_ExportedFunctions.find(sFunName);
	if (it != m_ExportedFunctions.end())
		return it->second;
	return NULL;
}

void IFMemoryDLLLoader::ExportAllFunctionDef(IFStream* pStream)
{
	IFString exports = "EXPORTS\r\n";
	pStream->write(exports.c_str(), exports.length());
	for (auto pr : m_ExportedFunctions)
	{
		pStream->write(pr.first.c_str(), pr.first.size());
		pStream->write("\r\n", 2);
	}
}

bool IFMemoryDLLLoader::checkDLL(const char* pDLLData, int dataSize)
{
	if (dataSize < sizeof(IMAGE_DOS_HEADER)) return false;

	m_pDosHeader = (PIMAGE_DOS_HEADER)pDLLData; // DOS头

	//检查dos头的标记

	if (m_pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false; //0*5A4D : MZ

	//检查长度

	if ((DWORD)dataSize < (m_pDosHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS)))
		return false;

	//取得pe头

	m_pNTHeader = (PIMAGE_NT_HEADERS)(pDLLData + m_pDosHeader->e_lfanew); // PE头

	//检查pe头的合法性

	if (m_pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return false; //0*00004550 : PE00

	if ((m_pNTHeader->FileHeader.Characteristics & (IMAGE_FILE_DLL | IMAGE_FILE_EXECUTABLE_IMAGE)) == 0) //0*2000 : File is a DLL
		return false;

	//if ((m_pNTHeader->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0) //0*0002 : 指出文件可以运行

	//	return false;

	if (m_pNTHeader->FileHeader.SizeOfOptionalHeader != sizeof(IMAGE_OPTIONAL_HEADER))
		return false;

	//取得节表（段表）

	m_pSectionHeader = (PIMAGE_SECTION_HEADER)((char*)m_pNTHeader + sizeof(IMAGE_NT_HEADERS));

	//验证每个节表的空间

	for (int i = 0; i < m_pNTHeader->FileHeader.NumberOfSections; i++)

	{

		if ((m_pSectionHeader[i].PointerToRawData + m_pSectionHeader[i].SizeOfRawData) > (DWORD)dataSize)
			return false;

	}

	return true;
}

int IFMemoryDLLLoader::calcImageSize()
{
	int Size;


	int nAlign = m_pNTHeader->OptionalHeader.SectionAlignment; //段对齐字节数

	// 计算所有头的尺寸。包括dos, coff, pe头和段表的大小

	Size = GetAlignedSize(m_pNTHeader->OptionalHeader.SizeOfHeaders, nAlign);

	// 计算所有节的大小

	for (int i = 0; i < m_pNTHeader->FileHeader.NumberOfSections; ++i)

	{

		//得到该节的大小

		int CodeSize = m_pSectionHeader[i].Misc.VirtualSize;

		int LoadSize = m_pSectionHeader[i].SizeOfRawData;

		int MaxSize = (LoadSize > CodeSize) ? (LoadSize) : (CodeSize);

		int SectionSize = GetAlignedSize(m_pSectionHeader[i].VirtualAddress + MaxSize, nAlign);

		if (Size < SectionSize)

			Size = SectionSize; //Use the Max;

	}

	return Size;
}

void IFMemoryDLLLoader::copyToVirtualMemory(const char* pDLLData, char* pVitualMemory)
{
	// 计算需要复制的PE头+段表字节数

	int HeaderSize = m_pNTHeader->OptionalHeader.SizeOfHeaders;

	int SectionSize = m_pNTHeader->FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);



	int MoveSize = HeaderSize + SectionSize;



	//复制头和段信息

	memmove(pVitualMemory, pDLLData, MoveSize);



	//复制每个节

	for (int i = 0; i < m_pNTHeader->FileHeader.NumberOfSections; ++i)

	{

		if (m_pSectionHeader[i].VirtualAddress == 0 || m_pSectionHeader[i].SizeOfRawData == 0) continue;

		// 定位该节在内存中的位置

		auto pSectionAddress = (pVitualMemory + m_pSectionHeader[i].VirtualAddress);

		// 复制段数据到虚拟内存

		memmove((void*)pSectionAddress,

			(void*)(pDLLData + m_pSectionHeader[i].PointerToRawData),

			m_pSectionHeader[i].SizeOfRawData);

	}

	//修正指针，指向新分配的内存

	//新的dos头

	m_pDosHeader = (PIMAGE_DOS_HEADER)pVitualMemory;

	//新的pe头地址

	m_pNTHeader = (PIMAGE_NT_HEADERS)(pVitualMemory + (m_pDosHeader->e_lfanew));

	//新的节表地址

	m_pSectionHeader = (PIMAGE_SECTION_HEADER)((char*)m_pNTHeader + sizeof(IMAGE_NT_HEADERS));

	return;
}

void IFMemoryDLLLoader::relocateFunction()
{
	/* 重定位表的结构：

// DWORD sectionAddress, DWORD size (包括本节需要重定位的数据)

// 例如 1000节需要修正5个重定位数据的话，重定位表的数据是

// 00 10 00 00 14 00 00 00 xxxx xxxx xxxx xxxx xxxx 0000

// ———– ———– —-

// 给出节的偏移总尺寸=8+6*2 需要修正的地址用于对齐4字节

// 重定位表是若干个相连，如果address 和 size都是0 表示结束

// 需要修正的地址是12位的，高4位是形态字，intel cpu下是3

*/

//假设NewBase是0×600000,而文件中设置的缺省ImageBase是0×400000,则修正偏移量就是0×200000

	auto Delta = (ULONGLONG)(m_pBase - (char*)m_pNTHeader->OptionalHeader.ImageBase);

	//注意重定位表的位置可能和硬盘文件中的偏移地址不同，应该使用加载后的地址

	PIMAGE_BASE_RELOCATION pLoc = (PIMAGE_BASE_RELOCATION)(m_pBase + m_pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);

	while ((pLoc->VirtualAddress + pLoc->SizeOfBlock) != 0) //开始扫描重定位表

	{

		WORD* pLocData = (WORD*)((char*)pLoc + sizeof(IMAGE_BASE_RELOCATION));

		//计算本节需要修正的重定位项（地址）的数目

		int NumberOfReloc = (pLoc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);



		for (int i = 0; i < NumberOfReloc; i++)

		{

			//if ((DWORD)(pLocData[i] & 0xF000) == 0x00003000) //这是一个需要修正的地址
			if ((DWORD)(pLocData[i] & 0xF000) == 0x0000A000) //这是一个需要修正的地址 //x64
			{

				// 举例：

				// pLoc->VirtualAddress = 0×1000;

				// pLocData[i] = 0×313E; 表示本节偏移地址0×13E处需要修正

				// 因此 pAddress = 基地址 + 0×113E

				// 里面的内容是 A1 ( 0c d4 02 10) 汇编代码是： mov eax , [1002d40c]

				// 需要修正1002d40c这个地址

				auto pAddress = (ULONGLONG*)(m_pBase + pLoc->VirtualAddress + (pLocData[i] & 0x0FFF));



				*pAddress += Delta;



			}

		}



		//转移到下一个节进行处理

		pLoc = (PIMAGE_BASE_RELOCATION)((char*)pLoc + pLoc->SizeOfBlock);



	}
	if (m_pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size > 0)
	{
		unsigned long Offset = m_pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
		PIMAGE_EXPORT_DIRECTORY pID = (PIMAGE_EXPORT_DIRECTORY)(m_pBase + Offset);

		auto pNameAddress = (int*)(m_pBase + pID->AddressOfNames);
		auto pFunAddress = (int*)(m_pBase + pID->AddressOfFunctions);
		for (int i = 0; i < pID->NumberOfNames; i++)
		{
			auto name = pNameAddress[i];
			auto pName = (char*)(m_pBase + name);
			auto fun = pFunAddress[i];
			auto pFun = m_pBase + fun;
			m_ExportedFunctions.insert(makeIFPair<IFString, void*>(pName, pFun));

		}
		
	}


}

bool IFMemoryDLLLoader::setImportFunction()
{
	// 引入表实际上是一个 IMAGE_IMPORT_DESCRIPTOR 结构数组，全部是0表示结束

	// 数组定义如下：

	//

	// DWORD OriginalFirstThunk; // 0表示结束，否则指向未绑定的IAT结构数组

	// DWORD TimeDateStamp;

	// DWORD ForwarderChain; // -1 if no forwarders

	// DWORD Name; // 给出DLL的名字

	// DWORD FirstThunk; // 指向IAT结构数组的地址(绑定后，这些IAT里面就是实际的函数地址)

	unsigned long Offset = m_pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

	if (Offset == 0) return TRUE; //No Import Table

	PIMAGE_IMPORT_DESCRIPTOR pID = (PIMAGE_IMPORT_DESCRIPTOR)(m_pBase + Offset);

	while (pID->Characteristics != 0)

	{

		PIMAGE_THUNK_DATA pRealIAT = (PIMAGE_THUNK_DATA)(m_pBase + pID->FirstThunk);



		PIMAGE_THUNK_DATA pOriginalIAT = (PIMAGE_THUNK_DATA)(m_pBase + pID->OriginalFirstThunk);

		////获取DLL的名字

		//char buf[256]; //DLL name;

		////修改,需要将buf清零,否则DLL名称不对

		//memset(buf, 0, sizeof(buf));
		IFString dllName;

		auto pName = (char*)(m_pBase + pID->Name);

		for (int i = 0; i < 256; i++)

		{

			if (pName[i] == 0)break;

			dllName.push_back(pName[i]);
			//buf[i] = pName[i];

		}
		HMODULE hDLL;
		auto existIt = m_LoadedModules.find(dllName);
		if (existIt != m_LoadedModules.end())
			hDLL = existIt->second;
		else
		{
			IFStringW dllNameW = IFStringW(dllName);
			hDLL = GetModuleHandle(dllNameW.c_str());

			if (hDLL == NULL)
			{
				hDLL = LoadLibrary(dllNameW); //有可能依赖的DLL还没有加载,如果没有加载加载后再判断是否加载成功
				if (hDLL == NULL)
				{
					IFLogError("can't load dll:%s\r\n", dllName.c_str());
					return false; //NOT FOUND DLL
				}

			}
		}
		 //获取DLL中每个导出函数的地址，填入IAT

		//每个IAT结构是：

		// union { PBYTE ForwarderString;

		// PDWORD Function;

		// DWORD Ordinal;

		// PIMAGE_IMPORT_BY_NAME AddressOfData;

		// } u1;

		// 长度是一个DWORD ，正好容纳一个地址。

		for (int i = 0; ; i++)
		{

			if (pOriginalIAT[i].u1.Function == 0) break;

			FARPROC lpFunction = NULL;

			if (pOriginalIAT[i].u1.Ordinal & IMAGE_ORDINAL_FLAG) //这里的值给出的是导出序号
			{
				lpFunction = GetProcAddress(hDLL, (LPCSTR)(pOriginalIAT[i].u1.Ordinal & 0x0000FFFF));
			}
			else //按照名字导入
			{

				//获取此IAT项所描述的函数名称

				PIMAGE_IMPORT_BY_NAME pByName = (PIMAGE_IMPORT_BY_NAME)

					(m_pBase + (DWORD)(pOriginalIAT[i].u1.AddressOfData));

				lpFunction = GetProcAddress(hDLL, (char*)pByName->Name);
 				if (!lpFunction)
				{
					IFLogError("can't find function:[%s] in dll:%s\r\n", (char*)pByName->Name, dllName.c_str());
				}

			}

			if (lpFunction != NULL) //找到了！
			{
				pRealIAT[i].u1.Function = (ULONGLONG)lpFunction;
			}
			else
			{
				return false;
			}

		}

		//move to next

		pID = (PIMAGE_IMPORT_DESCRIPTOR)((char*)pID + sizeof(IMAGE_IMPORT_DESCRIPTOR));

	}



	return TRUE;

}
#endif