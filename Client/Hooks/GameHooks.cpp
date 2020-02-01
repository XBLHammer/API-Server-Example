#include "../stdafx.h"
#include "../Tools/Utilities.h"
#include "../Tools/Hooks.h"
#include "GameHooks.h"
#include <xbdm.h>
#include <time.h> 

extern BOOL DashLoaded;

//DWORD other = 0x38600001;
//BYTE Color2[0x04] = { 0xFF, 0x66, 0x00, 0x66 };//purple
//BYTE Background[0x04] = { 0xFF, 0x39, 0x39, 0x39 };//background Colors
//
//void* HandleExists(const char* OriginalPE)
//{
//	PLDR_DATA_TABLE_ENTRY DataTableEntry = (PLDR_DATA_TABLE_ENTRY)GetModuleHandleA("xboxkrnl.exe");
//	PXEX_HEADER_STRING String;
//	DataTableEntry = (PLDR_DATA_TABLE_ENTRY)DataTableEntry->InLoadOrderLinks.Flink;
//	while (DataTableEntry != 0)
//	{
//		String = (PXEX_HEADER_STRING)RtlImageXexHeaderField(DataTableEntry->XexHeaderBase, 0x183FF);
//		if ((String != 0) && (String->Data[0] != 0))
//		{
//			if (stricmp((char*)String->Data, OriginalPE) == 0)
//			{
//				return (void*)DataTableEntry;
//			}
//		}
//		DataTableEntry = (PLDR_DATA_TABLE_ENTRY)DataTableEntry->InLoadOrderLinks.Flink;
//	}
//	return ((void*)-1);
//}
//
//void WriteToResource(const char* OriginalPE, char* Section, unsigned long Offset, unsigned char* Bytes)
//{
//	if (HandleExists(OriginalPE) != ((void*)-1))
//	{
//		void* SectionData = 0;
//		unsigned long SectionSize = 0;
//		if (XGetModuleSection(HandleExists(OriginalPE), Section, &SectionData, &SectionSize))
//		{
//			unsigned long Address = (unsigned long)SectionData + Offset;
//			memcpy((void*)Address, Bytes, sizeof(unsigned long));
//		}
//	}
//}

