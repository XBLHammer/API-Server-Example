#include "../stdafx.h"
#include <fstream>

#ifdef _DEBUG
#include <xbdm.h>
#endif

#pragma warning(disable:4101) // unreferenced variable warning(certain variables are only needed when compiling for XDK

void hooks::setupDetour(int addr, int dest) {
	address = (int*)addr;
	destination = dest;
	opcodes = (byte*)malloc(16);

	memcpy(opcodes, (byte*)addr, 16);

	for (int i = 0; i < 8; i++)
		stub[i] = 0x60000000;

	stub[7] = 0x4E800020;
}

void hooks::patchFunction() {
	address[0] = 0x3D600000 + (((destination >> 16) & 0xFFFF) + ((destination & 0x8000) != 0));
	address[1] = 0x396B0000 + (destination & 0xFFFF);
	address[2] = 0x7D6903A6;
	address[3] = 0x4E800420;
}

void hooks::restoreDetour() {
	memcpy((byte*)address, opcodes, 16);
	free(opcodes);
}

void hooks::detourFunctionStart() {
	int addrReloc = (int)(&stub[4]);

	stub[0] = 0x3D600000 + (((addrReloc >> 16) & 0xFFFF) + (addrReloc & 0x8000) ? 1 : 0);
	stub[1] = 0x396B0000 + (addrReloc & 0xFFFF);
	stub[2] = 0x7D6903A6;
}

namespace xbox {
	namespace utilities {
		// Resolve set memory
		pDmSetMemory DevSetMemory = NULL;

		#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
		DWORD ResolveFunction(PCHAR ModuleName, DWORD Ordinal) {
			return (DWORD)(GetProcAddress(GetModuleHandle(ModuleName), MAKEINTRESOURCEA(Ordinal)));
		}

		typedef void (*XNotifyQueueUI1)(XNOTIFYQUEUEUI_TYPE exnq, DWORD dwUserIndex, ULONGLONG qwAreas, PWCHAR displayText, PVOID contextData);
		XNotifyQueueUI1 XNotifyD = (XNotifyQueueUI1)ResolveFunction("xam.xex", 656);


		void patch_BLOCK_LIVE(BOOL enable) {
			
		}

