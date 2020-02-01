#ifndef __Utilities_H
#define __Utilities_H
#pragma once

#define read64(addy) Hvx::HvPeekQWORD(addy)
#define read32(addy) Hvx::HvPeekDWORD(addy)
#define read16(addy) Hvx::HvPeekWORD(addy)
#define read8(addy) Hvx::HvPeekBYTE(addy)
#define write64(addy, data) Hvx::HvPokeQWORD(addy, data)
#define write32(addy, data) Hvx::HvPokeDWORD(addy, data)
#define write16(addy, data) Hvx::HvPokeWORD(addy, data)
#define write8(addy, data) Hvx::HvPokeBYTE(addy, data)
#define XPhysicalAllocM(size) XPhysicalAlloc(size, MAXULONG_PTR, NULL, PAGE_READWRITE);

#define hvKvPtrDev      0x00000102000162E0
#define hvKvPtrRetail   0x80000102000163C0

#define FLAG_DEAD 0x02
#define FLAG_CROUCHED 0x04
#define FLAG_PRONE 0x08
#define AngleToShort(X) ((DWORD)((X) * 65536 / 360) & 65535)
#define ANGLE3SHORT(x) ((int)((x) * 0x10000 / 360) & 0xFFFF)
#define	SHORT2ANGLE(x)	((x) * (360.0/65536.0f))
#define GetPointer(X) *(DWORD*)(X);
#define GetPtr(X) *(PDWORD)(X)
#define M_PI 3.14159265358979323846f
#define XINPUT_GAMEPAD_LEFT_TRIGGER		state.Gamepad.bLeftTrigger
#define XINPUT_GAMEPAD_RIGHT_TRIGGER	state.Gamepad.bRightTrigger

extern BOOL DashLoaded;
extern PCHAR PATH_KV;
extern PCHAR PATH_CPU;
extern PCHAR PATH_XEX;
extern PCHAR PATH_INI;

class hooks {
public:
	void setupDetour(int addr, int dest);
	void patchFunction();
	void restoreDetour();
	void detourFunctionStart();
private:
	int stub[8];
	int* address;
	int destination;
	byte* opcodes;
};


typedef struct _LAUNCH_SYS_MSG {
	XNOTIFYQUEUEUI_TYPE Type;
	PWCHAR Message;
	DWORD Delay;
} LAUNCH_SYS_MSG, *PLAUNCH_SYS_MSG;

class MemoryBuffer
{
public:

	MemoryBuffer( DWORD dwSize = 512 )
	{
		m_pBuffer = NULL;
		m_dwDataLength = 0;
		m_dwBufferSize = 0;

		if( ( dwSize < UINT_MAX ) && ( dwSize != 0 ) )
		{
			m_pBuffer = ( BYTE* )malloc( dwSize + 1 );    // one more char, in case when using string funcions
			if( m_pBuffer )
			{
				m_dwBufferSize = dwSize;
				m_pBuffer[0] = 0;
			}
		}
	};

	~MemoryBuffer()
	{
		if( m_pBuffer )
			free( m_pBuffer );

		m_pBuffer = NULL;
		m_dwDataLength = 0;
		m_dwBufferSize = 0;
	};

