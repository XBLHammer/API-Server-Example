#include "stdafx.h"
#include "Tools/Hooks.h"

using namespace std;

BOOL DashLoaded;
BOOL FindPaths() 
{
	if ((XboxHardwareInfo->Flags & 0x20) == DM_XBOX_HW_FLAG_HDD) 
	{
		if (xbox::utilities::MountPath("Client:\\", "\\Device\\Harddisk0\\Partition1") != S_OK)
		{
			xbox::utilities::DbgOut("[Client] MountPath(Client:\\, \\Device\\Harddisk0\\Partition1 = FALSE \n");
			return FALSE;
		}
	} 
	else 
	{
		if (xbox::utilities::MountPath("Client:\\", "\\Device\\Mass0") != S_OK) 
		{
			xbox::utilities::DbgOut("[Client] MountPath(Client:\\, \\Device\\Mass0 = FALSE \n");
			return FALSE;
		}
	}

	XeCryptRandom(global::g_pSessionToken, 0x14);
	return TRUE;
}

VOID Start() 
{
	while (!DashLoaded)
		Sleep(1);

	xbox::utilities::setLiveBlock(FALSE);

	Sleep(1000);
	xbox::utilities::DbgOut("welcome to Xbox Live!!!!\n");
}

BOOL Initialize() 
{
	if (!FindPaths())
		return FALSE;

	global::modules::xam	= GetModuleHandle("xam.xex");
	global::modules::krnl	= GetModuleHandle("xboxkrnl.exe");

	//xbox::utilities::DbgOut("[Client] #1 \n");
	xbox::utilities::setLiveBlock(TRUE);
	xbox::Hvx::InitializeHvPeekPoke();
	xbox::utilities::getCPUKey(global::challenge::cpukey);

	//xbox::utilities::DbgOut("[Client] #2 \n");
	StartupServerCommunicator();

	//xbox::utilities::DbgOut("[Client] #3 \n");
	if (!global::isDevkit) 
	{
		xbox::System::patching::RGH();
	}

	//xbox::utilities::DbgOut("[Client] #4 \n");
	if (xbox::utilities::patchModuleImport(MODULE_XAM, MODULE_KERNEL, 404, (DWORD)xbox::hooks::XenonPrivilegeHook) != S_OK)
		return FALSE;

	//xbox::utilities::DbgOut("[Client] #5 \n");
	if (xbox::utilities::patchModuleImport(MODULE_XAM, MODULE_KERNEL, 0x198, (DWORD)xbox::hooks::XexLoadExecutableHook) != S_OK)
		return FALSE;

	//xbox::utilities::DbgOut("[Client] #6 \n");
	if (xbox::utilities::patchModuleImport(MODULE_XAM, MODULE_KERNEL, 0x199, (DWORD)xbox::hooks::XexLoadImageHook) != S_OK)
		return FALSE;

	//xbox::utilities::DbgOut("[Client] #7 \n");
	if (!xbox::utilities::SetKeyVault())
	{
		xbox::utilities::DbgOut("[Client] #1 -1\n");
	}

	//xbox::utilities::DbgOut("[Client] #8 \n");
	if (!Authenticate()) 
	{
		return FALSE;
	}
	else
		global::testA = TRUE;

	//xbox::utilities::DbgOut("[Client] #9 \n");
	if (System::InitializeSystemHooks() == FALSE)
	{
		xbox::utilities::DbgOut("[Client] #2 -1\n");
		return FALSE;
	}

	//xbox::utilities::DbgOut("[Client] #10 \n");
	xbox::utilities::MakeThread((LPTHREAD_START_ROUTINE)Start);

	//xbox::utilities::DbgOut("[Client] #11 \n");
	if (!global::isDevkit)
	{
		XBDMFiX();
	}
	return TRUE;
}

 BOOL PASCAL DllMain(HANDLE hInstDLL, ULONG fdwReason, LPVOID lpReserved)
 {
	global::modules::client = hInstDLL;
	global::isDevkit = *(DWORD*)0x8E038610 & 0x8000 ? FALSE : TRUE;
	xbox::utilities::setLiveBlock(TRUE);

	if (XamLoaderGetDvdTrayState() == DVD_TRAY_STATE_OPEN)
		return FALSE;

	if (!Initialize())
		return FALSE;

	return TRUE;
}