		string GetModuleNameFromAddress(DWORD dwAddress) {
			auto ldr = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(GetModuleHandle("xboxkrnl.exe"));
			auto CurrentEntry = ldr->InLoadOrderLinks.Flink;
			PLDR_DATA_TABLE_ENTRY Current = nullptr;

			char buffer[100];
			while (CurrentEntry != &ldr->InLoadOrderLinks && CurrentEntry != nullptr) {
				Current = CONTAINING_RECORD(CurrentEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
				if (!Current)
					break;

				if (dwAddress >= reinterpret_cast<DWORD>(Current->ImageBase)) {
					if (dwAddress <= reinterpret_cast<DWORD>(Current->ImageBase) + Current->SizeOfFullImage) {
						wcstombs(buffer, Current->BaseDllName.Buffer, sizeof(buffer));
						return std::string(buffer);
					}
				}
				CurrentEntry = CurrentEntry->Flink;
			}
			return nullptr;
		}

		DWORD MountPath(PCHAR Drive, PCHAR Device)
		{
			CHAR Destination[MAX_PATH] = { 0 };
			sprintf_s(Destination, MAX_PATH, (KeGetCurrentProcessType() == PROC_SYSTEM) ? OBJ_SYS_STRING : OBJ_USR_STRING, Drive);
			ANSI_STRING LinkName, DeviceName;
			RtlInitAnsiString(&LinkName, Destination);
			RtlInitAnsiString(&DeviceName, Device);
			ObDeleteSymbolicLink(&LinkName);
			return (DWORD)ObCreateSymbolicLink(&LinkName, &DeviceName);
		}

		DWORD applyPatches(VOID* patches)
		{
			DWORD patchCount = 0;
			MemoryBuffer mbPatches;
			DWORD* patchData = (DWORD*)patches;

			if (patchData == NULL)
				return 0;

			while (*patchData != 0xFFFFFFFF) {
				BOOL inHvMode = (patchData[0] < 0x40000);
				QWORD patchAddr = inHvMode ? (0x200000000 * (patchData[0] / 0x10000)) + patchData[0] : (QWORD)patchData[0];

				xbox::utilities::setMemory((VOID*)patchData[0], &patchData[2], patchData[1] * sizeof(DWORD));
				patchData += (patchData[1] + 2);
				patchCount++;
			}

			return patchCount;
		}

		PWCHAR charToWChar(__in LPCSTR c_buffer) {
			int wchars_num = MultiByteToWideChar(CP_ACP, 0, c_buffer, -1, NULL, 0);
			PWCHAR c_wbuffer = new WCHAR[wchars_num];
			MultiByteToWideChar(CP_ACP, 0, c_buffer, -1, (LPWSTR)c_wbuffer, wchars_num);
			return c_wbuffer;
		}

		int toWCHAR(char* input, WCHAR* output) {
			if (!input || !output) return 0;
			int len = strlen(input);

			memset(output, 0, (len * 2) + 2); //convert to wide string because xdk functions don't fucking work
			for (int i = 1, b = 0; b < len; i += 2) {
				((char*)output)[i] = input[b];
				b++;
			}
			return len;
		}

		std::string utf8_encode(const std::wstring& wstr)
		{
			if (wstr.empty()) return std::string();
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
			std::string strTo(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
			return strTo;
		}

		std::wstring utf8_decode(const std::string& str)
		{
			if (str.empty()) return std::wstring();
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
			return wstrTo;
		}

		void DbgOut(const char* text, ...)
		{
			char dest[0x512]; // WARNING: SYSTEM CRASH IF EXCEEDS STACK SIZE
			va_list args;
			va_start(args, text);
			vsprintf(dest, text, args);
			va_end(args);
			printf(dest);

	
			std::ofstream log_file("Client:\\Client.log", std::ios_base::app);
			if (log_file.is_open())
				log_file << dest;
			else
				return;
		}

		std::string va(char* format, ...) 
		{
			char charBuffer[0x200];
			va_list arglist;
			va_start(arglist, format);
			vsprintf(charBuffer, format, arglist);
			va_end(arglist);
			return std::string(charBuffer);
		}

		HRESULT setLiveBlock(BOOL enable) 
		{
			if (global::isDevkit) {
				xbox::utilities::patch_BLOCK_LIVE(enable);
			}
			else  
			{
				DWORD value = enable ? 1 : 0;

				//xbox::utilities::DbgOut("[Client] setLiveBlock %s\n", enable ? "True" : "False");
				if (!dlaunchSetOptValByName) dlaunchSetOptValByName = (BOOL(__cdecl*)(CONST PCHAR, PDWORD))xbox::utilities::resolveFunction("launch.xex", 10);
				if (!dlaunchSetOptValByName("liveblock", &value))
					return E_FAIL;

				if (!dlaunchSetOptValByName("livestrong", &value))
					return E_FAIL;
			}

			return S_OK;
		}

		VOID launchDefaultApp()
		{
			XSetLaunchData(NULL, 0);
			XamLoaderLaunchTitleEx(XLAUNCH_KEYWORD_DEFAULT_APP, NULL, NULL, 0);
		}

		VOID patchInJump(DWORD* Address, DWORD Destination, bool Linked)
		{
			Address[0] = 0x3D600000 + ((Destination >> 16) & 0xFFFF);
			if (Destination & 0x8000)
				Address[0] += 1;
			Address[1] = 0x396B0000 + (Destination & 0xFFFF);
			Address[2] = 0x7D6903A6;
			Address[3] = Linked ? 0x4E800421 : 0x4E800420;
		}


		VOID PatchInBranch(DWORD* Address, DWORD Destination, BOOL Linked) 
		{
			Address[0] = (0x48000000 + ((Destination - (DWORD)Address) & 0x3FFFFFF));
			if (Linked) Address[0] += 1;
		}

		QWORD getFuseline(DWORD fuse) 
		{
			if ((fuse * 0x40) < 0x300)
				return Hvx::HvPeekQWORD(0x8000020000020000ULL + ((fuse * 0x40) << 3));
			return 0;
		}

		void getCPUKey(BYTE* KeyBuf) 
		{
			QWORD Key1 = getFuseline(3) | getFuseline(4);
			QWORD Key2 = getFuseline(5) | getFuseline(6);

			memcpy(KeyBuf, &Key1, 8);
			memcpy(KeyBuf + 8, &Key2, 8);
		}

		FARPROC resolveFunction(CHAR* ModuleName, DWORD Ordinal)
		{
			HMODULE mHandle = GetModuleHandle(ModuleName);
			return (mHandle == NULL) ? NULL : GetProcAddress(mHandle, (LPCSTR)Ordinal);
		}

		DWORD patchModuleImport(CHAR* Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress)
		{
			LDR_DATA_TABLE_ENTRY* moduleHandle = (LDR_DATA_TABLE_ENTRY*)GetModuleHandle(Module);
			return (moduleHandle == NULL) ? S_FALSE : patchModuleImport(moduleHandle, ImportedModuleName, Ordinal, PatchAddress);
		}

		IMAGE_SECTION_HEADER* findNtSection(IMAGE_SECTION_HEADER* Sections, WORD SectionCount, CHAR* SectionName) 
		{
			// Go through and search for our section
			for (WORD x = 0; x < SectionCount; x++) {
				if (strcmp((CHAR*)Sections[x].Name, SectionName) == 0)
					return &Sections[x];
			}

			return NULL;
		}

		DWORD getModuleImportCallAddress(LDR_DATA_TABLE_ENTRY* moduleHandle, CHAR* ImportedModuleName, DWORD Ordinal)
		{
			DWORD address = (DWORD)xbox::utilities::resolveFunction(ImportedModuleName, Ordinal);
			if (address == NULL)
				return S_FALSE;

			// Get our header field from this module
			VOID* headerBase = moduleHandle->XexHeaderBase;
			PXEX_IMPORT_DESCRIPTOR importDesc = (PXEX_IMPORT_DESCRIPTOR)RtlImageXexHeaderField(headerBase, 0x000103FF);
			if (importDesc == NULL)
				return S_FALSE;

			// Our result
			DWORD result = 2; // No occurances found

			// Get our string table position
			CHAR* stringTable = (CHAR*)(importDesc + 1);

			// Get our first entry
			XEX_IMPORT_TABLE_ORG* importTable = (XEX_IMPORT_TABLE_ORG*)(stringTable + importDesc->NameTableSize);

			// Loop through our table
			for (DWORD x = 0; x < importDesc->ModuleCount; x++)
			{
				// Go through and search all addresses for something that links
				DWORD* importAdd = (DWORD*)(importTable + 1);
				for (DWORD y = 0; y < importTable->ImportTable.ImportCount; y++)
				{
					// Check the address of this import
					DWORD value = *((DWORD*)importAdd[y]);
					if (value == address)
					{
						if (result != 2)
						{
							HalReturnToFirmware(HalPowerDownRoutine);
							return S_FALSE;
						}

						// We found a matching address address
						result = importAdd[y + 1];
					}
				}

				// Goto the next table
				importTable = (XEX_IMPORT_TABLE_ORG*)(((BYTE*)importTable) + importTable->TableSize);
			}

			// Return our result
			return result;
		}

		DWORD patchModuleImport(PLDR_DATA_TABLE_ENTRY Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress)
		{
			DWORD address = (DWORD)resolveFunction(ImportedModuleName, Ordinal);
			if (address == NULL)
				return S_FALSE;

			VOID* headerBase = Module->XexHeaderBase;
			PXEX_IMPORT_DESCRIPTOR importDesc = (PXEX_IMPORT_DESCRIPTOR)RtlImageXexHeaderField(headerBase, 0x000103FF);
			if (importDesc == NULL)
				return S_FALSE;

			DWORD result = 2;

			CHAR* stringTable = (CHAR*)(importDesc + 1);

			XEX_IMPORT_TABLE_ORG* importTable = (XEX_IMPORT_TABLE_ORG*)(stringTable + importDesc->NameTableSize);

			for (DWORD x = 0; x < importDesc->ModuleCount; x++)
			{
				DWORD* importAdd = (DWORD*)(importTable + 1);
				for (DWORD y = 0; y < importTable->ImportTable.ImportCount; y++)
				{
					DWORD value = *((DWORD*)importAdd[y]);
					if (value == address)
					{
						setMemory((DWORD*)importAdd[y], &PatchAddress, 4);
						DWORD newCode[4];
						patchInJump(newCode, PatchAddress, FALSE);
						setMemory((DWORD*)importAdd[y + 1], newCode, 16);
						result = S_OK;
					}
				}
				importTable = (XEX_IMPORT_TABLE_ORG*)(((BYTE*)importTable) + importTable->TableSize);
			}
			return result;
		}

		HRESULT PatchModuleImport(PLDR_DATA_TABLE_ENTRY Handle, PCHAR Import, DWORD Ordinal, DWORD Destination, BOOL Linked)
		{
			DWORD OriginalHook = (DWORD)resolveFunction(Import, Ordinal);
			PXEX_IMPORT_DESCRIPTOR ImportDescriptor = (PXEX_IMPORT_DESCRIPTOR)RtlImageXexHeaderField(Handle->XexHeaderBase, (((0x103) << 0x08) | 0xFF));
			if (OriginalHook == NULL || ImportDescriptor == NULL)
			{
				return FALSE;
			}

			PXEX_IMPORT_TABLE ImportTable = (PXEX_IMPORT_TABLE)((PBYTE)ImportDescriptor + sizeof(*ImportDescriptor) + ImportDescriptor->NameTableSize);
			for (DWORD dw = 0x00; dw < ImportDescriptor->ModuleCount; dw++)
			{
				for (WORD w = 0x00; w < ImportTable->ImportCount; w++)
				{
					DWORD StubHook = *((PDWORD)ImportTable->ImportStubAddr[w]);
					if (OriginalHook != StubHook) continue;
					StubHook = (DWORD)ImportTable->ImportStubAddr[w + 0x01];
					patchInJump((PDWORD)StubHook, Destination, Linked ? 0 : 1);
					w = ImportTable->ImportCount;
				}
				ImportTable = (PXEX_IMPORT_TABLE)((PBYTE)ImportTable + ImportTable->TableSize);
			}

			return ERROR_SUCCESS;
		}

		HRESULT PatchModuleImport(PCHAR Module, PCHAR Import, DWORD Ordinal, DWORD Destination, BOOL Linked)
		{
			PLDR_DATA_TABLE_ENTRY TableEntry = (PLDR_DATA_TABLE_ENTRY)GetModuleHandleA(Module);
			if (TableEntry == NULL)
			{
				return ERROR_INVALID_FUNCTION;
			}
			else
			{
				PatchModuleImport(TableEntry, Import, Ordinal, Destination, Linked);
			}

			return ERROR_SUCCESS;
		}


		DWORD ReadHighLow(DWORD Address, DWORD HighAdditive, DWORD LowAdditive, char label[50]) 
		{
			DWORD returnAddr = (*(PWORD)(Address + HighAdditive) << 16) | *(PWORD)(Address + LowAdditive);
			DWORD returnFinal = (returnAddr & 0x8000) ? returnAddr - 0x10000 : returnAddr;

			return returnFinal;
		}

		VOID __declspec(naked) GLPR(VOID) {
			__asm
			{
				std     r14, -0x98(sp)
				std     r15, -0x90(sp)
				std     r16, -0x88(sp)
				std     r17, -0x80(sp)
				std     r18, -0x78(sp)
				std     r19, -0x70(sp)
				std     r20, -0x68(sp)
				std     r21, -0x60(sp)
				std     r22, -0x58(sp)
				std     r23, -0x50(sp)
				std     r24, -0x48(sp)
				std     r25, -0x40(sp)
				std     r26, -0x38(sp)
				std     r27, -0x30(sp)
				std     r28, -0x28(sp)
				std     r29, -0x20(sp)
				std     r30, -0x18(sp)
				std     r31, -0x10(sp)
				stw     r12, -0x8(sp)
				blr
			}
		}

		DWORD RelinkGPLR(DWORD SFSOffset, DWORD* SaveStubAddress, DWORD* OriginalAddress) 
		{
			DWORD Instruction = NULL, Replacing;
			DWORD* Saver = (DWORD*)GLPR;

			if (SFSOffset & 0x2000000)
				SFSOffset |= 0xFC000000;

			Replacing = OriginalAddress[SFSOffset / 0x04];
			for (DWORD i = NULL; i < 0x14; i++) {
				if (Replacing == Saver[i]) {
					DWORD NewAddress = (DWORD)&Saver[i] - (DWORD)SaveStubAddress;
					Instruction = (0x48000001 | (NewAddress & 0x03FFFFFC));
				}
			}
			return Instruction;
		}

		void HookFunctionStart(DWORD* Address, DWORD* SaveStub, DWORD Destination) 
		{
			if ((Address != NULL) && (SaveStub != NULL))
			{
				DWORD Relocation = (DWORD)(&Address[0x04]);
				SaveStub[0x00] = (0x3D600000 + ((Relocation >> 0x10) & 0xFFFF));
				SaveStub[0x01] = (0x616B0000 + (Relocation & 0xFFFF));
				SaveStub[0x02] = 0x7D6903A6;

				for (DWORD i = 0; i < 0x04; i++) 
				{
					if ((Address[i] & 0x48000003) == 0x48000001)
						SaveStub[i + 0x03] = RelinkGPLR((Address[i] & ~0x48000003), &SaveStub[i + 0x03], &Address[i]);
					else SaveStub[i + 0x03] = Address[i];
				}

				SaveStub[0x07] = 0x4E800420;
				__dcbst(NULL, SaveStub);
				__sync();
				__isync();
				patchInJump(Address, Destination, false);
			}
		}

		char m_hookSection[0x500];
		int m_hookCount;
		void FadeStart()
		{
			int Fadetime = clock();
			global::Fade[0] > 0.0f && global::Fade[2] <= 0.0f ? global::Fade[0] = global::Fade[0] - global::FadeSpeed, global::Fade[1] = global::Fade[1] + global::FadeSpeed : false;
			global::Fade[1] > 0.0f && global::Fade[0] <= 0.0f ? global::Fade[1] = global::Fade[1] - global::FadeSpeed, global::Fade[2] = global::Fade[2] + global::FadeSpeed : false;
			global::Fade[2] > 0.0f && global::Fade[1] <= 0.0f ? global::Fade[0] = global::Fade[0] + global::FadeSpeed, global::Fade[2] = global::Fade[2] - global::FadeSpeed : false;
		}

		DWORD HookFunctionStub(DWORD _Address, void* Function) 
		{
			DWORD* startStub = (DWORD*)&m_hookSection[m_hookCount * 32];
			m_hookCount++;

			for (auto i = 0; i < 7; i++)
				startStub[i] = 0x60000000;
			startStub[7] = 0x4E800020;

			HookFunctionStart((DWORD*)_Address, startStub, (DWORD)Function);
			return (DWORD)startStub;
		}

		DWORD HookFunctionStub(CHAR* ModuleName, DWORD Ordinal, void* Destination) 
		{
			return HookFunctionStub((DWORD)ResolveFunction(ModuleName, Ordinal), Destination);
		}


		VOID hookFunctionStart(PDWORD Address, PDWORD SaveStub, DWORD Destination) 
		{
			if ((SaveStub != NULL) && (Address != NULL)) {
				DWORD AddressRelocation = (DWORD)(&Address[4]);
				if (AddressRelocation & 0x8000) {
					SaveStub[0] = 0x3D600000 + (((AddressRelocation >> 16) & 0xFFFF) + 1);
				} else {
					SaveStub[0] = 0x3D600000 + ((AddressRelocation >> 16) & 0xFFFF);
				}
				SaveStub[1] = 0x396B0000 + (AddressRelocation & 0xFFFF);
				SaveStub[2] = 0x7D6903A6;

				for (int i = 0; i < 4; i++) {
					if ((Address[i] & 0x48000003) == 0x48000001) {
						SaveStub[i + 3] = RelinkGPLR((Address[i] & ~0x48000003), &SaveStub[i + 3], &Address[i]);
					} else {
						SaveStub[i + 3] = Address[i];
					}
				}

				SaveStub[7] = 0x4E800420;
				__dcbst(0, SaveStub);
				__sync();
				__isync();
				patchInJump(Address, Destination, FALSE);
			}
		}

		HRESULT setMemory(VOID* Destination, VOID* Source, DWORD Length)
		{
			if (DevSetMemory == NULL)
			{
				DevSetMemory = (pDmSetMemory)resolveFunction("xbdm.xex", 40);
			}

			if (DevSetMemory == NULL)
			{
				memcpy(Destination, Source, Length);
				return ERROR_SUCCESS;
			}else{
				if (DevSetMemory(Destination, Length, Source, NULL) == MAKE_HRESULT(0, 0x2da, 0))
					return ERROR_SUCCESS;
			}
			return E_FAIL;
		}

		VOID ComparePointerWrite(VOID* Address, VOID* Value, DWORD Length)
		{
			setMemory((PVOID)Address, Value, Length);
		}

		VOID ComparePointerWrite(DWORD Address, DWORD Value)
		{
			if (!MmIsAddressValid((PVOID)Address))
			{
				return;
			}

			if (*(DWORD*)Address == NULL)
			{
				return;
			}

			setMemory((PVOID)Address, Value);
		}

		HRESULT setMemory(VOID* Destination, DWORD Value)
		{
			return setMemory(Destination, &Value, 4);
		}

		BOOL GetSectionInfo(CONST PCHAR SectionName, PDWORD Address, PDWORD Length) {
			DWORD SectionInfoOffset = 0x82000000;
			while (!strcmp(".rdata", (PCHAR)SectionInfoOffset) == FALSE) {
				SectionInfoOffset += 4;
			}
			PIMAGE_SECTION_HEADER DefaultSections = (PIMAGE_SECTION_HEADER)SectionInfoOffset;

			BOOL Succeded = FALSE;
			for (DWORD i = 0; strlen((PCHAR)DefaultSections[i].Name); i++) {
				if (!strcmp(SectionName, (PCHAR)DefaultSections[i].Name) == TRUE) {
					*Address = 0x82000000 + _byteswap_ulong(DefaultSections[i].VirtualAddress);
					*Length = _byteswap_ulong(DefaultSections[i].Misc.VirtualSize);
					Succeded = TRUE;
					break;
				}
			}

			return Succeded;
		}

		BOOL DataCompare(PBYTE pbData, PBYTE pbMask, PCHAR szMask) 
		{
			for (; *szMask; ++szMask, ++pbData, ++pbMask) 
			{
				if (*szMask == 'X' && *pbData != *pbMask) 
				{
					return FALSE;
				}
			}
			return (*szMask == NULL);
		}

		DWORD FindPattern(PCHAR SectionName, PCHAR pbMask, PCHAR szMask)
		{
			DWORD Address, Length;
			if (GetSectionInfo(SectionName, &Address, &Length) == TRUE) 
			{
				for (DWORD i = 0; i < Length; i++) 
				{
					if (DataCompare((PBYTE)(Address + i), (PBYTE)pbMask, szMask) == TRUE) 
					{
						return Address + i;
					}
				}
			}
			return NULL;
		}

		DWORD ReadHighLow(DWORD Address, DWORD HighAdditive, DWORD LowAdditive) 
		{
			DWORD ReturnAddress = (*(PWORD)(Address + HighAdditive) << 16) | *(PWORD)(Address + LowAdditive);
			return (ReturnAddress & 0x8000) ? ReturnAddress - 0x10000 : ReturnAddress;
		}

		VOID MakeThread(LPTHREAD_START_ROUTINE Address) 
		{
			HANDLE Handle = 0;
			ExCreateThread(&Handle, 0, 0, XapiThreadStartup, Address, 0, (EX_CREATE_FLAG_SUSPENDED | EX_CREATE_FLAG_SYSTEM | 0x18000424));
			XSetThreadProcessor(Handle, 4);
			SetThreadPriority(Handle, THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(Handle);
			CloseHandle(Handle);
		}

		VOID MakeThread(LPTHREAD_START_ROUTINE Address, LPVOID lpParam) 
		{
			HANDLE Handle = 0;
			ExCreateThread(&Handle, 0, 0, XapiThreadStartup, Address, lpParam, (EX_CREATE_FLAG_SUSPENDED | EX_CREATE_FLAG_SYSTEM | 0x18000424));
			XSetThreadProcessor(Handle, 4);
			SetThreadPriority(Handle, THREAD_PRIORITY_ABOVE_NORMAL);
			ResumeThread(Handle);
			CloseHandle(Handle);
		}

		BOOL pfShow = (BOOL)0xDEADBEEF;  //flag to init values
		BOOL pfShowMovie;
		BOOL pfPlaySound;
		BOOL pfShowIPTV;

		VOID toggleNotify(BOOL on) 
		{
			if ((int)pfShow == 0xDEADBEEF) //init values
				XNotifyUIGetOptions(&pfShow, &pfShowMovie, &pfPlaySound, &pfShowIPTV);

			if (!on) 
			{
				XNotifyUISetOptions(pfShow, pfShowMovie, pfPlaySound, pfShowIPTV);  //set back original values
			} 
			else 
			{
				XNotifyUISetOptions(true, true, true, true);  //turn on notifications so XBLSE msgs always show..
			}
			Sleep(500);
		}

		VOID XDoNotify(PWCHAR Message) 
		{
			toggleNotify(true);
			XNotifyD(XNOTIFYUI_TYPE_GENERIC, XUSER_INDEX_ANY, XNOTIFYUI_PRIORITY_HIGH, Message, NULL);
			toggleNotify(false);
		}

		VOID XNotify(std::string str) 
		{
			std::wstring wstr = std::wstring(str.begin(), str.end());
			const PWCHAR pwszStringParam = (PWCHAR)wstr.c_str();
			if (KeGetCurrentProcessType() != PROC_USER) 
			{
				HANDLE th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)XDoNotify, (LPVOID)pwszStringParam, CREATE_SUSPENDED, NULL);
				if (th == NULL) return;
				ResumeThread(th);
				return;
			} 
			else
				XDoNotify(pwszStringParam);
		}


		BOOL fileExists(LPCSTR lpFileName) 
		{
			if (GetFileAttributes(lpFileName) == -1) 
			{
				DWORD lastError = GetLastError();
				if (lastError == ERROR_PATH_NOT_FOUND) 
				{
					//xbox::utilities::DbgOut("[Client] %s ERROR_PATH_NOT_FOUND\n", lpFileName);
					return FALSE;
				}

				if (lastError == ERROR_FILE_NOT_FOUND) 
				{
					//xbox::utilities::DbgOut("[Client] %s ERROR_FILE_NOT_FOUND\n", lpFileName);
					return FALSE;
				}
			}
			return TRUE;
		}

		BOOL writeFile(const CHAR* FilePath, const VOID* Data, DWORD Size) 
		{
			// Open our file
			HANDLE fHandle = CreateFile(FilePath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (fHandle == INVALID_HANDLE_VALUE) 
			{
				//xbox::utilities::DbgOut("[Client] writeFile - CreateFile failed");
				return FALSE;
			}

			// Write our data and close
			DWORD writeSize = Size;
			if (WriteFile(fHandle, Data, writeSize, &writeSize, NULL) != TRUE) 
			{
				//xbox::utilities::DbgOut("[Client] writeFile - WriteFile failed");
				return FALSE;
			}

			CloseHandle(fHandle);
			return TRUE;
		}

		BOOL readFile(const CHAR* FileName, MemoryBuffer& pBuffer) 
		{
			HANDLE hFile; DWORD dwFileSize, dwNumberOfBytesRead;
			hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE) 
			{
				//xbox::utilities::DbgOut("[Client] readFile - CreateFile failed");
				return FALSE;
			}
			dwFileSize = GetFileSize(hFile, NULL);
			PBYTE lpBuffer = (BYTE*)malloc(dwFileSize);
			if (lpBuffer == NULL) 
			{
				CloseHandle(hFile);
				//xbox::utilities::DbgOut("[Client] readFile - malloc failed");
				return FALSE;
			}

			if (ReadFile(hFile, lpBuffer, dwFileSize, &dwNumberOfBytesRead, NULL) == FALSE) 
			{
				free(lpBuffer);
				CloseHandle(hFile);
				//xbox::utilities::DbgOut("[Client] readFile - ReadFile failed");
				return FALSE;
			}
			else if (dwNumberOfBytesRead != dwFileSize) 
			{
				free(lpBuffer);
				CloseHandle(hFile);
				//xbox::utilities::DbgOut("[Client] readFile - Failed to read all the bytes");
				return FALSE;
			}
			CloseHandle(hFile);
			pBuffer.Add(lpBuffer, dwFileSize);
			free(lpBuffer);
			return TRUE;
		}

		namespace data 
		{
			KEY_VAULT buffer;
			DWORD updSeqFlags;
			DWORD cTypeFlags;
			DWORD hardwareFlags;
			DWORD hvStatusFlags = 0x23289D3;
			DWORD ConsoleTypeSeqAllow;
			DWORD DXOSC;
			WORD bldrFlags = 0xD83E;
			WORD BLDR_FLAGS_KV1 = (~0x20);
			BYTE consoleType;
			BYTE SerialByte;
			BYTE cpuKey[0x10];
			BYTE XOSCHash[0x10];
			BYTE cpuKeyDigest[0x14];
			BYTE keyvaultDigest[0x14];
			BOOL fcrtRequired = FALSE;
			BOOL type1KV = FALSE;
			BYTE partnumber[0x08];
			BYTE kvsignature[0x100];
			BOOL crl = FALSE;
			BYTE proccessDigest[0x14];
			BYTE zeroEncryptedConsoleType[0x10];
		}

		PBYTE GetKeyVault()
		{
			QWORD KVAddress = xbox::Hvx::HvPeekQWORD(0x80000102000163C0);
			if (xbox::Hvx::HvPeekBytes(KVAddress, &data::buffer, sizeof(KEY_VAULT)) != S_OK) return 0;
			return (PBYTE)&data::buffer;
		}

		BYTE char2byte(char input) 
		{
			if (input >= '0' && input <= '9')
				return input - '0';
			if (input >= 'A' && input <= 'F')
				return input - 'A' + 10;
			if (input >= 'a' && input <= 'f')
				return input - 'a' + 10;
			return 0;
		}

		CONST BYTE RetailRoamableObfuscationKey[0x10] = { 0xE1, 0xBC, 0x15, 0x9C, 0x73, 0xB1, 0xEA, 0xE9, 0xAB, 0x31, 0x70, 0xF3, 0xAD, 0x47, 0xEB, 0xF3 };

		CONST BYTE MasterKey[0x110] = {
			0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			0xDD, 0x5F, 0x49, 0x6F, 0x99, 0x4D, 0x37, 0xBB, 0xE4, 0x5B, 0x98, 0xF2, 0x5D, 0xA6, 0xB8, 0x43,
			0xBE, 0xD3, 0x10, 0xFD, 0x3C, 0xA4, 0xD4, 0xAC, 0xE6, 0x92, 0x3A, 0x79, 0xDB, 0x3B, 0x63, 0xAF,
			0x38, 0xCD, 0xA0, 0xE5, 0x85, 0x72, 0x01, 0xF9, 0x0E, 0x5F, 0x5A, 0x5B, 0x08, 0x4B, 0xAD, 0xE2,
			0xA0, 0x2A, 0x42, 0x33, 0x85, 0x34, 0x53, 0x83, 0x1E, 0xE5, 0x5B, 0x8F, 0xBF, 0x35, 0x8E, 0x63,
			0xD8, 0x28, 0x8C, 0xFF, 0x03, 0xDC, 0xC4, 0x35, 0x02, 0xE4, 0x0D, 0x1A, 0xC1, 0x36, 0x9F, 0xBB,
			0x90, 0xED, 0xDE, 0x4E, 0xEC, 0x86, 0x10, 0x3F, 0xE4, 0x1F, 0xFD, 0x96, 0xD9, 0x3A, 0x78, 0x25,
			0x38, 0xE1, 0xD3, 0x8B, 0x1F, 0x96, 0xBD, 0x84, 0xF6, 0x5E, 0x2A, 0x56, 0xBA, 0xD0, 0xA8, 0x24,
			0xE5, 0x02, 0x8F, 0x3C, 0xA1, 0x9A, 0xEB, 0x93, 0x59, 0xD7, 0x1B, 0x99, 0xDA, 0xC4, 0xDF, 0x7B,
			0xD0, 0xC1, 0x9A, 0x12, 0xCC, 0x3A, 0x17, 0xBF, 0x6E, 0x4D, 0x78, 0x87, 0xD4, 0x2A, 0x7F, 0x6B,
			0x9E, 0x2F, 0xCD, 0x8D, 0x4E, 0xF5, 0xCE, 0xC2, 0xA0, 0x5A, 0xA3, 0x0F, 0x9F, 0xAD, 0xFE, 0x12,
			0x65, 0x74, 0x20, 0x6F, 0xF2, 0x5C, 0x52, 0xE4, 0xB0, 0xC1, 0x3C, 0x25, 0x0D, 0xAE, 0xD1, 0x82,
			0x7C, 0x60, 0xD7, 0x44, 0xE5, 0xCD, 0x8B, 0xEA, 0x6C, 0x80, 0xB5, 0x1B, 0x7A, 0x0C, 0x02, 0xCE,
			0x0C, 0x24, 0x51, 0x3D, 0x39, 0x36, 0x4A, 0x3F, 0xD3, 0x12, 0xCF, 0x83, 0x8D, 0x81, 0x56, 0x00,
			0xB4, 0x64, 0x79, 0x86, 0xEA, 0xEC, 0xB6, 0xDE, 0x8A, 0x35, 0x7B, 0xAB, 0x35, 0x4E, 0xBB, 0x87,
			0xEA, 0x1D, 0x47, 0x8C, 0xE1, 0xF3, 0x90, 0x13, 0x27, 0x97, 0x55, 0x82, 0x07, 0xF2, 0xF3, 0xAA,
			0xF9, 0x53, 0x47, 0x8F, 0x74, 0xA3, 0x8E, 0x7B, 0xAE, 0xB8, 0xFC, 0x77, 0xCB, 0xFB, 0xAB, 0x8A
		};


		BOOL XeKeysPkcs1Verify(const BYTE* pbHash, const BYTE* pbSig, XECRYPT_RSA* pRsa) 
		{
			BYTE scratch[256];
			DWORD val = pRsa->cqw << 3;
			if (val <= 0x200) 
			{
				XeCryptBnQw_SwapDwQwLeBe((QWORD*)pbSig, (QWORD*)scratch, val >> 3);
				if (XeCryptBnQwNeRsaPubCrypt((QWORD*)scratch, (QWORD*)scratch, pRsa) == 0) return FALSE;
				XeCryptBnQw_SwapDwQwLeBe((QWORD*)scratch, (QWORD*)scratch, val >> 3);
				return XeCryptBnDwLePkcs1Verify((const PBYTE)pbHash, scratch, val);
			}
			else return FALSE;
		}

		BYTE CoronaHash[0x10]	= { 0xD1, 0x32, 0xFB, 0x43, 0x9B, 0x48, 0x47, 0xE3, 0x9F, 0xE5, 0x46, 0x46, 0xF0, 0xA9, 0x9E, 0xB1 };
		BYTE XenonHash[0x10]	= { 0x02, 0x24, 0xEE, 0xA6, 0x1E, 0x89, 0x8B, 0xA1, 0x55, 0xB5, 0xAF, 0x74, 0xAA, 0x78, 0xAD, 0x0B };
		BYTE FalconHash[0x10]	= { 0x4E, 0xEA, 0xA3, 0x32, 0x3D, 0x9F, 0x40, 0xAA, 0x90, 0xC0, 0x0E, 0xFC, 0x5A, 0xD5, 0xB0, 0x00 };
		BYTE ZephyrHash[0x10]	= { 0x4E, 0xEA, 0xA3, 0x32, 0x3D, 0x9F, 0x40, 0xAA, 0x90, 0xC0, 0x0E, 0xFC, 0x5A, 0xD5, 0xB0, 0x00 };
		BYTE JasperHash[0x10]	= { 0xFF, 0x23, 0x99, 0x90, 0xED, 0x61, 0xD1, 0x54, 0xB2, 0x31, 0x35, 0x99, 0x0D, 0x90, 0xBD, 0xBC };
		BYTE TrinityHash[0x10]	= { 0xDB, 0xE6, 0x35, 0x87, 0x78, 0xCB, 0xFC, 0x2F, 0x52, 0xA3, 0xBA, 0xF8, 0x92, 0x45, 0x8D, 0x65 };

		HRESULT setupSpecialValues(DWORD updSeq) 
		{
			//======================================================
			data::fcrtRequired	= (xbox::utilities::data::buffer.OddFeatures & ODD_POLICY_FLAG_CHECK_FIRMWARE) != 0;
			data::SerialByte	= (((char2byte(data::buffer.ConsoleCertificate.ConsolePartNumber[2]) << 4) & 0xF0) | ((char2byte(data::buffer.ConsoleCertificate.ConsolePartNumber[3]) & 0x0F)));
			data::bldrFlags		= (data::type1KV == 1) ? ((WORD)(data::bldrFlags & data::BLDR_FLAGS_KV1)) : data::bldrFlags;
			//======================================================
			data::hvStatusFlags = (data::crl) ? (data::hvStatusFlags | 0x10000) : data::hvStatusFlags;
			data::hvStatusFlags = (data::fcrtRequired) ? (data::hvStatusFlags | 0x1000000) : data::hvStatusFlags;
			data::updSeqFlags	= updSeq;
			//======================================================
			xbox::Hvx::HvPokeDWORD(0x30, data::hvStatusFlags);
			//======================================================
			if (data::SerialByte <= 0x10) 
			{
				data::consoleType = 0;
				data::cTypeFlags = 0x010B0FFB;
				data::ConsoleTypeSeqAllow = 0x010B0524;

				data::DXOSC = 0x00000207;
				memcpy(data::XOSCHash, XenonHash, 0x10);
			}
			else if (data::SerialByte <= 0x14) 
			{
				data::consoleType = 1;
				data::cTypeFlags = 0x010B0524;
				data::ConsoleTypeSeqAllow = 0x010C0AD0;

				data::DXOSC = 0x10000227;
				memcpy(data::XOSCHash, ZephyrHash, 0x10);
			} 
			else if (data::SerialByte <= 0x18) 
			{
				data::consoleType = 2;
				data::cTypeFlags = 0x010C0AD8;
				data::ConsoleTypeSeqAllow = 0x010C0AD8;

				data::DXOSC = 0x20000227;
				memcpy(data::XOSCHash, FalconHash, 0x10);
			} 
			else if (data::SerialByte <= 0x52) 
			{
				data::consoleType = 3;
				data::cTypeFlags = 0x010C0AD0;
				data::ConsoleTypeSeqAllow = 0x010C0FFB;

				data::DXOSC = 0x30000227;
				memcpy(data::XOSCHash, JasperHash, 0x10);
			} 
			else if (data::SerialByte <= 0x58) 
			{
				data::consoleType = 4;
				data::cTypeFlags = 0x0304000D;
				data::ConsoleTypeSeqAllow = 0x0304000D;

				data::DXOSC = 0x40000227;
				memcpy(data::XOSCHash, TrinityHash, 0x10);
			}	
			else if (data::SerialByte <= 0x70) 
			{
				data::consoleType = 5;
				data::cTypeFlags = 0x0304000E;
				data::ConsoleTypeSeqAllow = 0x0304000E;

				data::DXOSC = 0x50000227;
				memcpy(data::XOSCHash, CoronaHash, 0x10);
			}
			//======================================================
			xbox::utilities::setMemory((byte*)0x8E03AA50, data::XOSCHash, 0x10);
			//======================================================
			data::hardwareFlags		= (XboxHardwareInfo->Flags & 0x0FFFFFFF) | ((data::consoleType & 0xF) << 28);
			data::hardwareFlags		= data::hardwareFlags & ~0x20;
			//======================================================
			return S_OK;
		}

		int HexStringToByteArray(PBYTE Array, const char* hexstring, unsigned int len)
		{
			const char* pos = hexstring;
			char* endptr;
			size_t count = 0;

			if ((hexstring[0] == '\0') || (strlen(hexstring) % 2))
			{
				//hexstring contains no data
				//or hexstring has an odd length
				return -1;
			}

			PBYTE Data = (PBYTE)malloc(len + 1);
			ZeroMemory(Data, len + 1);

			for (count = 0; count < len; count++)
			{
				char buf[5] = { '0', 'x', pos[0], pos[1], 0 };
				Data[count] = strtol(buf, &endptr, 0);
				pos += 2 * sizeof(char);

				if (endptr[0] != '\0')
				{
					//non-hexadecimal character encountered
					free(Data);
					return -1;
				}
			}
			memcpy(Array, Data, len);
			free(Data);

			return 0;
		}

		BOOL ProcessCPUKeyTXT() 
		{
			if (fileExists("Client:\\CPUKey.txt")) 
			{
				MemoryBuffer mbCpu;
				if (!readFile("Client:\\CPUKey.txt", mbCpu)) 
				{
					BYTE CPUKey[0x10];
					//HexStringToByteArray(CPUKey, mbCpu.GetData(), 0x10);
					return FALSE;
				}
				return FALSE;
			}
			return FALSE;
		}

		BOOL ProcessCPUKeyBin() {
			Sleep(50);
			//xbox::utilities::DbgOut("[Client] searching for CPU key spoof\n");
			if (fileExists("Client:\\CPUKey.bin")) {
				////xbox::utilities::DbgOut("[Client] CPUKey Found at Client:\\CPUKey.bin\n");

				MemoryBuffer mbCpu;
				if (!readFile("Client:\\CPUKey.bin", mbCpu)) {
					//xbox::utilities::DbgOut("[Client] CPUKey Not Loaded at Client:\\CPUKey.bin\n");
					return FALSE;
				}

				memcpy(data::cpuKey, mbCpu.GetData(), 0x10);
			} else {
				//xbox::utilities::DbgOut("[Client] CPUKey not Found at Client:\\CPUKey.bin\n");
				return FALSE;
			}

			Sleep(50);
			//xbox::utilities::DbgOut("[Client] setting up CPUKey data\n");
			XeCryptSha(data::cpuKey, 0x10, 0, 0, 0, 0, data::cpuKeyDigest, 0x14);
			return TRUE;
		}

		BOOL ProcessKVBin() {
			Sleep(50);
			xbox::utilities::DbgOut("[Client] searching for KeyVault spoof\n");
			if (fileExists("Client:\\KV.bin")) {
				//xbox::utilities::DbgOut("[Client] KV Found at Client:\\KV.bin\n");

				MemoryBuffer mbKV;
				if (!readFile("Client:\\KV.bin", mbKV)) {
					//xbox::utilities::DbgOut("[Client] KV Not Loaded at Client:\\KV.bin\n");
					return FALSE;
				}

				////xbox::utilities::DbgOut("[Client] setting up KeyVault data\n");
				memcpy(&data::buffer, mbKV.GetData(), sizeof(KEY_VAULT));
				return TRUE;
			} else {
				//xbox::utilities::DbgOut("[Client] KV not Found at Client:\\KV.bin\n");
				return FALSE;
			}

			return TRUE;
		}

		BOOL VerifyKeyVault() 
		{
			Sleep(50);
			XECRYPT_HMACSHA_STATE hmacSha;
			XeCryptHmacShaInit(&hmacSha, data::cpuKey, 0x10);
			XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&data::buffer.OddFeatures, 0xD4);
			XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&data::buffer.DvdKey, 0x1CF8);
			XeCryptHmacShaUpdate(&hmacSha, (BYTE*)&data::buffer.CardeaCertificate, 0x2108);
			XeCryptHmacShaFinal(&hmacSha, data::keyvaultDigest, XECRYPT_SHA_DIGEST_SIZE);
			memcpy((PVOID)0x8E03AA40, data::keyvaultDigest, 0x14);
			
