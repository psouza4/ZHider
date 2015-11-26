#ifndef __MSHOOK_H__
#define __MSHOOK_H__

#define MSMACRO_VERSION     "1.0"

extern "C" BOOL InstallMsHook(HWND hwndNotify);
extern "C" BOOL UnInstallMsHook(VOID);

#endif
