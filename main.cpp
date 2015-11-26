#include "Compiler Settings.H"

#include <windows.h>
#include <stdio.h>
#include <commctrl.h>

#include "resource.h"
#include "hooks\kbhook.h"

// Symbols from 'DllInjector.cpp':
#ifdef WANT_DLL_INJECTION
extern BOOL bInstalled;
extern VOID StartItUp(VOID);
#endif

// Symbols from 'TaskManager.cpp':
extern HWND FindWindowToSendTo(VOID);
extern VOID SynchronizeTaskManager(VOID);

// Symbols from 'UtilityFunctions.cpp':
extern VOID FlushMessages(HWND);
extern HBRUSH BrushIt(HWND, UINT, UINT, UINT);

// Symbols from 'Window__List.cpp':
extern BOOL bHiddenOrNot;
extern VOID HideThisWindow(HWND);
extern VOID HideThisWindow(VOID);
extern VOID HideAllWindows(VOID);
extern VOID UnHideAllWindows(VOID);
extern VOID RemoveAllWindows(VOID);
extern BOOL CALLBACK ListCBProc(HWND, UINT, WPARAM, LPARAM);

// Function prototypes:
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

// Global data:
HINSTANCE	hInst;
LPTSTR		szClassName = "ZHider_MainWndClass";
LPTSTR		szAppName   = "ZHider";
HICON		hHiderIcon;
HWND		chWnd;
HCURSOR		hCursorMain;
RECT		rOrigRect;
DWORD		iteration_count;
BOOL		bWantInjection = TRUE;

HWND g_hWndFromInst;

BOOL CALLBACK InstanceFinderEnumProc(HWND hWnd, LPARAM p)
{
	DWORD pid;
	CHAR szTitle[1024];

	GetWindowThreadProcessId(hWnd, &pid);

	if ((DWORD)p == pid)
	{
		g_hWndFromInst = hWnd;
		for (;;)
		{
			if (GetParent(g_hWndFromInst) != NULL)
				g_hWndFromInst = GetParent(g_hWndFromInst);
			else
				break;
		}

		memset((CHAR *)szTitle, (CHAR)0, sizeof(CHAR) * 1024);
		SendMessage(g_hWndFromInst, WM_GETTEXT, (WPARAM)1023, (LPARAM)szTitle);
		if (szTitle[0] != '\0')
			HideThisWindow(hWnd);
	}

	return TRUE;
}

HWND HideWindowsByProcessId(DWORD dwProcessID)
{
	g_hWndFromInst = NULL;
	EnumWindows(InstanceFinderEnumProc, (LPARAM)dwProcessID);
	return g_hWndFromInst;
}

