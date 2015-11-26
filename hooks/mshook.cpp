/*
	DLL used for global keyboard hooking
*/

#define STRICT
#include <windows.h>
#include "mshook.h"

#pragma comment(linker, "-section:Shared,rws")
#pragma data_seg("Shared")

static HHOOK        g_hHook = NULL;
static HWND         g_hwndNotify = NULL;

#pragma data_seg()

LRESULT CALLBACK MSHookProc(int nCode, WPARAM wParam, LPARAM lParam )
{
    BOOL bHandled = FALSE;

    //NOTE: deceiving that lParam is 32-bit but KF_UP is 16-bit

    //NOTE: must use GetAsyncKeyState instead of GetKeyState to fix weirdness
    // with the keystate sticking for long periods of time (espcially if the 
    // hotkey is triggered while focus is in a cmd window)

    if (nCode == HC_ACTION)
    {
		bHandled = TRUE;
    }
    
    if (bHandled == TRUE)
        return TRUE;
    else
        return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

BOOL InstallMsHook(HWND hwndNotify)
{
    HINSTANCE hMod = GetModuleHandle("MSHOOK");

    if (hwndNotify == NULL)
        return FALSE;

    g_hHook = SetWindowsHookEx(WH_MOUSE, (HOOKPROC)GetProcAddress(hMod, "MSHookProc"), hMod, 0);
    if (g_hHook == NULL)
	{
		MessageBox(hwndNotify, "Unable to SetWindowsHookEx( ).", "Error", 16);
        return FALSE;
	}

    g_hwndNotify = hwndNotify;

    return TRUE;
}

BOOL UnInstallMsHook(VOID)
{
    BOOL bUninstalled = FALSE;

    if (g_hHook)
    {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
        g_hwndNotify = NULL;
        bUninstalled = TRUE;
    }

    return bUninstalled;
}


BOOL WINAPI DllMain (HANDLE hModule, ULONG ulReason, LPVOID lpReserved)
{
    switch (ulReason)
    {
        case DLL_PROCESS_ATTACH: 
        case DLL_PROCESS_DETACH: 
            break;
    }

	return TRUE;
}