    // Add chunk of memory to buffer
    BOOL    Add( const void* p, DWORD dwSize )
    {
        if( CheckSize( dwSize ) )
        {
            memcpy( m_pBuffer + m_dwDataLength, p, dwSize );
            m_dwDataLength += dwSize;
            *( m_pBuffer + m_dwDataLength ) = 0;    // fill end zero
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    };

    // Get the data in buffer
    BYTE* GetData() const
    {
        return m_pBuffer;
    };

    // Get the length of data in buffer
    DWORD   GetDataLength() const
    {
        return m_dwDataLength;
    };

    // Rewind the data pointer to the begining
    void    Rewind()
    {
        m_dwDataLength = 0; m_pBuffer[ 0 ] = 0;
    };

    // Automatically adjust increase buffer size if necessary
    BOOL    CheckSize( DWORD dwSize )
    {
        if( m_dwBufferSize >= ( m_dwDataLength + dwSize ) )
        {
            return TRUE;    // Enough space
        }
        else
        {
            // Try to double it
            DWORD dwNewSize = max( m_dwDataLength + dwSize, m_dwBufferSize * 2 );
            BYTE* pNewBuffer = ( UCHAR* )realloc( m_pBuffer, dwNewSize + 1 );        // one more char
            if( pNewBuffer )
            {
                m_pBuffer = pNewBuffer;
                m_dwBufferSize = dwNewSize;
                return TRUE;
            }
            else
            {
                // Failed
                return FALSE;
            }
        }
    }

	private:

	BYTE* m_pBuffer;

    DWORD m_dwDataLength;

    DWORD m_dwBufferSize;
};

//#define GetPointer(X) *(DWORD*)(X)

static BOOL(__cdecl *dlaunchSetOptValByName)(CONST PCHAR optName, PDWORD val); // set when setLiveBlock is called
static HRESULT(__cdecl *DevSetMemory)(LPVOID lpbAddr, DWORD cb, LPCVOID lpbBuf, LPDWORD pcbRet); // set when xbox::utilities::setMemory is called
static LAUNCH_SYS_MSG notifyData;


#pragma pack(1)
typedef enum _ODD_POLICY {
	ODD_POLICY_FLAG_CHECK_FIRMWARE = 0x120,
} ODD_POLICY;

typedef union _INQUIRY_DATA {
	struct {
		BYTE DeviceType : 5;
		BYTE DeviceTypeQualifier : 3;
		BYTE DeviceTypeModifier : 7;
		BYTE RemovableMedia : 1;
		BYTE Versions : 8;
		BYTE ResponseDataFormat : 4;
		BYTE HiSupport : 1;
		BYTE NormACA : 1;
		BYTE ReservedBit : 1;
		BYTE AERC : 1;
		BYTE AdditionalLength : 8;
		WORD Reserved : 16;
		BYTE SoftReset : 1;
		BYTE CommandQueue : 1;
		BYTE Reserved2 : 1;
		BYTE LinkedCommands : 1;
		BYTE Synchronous : 1;
		BYTE Wide16Bit : 1;
		BYTE Wide32Bit : 1;
		BYTE RelativeAddressing : 1;
		BYTE VendorId[8];
		BYTE ProductId[16];
		BYTE ProductRevisionLevel[4];
	};
	BYTE Data[0x24];
} INQUIRY_DATA, * pINQUIRY_DATA;

typedef struct _XEIKA_ODD_DATA {
	BYTE         Version;
	BYTE         PhaseLevel;
	INQUIRY_DATA InquiryData;
} XEIKA_ODD_DATA, * PXEIKA_ODD_DATA;// size 0x26

typedef struct _XEIKA_DATA {
	XECRYPT_RSAPUB_2048 PublicKey;// 0x110
	DWORD               Signature; // 0x4
	WORD                Version; // 0x2
	XEIKA_ODD_DATA      OddData;// 0x26
	BYTE                Padding[4];//0x4
} XEIKA_DATA, * PXEIKA_DATA; //0x140

typedef struct _XEIKA_CERTIFICATE {
	WORD       Size;//0x2
	XEIKA_DATA Data;//0x140
	BYTE       Padding[0x1146];
} XEIKA_CERTIFICATE, * PXEIKA_CERTIFICATE;// size of 0x1288

typedef struct _KEY_VAULT {                     // Key #
	BYTE  HmacShaDigest[0x10];                  // 0x0
	BYTE  Confounder[0x08];                     // 0x10
	BYTE  ManufacturingMode;                    // 0x18
	BYTE  AlternateKeyVault;                    // 0x19
	BYTE  RestrictedPrivilegesFlags;            // 0x1A
	BYTE  ReservedByte3;                        // 0x1B
	WORD  OddFeatures;                          // 0x1C
	WORD  OddAuthtype;                          // 0x1E
	DWORD RestrictedHvextLoader;                // 0x20
	DWORD PolicyFlashSize;                      // 0x24
	DWORD PolicyBuiltinUsbmuSize;               // 0x28
	DWORD ReservedDword4;                       // 0x2C
	QWORD RestrictedPrivileges;                 // 0x30
	QWORD ReservedQword2;                       // 0x38
	QWORD ReservedQword3;                       // 0x40
	QWORD ReservedQword4;                       // 0x48
	BYTE  ReservedKey1[0x10];                   // 0x50
	BYTE  ReservedKey2[0x10];                   // 0x60
	BYTE  ReservedKey3[0x10];                   // 0x70
	BYTE  ReservedKey4[0x10];                   // 0x80
	BYTE  ReservedRandomKey1[0x10];             // 0x90
	BYTE  ReservedRandomKey2[0x10];             // 0xA0
	BYTE  ConsoleSerialNumber[0xC];             // 0xB0
	BYTE  MoboSerialNumber[0xC];                // 0xBC
	WORD  GameRegion;                           // 0xC8
	BYTE  Padding1[0x6];                        // 0xCA
	BYTE  ConsoleObfuscationKey[0x10];          // 0xD0
	BYTE  KeyObfuscationKey[0x10];              // 0xE0
	BYTE  RoamableObfuscationKey[0x10];         // 0xF0
	BYTE  DvdKey[0x10];                         // 0x100
	BYTE  PrimaryActivationKey[0x18];           // 0x110
	BYTE  SecondaryActivationKey[0x10];         // 0x128
	BYTE  GlobalDevice2desKey1[0x10];           // 0x138
	BYTE  GlobalDevice2desKey2[0x10];           // 0x148
	BYTE  WirelessControllerMs2desKey1[0x10];   // 0x158
	BYTE  WirelessControllerMs2desKey2[0x10];   // 0x168
	BYTE  WiredWebcamMs2desKey1[0x10];          // 0x178
	BYTE  WiredWebcamMs2desKey2[0x10];          // 0x188
	BYTE  WiredControllerMs2desKey1[0x10];      // 0x198
	BYTE  WiredControllerMs2desKey2[0x10];      // 0x1A8
	BYTE  MemoryUnitMs2desKey1[0x10];           // 0x1B8
	BYTE  MemoryUnitMs2desKey2[0x10];           // 0x1C8
	BYTE  OtherXsm3DeviceMs2desKey1[0x10];      // 0x1D8
	BYTE  OtherXsm3DeviceMs2desKey2[0x10];      // 0x1E8
	BYTE  WirelessController3p2desKey1[0x10];   // 0x1F8
	BYTE  WirelessController3p2desKey2[0x10];   // 0x208
	BYTE  WiredWebcam3p2desKey1[0x10];          // 0x218
	BYTE  WiredWebcam3p2desKey2[0x10];          // 0x228
	BYTE  WiredController3p2desKey1[0x10];      // 0x238
	BYTE  WiredController3p2desKey2[0x10];      // 0x248
	BYTE  MemoryUnit3p2desKey1[0x10];           // 0x258
	BYTE  MemoryUnit3p2desKey2[0x10];           // 0x268
	BYTE  OtherXsm3Device3p2desKey1[0x10];      // 0x278
	BYTE  OtherXsm3Device3p2desKey2[0x10];      // 0x288
	XECRYPT_RSAPRV_1024 ConsolePrivateKey;      // 0x298 //length 0x1D0
	XECRYPT_RSAPRV_2048 XeikaPrivateKey;        // 0x468 //length 0x390
	XECRYPT_RSAPRV_1024 CardeaPrivateKey;       // 0x7F8
	XE_CONSOLE_CERTIFICATE ConsoleCertificate;  // 0x9C8
	XEIKA_CERTIFICATE XeikaCertificate;         // 0xB70
	BYTE  KeyVaultSignature[0x100];             // 0x1DF8
	BYTE  CardeaCertificate[0x2108];            // 0x1EF8 to 0x4000
} KEY_VAULT, * PKEY_VAULT;
#pragma pack()


namespace xbox {
	namespace utilities {
		typedef HRESULT(*pDmSetMemory)(LPVOID lpbAddr, DWORD cb, LPCVOID lpbBuf, LPDWORD pcbRet);