//
BYTE ARGB[0x04] = { 0xFF, 0x66, 0x00, 0x66 };
bool InitializedCheats = false;
VOID InitializeGameHooks(PLDR_DATA_TABLE_ENTRY ModuleHandle) {
	if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"dash.xex") == 0) 
	{
		xbox::utilities::DbgOut("[Client] dash loaded \n");
		DashLoaded = TRUE;

		//xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 205, (DWORD)xbox::hooks::NetDll_XHttpConnectHook);
		//xbox::hooks::XHTTPHOOK();

		if (global::seting::UseSetDashUI == TRUE) 
		{
			xbox::System::patching::SetDashUI();
			//WriteToResource("dash.exe", "dashuisk", 0x7828, ARGB);
		}
	} else if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"xshell.xex") == 0)
		DashLoaded = TRUE;

	if (DashLoaded) 
	{
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 407, (DWORD)xbox::hooks::XexGetProcedureAddressHook);
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 408, (DWORD)xbox::hooks::XexLoadExecutableHook);
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 409, (DWORD)xbox::hooks::XexLoadImageHook);
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 404, (DWORD)xbox::hooks::XenonPrivilegeHook);

		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 530, (DWORD)xbox::hooks::XexLiveGoldHook);
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 535, (DWORD)xbox::hooks::HookXamUserGetMembershipTierFromXUID);
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 539, (DWORD)xbox::hooks::HookXamUserGetMembershipTier);
		
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 604, (DWORD)xbox::hooks::XamContentCreateEnumerator2);
		xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 551, (DWORD)xbox::hooks::XamUserGetSigninInfo2);


		if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"hud.xex") == 0) 
		{
			//xbox::utilities::DbgOut("[Client] HUD loaded \n");
			//hud::InitializeHudHooks(ModuleHandle);
			//xbox::System::patching::SetHUDUI();
		}
		else if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"dash.social.lex") == 0) 
		{
			//xbox::utilities::DbgOut("[Client] dash.socia.lex loaded \n");
			if (global::seting::UseSetDashUI == TRUE) 
			{
				/* Social Online */
				xbox::utilities::ComparePointerWrite(0x9AFC4876, 0xFF660066);
			}
		}
		else if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"dash.mp.contentexplorer.lex") == 0) {
			//xbox::utilities::DbgOut("[Client] dash.mp.contentexplorer.lex loaded \n");
			if (global::seting::UseSetDashUI == TRUE) 
			{
				/*Purchase Games Buttons*/
				xbox::utilities::ComparePointerWrite(0x9B111169, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B11CBB8, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B11DFCF, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B11F133, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B11F31A, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B120CF0, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B120F1C, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B121556, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B1224FA, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B12271F, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B1243C7, 0xFF660066);
				xbox::utilities::ComparePointerWrite(0x9B124609, 0xFF660066);
			}
		}else{

			XEX_EXECUTION_ID* pExecutionId = (XEX_EXECUTION_ID*)RtlImageXexHeaderField(ModuleHandle->XexHeaderBase, 0x00040006);
			if (pExecutionId != 0)
			{
				DWORD TitleID = pExecutionId->TitleID;
				DWORD dwVersion = (pExecutionId->Version >> 8) & 0xFF;
				xbox::utilities::DbgOut("[Client] %S loaded TitleID: %04X dwVersion: %04X\n", ModuleHandle->BaseDllName.Buffer, TitleID, dwVersion);

				if (TitleID == HomeGTAV && dwVersion != 0)
				{
					xbox::utilities::ComparePointerWrite(0x82C8B3B0, 0x60000000);
					xbox::utilities::ComparePointerWrite(0x838BFFF8, 0x60000000);
				}
				else if (TitleID == HomeBO2 && dwVersion != 0)
				{
					if (global::seting::UseBYPASSBO2) {
						//xbox::utilities::XNotifyUI(XNOTIFYUI_TYPE_PREFERRED_REVIEW, L"[Client] Bo2!!!");
						xbox::utilities::DbgOut("[Client] Bo2 loaded TitleID: %04X dwVersion: %04X\n", ModuleHandle->BaseDllName.Buffer, TitleID, dwVersion);

						xbox::hooks::GenerateRandomValues(ModuleHandle);

						if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"default.xex") == 0)
						{
							xbox::utilities::ComparePointerWrite(0x824A7CB8, 0x60000000); // Disables CRC32_Split hash // Bypass 2 - Unbannable for 2 weeks and counting // spPatch4BO2
							xbox::utilities::ComparePointerWrite(0x82320F60, 0x38600000); // xbdm check
						}
						else if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"default_mp.xex") == 0)
						{
							if (global::isDevkit)
								xbox::utilities::ComparePointerWrite(0x8228CF80, 0x48000018); // Didn't need to hide this, but it would have stuck out like a sore thumb that we were doing something fishy //mpPatch4BO2

							xbox::utilities::ComparePointerWrite(0x823C1D70, 0x38600000); // xbdm check
							xbox::utilities::ComparePointerWrite(0x8259A65C, 0x60000000); // Disables CRC32_Split hash // mpPatch5BO2
							xbox::utilities::ComparePointerWrite(0x8259A65C, 0x60000000);
							xbox::utilities::ComparePointerWrite(0x82497EB0, 0x60000000);
							xbox::utilities::ComparePointerWrite(0x825BEA7C, 0x60000000);
							xbox::utilities::ComparePointerWrite(0x825C6070, 0x60000000);

							*(int*)0x824E0DEC = 0x3940331C;
							*(int*)0x82409118 = 0x3900331C;
							*(int*)0x823E3ADC = 0x39400000;
							*(int*)0x823E3E78 = 0x39600000;
						}
					}
				}
				else if (TitleID == 0x415608FC && dwVersion != 0)
				{
					//xbox::utilities::XNotifyUI(XNOTIFYUI_TYPE_PREFERRED_REVIEW, L"[Client] GHOSTS!!!");
					xbox::utilities::DbgOut("[Client] GHOSTS loaded TitleID: %04X dwVersion: %04X\n", ModuleHandle->BaseDllName.Buffer, TitleID, dwVersion);

					// Generate our values
					xbox::hooks::GenerateRandomValues(ModuleHandle);

					if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"default.xex") == 0)
					{
						//xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 405, (DWORD)xbox::hooks::XexGetModuleHandleHook);

						xbox::utilities::ComparePointerWrite(0x8251174C, 0x48000010);
						xbox::utilities::ComparePointerWrite(0x82511714, 0x38600000);
						xbox::utilities::ComparePointerWrite(0x82511720, 0x39600001);
					}
					else if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"default_mp.xex") == 0)
					{						
						Sleep(10000);

						// This is specific to multiplayer
						xbox::utilities::ComparePointerWrite(0x826276CC, 0x38600000); // disable xbdm flag

						xbox::utilities::ComparePointerWrite(0x82627614, 0x39200009); //mpPatch1Ghosts
						xbox::utilities::ComparePointerWrite(0x8262767C, 0x48000010); //mpPatch2Ghosts
						xbox::utilities::ComparePointerWrite(0x82627628, 0x38600000); //mpPatch3Ghosts
						xbox::utilities::ComparePointerWrite(0x82627634, 0x39600001); //mpPatch4Ghosts

						xbox::utilities::ComparePointerWrite(0x82627684, 0x38600002);
						xbox::utilities::ComparePointerWrite(0x826276D4, 0x48000010);

						*(PBYTE)(0x8259175C + 0x03) = 0x00; //Prevent blacklist in console details

						BYTE test[8] = { 0x89, 0x21, 0x00, 0x50, 0x61, 0x49, 0x00, 0x00 };
						xbox::utilities::ComparePointerWrite((PVOID)0x82627684, test, 0x08);
						xbox::utilities::ComparePointerWrite((PVOID)0x826276DC, test, 0x08);
					}
				}
				else if (TitleID == HomeBO3 && dwVersion != 0)
				{
					if (global::seting::UseBYPASSBO3)
					{
						xbox::utilities::XNotifyUI(XNOTIFYUI_TYPE_PREFERRED_REVIEW, L"[Client] Bo3!!!");

						// Generate our values
						xbox::hooks::GenerateRandomValues(ModuleHandle);

						// Apply our bypasses
						xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 64, (DWORD)xbox::hooks::NetDll_XNetXnAddrToMachineIdHook);
						xbox::utilities::patchModuleImport(ModuleHandle, MODULE_XAM, 73, (DWORD)xbox::hooks::NetDll_XNetGetTitleXnAddrHook);
						xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 405, (DWORD)xbox::hooks::XexGetModuleHandleHook);
						xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 580, (DWORD)xbox::hooks::XeKeysGetKeyHook);
						xbox::utilities::patchModuleImport(ModuleHandle, MODULE_KERNEL, 582, (DWORD)xbox::hooks::XeKeysGetConsoleIDHook);

						if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"default.xex") == 0)
						{
							xbox::utilities::ComparePointerWrite(0x8253A5F8, 0x39600000);
							xbox::utilities::ComparePointerWrite(0x8253A614, 0x48000010);
							xbox::utilities::ComparePointerWrite(0x8253A60C, 0x38600000);
							xbox::utilities::ComparePointerWrite(0x8253A618, 0x39600001);
						}
						else if (wcscmp(ModuleHandle->BaseDllName.Buffer, L"default_zm.xex") == 0)
						{
							xbox::utilities::ComparePointerWrite(0x82539848, 0x48000010);
							xbox::utilities::ComparePointerWrite(0x82539840, 0x60000000);
							xbox::utilities::ComparePointerWrite(0x8253984C, 0x39600001);
						}

						BOOL isSingleplayer = wcscmp(ModuleHandle->BaseDllName.Buffer, L"default.xex") == 0;
						if (isSingleplayer)
						{
							xbox::utilities::ComparePointerWrite(0x8253A53C, 0x38600000); // this is really multiplayer but whatever
						}
						else
						{
							xbox::utilities::ComparePointerWrite(0x82539774, 0x38600000);
							*(__int64*)0x82332018 = 0x386000014E800020; // TU7/8
						}
					}
				}
			}
		}
	}
}