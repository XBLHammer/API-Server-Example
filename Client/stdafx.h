#pragma once

// Xbox Includes
#include <xtl.h>
#include <xbox.h>
#include <stdio.h>
#include <xbdm.h>
#include <xboxmath.h>
#include <xuiapp.h>
#include <xui.h>
#include <xuiapp.h>
#include <xuiresource.h>
#include <xuirender.h>
#include <xgraphics.h>
#include <xuianimate.h>
#include <xavatar.h>
#include <xmedia2.h>
#include <xhttp.h>
#include <xauth.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <xkelib.h>
#include <time.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

using namespace std;

//defines
#define MODULE_DASH				"dash.xex"
#define MODULE_XSHELL			"xshell.xex"
#define MODULE_TITLE			"default.xex"

//Type defines
typedef unsigned int			uint;
typedef unsigned __int64		QWORD;

//
#include "struct/ntstatus.h"
#include "struct/XKE.h"
#include "struct/XOSC.h"

#include "Tools/patching.h"
#include "Tools/Utilities.h"
#include "Tools/HvxCalls.h"

#include "Hooks/GameHooks.h"
#include "Hooks/System.h"

#include "Network/Network.h"

#include "XBLAPI.h"

//
namespace global 
{
	//
	extern BOOL isDevkit;
	extern BOOL isAuthed; 
	extern BOOL IsINI;
	extern DWORD supportedVersion;
	extern WCHAR wNotifyMsg[100];
	extern DWORD cryptData[6];
	extern BYTE g_pSessionToken[0x14];
	extern BOOL testA;

	extern float Fade[4];
	extern bool ToggleFade;
	extern float FadeSpeed;

	//
	extern PCHAR PATH_KV;
	extern PCHAR PATH_CPU;
	extern PCHAR PATH_XEX;

	namespace seting 
	{
		//
		extern BOOL UseSetDashUI;
		extern BOOL UseSetGold;

		//BYPASS
		extern BOOL UseBYPASSBO2;
		extern BOOL UseBYPASSBO3;
		extern BOOL UseBYPASSGHOSTS;
	}

	namespace DASHRESPONSE
	{
		extern BYTE Color[];
		extern BYTE Background[];
	}

	namespace modules 
	{
		extern HANDLE client;
		extern HANDLE xam;
		extern HANDLE krnl;
		extern HANDLE ModuleHandle;
		extern BYTE ModuleDigest[];
	}
}

namespace Keys
{
	extern BYTE BLKey[0x10];			// 1BL Key
	extern BYTE blSlimRSA[0x110];		// public bootloader rsa key
}