VOID ProcessCommandLine(CHAR *szCommandLine_real)
{
	CHAR *szT1, *szT2;
	CHAR szExec[4096], szParam[4096], szWorkingDirectory[4096];
	CHAR szCommandLine[4096];

	memset((CHAR *)szCommandLine, (CHAR)0, sizeof(CHAR) * 4096);
	strcpy(szCommandLine, szCommandLine_real);

	memset((CHAR *)szExec, (CHAR)0, sizeof(CHAR) * 4096);
	memset((CHAR *)szParam, (CHAR)0, sizeof(CHAR) * 4096);
	memset((CHAR *)szWorkingDirectory, (CHAR)0, sizeof(CHAR) * 4096);

	if (strstr(szCommandLine, "/noinjection"))
		bWantInjection = FALSE;

	szT1 = strstr(szCommandLine, "/exec:");
	if (szT1)
	{
		strncpy(szExec, szT1 + (6 * sizeof(CHAR)), (strlen(szT1) > 4095) ? 4095 : strlen(szT1));
		szT2 = strstr(szExec, "|");
		if (szT2)
		{
			szT2[0] = '\0';

			szT1 = strstr(szCommandLine, "/param:");
			if (szT1)
			{
				strncpy(szParam, szT1 + (7 * sizeof(CHAR)), (strlen(szT1) > 4095) ? 4095 : strlen(szT1));
				szT2 = strstr(szParam, "|");
				szT2[0] = '\0';
			}

			szT1 = strstr(szCommandLine, "/dir:");
			if (szT1)
			{
				strncpy(szWorkingDirectory, szT1 + (5 * sizeof(CHAR)), (strlen(szT1) > 4095) ? 4095 : strlen(szT1));
				szT2 = strstr(szWorkingDirectory, "|");
				szT2[0] = '\0';
			}

			SHELLEXECUTEINFO sei;

			memset((SHELLEXECUTEINFO *)&sei, 0, sizeof(SHELLEXECUTEINFO));
			sei.cbSize = sizeof(SHELLEXECUTEINFO);

			sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI | SEE_MASK_DOENVSUBST | SEE_MASK_CONNECTNETDRV | 0x00800000;
			sei.hwnd = (HWND)0;
			sei.lpVerb = "open";
			sei.lpFile = szExec;
			sei.lpParameters = (szParam[0] == 0) ? NULL : szParam;
			sei.lpDirectory = (szWorkingDirectory[0] == 0) ? NULL : szWorkingDirectory;
			sei.nShow = SW_HIDE;
			sei.hProcess = NULL;
			ShellExecuteEx(&sei);

			::Sleep(100);
			HideWindowsByProcessId((DWORD)GetProcessId(sei.hProcess));
			::Sleep(1000);
			FlushMessages(chWnd);
			HideWindowsByProcessId((DWORD)GetProcessId(sei.hProcess));
			::Sleep(1000);
			FlushMessages(chWnd);
			HideWindowsByProcessId((DWORD)GetProcessId(sei.hProcess));
			::Sleep(1000);
			FlushMessages(chWnd);
			HideWindowsByProcessId((DWORD)GetProcessId(sei.hProcess));
			::Sleep(1000);
			FlushMessages(chWnd);
			HideWindowsByProcessId((DWORD)GetProcessId(sei.hProcess));
			::Sleep(1000);
			FlushMessages(chWnd);

			/*
			if (hWndNew)
			{
				sprintf(szDebug, "Found window by process handle\r\n\r\nsei.hProcess=%08X\r\nID=%ld\r\nhWndNew=%ld (%08X)", sei.hProcess, GetProcessId(sei.hProcess), hWndNew, hWndNew);
				MessageBox(NULL, szDebug, "Debug", 48);
			}
			else
			{
				sprintf(szDebug, "Could not find window by process handle\r\n\r\nsei.hProcess=%08X", sei.hProcess);
				MessageBox(NULL, szDebug, "Debug", 48);
			}
			*/
		}
	}
}

HWND FindZHiderWindow(VOID)
{
	HWND hWndTemp;
	CHAR szTemp[500];

	hWndTemp = GetTopWindow(NULL);
	if (hWndTemp == NULL)
		return NULL;

	memset((CHAR *)szTemp, (CHAR)0, sizeof(CHAR) * 500);
	SendMessage((HWND)hWndTemp, (UINT)WM_GETTEXT, (WPARAM)499, (LPARAM)szTemp);

	if (!strcmp(szTemp, szAppName))
		return (hWndTemp);

	for (;;)
	{
		hWndTemp = GetNextWindow(hWndTemp, GW_HWNDNEXT);
		if (hWndTemp == NULL)
			return NULL;

		memset((CHAR *)szTemp, (CHAR)0, sizeof(CHAR) * 500);
		SendMessage((HWND)hWndTemp, (UINT)WM_GETTEXT, (WPARAM)499, (LPARAM)szTemp);

		if (!strcmp(szTemp, szAppName))
			return (hWndTemp);
	}

	return NULL;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	WNDCLASS wc;
	HWND hWnd = NULL;
	BOOL bProgLoop = TRUE; 
	MSG msgMessage;

	InitCommonControls();

    HANDLE hMutex = CreateMutex(NULL, FALSE, "ZHIDER_MUTEX");
    if ((hMutex == NULL) || (GetLastError() == ERROR_ALREADY_EXISTS))
    {
		HWND hWndOrig = FindZHiderWindow( );

		if (hWndOrig)
		{
			SendMessage(GetDlgItem(hWndOrig, 100), WM_SETTEXT, (WPARAM)0, (LPARAM)lpCmdLine);
			SendMessage(hWndOrig,                  0x7056,     (WPARAM)0, (LPARAM)0);
		}

        if (hMutex)
            CloseHandle(hMutex);

        return 0;
    }

	hInst = hInstance;
	hHiderIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROGRAM_ICON));
	hCursorMain = LoadCursor(NULL, IDC_ARROW);

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = BrushIt(NULL, 191, 191, 191);
	wc.hCursor = hCursorMain;
	wc.hIcon = hHiderIcon;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = MainWndProc;
	wc.lpszClassName = szClassName;
	wc.lpszMenuName = NULL;
	wc.style = 0;

	if (RegisterClass(&wc) == 0)
		return 0x10;

	hWnd = CreateWindow(szClassName, szAppName, WS_POPUP | WS_CAPTION, 10000, 10000, 280, 112, NULL, NULL, hInstance, NULL);

	if (!IsWindow(hWnd))
		return 0;

	ProcessCommandLine(lpCmdLine);

	while (bProgLoop == TRUE)
	{
		switch (GetMessage(&msgMessage, hWnd, 0, 0))
		{
			case -1:
			case 0:
				bProgLoop = FALSE;
				break;

			default:
				TranslateMessage(&msgMessage);
				DispatchMessage(&msgMessage);
				break;
		}
	}

	ReleaseMutex(hMutex);
	CloseHandle(hMutex);
	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef WANT_DLL_INJECTION
	HWND hOut;
