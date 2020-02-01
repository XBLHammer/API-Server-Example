#include "../stdafx.h"

namespace xbox 
{
	namespace System 
	{
		namespace patching 
		{

			VOID SetDashUI() 
			{
				///////////////////////////////////////////////////
				if (global::seting::UseSetDashUI == TRUE) 
				{
					BYTE Color[] = { 0xFF, 0x66, 0x00, 0x66 };
					BYTE SettingsBackground[] = { 0xFF, 0x00, 0x00, 0x00 };
					BYTE DashBackGroundColour[] = { 0xFF, 0x00, 0x00, 0x00 };

					memcpy(SettingsBackground,		global::DASHRESPONSE::Background,	0x04);
					memcpy(DashBackGroundColour,	global::DASHRESPONSE::Background,	0x04);
					memcpy(Color,					global::DASHRESPONSE::Color,		0x04);

					///////////////////////////////////////////////////
					/*DASH TITLES DASH: 17559*/
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92ACAA30, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B570E2, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92C58F28, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92C72B73, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92DB07E5, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F0E15B, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F54622, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F5F852, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92BACFC0, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92C58F24, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F0E16F, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					/*DASH TITLES DASH: 17559*/
					///////////////////////////////////////////////////

					///////////////////////////////////////////////////
					/*Dash and setting Background*/ // 17559
					///////////////////////////////////////////////////
					BYTE Data[0x13];
					xbox::utilities::ComparePointerWrite((PDWORD)0x92B19236, Data, 0x13);// aura_background nulled
					///////////////////////////////////////////////////

					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDA7, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F58323, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDAF, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDB3, DashBackGroundColour, ARRAYSIZE(DashBackGroundColour));
					/////////////////////////////////////////////////////
					/*Settings background*/
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FE03, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDB3, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDAF, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FD9F, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FD93, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDA3, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F54632, SettingsBackground, ARRAYSIZE(SettingsBackground));
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F0E053, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D5749E, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D57213, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D56FAA, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D56D79, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D56B48, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D56926, SettingsBackground, ARRAYSIZE(SettingsBackground));
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D566BB, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D1E13C, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D1DB13, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92CE54F8, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92CC7138, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92CBE9D8, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92C6C662, SettingsBackground, ARRAYSIZE(SettingsBackground));
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92C58F84, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B95A6D, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B9524B, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B55FA9, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B53D33, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDAB, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92F58323, SettingsBackground, ARRAYSIZE(SettingsBackground));
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDA7, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92106CE7, SettingsBackground, ARRAYSIZE(SettingsBackground));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92B4FDA7, SettingsBackground, ARRAYSIZE(SettingsBackground));

					///////////////////////////////////////////////////
					//* PNG's*/
					///////////////////////////////////////////////////
					xbox::utilities::ComparePointerWrite((PVOID*)0x92DB07DF, Color, ARRAYSIZE(Color));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D2B2EC, Color, ARRAYSIZE(Color));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D2B12B, Color, ARRAYSIZE(Color));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D2AF9D, Color, ARRAYSIZE(Color));
					xbox::utilities::ComparePointerWrite((PVOID*)0x92D2ACDC, Color, ARRAYSIZE(Color));
					///////////////////////////////////////////////////
				}
				///////////////////////////////////////////////////
			}

			VOID SetHUDUI() 
			{

			}

			BOOL Dev() 
			{
				return TRUE;
			}

			BOOL RGH() 
			{
				///////////////////////////////////////////////////
				unsigned char bytes0[24] = {
					0x3E, 0xA0, 0x80, 0x04, 0xE9, 0xC7, 0x00, 0x00, 0x54, 0xA5, 0xE1, 0x3F,
					0xEA, 0x07, 0x00, 0x08, 0x3A, 0xB5, 0x61, 0x48, 0x83, 0xA3, 0x01, 0xD0
				};
				xbox::utilities::setMemory((PVOID)0x8011035C, bytes0, 24);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x81682544, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x816798EC, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x816FEE9C, 0x60000000);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x82497EB0, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x8259A65C, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x825C6070, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x822E4DF0, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x8260B178, 0x60000000);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x8167F978, 0x38600000);
				xbox::utilities::ComparePointerWrite(0x8167C4B4, 0x38600000);
				xbox::utilities::ComparePointerWrite(0x8192BDA8, 0x38600000);
				xbox::utilities::ComparePointerWrite(0x81A3CD48, 0x39200000);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x816DAC84, 0x38600006);
				xbox::utilities::ComparePointerWrite(0x81A3CD60, 0x38600001);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x816DD688, 0x39600001);
				xbox::utilities::ComparePointerWrite(0x816DD6FC, 0x39600001);
				xbox::utilities::ComparePointerWrite(0x816DD6F4, 0x39600001);
				xbox::utilities::ComparePointerWrite(0x816DD6E8, 0x39600001);
				xbox::utilities::ComparePointerWrite(0x818F2130, 0x480000C8);
				xbox::utilities::ComparePointerWrite(0x8167C47C, 0x38600001);
				xbox::utilities::ComparePointerWrite(0x8192BD24, 0x38600001);
				xbox::utilities::ComparePointerWrite(0x8192BD18, 0x60000000);
				xbox::utilities::ComparePointerWrite(0x816DCCC8, 0x480000CC);
				xbox::utilities::ComparePointerWrite(0x8169FF78, 0x38600001);
				xbox::utilities::ComparePointerWrite(0x816710AC, 0x38600001);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x8007CD04, 0x38600001);
				xbox::utilities::ComparePointerWrite(0x8007AB40, 0x38600001);
				///////////////////////////////////////////////////
				xbox::utilities::ComparePointerWrite(0x84DC5FE4, 0x00000000);
				xbox::utilities::ComparePointerWrite(0x8414A1D4, 0x00000000);
				///////////////////////////////////////////////////
				return TRUE;
			}
		}
	}
}