			Sleep(50);
			data::type1KV = TRUE;
			for (DWORD x = 0; x < 0x100; x++){
				if (data::buffer.KeyVaultSignature[x] != NULL){
					data::type1KV = FALSE;
					return TRUE;
				}
			}
			return XeKeysPkcs1Verify(data::keyvaultDigest, data::buffer.KeyVaultSignature, (XECRYPT_RSA*)MasterKey);
		}

		BOOL ProcessSetDate() {
			Sleep(50);
			xbox::utilities::setMemory((PVOID)0x8E03A000, &data::buffer.ConsoleCertificate, 0x1A8);
			xbox::utilities::setMemory((PVOID)0x8E038020, &data::buffer.ConsoleCertificate.ConsoleId.abData, 5);
			
			Sleep(50);
			// CXNetLogonTask * g_pXNetLogonTask handle // v16203
			if (global::isDevkit) xbox::utilities::setMemory((BYTE*)((*(DWORD*)0x81D6B198) + 0x30BC), &data::buffer.ConsoleCertificate, 0x1A8);
			XeCryptSha((PBYTE)0x8E038014, 0x3EC, 0, 0, 0, 0, (PBYTE)0x8E038000, 0x14);
			
			Sleep(50);
			//Hash our CPUKEY Digest
			BYTE newHash[XECRYPT_SHA_DIGEST_SIZE];
			XeCryptSha((BYTE*)0x8E038014, 0x3EC, NULL, NULL, NULL, NULL, newHash, XECRYPT_SHA_DIGEST_SIZE);
			xbox::utilities::setMemory((PVOID)0x8E038000, newHash, XECRYPT_SHA_DIGEST_SIZE);
			
			Sleep(50);
			BYTE sataUpdateHash[0x14];
			XeCryptSha((BYTE*)0x8E038794, 0x8C, 0, 0, 0, 0, sataUpdateHash, 0x14);
			xbox::utilities::setMemory((PVOID)0x8E038794, sataUpdateHash, 0x14);
			
			Sleep(50);
			QWORD KVAddress = (global::isDevkit) ? xbox::Hvx::HvPeekQWORD(hvKvPtrDev) : xbox::Hvx::HvPeekQWORD(hvKvPtrRetail);
			if (xbox::Hvx::HvPeekBytes(KVAddress + 0xD0, &data::buffer.ConsoleObfuscationKey, 0x40) != S_OK) 
				return FALSE;

			Sleep(50);
			memcpy(&data::buffer.RoamableObfuscationKey, RetailRoamableObfuscationKey, 0x10);

			Sleep(50);
			if (xbox::Hvx::HvPokeBytes(KVAddress, &data::buffer, sizeof(KEY_VAULT)) != S_OK) 
				return FALSE;

			return TRUE;
		}

