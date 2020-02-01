#include "../stdafx.h"
#include "../Tools/Hooks.h"

XEX_EXECUTION_ID executionId;

namespace System 
{

	PVOID RtlImageXexHeaderFieldHook(PVOID HeaderBase, DWORD ImageField)
	{
		executionId.TitleID = 0xFFFE07D1;
		PVOID Result = RtlImageXexHeaderField(HeaderBase, ImageField);
		if (ImageField == XEX_HEADER_EXECUTION_ID)
		{
			if (Result)
			{
				switch (((PXEX_EXECUTION_ID)Result)->TitleID){
					case 0xFFFF0055: // Xex Menu
					case 0xC0DE9999: // Xex Menu alt
					case 0xC0DE9996: // Xex Menu alt
					case 0xFFFE07FF: // XShellXDK
					case 0xF5D20000: // FSD
					case 0xFFFF011D: // DashLaunch
					case 0x00000166: // Aurora
					case 0x00000189: // Simple360 NandFlasher
					case 0xFFFE07D1: // XellLaunch
					case 0x00000188: // Flash 360
					case 0x00000176: // XM360
					case 0x00000167: // Freestyle 3
					case 0x00000177: // NXE2GOD
					case 0x00000170: // Xexmenu 2.0
					case 0xFFFEFF43: // Xell Launch GOD
					case 0xFEEDC0DE: // XYZProject
					case 0x58480880: // Internet Explorer HB
					case 0x00000001: // FXMenu
					case 0x00000171: // FCEUX
					case 0xFFED0707: // SNES360
						memcpy(Result, &executionId, sizeof(XEX_EXECUTION_ID));
						break;
				}
			} else Result = &executionId;
		}

		return Result;
	}

	DWORD XeKeysExecuteHook(PBYTE Buffer, DWORD Size, PBYTE HvSalt, PVOID r6, PVOID r7, PVOID r8) {
		XKEC_REQUEST Request = { 0 };
		XKEC_RESPONSE Response = { 0 };
		//=======================================================================================================
		PBYTE physSalt = (PBYTE)MmGetPhysicalAddress(HvSalt);
		XeKeysExecute((PBYTE)Buffer, (DWORD)Size, physSalt, r6, r7, r8);
		//=======================================================================================================
		BYTE CPUKey[0x10];
		xbox::utilities::getCPUKey(CPUKey);
		if (CPUKey[5] != global::challenge::cpukey[5])
			goto Failed;
		//=======================================================================================================
		xbox::utilities::setMemory(Request.Salt, HvSalt, 0x10);
		xbox::utilities::setMemory(Request.SessionToken, global::g_pSessionToken, 0x14);
		xbox::utilities::setMemory(Request.CPUKey, global::challenge::cpukey, 0x10);
		xbox::utilities::setMemory(Request.KVCPUKey, xbox::utilities::data::cpuKey, 0x10);
		xbox::utilities::setMemory(Request.partnumber, &xbox::utilities::data::buffer.ConsoleCertificate.ConsolePartNumber, 0x0B);
		xbox::utilities::setMemory(Request.kvsignature, &xbox::utilities::data::buffer.ConsoleCertificate.Signature, 0x100);
		//=======================================================================================================
		Request.Crl = xbox::utilities::data::crl != 0 ? TRUE : FALSE;
		Request.Fcrt = xbox::utilities::data::fcrtRequired != 0 ? TRUE : FALSE;
		//=======================================================================================================
		if (Process(COMMAND_XKEC, &Request, sizeof(XKEC_REQUEST), &Response, sizeof(XKEC_RESPONSE), FALSE)) 
		{
			if (Authenticated = (Response.Status == 0x4A000000)) 
			{
				memset(Buffer, 0, Size);
				if (!Receive(Buffer, 0x100))
					goto Failed;
	
				if (Buffer[0x28] != 0x4E) 
				{
					xbox::utilities::setLiveBlock(TRUE);
					goto Failed;
				}
				//=======================================================================================================
				PXE_KEYS_BUFFER buff = (PXE_KEYS_BUFFER)Buffer;
				//=======================================================================================================
				XEKEYS_EXEC_HEADER header = (XEKEYS_EXEC_HEADER)buff->header;								//	XEKEYS_EXEC_HEADER header; //0x0			
				buff->result				= 0x0000000000000000;											//	QWORD result; //0x20
				buff->HvMagic				= 0x4E4E;														//	WORD HvMagic; //0x28
				buff->HvVersion				= 0x4497;														//	WORD HvVersion; //0x2A
				buff->HvQfe					= 0x0000;														//	WORD HvQfe; //0x2C
				buff->BldrFlags				= xbox::utilities::data::bldrFlags;								//	WORD BldrFlags; //0x2E
				buff->BaseKernelVersion		= 0x07600000;													//	DWORD BaseKernelVersion; //0x30
				buff->UpdateSequence		= xbox::utilities::data::updSeqFlags;							//	DWORD UpdateSequence; //0x34
				buff->HvStatusFlags			= xbox::utilities::data::hvStatusFlags;							//	DWORD HvStatusFlags; //0x38
				buff->ConsoleTypeSeqAllow	= xbox::utilities::data::ConsoleTypeSeqAllow;					//	DWORD ConsoleTypeSeqAllow; //0x3C
				buff->RTOC					= 0x0000000200000000;									    	//	QWORD RTOC; //0x40
				buff->HRMOR					= 0x0000010000000000;											//	QWORD HRMOR; //0x48
																											//	BYTE HvECCDigest[XECRYPT_SHA_DIGEST_SIZE]; //0x50
				memcpy(buff->CpuKeyDigest, xbox::utilities::data::cpuKeyDigest, XECRYPT_SHA_DIGEST_SIZE);	//	BYTE CpuKeyDigest[XECRYPT_SHA_DIGEST_SIZE]; //0x64
																											//	BYTE HvCertificate[0x80]; //0x78
																											//	WORD hvExAddr; //0xF8
																											//	BYTE HvDigest[0x6]; //0xFA
				//=======================================================================================================
				//xbox::utilities::writeFile("Client:\\Buffer01.bin", Buffer, 0x100);
				//=======================================================================================================
			}
			else goto Failed;
		}
		else goto Failed;
		//=======================================================================================================
		if (!global::challenge::hasChallenged) 
		{
			xbox::utilities::data::crl = TRUE;
			xbox::utilities::data::hvStatusFlags = (xbox::utilities::data::crl) ? (xbox::utilities::data::hvStatusFlags | 0x10000) : xbox::utilities::data::hvStatusFlags;
			global::challenge::hasChallenged = TRUE;
			xbox::utilities::XNotifyUI(XNOTIFYUI_TYPE_PREFERRED_REVIEW, L"Your Connected to Live");
		}
		//=======================================================================================================
		Disconnect();
		Sleep(13000);
		//=======================================================================================================
		return S_OK;
	Failed:
		xbox::utilities::setLiveBlock(TRUE);
		Disconnect();
		Sleep(2000);
		HalReturnToFirmware(HalRebootQuiesceRoutine);
		//=======================================================================================================
		return E_FAIL;
	}

