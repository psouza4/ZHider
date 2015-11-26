/*
	DLL used for global keyboard hooking
*/

#define STRICT
#include <windows.h>
#include "kbhook.h"

#pragma comment(linker, "-section:Shared,rws")
#pragma data_seg("Shared")

static HHOOK        g_hHook = NULL;
static HWND         g_hwndNotify = NULL;
static UINT         g_bHookType = 0;

#pragma data_seg()

LRESULT CALLBACK KBHookProc(int nCode, WPARAM wParam, LPARAM lParam )
{
	UINT uNotifyType = WM_NOTIFY_REGKEY;

    //NOTE: deceiving that lParam is 32-bit but KF_UP is 16-bit

    //NOTE: must use GetAsyncKeyState instead of GetKeyState to fix weirdness
    // with the keystate sticking for long periods of time (espcially if the 
    // hotkey is triggered while focus is in a cmd window)

    if ((nCode == HC_ACTION) && !(HIWORD(lParam) & (KF_UP)))
    {
		if ((GetAsyncKeyState(VK_MENU) & 0x8000 > 0) && (GetAsyncKeyState(VK_CONTROL) & 0x8000 > 0))
			uNotifyType = WM_NOTIFY_CTRLALT;
		else if ((GetAsyncKeyState(VK_LWIN) & 0x8000 > 0) || (GetAsyncKeyState(VK_RWIN) & 0x8000 > 0))
			uNotifyType = WM_NOTIFY_WINDOWSKEY;
		else if (GetAsyncKeyState(VK_CONTROL) & 0x8000 > 0)
			uNotifyType = WM_NOTIFY_CTRLKEY;
		else if (GetAsyncKeyState(VK_MENU) & 0x8000 > 0)
			uNotifyType = WM_NOTIFY_ALTKEY;
		else if (GetAsyncKeyState(VK_SHIFT) & 0x8000 > 0)
			uNotifyType = WM_NOTIFY_SHIFTKEY;

		if ((uNotifyType == WM_NOTIFY_CTRLALT) || (uNotifyType == WM_NOTIFY_WINDOWSKEY))
			PostMessage(g_hwndNotify, uNotifyType, wParam, lParam);

		if ((g_bHookType == 1) || (wParam == 'Z' && uNotifyType == WM_NOTIFY_CTRLALT) || (wParam == 'X' && uNotifyType == WM_NOTIFY_CTRLALT) || (wParam == 'C' && uNotifyType == WM_NOTIFY_CTRLALT) || (wParam == 'M' && uNotifyType == WM_NOTIFY_CTRLALT) || (wParam == 'L' && uNotifyType == WM_NOTIFY_CTRLALT))
			return TRUE;
		if ((g_bHookType == 1) || (wParam == 'Z' && uNotifyType == WM_NOTIFY_WINDOWSKEY) || (wParam == 'X' && uNotifyType == WM_NOTIFY_WINDOWSKEY) || (wParam == 'C' && uNotifyType == WM_NOTIFY_WINDOWSKEY) || (wParam == 'L' && uNotifyType == WM_NOTIFY_WINDOWSKEY))
			return TRUE;
    }
    
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

BOOL InstallKbHook(HWND hwndNotify)
{
    HINSTANCE hMod = GetModuleHandle("KBHOOK");

    if (hwndNotify == NULL)
        return FALSE;

    g_hHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)GetProcAddress(hMod, "KBHookProc"), hMod, 0);
    if (g_hHook == NULL)
	{
		MessageBox(hwndNotify, "Unable to SetWindowsHookEx( ).", "Error", 16);
        return FALSE;
	}

    g_hwndNotify = hwndNotify;
	g_bHookType = 1;

    return TRUE;
}

BOOL InstallKbHookInvisMode(HWND hwndNotify)
{
    HINSTANCE hMod = GetModuleHandle("KBHOOK");

    if (hwndNotify == NULL)
        return FALSE;

    g_hHook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)GetProcAddress(hMod, "KBHookProc"), hMod, 0);
    if (g_hHook == NULL)
	{
		MessageBox(hwndNotify, "Unable to SetWindowsHookEx( ).", "Error", 16);
        return FALSE;
	}

    g_hwndNotify = hwndNotify;
	g_bHookType = 2;

    return TRUE;
}

BOOL UnInstallKbHook(VOID)
{
    BOOL bUninstalled = FALSE;

    if (g_hHook)
    {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = NULL;
        g_hwndNotify = NULL;
        bUninstalled = TRUE;
		g_bHookType = 0;
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