		BOOL ProcessSetMac() {

			BYTE currentMacAddress[6];
			BYTE spoofedMacAddress[6] = {
				0xFF, 0xFF, 0xFF,
				data::buffer.ConsoleCertificate.ConsoleId.asBits.MacIndex3,
				data::buffer.ConsoleCertificate.ConsoleId.asBits.MacIndex4,
				data::buffer.ConsoleCertificate.ConsoleId.asBits.MacIndex5
			};

			if ((XboxHardwareInfo->Flags & 0xF0000000) > 0x40000000) {
				spoofedMacAddress[0] = 0x7C;
				spoofedMacAddress[1] = 0xED;
				spoofedMacAddress[2] = 0x8D;
			} else {
				spoofedMacAddress[0] = 0x00;
				spoofedMacAddress[1] = 0x22;
				spoofedMacAddress[2] = 0x48;
			}

			memcpy(global::challenge::spoofedMacAddress, spoofedMacAddress, 6);
			if (NT_SUCCESS(ExGetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, currentMacAddress, 6, NULL))) {
				if (memcmp(currentMacAddress, global::challenge::spoofedMacAddress, 6) != 0) {
					if (NT_SUCCESS(ExSetXConfigSetting(XCONFIG_SECURED_CATEGORY, XCONFIG_SECURED_MAC_ADDRESS, global::challenge::spoofedMacAddress, 6))) {
						XamCacheReset(XAM_CACHE_ALL);
						HalReturnToFirmware(HalFatalErrorRebootRoutine);
					}
				}
			}

