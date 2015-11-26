#ifndef __KBHOOK_H__
#define __KBHOOK_H__

#define KBMACRO_VERSION       "1.0"
#define WM_NOTIFY_WINKEY      (WM_APP+2)
#define WM_NOTIFY_REGKEY      (WM_APP+3)
#define WM_NOTIFY_CTRLKEY     (WM_APP+4)
#define WM_NOTIFY_ALTKEY      (WM_APP+5)
#define WM_NOTIFY_SHIFTKEY    (WM_APP+6)
#define WM_NOTIFY_WINDOWSKEY  (WM_APP+7)
#define WM_NOTIFY_CTRLALT     (WM_APP+8)

extern "C" BOOL InstallKbHook(HWND hwndNotify);
extern "C" BOOL InstallKbHookInvisMode(HWND hwndNotify);
extern "C" BOOL UnInstallKbHook(VOID);

#endif