#endif

	chWnd = hWnd;

	switch (uMsg)
	{
#ifdef WANT_DLL_INJECTION
		case 0x7053:
			if (bWantInjection == TRUE)
			{
				SynchronizeTaskManager( );
				iteration_count = 0;
				KillTimer(hWnd, 43550);
				SetTimer(hWnd, 43550, 50, NULL);
			}
			return TRUE;

		case 0x7055:
			if (bWantInjection == TRUE)
			{
				if (lParam == 0)
				{
					bInstalled = FALSE;
				}
				else if (lParam == 1)
				{
					SynchronizeTaskManager( );
					KillTimer(hWnd, 43550);
				}
			}
			return TRUE;

		case WM_TIMER:
			if (bWantInjection == TRUE)
			{
				hOut = FindWindowToSendTo( );
				SendMessage(hOut,            0x7054, (WPARAM)0, (LPARAM)hWnd);
				SendMessage(GetParent(hOut), 0x7054, (WPARAM)0, (LPARAM)hWnd);
				SynchronizeTaskManager( );
				if (iteration_count++ > 3000) // 1.5 seconds (at 50ms intervals)
					KillTimer(hWnd, 43550);
			}
			else
				KillTimer(hWnd, 43550);
			break;
#endif
		case 0x7056:
			{{
				CHAR szTemp[4096];
				memset((CHAR *)szTemp, (CHAR)0, sizeof(CHAR) * 4096);
				SendMessage(GetDlgItem(hWnd, 100), WM_GETTEXT, (WPARAM)4095, (LPARAM)szTemp);
				if (strcmp(szTemp, ""))
					ProcessCommandLine(szTemp);
			}}
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			UnHideAllWindows( );
			RemoveAllWindows( );
			UnInstallKbHook( );
			PostQuitMessage(0);
			break;

		case WM_NOTIFY_CTRLALT:
		case WM_NOTIFY_WINDOWSKEY:
			if (wParam == 'Z')
			{
				HideThisWindow( );
			}
			else if (wParam == 'X')
			{
				if (bHiddenOrNot == TRUE)
					UnHideAllWindows( );
				else
					HideAllWindows( );
			}
			else if (wParam == 'C')
			{
				UnHideAllWindows( );
				RemoveAllWindows( );
			}
			else if (wParam == 'L')
			{
				UnInstallKbHook( );
				ShowWindow(hWnd, SW_SHOWNA);
				DialogBox(hInst, MAKEINTRESOURCE(IDD_LIST), hWnd, ListCBProc);
				if (InstallKbHookInvisMode(hWnd) == FALSE)
				{
					MessageBox(hWnd, "Unable to install keyboard hook.\n\nExiting...", "Critical Error", 16);
					PostQuitMessage(0);
					break;
				}
				ShowWindow(hWnd, SW_HIDE);
			}
			else if (wParam == 'M')
			{
				UnHideAllWindows( );
				RemoveAllWindows( );
				UnInstallKbHook( );
				PostQuitMessage(0);
			}
			break;

		case WM_CREATE:
			if (InstallKbHookInvisMode(hWnd) == FALSE)
			{
				MessageBox(hWnd, "Unable to install keyboard hook.\n\nExiting...", "Critical Error", 16);
				PostQuitMessage(0);
				break;
			}
			ShowWindow(hWnd, SW_HIDE);
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			CreateWindow("STATIC", "", WS_CHILD, 0, 0, 1, 1, hWnd, (HMENU)100, hInst, NULL);
#ifdef WANT_DLL_INJECTION
			if (bWantInjection == TRUE)
				StartItUp( );
#endif
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