			DWORD temp = 0;
			XeCryptSha(global::challenge::spoofedMacAddress, 6, NULL, NULL, NULL, NULL, (BYTE*)&temp, 4);
			if (setupSpecialValues(temp & ~0xFF) != S_OK)
				return FALSE;

			return TRUE;
		}


		BOOL SetKeyVault() {

			Sleep(50);
			if (!ProcessKVBin())
				return FALSE;

			Sleep(50);
			if (!ProcessCPUKeyBin())
				return FALSE;

			Sleep(50);
			if (!VerifyKeyVault())
				return FALSE;

			Sleep(50);
			if (!ProcessSetDate())
				return FALSE;

			Sleep(50);
			if (!ProcessSetMac())
				return FALSE;

			Sleep(50);
			if (!XamCacheReset(XAM_CACHE_TICKETS)) 
				return FALSE;

			return TRUE;
		}

		VOID XexThread(LPTHREAD_START_ROUTINE Thread) 
		{
			HANDLE hThread; 
			DWORD hThreadID;

			ExCreateThread(&hThread, 0, &hThreadID, (PVOID)XapiThreadStartup, (LPTHREAD_START_ROUTINE)Thread, NULL, 0x2);
			XSetThreadProcessor(hThread, 4); ResumeThread(hThread); CloseHandle(hThread);
		}

		INT snprintf(char* buffer, size_t len, const char* fmt, ...) 
		{
			va_list args;
			int i;
			va_start(args, fmt);
			i = vsnprintf(buffer, len, fmt, args);
			va_end(args);
			return i;
		}
	}
}

template <class T> void writeData(int address, T data) {  }
template <> void writeData<bool>(int address, bool data) 
{
	memcpy((void*)address, &data, 1);
}

template <class S> void writeBlock(int address, S data, char* info) {  }
template <> void writeBlock<byte>(int address, byte data, char* info) 
{
	memcpy((void*)address, &data, 1);
}

template <> void writeBlock<short>(int address, short data, char* info) 
{
	memcpy((void*)address, &data, 2);
}

template <> void writeBlock<int>(int address, int data, char* info) 
{
	memcpy((void*)address, &data, 4);
}

template <> void writeBlock<float>(int address, float data, char* info) 
{
	memcpy((void*)address, &data, 4);
}

template <> void writeBlock<double>(int address, double data, char* info) 
{
	memcpy((void*)address, &data, 4);
}

template <> void writeBlock<long long>(int address, long long data, char* info) 
{
	memcpy((void*)address, &data, 8);
}