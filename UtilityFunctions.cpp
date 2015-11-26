#include <windows.h>
#include <stdio.h>

VOID FlushMessages(HWND hWnd)
{
	MSG msgMessage;
	BOOL FlushLoop = TRUE;

	while (FlushLoop == TRUE)
	{
		switch (PeekMessage(&msgMessage, hWnd, 0, 0, TRUE))
		{
			case -1:
			case 0:
				FlushLoop = FALSE;
				break;

			default:
				TranslateMessage(&msgMessage);
				DispatchMessage(&msgMessage);
				break;
		}
	}
}

VOID CenterDialog(HWND hWnd)
{
	RECT r1, r2;
	GetClientRect(hWnd, &r1);
	SystemParametersInfo(SPI_GETWORKAREA, 0, (PVOID)&r2, 0);
	SetWindowPos(hWnd, NULL, ((r2.right - r2.left) / 2) - (r1.right /
		2), ((r2.bottom - r2.top) / 2) - (r1.bottom / 2), 0, 0,
		SWP_NOZORDER | SWP_NOSIZE);
}

HBRUSH BrushIt(HWND hWnd, UINT rVal, UINT gVal, UINT bVal)
{
	LOGBRUSH logBrush;
	HBRUSH hBrush;
	CHAR szErrorInfo[1000];

	memset((CHAR *)szErrorInfo, (CHAR)0, sizeof(CHAR) * 1000);

	if (rVal > 255 || gVal > 255 || bVal > 255)
	{
		sprintf(szErrorInfo, "Unable to create brush object.\n\nValues out of range.\n\nRed: %u\tGreen: %u\t Blue: %u", rVal, gVal, bVal);
		if (hWnd != NULL && IsWindow(hWnd))
			MessageBox(hWnd, szErrorInfo, "Error - BrushIt( )", 16);
		return NULL;
	}

	logBrush.lbColor = RGB(rVal, gVal, bVal);
	logBrush.lbHatch = 0;
	logBrush.lbStyle = BS_SOLID;

	hBrush = CreateBrushIndirect(&logBrush);
	if (hBrush == NULL)
	{
		sprintf(szErrorInfo, "Unable to create brush object.\n\nError %u", GetLastError( ));
		if (hWnd != NULL && IsWindow(hWnd))
			MessageBox(hWnd, szErrorInfo, "Error - BrushIt( )", 16);
		return NULL;
	}
	else
	{
		return hBrush;
	}
}