		namespace data {
			extern KEY_VAULT buffer;
			extern DWORD updSeqFlags;
			extern DWORD cTypeFlags;
			extern DWORD hardwareFlags;
			extern DWORD hvStatusFlags;
			extern WORD  bldrFlags;
			extern DWORD ConsoleTypeSeqAllow;
			extern DWORD DXOSC;
			extern BYTE consoleType;
			extern BYTE SerialByte;
			extern BYTE cpuKey[0x10];
			extern BYTE XOSCHash[0x10];
			extern BYTE cpuKeyDigest[0x14];
			extern BYTE keyvaultDigest[0x14];
			extern BOOL fcrtRequired;
			extern BOOL type1KV;
			extern BOOL crl;
			extern BYTE proccessDigest[0x14];
			extern BYTE zeroEncryptedConsoleType[0x10];
		}

		void patch_BLOCK_LIVE(BOOL enable);

		BYTE char2byte(char input);
		std::string utf8_encode(const std::wstring& wstr);
		std::wstring utf8_decode(const std::string& str);
		int toWCHAR(char* input, WCHAR* output);
		PWCHAR charToWChar(__in LPCSTR c_buffer);

		void getCPUKey(BYTE* KeyBuf);
		HRESULT setLiveBlock(BOOL enabled);

		void DbgOut(const char* text, ...);
		DWORD applyPatches(VOID* patches);