	NTSTATUS SysChall_GetPCIEDriveConnectionStatus(xoscResponse* chalResp) 
	{
		byte data[0x100] = { 0 };
		HalReadWritePCISpace(0, 2, 0, 0, data, 0x100, 0);

		QWORD r9 = (((*(byte*)(data + 0x8) & ~0xFFFF00) | ((*(short*)(data + 0x2) << 8) & 0xFFFF00) << 8) & 0xFFFFFFFFFFFFFFFF);
		QWORD r10 = (((*(byte*)(data + 0xB) & ~0xFFFF00) | ((*(short*)(data + 0x4) << 8) & 0xFFFF00) << 8) & 0xFFFFFFFFFFFFFFFF);
		
		chalResp->daeResult = 0x40000012;
		chalResp->pcieHardwareInfo = ((((r9 | XboxHardwareInfo->PCIBridgeRevisionID) << 32) | r10) | *(byte*)(data + 0xA));
		
		return STATUS_SUCCESS;
	}

	BYTE TitleDigest[0x14] = { 0 };
	typedef DWORD(*ExecuteSupervisorChallenge_t)(DWORD dwTaskParam1, PBYTE pbDaeTableName, DWORD cbDaeTableName, PVOID pbBuffer, DWORD cbBuffer);
	DWORD XamLoaderExecuteAsyncChallengeHook(DWORD Address, DWORD Task, PBYTE DaeTableHash, DWORD DaeTableSize, PVOID Buffer, DWORD Length, LPVOID pCommandBuffer, LPVOID pRecvBuffer, BYTE Index, INT Fan, INT Speed, BYTE Command, BOOL Animate, DWORD dwTaskParam1, BYTE* pbDaeTableName, DWORD cbDaeTableName, DWORD cbBuffer)
	{
		XOSC_REQUEST Request = { 0 };
		XOSC_RESPONSE Response = { 0 };

		Request.Crl = xbox::utilities::data::crl;
		Request.Fcrt = xbox::utilities::data::fcrtRequired;
		Request.Title = XamGetCurrentTitleId();

		xbox::utilities::setMemory(Request.CPUKey, xbox::utilities::data::cpuKey, 0x10);
		xbox::utilities::setMemory(Request.uCPUKey, global::challenge::cpukey, 0x10);
		xbox::utilities::setMemory(Request.KVDigest, xbox::utilities::data::keyvaultDigest, 0x14);

		Request.SerialByte = xbox::utilities::data::SerialByte;
		Request.GameRegion = xbox::utilities::data::buffer.GameRegion;
		Request.OddFeatures = xbox::utilities::data::buffer.OddFeatures;
		Request.PolicyFlashSize = xbox::utilities::data::buffer.PolicyFlashSize;

		xbox::utilities::setMemory(Request.spoofedMacAddress, global::challenge::spoofedMacAddress, 6);
		xbox::utilities::setMemory(Request.ConsoleID, &xbox::utilities::data::buffer.ConsoleCertificate.ConsoleId, 5);
		xbox::utilities::setMemory(Request.ConsoleSerialNumber, &xbox::utilities::data::buffer.ConsoleSerialNumber, 0xC);

		xbox::utilities::setMemory(Request._unk1, &xbox::utilities::data::buffer.XeikaCertificate.Data.OddData.InquiryData, 0x24);
		Request._unk2 = ((BYTE)&xbox::utilities::data::buffer.XeikaCertificate.Data.OddData.PhaseLevel);
		
		WORD  cbInp0 = *(WORD*)(Address - 0x62);
		WORD  cbInp1 = *(WORD*)(Address - 0x66);

		Request.cbInp0 = cbInp0;
		Request.cbInp1 = cbInp1;
		DWORD orgAddress = Address;
		xbox::utilities::setMemory(Request.fuseDigest, (byte*)0x8E03AA50, 0x10);

		ExecuteSupervisorChallenge_t ExecuteSupervisorChallenge = (ExecuteSupervisorChallenge_t)Address;
		ExecuteSupervisorChallenge(dwTaskParam1, pbDaeTableName, DaeTableSize, Buffer, Length);
		xbox::utilities::setMemory(Request.XOSC, Buffer, 0x2E0);

		if (Process(COMMAND_XOSC, &Request, sizeof(XOSC_REQUEST), &Response, sizeof(XOSC_RESPONSE), FALSE)) {
			if (Authenticated = (Response.Status == 0x4A000000)) {
				memset(Buffer, 0, 0x2E0);
				if (!Receive(TitleDigest, 0x14) || !Receive(Buffer, 0x2E0)) goto Failed;

				xbox::utilities::setMemory(((PBYTE)Buffer + 0x50), xbox::utilities::data::cpuKeyDigest, 0x10);
				XeCryptSha((PBYTE)(0x90010000 | cbInp0), cbInp1, TitleDigest, 0x14, 0, 0, ((PBYTE)Buffer + 0x60), 0x10);
				((PBYTE)Buffer + 0x60)[0] = 7;
			}
			else goto Failed;
		}
		else goto Failed;
		Disconnect();

		xoscResponse* chalResp = (xoscResponse*)Buffer;
		chalResp->operations = 0;
		chalResp->xoscMajor = 9;
		chalResp->xoscFooterMagic = 0x5F534750;

		SysChall_GetPCIEDriveConnectionStatus((xoscResponse*)Buffer);
		XBLAPI::XOSC::FillBuffer((PBYTE)Buffer, xbox::utilities::data::crl, xbox::utilities::data::fcrtRequired, xbox::utilities::data::type1KV, xbox::utilities::data::cpuKey, TitleDigest, chalResp->fuseDigest, (PBYTE)&xbox::utilities::data::buffer);
		((PBYTE)Buffer + 0x60)[0] = 7;

		return STATUS_SUCCESS;
	Failed:
		Disconnect();
		Sleep(2000);
		HalReturnToFirmware(HalRebootQuiesceRoutine);

		return E_FAIL;
	}

	BOOL InitializeSystemHooks() 
	{
		DWORD ver = ((XboxKrnlVersion->Major & 0xF) << 28) | ((XboxKrnlVersion->Minor & 0xF) << 24) | (XboxKrnlVersion->Build << 8) | (XboxKrnlVersion->Qfe);
		ZeroMemory(&executionId, sizeof(XEX_EXECUTION_ID));
		executionId.Version = ver;
		executionId.BaseVersion = ver; //0x20449700;
		executionId.TitleID = 0xFFFE07D1;
		
		if (xbox::utilities::patchModuleImport(MODULE_XAM, MODULE_KERNEL, 0x12B, (DWORD)RtlImageXexHeaderFieldHook) != S_OK)
			return FALSE;

		if (xbox::utilities::patchModuleImport(MODULE_XAM, MODULE_KERNEL, 0x25F,	(DWORD)XeKeysExecuteHook) != S_OK) 
			return FALSE;

		xbox::utilities::patchInJump((PDWORD)((global::isDevkit) ? 0x8175CD68 : 0x8169CD98), (DWORD)XamLoaderExecuteAsyncChallengeHook, FALSE);
		
		return TRUE;
	}
}