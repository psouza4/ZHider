#include <windows.h>

extern HWND chWnd;
extern BOOL bWantInjection;

BOOL bInstalled = FALSE;

void Install(HWND hWnd, DWORD pid)
{
	HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
									FALSE, pid);	

	if (hProcess != NULL)
	{
		HANDLE hThread;
		char   szLibPath [_MAX_PATH];
		void*  pLibRemote = 0;
		DWORD  hLibModule = 0;

		HMODULE hKernel32 = ::GetModuleHandle("Kernel32");

		strcpy(szLibPath, "taskhook.dll");

		pLibRemote = ::VirtualAllocEx( hProcess, NULL, sizeof(szLibPath), MEM_COMMIT, PAGE_READWRITE );
		
		if (pLibRemote == NULL)
			return;

		::WriteProcessMemory(hProcess, pLibRemote, (void*)szLibPath,sizeof(szLibPath),NULL);

		hThread = ::CreateRemoteThread( hProcess, NULL, 0,	
						(LPTHREAD_START_ROUTINE) ::GetProcAddress(hKernel32,"LoadLibraryA"), 
						pLibRemote, 0, NULL );

		if( hThread != NULL )
		{
			::SendMessage(chWnd, 0x7053, (WPARAM)0, (LPARAM)0);
			bInstalled = TRUE;			
			::WaitForSingleObject( hThread, INFINITE );
			::GetExitCodeThread( hThread, &hLibModule );
			::CloseHandle( hThread );
		}
	}
}

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM p)
{
	char szTitle[24];

	::GetWindowText(hWnd, szTitle, sizeof(szTitle));

	if (strcmp(szTitle, "Windows Task Manager") == 0)
	{
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		Install(hWnd, pid);
	}

	return TRUE;
}

DWORD WINAPI GetTaskManagerThread(LPVOID p)
{
	while (bWantInjection == TRUE)
	{
//		if (bInstalled == FALSE)
//		{
			EnumWindows(EnumProc, (LPARAM)p);
			::Sleep(100);
//		}
//		else
//			::Sleep(100);
	}

	return 0;
}

void StartItUp(void)
{
	DWORD tid;
	CreateThread(NULL, 0, GetTaskManagerThread, NULL, 0, &tid);
}