		DWORD MountPath(PCHAR Drive, PCHAR Device);
		BOOL writeFile(const CHAR* FilePath, const VOID* Data, DWORD Size);
		BOOL readFile(const CHAR* FileName, MemoryBuffer& pBuffer);
		DWORD ReadHighLow(DWORD Address, DWORD HighAdditive, DWORD LowAdditive);
		DWORD FindPattern(PCHAR SectionName, PCHAR pbMask, PCHAR szMask);
		FARPROC resolveFunction(CHAR* ModuleName, DWORD Ordinal);
		DWORD patchModuleImport(CHAR* Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress);
		DWORD patchModuleImport(PLDR_DATA_TABLE_ENTRY Module, CHAR* ImportedModuleName, DWORD Ordinal, DWORD PatchAddress);

		BOOL fileExists(LPCSTR lpFileName);
		HRESULT setMemory(VOID* Destination, DWORD Value);
		HRESULT setMemory(VOID* Destination, VOID* Source, DWORD Length);
		VOID ComparePointerWrite(VOID* Address, VOID* Value, DWORD Length);
		VOID XexThread(LPTHREAD_START_ROUTINE Thread);

		static VOID XNotifyUI(XNOTIFYQUEUEUI_TYPE Type, PWCHAR String) { XNotifyQueueUI(Type, XUSER_INDEX_ANY, XNOTIFYUI_PRIORITY_HIGH, String, 0); }
		
		VOID MakeThread(LPTHREAD_START_ROUTINE Address, LPVOID lpParam);
		VOID MakeThread(LPTHREAD_START_ROUTINE Address);
		VOID XNotify(std::string str);
		VOID patchInJump(DWORD* Address, DWORD Destination, bool Linked);
		void FadeStart();

		// hook a function start based on address, using 8 instruction saveStub to do the deed
		// addr = address of the hook
		// saveStub = address of the area to create jump stub for replaced instructions
		// dest = where the hook at addr is pointing to
		VOID hookFunctionStart(PDWORD addr, PDWORD saveStub, DWORD dest);

		HRESULT PatchModuleImport(PCHAR Module, PCHAR Import, DWORD Ordinal, DWORD Destination, BOOL Linked);
		VOID ComparePointerWrite(DWORD Address, DWORD Value);
		DWORD getModuleImportCallAddress(LDR_DATA_TABLE_ENTRY* moduleHandle, CHAR* ImportedModuleName, DWORD Ordinal);

		BOOL SetKeyVault();

		DWORD HookFunctionStub(DWORD _Address, void* Function);
		string GetModuleNameFromAddress(DWORD dwAddress);
		std::string va(char* format, ...);
	}
}


template <class T> extern void writeData(int address, T data);
template <> extern void writeData<bool>(int address, bool data);

template <class S> void writeBlock(int address, S data, char* info);
template <> extern void writeBlock<byte>(int address, byte data, char* info);
template <> extern void writeBlock<short>(int address, short data, char* info);
template <> extern void writeBlock<int>(int address, int data, char* info);
template <> extern void writeBlock<float>(int address, float data, char* info);
template <> extern void writeBlock<double>(int address, double data, char* info);
template <> void writeBlock<long long>(int address, long long data, char* info);

template<typename R>
R Invoke(int Address) { return ((R(*)())Address)(); }
template<typename R, typename A1>
R Invoke(int Address, A1 a1) { return ((R(*)(A1 a1))Address)(a1); }
template<typename R, typename A1, typename A2>
R Invoke(int Address, A1 a1, A2 a2) { return ((R(*)(A1 a1, A2 a2))Address)(a1, a2); }
template<typename R, typename A1, typename A2, typename A3>
R Invoke(int Address, A1 a1, A2 a2, A3 a3) { return ((R(*)(A1 a1, A2 a2, A3 a3))Address)(a1, a2, a3); }
template<typename R, typename A1, typename A2, typename A3, typename A4>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4))Address)(a1, a2, a3, a4); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5))Address)(a1, a2, a3, a4, a5); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6))Address)(a1, a2, a3, a4, a5, a6); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7))Address)(a1, a2, a3, a4, a5, a6, a7); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8))Address)(a1, a2, a3, a4, a5, a6, a7, a8); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16, typename A17>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16, typename A17, typename A18>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18); }
template<typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9, typename A10, typename A11, typename A12, typename A13, typename A14, typename A15, typename A16, typename A17, typename A18, typename A19>
R Invoke(int Address, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19) { return ((R(*)(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10, A11 a11, A12 a12, A13 a13, A14 a14, A15 a15, A16 a16, A17 a17, A18 a18, A19 a19))Address)(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19); }
#endif