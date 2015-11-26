#include "Compiler Settings.H"

#include <windows.h>
#include "TransparencyClass.h"

#ifdef WANT_TRANSPARENCY

HMODULE			hUserDll = NULL;

trWindow::trWindow(HWND hInput)
{
	this->hWnd = hInput;
	if (hUserDll == NULL)
		hUserDll = ::LoadLibrary("USER32.dll");
}

trWindow::~trWindow(VOID)
{
	if (hUserDll)
	{
		::FreeLibrary(hUserDll);
		hUserDll = NULL;
	}
}

BOOL trWindow::trans(COLORREF crKey, BYTE bAlpha, DWORD dwFlags)
{
	if (hUserDll == NULL)
		hUserDll = ::LoadLibrary("USER32.dll");

	this->curTrans = (((int)bAlpha) * 100) / 255;

	SetWindowLong(this->hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

    BOOL    bRet = TRUE;
    typedef BOOL (WINAPI* lpfnSetTransparent)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
    
    if ( hUserDll )
    {
        lpfnSetTransparent pFnSetTransparent  = NULL;
        pFnSetTransparent  = (lpfnSetTransparent)GetProcAddress(hUserDll, "SetLayeredWindowAttributes");
        if (pFnSetTransparent )
            bRet = pFnSetTransparent(hWnd, crKey, bAlpha, dwFlags);

        else
            bRet = FALSE;
    }

	UpdateWindow(this->hWnd);

    return bRet;
}

BOOL trWindow::trans(int percentage)
{
	this->curTrans = percentage;
	return (this->trans(RGB(0,0,0), 255 * percentage/100, LWA_ALPHA));
}

void HRT::start(void)
{
	QueryPerformanceFrequency(&(this->res_timer));
	QueryPerformanceCounter(&(this->start_timer));
}

__int64 HRT::end(void)
{
	QueryPerformanceCounter(&(this->end_timer));

	return ((this->end_timer.QuadPart - this->start_timer.QuadPart) / (this->res_timer.QuadPart / 1000));
}

#endif
